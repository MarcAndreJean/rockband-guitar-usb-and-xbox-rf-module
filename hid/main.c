#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"

// External reference to MS OS 1.0 descriptors
extern uint8_t const desc_ms_os_1_0_compat_id[];
#define MS_OS_1_0_COMPAT_ID_LEN  (16 + 24 * 1)  // Header + 1 function section

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

// Interface index depends on the order in configuration descriptor
enum {
  ITF_VENDOR_0 = 0,
  ITF_VENDOR_1 = 1,
  ITF_VENDOR_2 = 2,
  ITF_VENDOR_3 = 3
};

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void vendor_task(void);

/*------------- MAIN -------------*/
int main(void)
{
  board_init();

  // init device stack on configured roothub port
  tud_init(BOARD_TUD_RHPORT);

  while (1)
  {
    tud_task(); // tinyusb device task
    led_blinking_task();

    vendor_task();
  }

  return 0;
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

//--------------------------------------------------------------------+
// USB Vendor (Xbox 360 Controller)
//--------------------------------------------------------------------+

void vendor_task(void)
{
  // Poll every 10ms
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;

  uint32_t const btn = board_button_read();

  // Remote wakeup
  if ( tud_suspended() && btn )
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  }

  // TODO: Implement Xbox 360 controller input reports
  // For now, we just handle enumeration
}

//--------------------------------------------------------------------+
// Use vendor class
//--------------------------------------------------------------------+

#define URL  "example.tinyusb.org/webusb-serial/"
const tusb_desc_webusb_url_t desc_url =
{
  .bLength         = 3 + sizeof(URL) - 1,
  .bDescriptorType = 3, // WEBUSB URL type
  .bScheme         = 1, // 0: http, 1: https
  .url             = URL
};

// Invoked when a control transfer occurred on an interface of this class
// Driver response accordingly to the request and the transfer stage (setup/data/ack)
// return false to stall control endpoint (e.g unsupported request)
bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request)
{
  // Nothing to do with DATA & ACK stages, let TinyUSB handle them
  if (stage != CONTROL_STAGE_SETUP) return true;

  // Microsoft OS 1.0 Descriptor requests
  // bmRequestType = 0xC0 (Device, Vendor, IN) or 0xC1 (Interface, Vendor, IN)
  // bRequest = 0x90 (MS_OS_1_0_VENDOR_CODE)
  // wIndex = 0x0004 (Extended Compat ID) or 0x0005 (Extended Properties)
  if (request->bRequest == 0x90 && request->wIndex == 0x0004)
  {
    // Extended Compat ID OS Feature Descriptor
    // This returns "XUSB10" which tells Windows to load xusb22.sys driver
    uint16_t len = request->wLength < MS_OS_1_0_COMPAT_ID_LEN ? request->wLength : MS_OS_1_0_COMPAT_ID_LEN;
    return tud_control_xfer(rhport, request, (void*)(uintptr_t)desc_ms_os_1_0_compat_id, len);
  }
  else if (request->bRequest == 0x90 && request->wIndex == 0x0005)
  {
    // Extended Properties - not supported
    return false;
  }

  // Handle other vendor requests
  switch (request->bmRequestType_bit.type)
  {
    case TUSB_REQ_TYPE_VENDOR:
      // Legacy vendor requests (kept for compatibility)
      if (request->bRequest == 0)
      {
        // match vendor request in BOS descriptor
        // Get landing page url
        return tud_control_xfer(rhport, request, (void*)(uintptr_t) &desc_url, desc_url.bLength);
      }
    break;

    case TUSB_REQ_TYPE_CLASS:
      if (request->bRequest == 0x22)
      {
        // response with status OK
        return tud_control_status(rhport, request);
      }
    break;

    default: break;
  }

  // stall unknown request
  return false;
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}
