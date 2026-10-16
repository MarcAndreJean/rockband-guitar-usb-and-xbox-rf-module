<div id="top"></div>

<!-- PROJECT SHIELDS -->
[![Contributors][contributors-shield]][contributors-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]
[![LinkedIn][linkedin-shield]][linkedin-url]



<!-- PROJECT LOGO -->
<br />
<div align="center">
  <a href="https://github.com/MarcAndreJean/rockband-guitar-usb-and-xbox-rf-module">
    <img src="images/logo.png" alt="Logo" width="80" height="80">
  </a>

  <h3 align="center">Guitar controller - USB & wireless (multi-platform)</h3>

  <p align="center">
    Guitar controller to rock! [Wii | Xbox | PS | PC]
    <br />
    <br />
    <a href="https://github.com/MarcAndreJean/rockband-guitar-usb-and-xbox-rf-module/issues">Report Bug</a>
    ·
    <a href="https://github.com/MarcAndreJean/rockband-guitar-usb-and-xbox-rf-module/issues">Request Feature</a>
  </p>
</div>



<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li><a href="#about-the-project">About The Project</a></li>
    <li><a href="#current-status">Current Status</a></li>
    <li><a href="#known-limitation-xbox-360-security-authentication">Known Limitation: Xbox 360 Security Authentication</a></li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
    <li><a href="#acknowledgments">Acknowledgments</a></li>
  </ol>
</details>



<!-- ABOUT THE PROJECT -->
## About The Project

<!--[![Product Name Screen Shot][product-screenshot]](https://example.com)-->

To feed

<p align="right">(<a href="#top">back to top</a>)</p>

<!-- CURRENT STATUS -->
## Current Status

The firmware runs on a Raspberry Pi Pico (RP2040) with TinyUSB and pretends to be a Harmonix Rock Band guitar for Xbox 360 (VID `0x1BAD`, PID `0x0002`).

What works:
- [x] Device descriptor matches the real Harmonix guitar
- [x] Configuration descriptor with all 4 vendor interfaces and their endpoints matches the real guitar
- [x] Microsoft OS 1.0 descriptors (OS string descriptor + OS feature descriptor)

What doesn't work:
- [ ] Xbox 360 security handshake (see below)
- [ ] Real guitar input reports (frets, strum, whammy, etc.)

<p align="right">(<a href="#top">back to top</a>)</p>

<!-- KNOWN LIMITATION -->
## Known Limitation: Xbox 360 Security Authentication

This is the wall the project is stuck on.

Interface 1 (subclass `0x5D`, protocol `0x01`) is the Xbox 360 security interface. When the controller is plugged in, the console sends it a cryptographic challenge through vendor control requests (e.g. `wValue` `0x5C10` and `0x5B17`). A licensed controller answers with a Microsoft security chip, which holds keys that aren't public.

The Pico has no such chip, so it can't build a valid answer. The current code only sends placeholder (zeroed) data back. The console sees the auth fail and doesn't accept the controller.

Firmware alone can't fix this. The options are:
- **Pass-through auth:** relay the challenge to a genuine licensed controller (or its security chip) and send its answer back, like [GIMX](https://github.com/matlo/GIMX) does.
- **Modded console:** run it on a console that skips the auth check.
- **Aim for PC instead:** the Windows `xusb22` driver doesn't need this auth, so PC support doesn't hit this limit.

Captures and descriptor dumps from the real guitar are in [`information/`](information/).

<p align="right">(<a href="#top">back to top</a>)</p>

<!-- ROADMAP -->
## Roadmap

V0.1
- [ ] Mini-controller code
  - [ ] Reproduce mini-controller with an arduino (receive inputs)
  - [ ]  Send output to USB (xbox and pc)

V0.2
- [ ] Body Parts
  - [ ] Prototype whammy
  - [ ] Prototype strum
  - [ ] Prototype boost
  - [ ] Prototype default button (back, start, A, B, X, Y, Up, Down, Bottom, Left, Right)
  - [ ] Prototype controller number (4 leds) + menu button
  - [ ] Prototype effect selector (ez)

V0.3
- [ ] Neck Parts
  - [ ] Prototype one fret button
  - [ ] Prototype guitar fret with 5 frets

V0.4
- [ ] Guitar
  - [ ] Prototype guitar body
  - [ ] Prototype guitar neck
  - [ ] Full prototype assembly

V1
- [ ] Guitar body
- [ ] Guitar neck
- [ ] First assembly

V2
- [ ] Custom mini-controller

See the [open issues](https://github.com/MarcAndreJean/rockband-guitar-usb-and-xbox-rf-module/issues) for a full list of proposed features (and known issues).

<p align="right">(<a href="#top">back to top</a>)</p>


<!-- LICENSE -->
## License

Distributed under the MIT License. See `LICENSE.txt` for more information.

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- CONTACT -->
## Contact

Marc-André Jean - [@MkII_4ndr3](https://twitter.com/MkII_4ndr3) - the.mapaq@gmail.com

Project Link: [https://github.com/MarcAndreJean/rockband-guitar-usb-and-xbox-rf-module](https://github.com/MarcAndreJean/rockband-guitar-usb-and-xbox-rf-module)

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- ACKNOWLEDGMENTS -->
## Acknowledgments

<!--* [Choose an Open Source License](https://choosealicense.com)-->

<p align="right">(<a href="#top">back to top</a>)</p>

<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/MarcAndreJean/rockband-guitar-usb-and-xbox-rf-module.svg?style=for-the-badge
[contributors-url]: https://github.com/MarcAndreJean/rockband-guitar-usb-and-xbox-rf-module/graphs/contributors
[forks-shield]: https://img.shields.io/MarcAndreJean/rockband-guitar-usb-and-xbox-rf-module.svg?style=for-the-badge
[forks-url]: https://github.com/MarcAndreJean/rockband-guitar-usb-and-xbox-rf-module/network/members
[stars-shield]: https://img.shields.io/github/stars/MarcAndreJean/rockband-guitar-usb-and-xbox-rf-module.svg?style=for-the-badge
[stars-url]: https://github.com/MarcAndreJean/rockband-guitar-usb-and-xbox-rf-module/stargazers
[issues-shield]: https://img.shields.io/github/issues/MarcAndreJean/rockband-guitar-usb-and-xbox-rf-module.svg?style=for-the-badge
[issues-url]: https://github.com/MarcAndreJean/rockband-guitar-usb-and-xbox-rf-module/issues
[license-shield]: https://img.shields.io/github/license/MarcAndreJean/rockband-guitar-usb-and-xbox-rf-module.svg?style=for-the-badge
[license-url]: https://github.com/MarcAndreJean/rockband-guitar-usb-and-xbox-rf-module/blob/master/LICENSE.txt
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://www.linkedin.com/in/marc-andr%C3%A9-jean
[product-screenshot]: images/screenshot.png
