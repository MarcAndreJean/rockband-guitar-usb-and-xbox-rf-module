# Build on MS Windows

To build you will need to install some extra tools.

- [Arm GNU Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/downloads) (you need the filename ending with-arm-none-eabi.exe)
- [CMake](https://cmake.org/download/)
- [Build Tools for Visual Studio 2022](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022)
- [Python 3.10](https://www.python.org/downloads/windows/)
- [Git](https://git-scm.com/download/win)

## Get the sdk
```
C:\dev> git clone -b master https://github.com/raspberrypi/pico-sdk.git
C:\dev> cd pico-sdk
C:\dev\pico-sdk> git submodule update --init --recursive
C:\dev\pico-sdk> setx PICO_SDK_PATH "C:\dev\pico-sdk"
```

## Build

```
C:\dev\rockband-guitar-usb-and-xbox-rf-module\hid> mkdir build
C:\dev\rockband-guitar-usb-and-xbox-rf-module\hid> cd build
C:\dev\rockband-guitar-usb-and-xbox-rf-module\hid> cmake -G "NMake Makefiles" .. -DFAMILY=rp2040
C:\dev\rockband-guitar-usb-and-xbox-rf-module\hid> nmake
```