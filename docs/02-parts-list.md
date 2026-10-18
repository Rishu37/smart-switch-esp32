# 2. Parts list and tools

## Core parts (every variant)

| Part | Notes |
|---|---|
| ESP32 development board (ESP32-WROOM-32, "DevKit V1" style) | Use a WROOM-32 board. On WROVER boards GPIO 16/17 are used by PSRAM |
| Relay module, 5 V coil, opto-isolated, 10 A / 250 V AC contacts | 2, 4 or 8 channels (see below) |
| Jumper wires (female-female or female-male) | For the low-voltage bench build |
| USB cable (data capable) and a USB 5 V supply, at least 2 A | For flashing and bench testing |
| 2.4 GHz WiFi network | The ESP32 does not support 5 GHz |
| Phone with the **ESP RainMaker Home** app | Android or iOS |

## Choose the relay board

| Variant | Relay board | Typical 5 V draw (relays only, rough) |
|---|---|---|
| 2 relays | 2-channel module | about 0.2 A |
| 4 relays | 4-channel module | about 0.4 A |
| 8 relays | 8-channel module | about 0.7 A |

Add roughly 0.25 A for the ESP32 itself. Use a 5 V supply with comfortable
headroom, for example **2 A for the 4- and 8-relay builds**.

Look for boards with an **opto-isolator on every channel**. Many boards are
**active-low** (relay turns on when the input pin goes LOW). The firmware
supports both: see `RELAY_ON_LEVEL` in `relay_config.h`.

## Extra parts for a mains-connected unit

Only add these after the low-voltage build works. See
[07-safety-and-installation.md](07-safety-and-installation.md).

| Part | Notes |
|---|---|
| Certified isolated AC-DC 5 V module (Hi-Link style) | About 1 A for 2 relays, about 2 A for 4 or 8 relays. Prefer this over a home-made supply for anything left running |
| Fuse + holder on the Live input | Rated for the total load and not above the cord/relay rating |
| MOV (varistor) across Live and Neutral, after the fuse | 275-300 V AC class, for surge protection |
| Proper sockets, mains cord with earth, strain relief | Rated for your total load |
| Terminal blocks and ferrules | No loose or twisted wire ends in screw terminals |
| Fire-retardant, fully closed enclosure | No exposed mains parts |
| Mains-rated wire | Size it for the load (1.0-1.5 mm² is common for small appliances; check your local code) |

## Tools

- Screwdrivers, wire stripper, small pliers
- Multimeter (continuity and voltage)
- A lamp or small load for the first mains test, and an RCD/GFCI-protected socket
- A PC with ESP-IDF, Git and a text editor (VS Code works well)

## Loads that do NOT suit this build

Heavy or inductive loads such as geysers/water heaters, air conditioners,
heaters and large motors can exceed the relay's real rating or cause contact
welding. Use a properly rated contactor for those, driven by a relay.
