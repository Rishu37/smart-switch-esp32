# Firmware

This folder holds the two files that turn the stock ESP RainMaker **switch**
example into a 2 / 4 / 8-relay switch:

| File | Purpose |
|---|---|
| `main/app_main.c` | Replaces the example's `app_main.c` |
| `main/relay_config.h` | New file: variant, pins, names, relay polarity |

The rest of the project (WiFi provisioning, cloud, OTA, button handling) comes
from the example in Espressif's `esp-rainmaker` repository and is not copied
here. Follow [../docs/04-build-and-flash.md](../docs/04-build-and-flash.md)
to create the project and copy these files into it.

How the code works: [../docs/06-firmware-explained.md](../docs/06-firmware-explained.md).

**Status:** compiles with ESP-IDF (Windows, official installer, ESP32 target).
Hardware test results are tracked in the main README.
