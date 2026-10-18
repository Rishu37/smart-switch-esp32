# 1. How it works

## The big picture

```
 ┌──────────────┐   HTTPS (REST)   ┌────────────────────┐   MQTT over TLS   ┌──────────┐
 │  Phone app   │ ───────────────► │  ESP RainMaker     │ ◄───────────────► │  ESP32   │
 │  (anywhere)  │ ◄─────────────── │  cloud (Espressif) │                   │ (at home)│
 └──────────────┘                  └────────────────────┘                   └────┬─────┘
                                                                                 │ GPIO
                                                                                 ▼
                                                                           ┌───────────┐
                                                                           │ Relay board│──► appliances
                                                                           └───────────┘
```

- The **ESP32** keeps one outgoing, encrypted connection to the cloud.
- The **phone app** sends commands to the cloud, and the cloud passes them to
  the ESP32 over that connection. State changes flow back the same way.
- Because both sides only talk to the cloud, control works from any network,
  including mobile data. The phone and the ESP32 never connect directly
  (except for optional local control when both are on the same WiFi).

## Terms used by RainMaker

| Term | Meaning in this project |
|---|---|
| **Node** | One physical ESP32 board. One node = one certificate = one WiFi connection |
| **Device** | One controllable thing inside a node. Here: one relay = one Switch device |
| **Parameter** | A value of a device. For a switch: `Power` (true/false) and `Name` |

So a 4-relay unit is **1 node containing 4 devices**. This matters for the
cloud quota, which counts nodes, not devices.

## First-time setup (provisioning)

```
Power on, no saved WiFi
   │
   ▼
ESP32 starts a setup link (SoftAP/BLE) and prints a QR code
   │
   ▼
App scans the QR code → phone connects to the ESP32 directly
   │
   ▼
You pick your home WiFi and type the password in the app
   │
   ▼
ESP32 saves the credentials, restarts, joins your WiFi
   │
   ▼
ESP32 connects to the cloud and appears in your app
```

After that, the ESP32 reconnects by itself after every power cut. Holding the
BOOT button for about 3 seconds erases only the WiFi credentials and brings
back the setup mode.

## What happens when you tap a switch

1. The app sends "turn Switch 2 ON" to the cloud (HTTPS, with your login token).
2. The cloud sends the command to your ESP32 (MQTT over TLS).
3. The firmware's write callback runs: it sets the GPIO pin, saves the new
   state to flash, and reports the new value back to the cloud.
4. The relay clicks. The app shows the confirmed state.

## Security model

- ESP32 ↔ cloud: MQTT over TLS, with a certificate unique to each device.
- Phone ↔ cloud: HTTPS with token-based login.
- The QR code is only used during first-time setup. Sharing it does not give
  anyone control. To let someone else control the unit, use the app's
  **Sharing** feature instead.
- The firmware in this repository does **not** add any extra encryption on
  top of what RainMaker provides.

## About the cloud

The public ESP RainMaker cloud is run by Espressif and is free to use for
prototypes and evaluation. Its limits are described in
[09-limitations-and-roadmap.md](09-limitations-and-roadmap.md). Implementation
details of the backend can change, so check Espressif's documentation at
<https://docs.rainmaker.espressif.com/> for the current picture.
