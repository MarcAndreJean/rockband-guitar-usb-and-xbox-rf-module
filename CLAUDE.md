# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a guitar controller firmware project targeting the Raspberry Pi Pico (RP2040) platform. It implements a USB HID device that emulates a Rock Band/Guitar Hero style guitar controller compatible with Xbox 360, PC, Wii, and PS platforms. The firmware uses TinyUSB for USB device functionality and the Pico SDK for hardware abstraction.

## Build System

### Prerequisites (Windows)
- Arm GNU Toolchain (arm-none-eabi)
- CMake
- Build Tools for Visual Studio 2022
- Python 3.10
- Git
- Pico SDK (must be cloned separately and PICO_SDK_PATH environment variable set)

### Setting up Pico SDK
```bash
C:\dev> git clone -b master https://github.com/raspberrypi/pico-sdk.git
C:\dev> cd pico-sdk
C:\dev\pico-sdk> git submodule update --init --recursive
C:\dev\pico-sdk> setx PICO_SDK_PATH "C:\dev\pico-sdk"
```

### Building the firmware
```bash
cd hid
mkdir build
cd build
cmake -G "NMake Makefiles" .. -DFAMILY=rp2040
nmake
```

The build output will be `.uf2` files in `hid/build/` that can be flashed to the Pico.

## Code Architecture

### USB Device Implementation
The project implements a custom USB device with vendor-specific descriptors to emulate Xbox 360 guitar controller behavior:

- **VID**: 0x1BAD (Harmonix Music Systems)
- **PID**: 0x0002 (Guitar controller)
- Device presents as vendor class (0xFF) with multiple interfaces

### Key Components

**hid/main.c**: Main application logic
- Initializes TinyUSB device stack
- Runs main event loop with `tud_task()`, `led_blinking_task()`, and `hid_task()`
- `hid_task()` polls button inputs every 10ms and sends HID reports
- Implements device callbacks (mount, unmount, suspend, resume)
- Handles vendor control transfers including Microsoft OS 2.0 descriptors

**hid/usb_descriptors.c**: USB descriptor definitions
- Device descriptor with Xbox 360 guitar controller identifiers
- Configuration descriptor with 4 vendor-specific interfaces mimicking Xbox 360 protocol:
  - Interface 1: Subclass 0x5D, Protocol 0x01 (Security/Auth interface)
  - Interface 2: Subclass 0x5D, Protocol 0x03 (Main controller interface with 4 endpoints)
  - Interface 3: Subclass 0x5D, Protocol 0x02 (Secondary interface)
  - Interface 4: Subclass 0xFD, Protocol 0x13 (Additional Xbox interface)
- Microsoft OS 2.0 descriptors for Windows compatibility (WINUSB GUID registration)
- String descriptors identifying as "Harmonix Guitar for Xbox 360"

**hid/tusb_config.h**: TinyUSB configuration
- Enables HID (CFG_TUD_HID=1), CDC (CFG_TUD_CDC=1), and Vendor (CFG_TUD_VENDOR=1) classes
- Configures endpoint buffer sizes and FIFO sizes
- Sets endpoint 0 size to 8 bytes (required for Xbox 360 compatibility)

### USB Descriptor Strategy
The firmware uses a sophisticated approach to achieve cross-platform compatibility:

1. **Vendor Class Interfaces**: Instead of standard HID class, uses vendor-specific class (0xFF) to closely match Xbox 360 behavior
2. **Custom HID Descriptors**: Each interface has custom HID descriptor bytes embedded in the interface descriptor (17-byte, 27-byte, 9-byte, and 6-byte descriptors)
3. **Microsoft OS Descriptors**: Implements both OS 1.0 (in BOS) and OS 2.0 descriptors to enable driver-less operation on Windows via WinUSB
4. **Multiple Endpoints**: Interface 2 uses 4 endpoints (0x83 IN, 0x04 OUT, 0x85 IN, 0x05 OUT) matching Xbox 360 controller topology

### Submodules
- **hid/tinyusb**: Git submodule pointing to the TinyUSB library (github.com/hathach/tinyusb)
  - Must be initialized with `git submodule update --init --recursive`

## Development Notes

### Current Implementation Status
The current code has placeholder keyboard/mouse HID reports in `hid_task()` but the USB descriptors are configured for Xbox 360 guitar controller emulation. The actual guitar input handling (fret buttons, strum bar, whammy bar, etc.) needs to be implemented.

### When modifying USB descriptors:
- Interface indices in `usb_descriptors.c` must match those in `main.c` enum
- Endpoint numbers must be unique and consistent across descriptor definitions
- Total descriptor length (CONFIG_TOTAL_LEN) must be updated when adding/removing interfaces
- Microsoft OS descriptor lengths (MS_OS_20_DESC_LEN) must match the actual descriptor size

### Testing changes:
After building, flash to Pico by:
1. Hold BOOTSEL button while plugging in Pico
2. Copy the `.uf2` file from `hid/build/` to the RPI-RP2 drive
3. Device will automatically reboot with new firmware

Monitor device status via LED blinking:
- 250ms blink: Not mounted (not connected to host)
- 1000ms blink: Mounted (connected and enumerated)
- 2500ms blink: Suspended
