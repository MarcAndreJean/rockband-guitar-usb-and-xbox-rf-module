# Firmware

Firmware for the Raspberry Pi Pico (RP2040), built with the Pico SDK and its bundled TinyUSB.

## Prerequisites (Windows)

- [Git](https://git-scm.com/download/win)
- [Python 3](https://www.python.org/downloads/windows/) (check "Add Python to PATH")
- [CMake](https://cmake.org/download/) 3.13 or later
- [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) (`arm-none-eabi`), added to PATH
- [Build Tools for Visual Studio 2022](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022) with "Desktop development with C++"
- [Pico SDK](https://github.com/raspberrypi/pico-sdk):

```powershell
git clone https://github.com/raspberrypi/pico-sdk.git C:\dev\pico-sdk
cd C:\dev\pico-sdk
git submodule update --init
setx PICO_SDK_PATH "C:\dev\pico-sdk"
```

Open a new terminal after `setx` so the variable is picked up.

## Build

Run from **Developer PowerShell for VS 2022**, which provides `nmake`:

```powershell
cd firmware
mkdir build; cd build
cmake -G "NMake Makefiles" ..
nmake
```

The output is `build/rockband_guitar.uf2`. After a code change, `nmake` alone is enough.

## Flash

1. Hold the **BOOTSEL** button while plugging the Pico into USB.
2. Copy `rockband_guitar.uf2` to the `RPI-RP2` drive that appears.
3. The Pico reboots and enumerates as "Harmonix Guitar for Xbox 360".

## Troubleshooting

| Error | Fix |
| --- | --- |
| `Could NOT find Python3` | Pass the interpreter explicitly: `cmake -G "NMake Makefiles" -DPython3_EXECUTABLE="C:\path\to\python.exe" ..` |
| `nmake is not recognized` | Use Developer PowerShell for VS 2022, not a regular terminal |
| `PICO_SDK_PATH not set` | Run the `setx` command above and reopen the terminal |
