#include "tusb.h"
#include "usb_descriptors.h"

// Every descriptor below is a byte-for-byte copy of a real Harmonix Rock Band
// guitar for Xbox 360. See docs/research/real-guitar/descriptor-dump.txt

#define HARMONIX_VID         0x1BAD
#define ROCKBAND_GUITAR_PID  0x0002
#define DEVICE_RELEASE       0x0103

enum
{
  STRID_LANGID = 0,
  STRID_MANUFACTURER,
  STRID_PRODUCT,
  STRID_SERIAL,
  STRID_SECURITY,
  STRID_MS_OS = 0xEE
};

//--------------------------------------------------------------------+
// Device Descriptor
//--------------------------------------------------------------------+

tusb_desc_device_t const desc_device =
{
  .bLength            = sizeof(tusb_desc_device_t),
  .bDescriptorType    = TUSB_DESC_DEVICE,
  .bcdUSB             = 0x0200,
  .bDeviceClass       = TUSB_CLASS_VENDOR_SPECIFIC,
  .bDeviceSubClass    = 0xFF,
  .bDeviceProtocol    = 0xFF,
  .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
  .idVendor           = HARMONIX_VID,
  .idProduct          = ROCKBAND_GUITAR_PID,
  .bcdDevice          = DEVICE_RELEASE,
  .iManufacturer      = STRID_MANUFACTURER,
  .iProduct           = STRID_PRODUCT,
  .iSerialNumber      = STRID_SERIAL,
  .bNumConfigurations = 1
};

uint8_t const * tud_descriptor_device_cb(void)
{
  return (uint8_t const *) &desc_device;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

enum
{
  ITF_NUM_GAMEPAD = 0,
  ITF_NUM_AUDIO,
  ITF_NUM_EXPANSION_PORT,
  ITF_NUM_SECURITY,
  ITF_NUM_TOTAL
};

#define XBOX_SUBCLASS           0x5D
#define XBOX_SECURITY_SUBCLASS  0xFD
#define ENDPOINT_MAX_PACKET     32

#define VENDOR_INTERFACE(_itfnum, _endpoint_count, _subclass, _protocol, _stridx) \
  9, TUSB_DESC_INTERFACE, _itfnum, 0, _endpoint_count, TUSB_CLASS_VENDOR_SPECIFIC, _subclass, _protocol, _stridx

#define INTERRUPT_ENDPOINT(_address, _interval) \
  7, TUSB_DESC_ENDPOINT, _address, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(ENDPOINT_MAX_PACKET), _interval

// Each interface is followed by an undocumented Xbox 360 descriptor (type 0x21 or 0x41)

#define GAMEPAD_INTERFACE_LEN  (9 + 17 + 7 + 7)
#define GAMEPAD_INTERFACE \
  VENDOR_INTERFACE(ITF_NUM_GAMEPAD, 2, XBOX_SUBCLASS, 0x01, 0), \
  17, 0x21, 0x10, 0x01, 0x06, 0x25, 0x81, 0x14, 0x03, 0x03, 0x03, 0x04, 0x13, 0x02, 0x08, 0x03, 0x03, \
  INTERRUPT_ENDPOINT(0x81, 4), \
  INTERRUPT_ENDPOINT(0x02, 8)

#define AUDIO_INTERFACE_LEN  (9 + 27 + 7 + 7 + 7 + 7)
#define AUDIO_INTERFACE \
  VENDOR_INTERFACE(ITF_NUM_AUDIO, 4, XBOX_SUBCLASS, 0x03, 0), \
  27, 0x21, 0x00, 0x01, 0x01, 0x01, 0x83, 0x40, 0x01, 0x04, 0x20, 0x16, 0x85, 0x00, 0x00, 0x00, \
  0x00, 0x00, 0x00, 0x16, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
  INTERRUPT_ENDPOINT(0x83, 2), \
  INTERRUPT_ENDPOINT(0x04, 4), \
  INTERRUPT_ENDPOINT(0x85, 64), \
  INTERRUPT_ENDPOINT(0x05, 16)

#define EXPANSION_PORT_INTERFACE_LEN  (9 + 9 + 7)
#define EXPANSION_PORT_INTERFACE \
  VENDOR_INTERFACE(ITF_NUM_EXPANSION_PORT, 1, XBOX_SUBCLASS, 0x02, 0), \
  9, 0x21, 0x00, 0x01, 0x01, 0x22, 0x86, 0x07, 0x00, \
  INTERRUPT_ENDPOINT(0x86, 16)

#define SECURITY_INTERFACE_LEN  (9 + 6)
#define SECURITY_INTERFACE \
  VENDOR_INTERFACE(ITF_NUM_SECURITY, 0, XBOX_SECURITY_SUBCLASS, 0x13, STRID_SECURITY), \
  6, 0x41, 0x00, 0x01, 0x01, 0x03

#define CONFIG_TOTAL_LEN \
  (TUD_CONFIG_DESC_LEN + GAMEPAD_INTERFACE_LEN + AUDIO_INTERFACE_LEN + EXPANSION_PORT_INTERFACE_LEN + SECURITY_INTERFACE_LEN)

uint8_t const desc_configuration[] =
{
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 500),
  GAMEPAD_INTERFACE,
  AUDIO_INTERFACE,
  EXPANSION_PORT_INTERFACE,
  SECURITY_INTERFACE
};

TU_VERIFY_STATIC(sizeof(desc_configuration) == CONFIG_TOTAL_LEN, "Incorrect configuration descriptor size");

uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
  (void) index;
  return desc_configuration;
}

//--------------------------------------------------------------------+
// Microsoft OS 1.0 Descriptors
//--------------------------------------------------------------------+

// Makes Windows bind the "XUSB10" compatible ID, which loads the xusb22.sys Xbox 360 driver
uint8_t const desc_ms_os_compat_id[MS_OS_COMPAT_ID_LEN] =
{
  // Header
  U32_TO_U8S_LE(MS_OS_COMPAT_ID_LEN),        // dwLength
  U16_TO_U8S_LE(0x0100),                     // bcdVersion
  U16_TO_U8S_LE(0x0004),                     // wIndex: Extended Compat ID
  1,                                         // bCount
  0, 0, 0, 0, 0, 0, 0,                       // Reserved

  // Function section
  ITF_NUM_GAMEPAD,                           // bFirstInterfaceNumber
  0x04,                                      // Reserved: the spec says 0x01, the real guitar sends 0x04
  'X', 'U', 'S', 'B', '1', '0', 0, 0,        // compatibleID
  0, 0, 0, 0, 0, 0, 0, 0,                    // subCompatibleID
  0, 0, 0, 0, 0, 0                           // Reserved
};

// "MSFT100" signature followed by the vendor code the host must use to fetch desc_ms_os_compat_id
TU_ATTR_ALIGNED(2) static uint8_t const desc_ms_os_string[] =
{
  18, TUSB_DESC_STRING,
  'M', 0, 'S', 0, 'F', 0, 'T', 0, '1', 0, '0', 0, '0', 0,
  MS_OS_VENDOR_CODE, 0
};

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

static char const * const string_desc_arr[] =
{
  [STRID_MANUFACTURER] = "Harmonix Music",
  [STRID_PRODUCT]      = "Harmonix Guitar for Xbox 360",
  [STRID_SERIAL]       = "003D4C96",
  [STRID_SECURITY]     = "Xbox Security Method 3, Version 1.00, \xA9 2005 Microsoft Corporation. All rights reserved."
};

#define LANGID_ENGLISH_US  0x0409
#define MAX_STRING_CHARS   95

static uint16_t desc_string_buffer[1 + MAX_STRING_CHARS];

uint16_t const * tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
  (void) langid;
  uint8_t char_count;

  if (index == STRID_MS_OS)
  {
    return (uint16_t const *) desc_ms_os_string;
  }

  if (index == STRID_LANGID)
  {
    desc_string_buffer[1] = LANGID_ENGLISH_US;
    char_count = 1;
  }
  else
  {
    if (index >= TU_ARRAY_SIZE(string_desc_arr)) return NULL;

    char const * str = string_desc_arr[index];
    char_count = (uint8_t) tu_min32(strlen(str), MAX_STRING_CHARS);

    // ASCII to UTF-16LE (Latin-1 characters like \xA9 map directly)
    for (uint8_t i = 0; i < char_count; i++)
    {
      desc_string_buffer[1 + i] = (uint8_t) str[i];
    }
  }

  desc_string_buffer[0] = (uint16_t) ((TUSB_DESC_STRING << 8) | (2 * char_count + 2));
  return desc_string_buffer;
}
