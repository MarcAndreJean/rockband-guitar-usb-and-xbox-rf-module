<div align="center">
  <img src="images/logo.png" alt="Logo" width="80" height="80">
  <h3>Rock Band Guitar Controller</h3>
  <p>Raspberry Pi Pico firmware that poses as a Harmonix Rock Band guitar for Xbox 360</p>

  [![MIT License](https://img.shields.io/github/license/MarcAndreJean/rockband-guitar-usb-and-xbox-rf-module.svg)](LICENSE)
</div>

## About

This project builds a custom guitar controller for rhythm games, starting with the USB side. The firmware runs on a Raspberry Pi Pico (RP2040) with [TinyUSB]. To the host it looks exactly like an original Harmonix Rock Band guitar for Xbox 360 (VID `0x1BAD`, PID `0x0002`).

I got there by reverse-engineering a real guitar. I dumped its USB descriptors, captured its traffic with Wireshark and a hardware bus sniffer, then rebuilt every descriptor byte for byte. The notes and captures are in [`docs/research/`](docs/research/NOTES.md).

## Status

What works:
- [x] Device and configuration descriptors identical to the real guitar (all 4 vendor interfaces and 7 endpoints)
- [x] Microsoft OS 1.0 descriptors that point Windows to its built-in Xbox 360 driver (`xusb22`)

Not done yet:
- [ ] Xbox 360 security handshake (see below)
- [ ] Guitar input reports (frets, strum, whammy, etc.)

## Known limitation: Xbox 360 authentication

The console only accepts licensed controllers. While the controller enumerates, the console sends it a challenge through vendor control requests. A licensed controller signs the answer with a Microsoft security chip, using keys that aren't public.

The Pico has no such chip. It answers with zeros, so the console rejects it. Firmware alone can't fix this. The options are:
- **Pass-through auth:** relay the challenge to a genuine controller and send its answer back, like [GIMX] does.
- **Modded console:** use a console that skips the check.
- **Target PC:** the Windows driver doesn't need this handshake.

## Project structure

```
firmware/           Pico firmware (C, TinyUSB)
  main.c            Main loop, vendor control requests, status LED
  usb_descriptors.c Device, configuration, string and Microsoft OS descriptors
  tusb_config.h     TinyUSB configuration
docs/research/      Descriptor dumps, USB captures and notes from the real guitar
```

## Build

See [`firmware/README.md`](firmware/README.md). In short, with the [Pico SDK] installed and `PICO_SDK_PATH` set:

```powershell
cd firmware
mkdir build; cd build
cmake -G "NMake Makefiles" ..
nmake
```

Then hold BOOTSEL while plugging in the Pico, and copy `rockband_guitar.uf2` to the `RPI-RP2` drive.

The onboard LED shows the USB state: fast blink (250 ms) means not mounted, slow blink (1 s) means mounted, and very slow blink (2.5 s) means suspended.

## Next steps

1. Send real input reports so the guitar works on PC
2. Wire the guitar hardware: frets, strum bar, whammy, buttons and LEDs
3. Build the guitar body and neck

## License

MIT. See [LICENSE](LICENSE).

## Contact

Marc-André Jean on [LinkedIn](https://www.linkedin.com/in/marc-andr%C3%A9-jean)

[TinyUSB]: https://github.com/hathach/tinyusb
[GIMX]: https://github.com/matlo/GIMX
[Pico SDK]: https://github.com/raspberrypi/pico-sdk
