# Research Notes

Reverse-engineering notes for the Harmonix Rock Band guitar for Xbox 360 (VID `0x1BAD`, PID `0x0002`).

## Files

| Path | Content |
| --- | --- |
| `real-guitar/descriptor-dump.txt` | Full descriptor dump of the real guitar (Thesycon USB Descriptor Dumper) |
| `real-guitar/config-descriptor.bin` | Raw 153-byte configuration descriptor of the real guitar |
| `pico/descriptor-dump.txt` | Same dump taken from the Pico firmware, for comparison |
| `captures/usbpcap-windows-*.pcapng` | USBPcap captures on Windows (open with Wireshark) |
| `captures/bus-sniffer-*.pcap(ng)` | Hardware USB bus sniffer captures (open with Wireshark) |

## Guitar inputs and outputs

| Part | Inputs | Type |
| --- | --- | --- |
| Frets (upper + lower) | 10 | Digital |
| Strum bar | 2 | Digital |
| D-pad + Start + Back | 6 | Digital |
| Xbox guide button | 1 | Digital |
| Effect selector | 2 | Analog |
| Whammy bar | 2 | Analog |
| Tilt / Overdrive | 1 | Digital |

Outputs: 4 player LEDs around the guide button.

## USB interfaces

| # | Class / Subclass / Protocol | Endpoints | Role |
| --- | --- | --- | --- |
| 0 | `FF / 5D / 01` | `0x81` IN, `0x02` OUT | Gamepad (input reports, LEDs, rumble) |
| 1 | `FF / 5D / 03` | `0x83` IN, `0x04` OUT, `0x85` IN, `0x05` OUT | Audio / headset |
| 2 | `FF / 5D / 02` | `0x86` IN | Expansion port |
| 3 | `FF / FD / 13` | none | Security (string "Xbox Security Method 3…") |

Each interface carries an undocumented vendor descriptor (type `0x21` or `0x41`) right after the interface descriptor. The firmware copies them byte for byte.

## Windows driver binding

Windows loads `xusb22.sys` ("Xbox 360 Controller for Windows") for the real guitar.

- Hardware IDs: `USB\VID_1BAD&PID_0002&REV_0103`, `USB\VID_1BAD&PID_0002`
- Compatible IDs: `USB\MS_COMP_XUSB10`, `USB\Class_FF&SubClass_5D&Prot_01`, `USB\Class_FF`

`MS_COMP_XUSB10` comes from the Microsoft OS 1.0 descriptors:

1. The host reads string descriptor `0xEE` and gets `MSFT100` with vendor code `0x90`.
2. The host sends vendor request `0x90` with `wIndex = 4` and gets the Extended Compat ID descriptor:

```
28 00 00 00 00 01 04 00   dwLength=40, bcdVersion=1.00, wIndex=4
01 00 00 00 00 00 00 00   bCount=1, reserved
00 04 58 55 53 42 31 30   interface 0, reserved=0x04, "XUSB10"
00 00 00 00 00 00 00 00   subCompatibleID
00 00 00 00 00 00 00 00   reserved
```

The real guitar uses `0x04` in the reserved byte, where the spec says `0x01`. The firmware copies this.

## Enumeration on Windows

```
GET_DESCRIPTOR  DEVICE
GET_DESCRIPTOR  CONFIGURATION (first 9 bytes, then full 153 bytes)
GET_STATUS
SET_CONFIGURATION
SET_INTERFACE (x2)
GET_DESCRIPTOR  STRING (product name)
```

Then the driver sends vendor requests:

| bmRequestType | bRequest | wValue | wIndex | wLength | Reply |
| --- | --- | --- | --- | --- | --- |
| `0xC1` (interface) | `0x01` | `0x0100` | 0 | 20 | none |
| `0xC1` (interface) | `0x01` | `0x0000` | 0 | 8 | none |
| `0xC0` (device) | `0x01` | `0x0000` | 0 | 4 | `01 03 0e ..` |

After that, the guitar streams 20-byte input reports on endpoint `0x81`:

```
00 14 00 00 19 00 00 00 00 00 ff 7f 00 00 00 00 00 00 00 00
00 14 00 00 4c 00 00 00 00 00 f0 7c 00 00 00 00 00 00 00 00
```

This is the standard Xbox 360 report: type `0x00`, length `0x14`, 2 bytes of buttons, 2 trigger bytes, then four 16-bit stick axes.

## References

- [Microsoft-defined USB descriptors](https://learn.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors)
- [How the USB stack enumerates a device](https://techcommunity.microsoft.com/t5/microsoft-usb-blog/how-does-usb-stack-enumerate-a-device/ba-p/270685)
- [Reverse-engineering a USB protocol (OpenRazer)](https://github.com/openrazer/openrazer/wiki/Reverse-Engineering-USB-Protocol)
- [Xbox 360 RF module controlled with an Arduino](https://www.electromaker.io/project/view/xbox-360-rf-module-controlled-with-an-arduino-1)
- [GIMX](https://github.com/matlo/GIMX), which handles Xbox 360 auth by pass-through
- [Thesycon USB Descriptor Dumper](https://www.thesycon.de/eng/usb_descriptordumper.shtml)
