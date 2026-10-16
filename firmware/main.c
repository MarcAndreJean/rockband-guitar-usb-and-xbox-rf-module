#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"

enum
{
  BLINK_NOT_MOUNTED_MS = 250,
  BLINK_MOUNTED_MS     = 1000,
  BLINK_SUSPENDED_MS   = 2500,
};

#define CONTROLLER_POLL_INTERVAL_MS  10

// Vendor requests sent by the Xbox 360 and the Windows xusb22 driver
#define MS_OS_COMPAT_ID_INDEX      0x0004
#define SECURITY_INIT_WVALUE       0x5C10
#define SECURITY_RESET_WVALUE      0x5B17
#define GET_INPUT_REPORT_REQUEST   0x01
#define UNKNOWN_A1_REQUEST         0xA1

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED_MS;
static uint8_t security_init_request_count = 0;
static bool is_security_handshake_done = false;

static void controller_task(void);
static void blink_led_task(void);

int main(void)
{
  board_init();
  tud_init(BOARD_TUD_RHPORT);

  while (true)
  {
    tud_task();
    controller_task();
    blink_led_task();
  }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED_MS;
}

void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED_MS;
}

void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED_MS;
}

void tud_resume_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED_MS;
}

//--------------------------------------------------------------------+
// Controller
//--------------------------------------------------------------------+

static void controller_task(void)
{
  static uint32_t last_poll_ms = 0;

  if (board_millis() - last_poll_ms < CONTROLLER_POLL_INTERVAL_MS) return;
  last_poll_ms += CONTROLLER_POLL_INTERVAL_MS;

  bool const is_button_pressed = board_button_read();

  if (tud_suspended() && is_button_pressed)
  {
    tud_remote_wakeup();
  }

  // TODO: read the guitar inputs and send them as input reports
}

//--------------------------------------------------------------------+
// Vendor control requests
//--------------------------------------------------------------------+

static bool reply(uint8_t rhport, tusb_control_request_t const * request, void const * data, uint16_t size)
{
  // TinyUSB truncates the reply to request->wLength
  return tud_control_xfer(rhport, request, (void *) (uintptr_t) data, size);
}

static bool accept_host_data(uint8_t rhport, tusb_control_request_t const * request)
{
  static uint8_t ignored_data[64];

  if (request->wLength > sizeof(ignored_data)) return false;
  return tud_control_xfer(rhport, request, ignored_data, request->wLength);
}

static bool handle_ms_os_descriptor_request(uint8_t rhport, tusb_control_request_t const * request)
{
  if (request->wIndex != MS_OS_COMPAT_ID_INDEX) return false;
  return reply(rhport, request, desc_ms_os_compat_id, MS_OS_COMPAT_ID_LEN);
}

// The console expects answers signed by a Microsoft security chip. The Pico has
// none, so it answers with zeros and the console rejects it (see README).
static bool handle_security_request(uint8_t rhport, tusb_control_request_t const * request)
{
  static uint8_t const unsigned_response[20] = { 0 };

  switch (request->wValue)
  {
    case SECURITY_INIT_WVALUE:
      security_init_request_count++;
      is_security_handshake_done = (security_init_request_count >= 2);
      return reply(rhport, request, unsigned_response, sizeof(unsigned_response));

    case SECURITY_RESET_WVALUE:
      return reply(rhport, request, unsigned_response, sizeof(unsigned_response));

    default:
      return false;
  }
}

static bool handle_controller_request(uint8_t rhport, tusb_control_request_t const * request)
{
  static uint8_t const input_report[] =
  {
    0x00, 0x14, 0xff, 0xf7, 0xff, 0xff, 0xc0, 0xff, 0xc0, 0xff,
    0xc0, 0xff, 0xc0, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };
  static uint8_t const device_id[]          = { 0x01, 0x00, 0x83, 0x66 };
  static uint8_t const unknown_a1_response[] = { 0x01, 0x02 };

  bool const is_for_interface = (request->bmRequestType_bit.recipient == TUSB_REQ_RCPT_INTERFACE);

  switch (request->bRequest)
  {
    case GET_INPUT_REPORT_REQUEST:
      return is_for_interface
        ? reply(rhport, request, input_report, sizeof(input_report))
        : reply(rhport, request, device_id, sizeof(device_id));

    case UNKNOWN_A1_REQUEST:
      return reply(rhport, request, unknown_a1_response, sizeof(unknown_a1_response));

    default:
      return false;
  }
}

bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request)
{
  if (stage != CONTROL_STAGE_SETUP) return true;
  if (request->bmRequestType_bit.type != TUSB_REQ_TYPE_VENDOR) return false;

  if (request->bRequest == MS_OS_VENDOR_CODE) return handle_ms_os_descriptor_request(rhport, request);
  if (request->bmRequestType_bit.direction == TUSB_DIR_OUT) return accept_host_data(rhport, request);
  if (!is_security_handshake_done) return handle_security_request(rhport, request);
  return handle_controller_request(rhport, request);
}

//--------------------------------------------------------------------+
// Status LED
//--------------------------------------------------------------------+

static void blink_led_task(void)
{
  static uint32_t last_toggle_ms = 0;
  static bool is_led_on = false;

  if (board_millis() - last_toggle_ms < blink_interval_ms) return;
  last_toggle_ms += blink_interval_ms;

  board_led_write(is_led_on);
  is_led_on = !is_led_on;
}
