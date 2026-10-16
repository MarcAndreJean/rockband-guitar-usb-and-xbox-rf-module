#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

#include <stdint.h>

// Vendor request code announced by the "MSFT100" string descriptor (index 0xEE)
#define MS_OS_VENDOR_CODE    0x90
#define MS_OS_COMPAT_ID_LEN  (16 + 24)

extern uint8_t const desc_ms_os_compat_id[MS_OS_COMPAT_ID_LEN];

#endif /* USB_DESCRIPTORS_H_ */
