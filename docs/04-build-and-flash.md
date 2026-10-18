# 4. Build and flash the firmware

All commands below are for **Windows PowerShell opened from the "ESP-IDF PowerShell" Start-menu shortcut**
(a normal PowerShell does not have the ESP-IDF tools loaded). Linux/macOS equivalents are in section 4.8.

## 4.1 One-time setup

1. Install **ESP-IDF** with Espressif's official Windows installer
   (<https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/>).
2. Install **Git** (<https://git-scm.com/downloads>).
3. Install the **ESP RainMaker Home** app on your phone and create an account.
4. If Windows does not see the board, install the USB driver for its serial
   chip (CP210x or CH340, look at the chip near the USB port).

## 4.2 Get the RainMaker SDK

```powershell
cd ~
git clone --recursive https://github.com/espressif/esp-rainmaker.git
```

This takes a few minutes. Do not interrupt it. It ends by returning to the prompt.

> In PowerShell, `~` means your home folder. (`%userprofile%` is a classic
> Command Prompt syntax and does **not** work in PowerShell.)

## 4.3 Create your project from the stock example

```powershell
cd ~\esp-rainmaker\examples
Copy-Item -Recurse switch smart_switch
cd smart_switch
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
Remove-Item sdkconfig -ErrorAction SilentlyContinue
```

Keep `smart_switch` **inside the `examples` folder**: the project uses shared
files from `examples\common` by relative path. Cleaning `build` and `sdkconfig`
removes leftovers from earlier builds that can cause confusing errors.

## 4.4 Copy this repository's firmware in

Copy these two files from the repository into `smart_switch\main\`:

| File in this repo | Destination |
|---|---|
| `firmware/main/app_main.c` | `~\esp-rainmaker\examples\smart_switch\main\app_main.c` (overwrite) |
| `firmware/main/relay_config.h` | `~\esp-rainmaker\examples\smart_switch\main\relay_config.h` (new file) |

Do not change `app_driver.c`, `app_priv.h` or any `CMakeLists.txt`.

Check:
```powershell
dir main
```
You should see `app_main.c`, `app_driver.c`, `app_priv.h`, `CMakeLists.txt`, `relay_config.h`.

## 4.5 Choose your variant

```powershell
notepad main\relay_config.h
```

Edit this line, then save:

```c
#define RELAY_VARIANT 4          /* 2, 4 or 8 */
```

| You want | Set |
|---|---|
| 2 relays | `2` |
| 4 relays (default) | `4` |
| 8 relays | `8` |

Also check `RELAY_ON_LEVEL`: `1` if your relay turns on when the input goes HIGH,
`0` if it turns on when the input goes LOW (active-low boards).

## 4.6 Build

```powershell
idf.py set-target esp32
idf.py build
```

The first build takes several minutes. Success looks like this:

```
Project build complete. To flash, run:
 idf.py flash
```

If it fails, scroll to the **first** red error line. Later errors are usually
consequences of the first one. See [08-troubleshooting.md](08-troubleshooting.md).

## 4.7 Flash and run

1. In the app, delete any older device that used this ESP32.
2. Plug in the ESP32. In **Device Manager → Ports (COM & LPT)** note the COM
   number (for example `COM6`).
3. Erase, then flash and open the monitor:

```powershell
idf.py -p COM6 erase-flash
idf.py -p COM6 flash monitor
```

If a command hangs on `Connecting......`, **hold the BOOT button** on the board
until it starts, then release it.

What you should see in the log, after the boot messages:

```
Variant: 4 relays, restored mask = 0x00
...
Provisioning QR : {"ver":"v1", ...}
```

followed by a QR code drawn in the terminal. Now go to
[05-app-setup-and-provisioning.md](05-app-setup-and-provisioning.md).

Exit the monitor with **Ctrl + ]**.

Useful commands:

| Goal | Command |
|---|---|
| Wipe everything on the chip | `idf.py -p COM6 erase-flash` |
| Compile only | `idf.py build` |
| Flash and show logs | `idf.py -p COM6 flash monitor` |
| Show logs only (no flashing) | `idf.py -p COM6 monitor` |
| Exit the monitor | `Ctrl + ]` |

## 4.8 Linux / macOS

Open a terminal where ESP-IDF's `export.sh` has been sourced, then:

```bash
cd ~
git clone --recursive https://github.com/espressif/esp-rainmaker.git
cd esp-rainmaker/examples
cp -r switch smart_switch
cd smart_switch
rm -rf build sdkconfig
cp /path/to/this/repo/firmware/main/app_main.c main/
cp /path/to/this/repo/firmware/main/relay_config.h main/
idf.py set-target esp32
idf.py build
idf.py -p /dev/ttyUSB0 erase-flash
idf.py -p /dev/ttyUSB0 flash monitor
```

The port is usually `/dev/ttyUSB0` (Linux) or `/dev/cu.usbserial-*` (macOS).

## 4.9 Changing the variant later

1. Edit `RELAY_VARIANT` in `relay_config.h`.
2. `idf.py build`
3. Delete the old device in the app, then `idf.py -p COM6 erase-flash`, `idf.py -p COM6 flash monitor`.
4. Provision again. The device list changes with the variant, so a fresh start is the reliable way.
