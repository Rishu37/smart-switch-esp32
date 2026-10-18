# 6. Firmware explained

The firmware is the stock ESP RainMaker **switch** example with two files
changed. Everything else (WiFi provisioning, cloud connection, OTA, schedules,
the BOOT-button reset) comes from the example and the RainMaker SDK.

| File | Role |
|---|---|
| `firmware/main/relay_config.h` | **The only file you normally edit**: variant, pins, names, polarity |
| `firmware/main/app_main.c` | Creates one RainMaker Switch device per relay, drives the pins, saves state |
| `app_driver.c` (stock, not in this repo) | BOOT-button handling and WiFi/factory reset |

## 6.1 relay_config.h

| Setting | Meaning |
|---|---|
| `RELAY_VARIANT` | `2`, `4` or `8`: selects the pin and name lists |
| `RELAY_ON_LEVEL` | `1` = relay on when pin is HIGH, `0` = active-low board |
| `RESTORE_LAST_STATE` | `1` = restore relay states after a power cut, `0` = always start OFF |
| `RELAY_GPIO_LIST` | The GPIO pin for each relay, in order |
| `RELAY_NAME_LIST` | The default app name for each relay, in order |

## 6.2 Program flow (app_main.c)

```
app_main()
  ├─ console + app_driver_init()      BOOT button, reset handling (stock)
  ├─ nvs_flash_init()                 flash key-value storage
  ├─ relay_mask_load()                read saved relay states (if enabled)
  ├─ relays_init_gpio()               set each pin level, then make it an output
  ├─ app_network_init()               WiFi
  ├─ esp_rmaker_node_init()           create the node
  ├─ for each relay:                  create a Switch device (name + power parameter)
  │     └─ register write_cb with the relay index as private data
  ├─ enable OTA, timezone, schedules, scenes
  ├─ esp_rmaker_start()               start the RainMaker agent
  └─ app_network_start()              connect to WiFi, or start provisioning (QR code)
```

## 6.3 What happens on a command (write_cb)

```c
int idx = (int)(intptr_t)priv_data;      // which relay this device controls
...
if (val.val.b) relay_mask |=  (1u << idx);   // remember ON
else           relay_mask &= ~(1u << idx);   // remember OFF
relay_write_pin(idx, val.val.b);             // drive the GPIO
relay_mask_save();                           // store the mask in flash
esp_rmaker_param_update(param, val);         // tell the cloud/app the new state
```

- All relays share one callback. The relay index travels as the device's private data.
- The state of all relays is stored as **one byte** (bit *i* = relay *i* is on) under the key `mask` in the NVS namespace `relays`.
- Each toggle writes to flash. That is fine at human speed. Do not toggle in a fast loop.

## 6.4 Boot behaviour

1. NVS is initialised, then the saved mask is read.
2. For each relay, the pin level is set **before** the pin becomes an output, so it comes up in the intended state.
3. A factory reset (BOOT about 10 s) clears the saved mask, so relays start OFF afterwards.

## 6.5 How to change things

| I want to... | Do this |
|---|---|
| Rename the relays | Edit `RELAY_NAME_LIST` for your variant, rebuild, erase flash, provision again |
| Use other pins | Edit `RELAY_GPIO_LIST` (see the pin rules in [03-wiring.md](03-wiring.md)) |
| Add a 3-, 5- or 6-relay variant | Add a new `#elif RELAY_VARIANT == N` block in `relay_config.h` with its `NUM_RELAYS`, pin list and name list |
| Always start OFF after a power cut | `RESTORE_LAST_STATE 0` |
| Use an active-low relay board | `RELAY_ON_LEVEL 0` |

Keep `NUM_RELAYS` at 8 or fewer: the saved state is a single byte.

## 6.6 Known issue: BOOT quick-tap

The stock driver toggles an output when BOOT is tapped briefly, and reports it
through a global called `switch_device`. This project points `switch_device` at
relay 1, so a quick tap can make the app show a wrong state for relay 1 without
moving the relay. Holding BOOT for a WiFi reset (about 3 s) or factory reset
(about 10 s) is unaffected. A planned fix is to remove the quick-tap callback
from `app_driver.c`.

## 6.7 Not in this firmware (yet)

- Status LEDs (WiFi/cloud/error)
- Physical push-button per relay
- Extra encryption beyond what RainMaker provides
- Power-on delay or staggered relay start-up
