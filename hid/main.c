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
// Xbox 360 Security Handshake State
//--------------------------------------------------------------------+

// Security handshake state machine
static uint8_t security_initialized = 0;
static uint8_t security_response_count = 0;

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
  // For now, we just handle enumeration and security handshake
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

  // Xbox 360 Security Handshake - Based on GIMX firmware
  // The Xbox 360 uses a vendor-specific security handshake during enumeration
  if (request->bmRequestType_bit.type == TUSB_REQ_TYPE_VENDOR)
  {
    // During initialization, the host sends specific vendor requests
    // wValue 0x5c10 and 0x5b17 are part of the security challenge-response
    if (!security_initialized)
    {
      // Track security handshake progress
      // NOTE: Full Xbox 360 security requires cryptographic responses from a security chip
      // This implementation provides basic responses for enumeration
      // Real authentication would require genuine security data

      if (request->bmRequestType_bit.direction == TUSB_DIR_IN)
      {
        // Device to Host requests during security handshake
        if (request->wValue == 0x5c10)
        {
          // Security initialization request
          // The response data would normally come from a security chip
          // For now, we return empty data to acknowledge the request
          static uint8_t security_response[20] = {0};
          uint16_t len = request->wLength < sizeof(security_response) ? request->wLength : sizeof(security_response);

          if (security_response_count > 0) {
            security_initialized = 1;  // Mark as initialized after second response
          }
          security_response_count++;

          return tud_control_xfer(rhport, request, security_response, len);
        }
        else if (request->wValue == 0x5b17)
        {
          // Another security request - can trigger reset in real implementation
          static uint8_t response[20] = {0};
          uint16_t len = request->wLength < sizeof(response) ? request->wLength : sizeof(response);
          return tud_control_xfer(rhport, request, response, len);
        }
      }
      else
      {
        // Host to Device requests - just acknowledge
        static uint8_t buffer[64];
        return tud_control_xfer(rhport, request, buffer, request->wLength);
      }
    }
    else
    {
      // After security handshake, handle standard vendor requests
      if (request->bmRequestType_bit.direction == TUSB_DIR_IN)
      {
        // Device to Host
        if (request->bRequest == 0x01)
        {
          // GET_REPORT equivalent for vendor class
          if (request->bmRequestType_bit.recipient == TUSB_REQ_RCPT_INTERFACE)
          {
            // Interface-specific request - return controller state (20 bytes)
            static uint8_t const report[] = {
              0x00, 0x14, 0xff, 0xf7, 0xff, 0xff, 0xc0, 0xff,
              0xc0, 0xff, 0xc0, 0xff, 0xc0, 0xff, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00
            };
            return tud_control_xfer(rhport, request, (void*)(uintptr_t)report, sizeof(report));
          }
          else
          {
            // Device-level request (4 bytes)
            static uint8_t const response[] = { 0x01, 0x00, 0x83, 0x66 };
            return tud_control_xfer(rhport, request, (void*)(uintptr_t)response, sizeof(response));
          }
        }
        else if (request->bRequest == 0xa1)
        {
          // Another vendor request
          static uint8_t const response[] = { 0x01, 0x02 };
          return tud_control_xfer(rhport, request, (void*)(uintptr_t)response, sizeof(response));
        }
        else if (request->wValue == 0x5b17)
        {
          // Reset request - could trigger hard reset
          // For now, just stall
          return false;
        }
      }
      else
      {
        // Host to Device - acknowledge
        static uint8_t buffer[64];
        return tud_control_xfer(rhport, request, buffer, request->wLength);
      }
    }

    // Legacy vendor requests (kept for compatibility)
    if (request->bRequest == 0)
    {
      // match vendor request in BOS descriptor
      // Get landing page url
      return tud_control_xfer(rhport, request, (void*)(uintptr_t) &desc_url, desc_url.bLength);
    }
  }

  // Handle class requests
  if (request->bmRequestType_bit.type == TUSB_REQ_TYPE_CLASS)
  {
    if (request->bRequest == 0x22)
    {
      // response with status OK
      return tud_control_status(rhport, request);
    }
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
