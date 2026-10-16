# Rock Band Guitar Controller Firmware - Build Guide for Windows

This guide will walk you through setting up your Windows development environment and building the firmware from scratch.

## Prerequisites

Before you begin, you'll need to install the following tools. **Install them in the order listed below.**

### 1. Git for Windows
**Recommended Version:** 2.40.0 or later

Download and install from: https://git-scm.com/download/win

- During installation, use default settings
- Ensure "Git from the command line and also from 3rd-party software" is selected

### 2. Python 3
**Recommended Version:** 3.10.x or 3.11.x

Download from: https://www.python.org/downloads/windows/

**IMPORTANT:** During installation:
- ✅ **CHECK** "Add Python to PATH" (this is critical!)
- Use default installation location
- After installation, verify in a new PowerShell window:
  ```powershell
  python --version
  ```
  Should show: `Python 3.10.x` or `Python 3.11.x`

### 3. CMake
**Recommended Version:** 3.25.0 or later

Download from: https://cmake.org/download/

- During installation, select "Add CMake to the system PATH for all users"

### 4. Arm GNU Toolchain
**Recommended Version:** 14.2.Rel1 (or latest stable release)

Download from: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/downloads

- Download the file ending with: `arm-none-eabi.exe`
- During installation, use default settings
- ✅ **CHECK** "Add path to environment variable" at the end of installation

### 5. Build Tools for Visual Studio 2022
**Recommended Version:** Latest

Download from: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022

- Run the installer
- Select "Desktop development with C++"
- You can deselect optional components to save space
- Installation size: ~2-7 GB depending on options

### Verify All Tools Are Installed

Open a **new** PowerShell window and run:
```powershell
git --version
python --version
cmake --version
arm-none-eabi-gcc --version
```

All commands should return version information. If any command fails, reinstall that tool and ensure it's added to PATH.

---

## Setup: Clone and Configure Pico SDK

### 1. Create Development Directory
```powershell
# Create a workspace (you can use any location you prefer)
mkdir C:\dev
cd C:\dev
```

### 2. Clone the Pico SDK
```powershell
# Clone the official Raspberry Pi Pico SDK
git clone -b master https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk

# Initialize all submodules (TinyUSB, etc.)
git submodule update --init --recursive
```

**Note:** The submodule update may take a few minutes.

### 3. Set the PICO_SDK_PATH Environment Variable
```powershell
# Set permanently for your user account
setx PICO_SDK_PATH "C:\dev\pico-sdk"
```

**IMPORTANT:** After running `setx`, you **MUST** close and reopen your PowerShell window for the variable to take effect.

### 4. Verify SDK Installation
Open a **new** PowerShell window and check:
```powershell
echo $env:PICO_SDK_PATH
```
Should output: `C:\dev\pico-sdk`

---

## Build the Firmware

### 1. Clone This Repository
```powershell
cd C:\dev
git clone <your-repository-url> rockband-guitar
cd rockband-guitar\hid
```

If this repository has submodules (TinyUSB), initialize them:
```powershell
git submodule update --init --recursive
```

### 2. Open Developer PowerShell for VS 2022

**DO NOT use regular PowerShell or Command Prompt!**

- Press Windows key
- Search for: **"Developer PowerShell for VS 2022"**
- Open it (should have blue background)

This special PowerShell has the Visual Studio build tools (nmake, cl.exe) in the PATH.

### 3. Navigate to Project Directory
```powershell
cd C:\dev\rockband-guitar\hid
```

### 4. Create Build Directory
```powershell
mkdir build
cd build
```

### 5. Run CMake
```powershell
cmake -G "NMake Makefiles" ..
```

This generates the build files. You should see output like:
```
-- Build files have been written to: C:/dev/rockband-guitar/hid/build
```

### 6. Build the Firmware
```powershell
nmake
```

Build time: ~30 seconds to 2 minutes depending on your system.

### 7. Build Output

If successful, you'll find these files in the `build` directory:
- **`hid.uf2`** - This is the firmware file you'll flash to your Pico
- `hid.elf` - ELF binary (for debugging)
- `hid.bin` - Raw binary
- `hid.hex` - Intel HEX format
- `hid.map` - Memory map

---

## Flashing the Firmware to Raspberry Pi Pico

### 1. Enter Bootloader Mode
1. **Disconnect** the Pico from USB
2. **Hold down** the BOOTSEL button on the Pico
3. **While holding BOOTSEL**, plug the Pico into your computer via USB
4. **Release** the BOOTSEL button

The Pico should appear as a USB drive named **RPI-RP2**

### 2. Copy Firmware
Simply drag and drop (or copy) the `hid.uf2` file from the `build` folder to the **RPI-RP2** drive.

### 3. Automatic Reboot
The Pico will automatically:
- Copy the firmware
- Reboot
- Start running your new firmware
- Disappear as a USB drive and appear as your custom USB device

---

## Rebuilding After Code Changes

After modifying code (e.g., `main.c`, `usb_descriptors.c`):

```powershell
# Make sure you're in the Developer PowerShell for VS 2022
cd C:\dev\rockband-guitar\hid\build
nmake
```

You don't need to run `cmake` again unless you:
- Add new source files
- Modify `CMakeLists.txt`
- Delete the build directory

---

## Troubleshooting

### "Python3 not found" Error
- Make sure Python is installed with "Add to PATH" checked
- Restart your terminal after installing Python
- Verify: `python --version` should work
- If needed, manually specify: `cmake -G "NMake Makefiles" -DPython3_EXECUTABLE="C:\Path\To\python.exe" ..`

### "arm-none-eabi-gcc not found" Error
- Reinstall Arm GNU Toolchain
- Ensure "Add path to environment variable" is checked during installation
- Restart your terminal

### "nmake is not recognized" Error
- You're not using Developer PowerShell for VS 2022
- Regular PowerShell/CMD won't work - you need the VS 2022 Developer version

### "PICO_SDK_PATH not set" Error
- Run: `setx PICO_SDK_PATH "C:\dev\pico-sdk"`
- Close and reopen your terminal
- Verify: `echo $env:PICO_SDK_PATH`

### Build Fails with "permission denied" or file locking errors
- Close any programs that might have files open (VSCode, editors, etc.)
- Delete the `build` folder and recreate it
- Try again

### Clean Build (Start Fresh)
```powershell
cd C:\dev\rockband-guitar\hid
rm -r -force build
mkdir build
cd build
cmake -G "NMake Makefiles" ..
nmake
```

---

## Project Structure

```
hid/
├── main.c                  # Main application code
├── usb_descriptors.c       # USB device descriptors (VID/PID, interfaces, etc.)
├── tusb_config.h           # TinyUSB configuration
├── CMakeLists.txt          # Build configuration
├── build/                  # Build output (created by you)
│   └── hid.uf2            # Firmware file to flash
└── readme.md              # This file
```

---

## Additional Resources

- [Raspberry Pi Pico Documentation](https://www.raspberrypi.com/documentation/microcontrollers/)
- [Pico SDK Documentation](https://raspberrypi.github.io/pico-sdk-doxygen/)
- [TinyUSB Documentation](https://docs.tinyusb.org/)
- [Getting Started with Pico (Official PDF)](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf)

---

## Quick Reference: Full Build from Scratch

```powershell
# 1. Install all prerequisites (see Prerequisites section above)

# 2. Setup Pico SDK
cd C:\dev
git clone -b master https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init --recursive
setx PICO_SDK_PATH "C:\dev\pico-sdk"

# 3. Close and reopen PowerShell, then clone project
cd C:\dev
git clone <repo-url> rockband-guitar
cd rockband-guitar\hid
git submodule update --init --recursive

# 4. Open Developer PowerShell for VS 2022, then build
cd C:\dev\rockband-guitar\hid
mkdir build
cd build
cmake -G "NMake Makefiles" ..
nmake

# 5. Flash hid.uf2 to Pico in BOOTSEL mode
```
