# 8. Troubleshooting

Find your symptom, apply the fix. Several of these are errors that came up
while building this project.

## Build and flash

| Symptom | Cause | Fix |
|---|---|---|
| `Cannot find path '...\%userprofile%'` | `%userprofile%` is Command Prompt syntax | In PowerShell use `cd ~` |
| `idf.py` is not recognised | Wrong terminal | Open **ESP-IDF PowerShell** from the Start menu |
| `undefined reference to 'switch_device'` at the link step | `app_main.c` is missing the global `switch_device` that the stock `app_driver.c` uses | Use the `app_main.c` from this repository (it defines it) |
| Strange build errors after copying the example | Old `build` folder or `sdkconfig` was copied too | `Remove-Item -Recurse -Force build` and `Remove-Item sdkconfig`, then `idf.py set-target esp32` and build again |
| Cannot find shared files (`common`) | Project folder was moved out of `examples` | Keep it in `esp-rainmaker\examples\`, or set `RMAKER_PATH` to the SDK folder |
| Build fails with many errors | Only the first error matters | Scroll to the **first** red line and fix that |
| `Connecting......` never finishes | Board is not in download mode | Hold the **BOOT** button until flashing starts |
| `could not open port COM6` | Wrong port or another program holds it | Check Device Manager for the real COM number, close other serial monitors |
| No COM port appears | Missing USB driver or charge-only cable | Install the CP210x/CH340 driver, use a data USB cable |

## Provisioning and app

| Symptom | Cause | Fix |
|---|---|---|
| QR code scan fails | Camera/permission or QR not fully visible | Enlarge the terminal, or use the manual Bluetooth/SoftAP option |
| WiFi list is empty or provisioning fails | 5 GHz-only network, wrong password | Use a 2.4 GHz network, retype the password |
| Device shows offline | WiFi dropped, power or router issue | Check power and WiFi, reopen the app |
| `User does not have available quota` | Too many nodes claimed on one account | Remove unused nodes, or use another account; see [09](09-limitations-and-roadmap.md) |
| New switches do not appear after changing the variant | Cloud still has the old device list | Delete the device in the app, `erase-flash`, flash, provision again |
| Two copies of the device in the app | Old device was not deleted before re-provisioning | Delete the stale one |

## Relays and behaviour

| Symptom | Cause | Fix |
|---|---|---|
| Relay is on when the app says off (and the reverse) | Active-low relay board | Set `RELAY_ON_LEVEL 0`, rebuild, flash |
| Relay clicks at power-up | Pin floats before the firmware starts | Normal for some boards; use opto-isolated boards, or add a pull-up/pull-down as the board's documentation suggests |
| App toggles, nothing clicks | Wiring, supply or pin mismatch | Check VCC/GND, common ground, and that `relay_config.h` pins match your wiring |
| Relays reset or the ESP32 reboots when relays switch | 5 V supply too weak | Use a supply with more current; check wiring and ground |
| ESP32 does not boot with the relay board connected | A relay input pulls a boot-strap pin | Use the recommended pins; avoid GPIO 0, 2, 12, 15 |
| Relay state shown in app is wrong after quick-tapping BOOT | Known quick-tap issue, see [06](06-firmware-explained.md) | Do not tap BOOT; hold 3 s or 10 s only |
| Relays do not return to their last state | `RESTORE_LAST_STATE` is 0, or a factory reset happened | Set it to 1; a factory reset clears the saved state |

## Useful diagnostics

```powershell
idf.py -p COM6 monitor          # live logs
```

Log lines worth looking for:

| Log line | Meaning |
|---|---|
| `Variant: N relays, restored mask = 0x..` | Firmware started, shows the variant and restored state |
| `Provisioning QR : ...` | Waiting for the app to provision |
| `MQTT Connected.` | Connected to the cloud |
| `Relay 2 (Switch 2) -> ON` | A command was received and applied |

If you ask for help, include: your variant, the first red error line or the
relevant log lines, your ESP-IDF version (`idf.py --version`) and your board.
