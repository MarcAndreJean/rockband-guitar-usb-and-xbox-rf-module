#include "tusb.h"
#include "usb_descriptors.h"

/* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
 * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]       MIDI | HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
// #define USB_PID           (0x0000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) |
//                            _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) )
#define USB_PID   0x0002
#define AUTO_USB_PID         (0x0000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
                            _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) )


#define USB_VID   0x1BAD
#define USB_BCD   0x0200

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,

    // Use Interface Association Descriptor (IAD) for CDC
    // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
    .bDeviceClass       = 0xFF,
    .bDeviceSubClass    = 0xFF,
    .bDeviceProtocol    = 0xFF,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = USB_VID, // 0x1BAD
    .idProduct          = USB_PID, // 0x0002
    .bcdDevice          = 0x0103,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const * tud_descriptor_device_cb(void)
{
  return (uint8_t const *) &desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

uint8_t const desc_hid_report1[] =
{
  TUD_HID_REPORT_DESC_KEYBOARD()
};

uint8_t const desc_hid_report2[] =
{
  TUD_HID_REPORT_DESC_MOUSE()
};

uint8_t const desc_hid_report3[] =
{
  TUD_HID_REPORT_DESC_MOUSE()
};

uint8_t const desc_hid_report4[] =
{
  TUD_HID_REPORT_DESC_MOUSE()
};

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_hid_descriptor_report_cb(uint8_t itf)
{
  if (itf == 0)
  {
    return desc_hid_report1;
  }
  else if (itf == 1)
  {
    return desc_hid_report2;
  }
  else if (itf == 2)
  {
    return desc_hid_report3;
  }
  else if (itf == 3)
  {
    return desc_hid_report4;
  }

  return NULL;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

enum
{
  ITF_NUM_HID1 = 0,
  ITF_NUM_HID2,
  ITF_NUM_HID3,
  ITF_NUM_HID4,
  ITF_NUM_TOTAL
};


// Interface #1
#define TUD_VENDOR_DESC_1_LEN  (9+17+7+7)

#define TUD_VENDOR_DESCRIPTOR_1(_itfnum, _stridx, _subClass, _protocol) \
  /* Interface */\
  9, TUSB_DESC_INTERFACE, _itfnum, 0, 2, TUSB_CLASS_VENDOR_SPECIFIC, _subClass, _protocol, _stridx,\
  /* HID */\
  17, 0x21, 0x10, 0x01, 0x06, 0x25, 0x81, 0x14, 0x03, 0x03, 0x03, 0x04, 0x13, 0x02, 0x08, 0x03, 0x03,\
  /* Endpoint In */\
  7, TUSB_DESC_ENDPOINT, 0x81, 0x03, U16_TO_U8S_LE(0x0020), 0x04,\
  /* Endpoint Out */\
  7, TUSB_DESC_ENDPOINT, 0x02, 0x03, U16_TO_U8S_LE(0x0020), 0x08


// Interface #2
#define TUD_VENDOR_DESC_2_LEN  (9+27+7+7+7+7)

#define TUD_VENDOR_DESCRIPTOR_2(_itfnum, _stridx, _subClass, _protocol) \
  /* Interface */\
  9, TUSB_DESC_INTERFACE, _itfnum, 0, 4, TUSB_CLASS_VENDOR_SPECIFIC, _subClass, _protocol, _stridx,\
  /* HID */\
  27, 0x21, 0x00, 0x01, 0x01, 0x01, 0x83, 0x40, 0x01, 0x04, 0x20, 0x16, 0x85, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
  /* Endpoint In 1*/\
  7, TUSB_DESC_ENDPOINT, 0x83, 0x03, U16_TO_U8S_LE(0x0020), 0x02,\
  /* Endpoint Out 1*/\
  7, TUSB_DESC_ENDPOINT, 0x04, 0x03, U16_TO_U8S_LE(0x0020), 0x04,\
  /* Endpoint In 2*/\
  7, TUSB_DESC_ENDPOINT, 0x85, 0x03, U16_TO_U8S_LE(0x0020), 0x40,\
  /* Endpoint Out 2*/\
  7, TUSB_DESC_ENDPOINT, 0x05, 0x03, U16_TO_U8S_LE(0x0020), 0x10


// Interface #3
#define TUD_VENDOR_DESC_3_LEN  (9+9+7)

#define TUD_VENDOR_DESCRIPTOR_3(_itfnum, _stridx, _subClass, _protocol) \
  /* Interface */\
  9, TUSB_DESC_INTERFACE, _itfnum, 0, 1, TUSB_CLASS_VENDOR_SPECIFIC, _subClass, _protocol, _stridx,\
  /* HID */\
  9, 0x21, 0x00, 0x01, 0x01, 0x22, 0x86, 0x07, 0x00,\
  /* Endpoint In */\
  7, TUSB_DESC_ENDPOINT, 0x86, 0x03, U16_TO_U8S_LE(0x0020), 0x10


// Interface #4
#define TUD_VENDOR_DESC_4_LEN  (9+6)

#define TUD_VENDOR_DESCRIPTOR_4(_itfnum, _stridx, _subClass, _protocol) \
  /* Interface */\
  9, TUSB_DESC_INTERFACE, _itfnum, 0, 0, TUSB_CLASS_VENDOR_SPECIFIC, _subClass, _protocol, _stridx,\
  /* HID */\
  6, 0x41, 0x00, 0x01, 0x01, 0x03


// Microsoft descriptor
#define MICROSOFT_OS_DESC_LEN 18
#define MICROSOFT_OS_DESC() \
  MICROSOFT_OS_DESC_LEN, 0x03, 0x4D, 0x00, 0x53, 0x00, 0x46, 0x00, 0x54, 0x00, 0x31, 0x00, 0x30, 0x00, 0x30, 0x00, 0x90, 0x00

#define  CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_VENDOR_DESC_1_LEN + TUD_VENDOR_DESC_2_LEN + TUD_VENDOR_DESC_3_LEN + TUD_VENDOR_DESC_4_LEN + MICROSOFT_OS_DESC_LEN)

#define EPNUM_HID1   0x81
#define EPNUM_HID2   0x82
#define EPNUM_HID3   0x83
#define EPNUM_HID4   0x84

uint8_t const desc_configuration[] =
{
  // Config number, interface count, string index, total length, attribute, power in mA
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

  // EP: 0 control, 1 In, 2 Bulk, 3 Iso, 4 In etc ...
  // bInterfaceNumber, iInterface, bInterfaceSubClass, bInterfaceProtocol
  TUD_VENDOR_DESCRIPTOR_1(ITF_NUM_HID1, 0x00, 0x5D, 0x01),
  TUD_VENDOR_DESCRIPTOR_2(ITF_NUM_HID2, 0x00, 0x5D, 0x03),
  TUD_VENDOR_DESCRIPTOR_3(ITF_NUM_HID3, 0x00, 0x5D, 0x02),
  TUD_VENDOR_DESCRIPTOR_4(ITF_NUM_HID4, 0x04, 0xFD, 0x13),

  // Microsoft OS Descriptor
  MICROSOFT_OS_DESC()
};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
  (void) index; // for multiple configurations
  return desc_configuration;
}

//--------------------------------------------------------------------+
// BOS Descriptor
//--------------------------------------------------------------------+

/* Microsoft OS 2.0 registry property descriptor
Per MS requirements https://msdn.microsoft.com/en-us/library/windows/hardware/hh450799(v=vs.85).aspx
device should create DeviceInterfaceGUIDs. It can be done by driver and
in case of real PnP solution device should expose MS "Microsoft OS 2.0
registry property descriptor". Such descriptor can insert any record
into Windows registry per device/configuration/interface. In our case it
will insert "DeviceInterfaceGUIDs" multistring property.

GUID is freshly generated and should be OK to use.

https://developers.google.com/web/fundamentals/native-hardware/build-for-webusb/
(Section Microsoft OS compatibility descriptors)
*/

#define VENDOR_REQUEST_1 1
#define VENDOR_REQUEST_2 2
#define ITF_NUM_VENDOR 2

#define BOS_TOTAL_LEN      (0x12)

#define MS_OS_20_DESC_LEN  0xB2

// BOS Descriptor is required
uint8_t const desc_bos[] =
{
  0x12, 0x03, 0x4D, 0x00, 0x53, 0x00, 0x46, 0x00, 0x54, 0x00, 0x31, 0x00, 0x30, 0x00, 0x30, 0x00, 0x90, 0x00
};

uint8_t const * tud_descriptor_bos_cb(void)
{
  return desc_bos;
}

uint8_t const desc_ms_os_20[] =
{
  // Set header: length, type, windows version, total length
  U16_TO_U8S_LE(0x000A), U16_TO_U8S_LE(MS_OS_20_SET_HEADER_DESCRIPTOR), U32_TO_U8S_LE(0x06030000), U16_TO_U8S_LE(MS_OS_20_DESC_LEN),

  // Configuration subset header: length, type, configuration index, reserved, configuration total length
  U16_TO_U8S_LE(0x0008), U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_CONFIGURATION), 0, 0, U16_TO_U8S_LE(MS_OS_20_DESC_LEN-0x0A),

  // Function Subset header: length, type, first interface, reserved, subset length
  U16_TO_U8S_LE(0x0008), U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_FUNCTION), ITF_NUM_VENDOR, 0, U16_TO_U8S_LE(MS_OS_20_DESC_LEN-0x0A-0x08),

  // MS OS 2.0 Compatible ID descriptor: length, type, compatible ID, sub compatible ID
  U16_TO_U8S_LE(0x0014), U16_TO_U8S_LE(MS_OS_20_FEATURE_COMPATBLE_ID), 'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // sub-compatible

  // MS OS 2.0 Registry property descriptor: length, type
  U16_TO_U8S_LE(MS_OS_20_DESC_LEN-0x0A-0x08-0x08-0x14), U16_TO_U8S_LE(MS_OS_20_FEATURE_REG_PROPERTY),
  U16_TO_U8S_LE(0x0007), U16_TO_U8S_LE(0x002A), // wPropertyDataType, wPropertyNameLength and PropertyName "DeviceInterfaceGUIDs\0" in UTF-16
  'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00, 'I', 0x00, 'n', 0x00, 't', 0x00, 'e', 0x00,
  'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00, 'e', 0x00, 'G', 0x00, 'U', 0x00, 'I', 0x00, 'D', 0x00, 's', 0x00, 0x00, 0x00,
  U16_TO_U8S_LE(0x0050), // wPropertyDataLength
	//bPropertyData: “{975F44D9-0D08-43FD-8B3E-127CA8AFFF9D}”.
  '{', 0x00, '9', 0x00, '7', 0x00, '5', 0x00, 'F', 0x00, '4', 0x00, '4', 0x00, 'D', 0x00, '9', 0x00, '-', 0x00,
  '0', 0x00, 'D', 0x00, '0', 0x00, '8', 0x00, '-', 0x00, '4', 0x00, '3', 0x00, 'F', 0x00, 'D', 0x00, '-', 0x00,
  '8', 0x00, 'B', 0x00, '3', 0x00, 'E', 0x00, '-', 0x00, '1', 0x00, '2', 0x00, '7', 0x00, 'C', 0x00, 'A', 0x00,
  '8', 0x00, 'A', 0x00, 'F', 0x00, 'F', 0x00, 'F', 0x00, '9', 0x00, 'D', 0x00, '}', 0x00, 0x00, 0x00, 0x00, 0x00
};

TU_VERIFY_STATIC(sizeof(desc_ms_os_20) == MS_OS_20_DESC_LEN, "Incorrect size");

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// array of pointer to string descriptors
char const* string_desc_arr [] =
{
  (const char[]) { 0x09, 0x04 },   // 0: is supported language is English (0x0409)
  "Harmonix Music",                // 1: Manufacturer
  "Harmonix Guitar for Xbox 360",  // 2: Product
  "003D4C96",                      // 3: Serials, should use chip ID
  "Xbox Security Method 3, Version 1.00"
};

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
  (void) langid;

  uint8_t chr_count;

  if ( index == 0)
  {
    memcpy(&_desc_str[1], string_desc_arr[0], 2);
    chr_count = 1;
  }else
  {
    // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

    if ( !(index < sizeof(string_desc_arr)/sizeof(string_desc_arr[0])) ) return NULL;

    const char* str = string_desc_arr[index];

    // Cap at max char
    chr_count = (uint8_t) strlen(str);
    if ( chr_count > 31 ) chr_count = 31;

    // Convert ASCII string into UTF-16
    for(uint8_t i=0; i<chr_count; i++)
    {
      _desc_str[1+i] = str[i];
    }
  }

  // first byte is length (including header), second byte is string type
  _desc_str[0] = (uint16_t) ((TUSB_DESC_STRING << 8 ) | (2*chr_count + 2));

  return _desc_str;
}
