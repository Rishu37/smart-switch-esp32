# ESP32 Smart Switch (2 / 4 / 8 relays)

A do-it-yourself WiFi smart switch built on an **ESP32**, a **relay board** and
the free **ESP RainMaker** cloud. Control 2, 4 or 8 appliances from your phone,
from anywhere, with schedules, saved relay states and a built-in WiFi reset.

This repository contains the firmware, the wiring guide, the safety guide and
a step-by-step build manual, so that anyone can reproduce the project.

> **⚠️ Mains voltage can kill and start fires.** The low-voltage part of this
> project (ESP32 + relay board on a USB supply) is safe to experiment with.
> Connecting 220–240 V appliances is not. Read
> [docs/07-safety-and-installation.md](docs/07-safety-and-installation.md)
> before you connect anything to mains, and have a licensed electrician inspect
> any unit that will stay in a home. This is an educational prototype, not a
> certified product.

---

## Features

- **One firmware, three sizes:** 2, 4 or 8 relays, chosen by changing one number
- **Phone app control** (ESP RainMaker Home, Android and iOS), local and remote
- **Easy WiFi setup:** scan a QR code, pick your WiFi, done (no hard-coded passwords)
- **Remembers relay states** after a power cut (can be switched off)
- **Schedules** with the app (e.g. "Light ON 18:30 every day")
- **WiFi reset:** hold the BOOT button about 3 s; factory reset about 10 s
- **Over-the-air updates** supported by the RainMaker platform
- Free to use for prototypes: no server of your own is needed

## How it works (short version)

```
 Phone app  <──── HTTPS ────>  ESP RainMaker cloud  <──── MQTT over TLS ────>  ESP32
 (anywhere)                    (run by Espressif)                               │
                                                                    GPIO pins ──┤
                                                                                ▼
                                                                         Relay board ──► your appliances
```

The phone and the ESP32 never talk to each other directly. Both talk to the
cloud, which is why it works from mobile data too. Details:
[docs/01-how-it-works.md](docs/01-how-it-works.md).

## Variants

| Variant | Relays | Default pins (GPIO) | Default app names |
|---|---|---|---|
| 2 | 2 | 16, 17 | Light, Fan |
| 4 | 4 | 16, 17, 18, 21 | Switch 1, Switch 2, Fan, Light |
| 8 | 8 | 16, 17, 18, 21, 22, 23, 25, 26 | Switch 1-6, Fan, Light |

Change the variant, pins, names and relay polarity in
[`firmware/main/relay_config.h`](firmware/main/relay_config.h).

## Quick start

You need: Windows PC, [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/),
Git, an ESP32 dev board, a relay board and the ESP RainMaker Home phone app.
The full walkthrough is in [docs/04-build-and-flash.md](docs/04-build-and-flash.md).

```powershell
# 1. Get the RainMaker SDK (open "ESP-IDF PowerShell" from the Start menu)
cd ~
git clone --recursive https://github.com/espressif/esp-rainmaker.git

# 2. Create your project from the stock "switch" example
cd esp-rainmaker\examples
Copy-Item -Recurse switch smart_switch
cd smart_switch
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
Remove-Item sdkconfig -ErrorAction SilentlyContinue

# 3. Copy this repository's two firmware files over the example's main\ folder
#    (firmware\main\app_main.c and firmware\main\relay_config.h)

# 4. Choose your variant (2, 4 or 8) in main\relay_config.h, then build and flash
idf.py set-target esp32
idf.py build
idf.py -p COM6 erase-flash
idf.py -p COM6 flash monitor
```

Then open the ESP RainMaker Home app, tap **Add Device**, scan the QR code that
appears in the terminal, and enter your 2.4 GHz WiFi password.

## Documentation

| # | Document | What it covers |
|---|---|---|
| 1 | [How it works](docs/01-how-it-works.md) | Architecture, provisioning flow, security |
| 2 | [Parts list](docs/02-parts-list.md) | What to buy for each variant, tools |
| 3 | [Wiring](docs/03-wiring.md) | Pin tables, low-voltage and mains wiring |
| 4 | [Build and flash](docs/04-build-and-flash.md) | Every command, with expected output |
| 5 | [App setup](docs/05-app-setup-and-provisioning.md) | Pairing, rooms, schedules, sharing, reset |
| 6 | [Firmware explained](docs/06-firmware-explained.md) | How the code works, how to change it |
| 7 | [Safety and installation](docs/07-safety-and-installation.md) | Mains safety, installing in a home |
| 8 | [Troubleshooting](docs/08-troubleshooting.md) | Errors we actually hit, and fixes |
| 9 | [Limitations and roadmap](docs/09-limitations-and-roadmap.md) | Limits, project ideas, next steps |
| 10 | [Learning guide](docs/10-learning-guide.md) | Concepts for self-study (MQTT, TLS, relays...) |

## Project status

| Item | Status |
|---|---|
| Stock RainMaker switch example: flash, QR provisioning, local and remote control | Verified |
| Multi-relay firmware (this repo) compiles with ESP-IDF | Verified |
| Multi-relay firmware on real hardware (relay clicks, saved state) | **Being tested, update this table after your own test** |
| Mains-voltage installation | Not part of the tested scope, see safety guide |
| Status LEDs, manual push-buttons per relay | Planned, not implemented |

## Known limitations

- Uses Espressif's **public** RainMaker cloud: free, but meant for prototypes
  and evaluation, with a per-account node quota and no uptime guarantee.
  See [docs/09-limitations-and-roadmap.md](docs/09-limitations-and-roadmap.md).
- A quick tap on the BOOT button can make the app show a wrong state for the
  first relay (inherited from the stock driver). Holding it for a WiFi or
  factory reset works correctly.
- Relays are plain ON/OFF. Fan speed and light dimming need different hardware.

## License and credits

Code and documentation in this repository: MIT (see [LICENSE](LICENSE)).
Built on [ESP-IDF](https://github.com/espressif/esp-idf) and
[ESP RainMaker](https://github.com/espressif/esp-rainmaker) by Espressif
Systems (Apache-2.0). Their code is not redistributed here.
