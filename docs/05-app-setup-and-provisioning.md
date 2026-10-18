# 5. App setup and provisioning

## 5.1 Install and sign in

1. Install **ESP RainMaker Home** from the Play Store or App Store.
2. Open it and choose the standard ESP RainMaker backend on the first screen.
3. Sign up with email (or Google/Apple), verify, and log in.

Use the same account every time you provision a unit that you want to manage.

## 5.2 Add the device (provisioning)

```
Flash the firmware → the terminal shows a QR code
   ↓
App → Add Device → scan the QR code
   ↓
The phone connects to the ESP32's setup link (SoftAP or Bluetooth)
   ↓
Choose your 2.4 GHz WiFi network and enter its password
   ↓
Wait 10-30 seconds. Do not close the app.
   ↓
The switches appear on the home screen
```

If the QR scan fails, use the manual options on the Add Device screen
(Bluetooth or SoftAP) and pick the device from the list.

The ESP32 supports **2.4 GHz WiFi only**. If your router mixes bands under one
name, make sure the 2.4 GHz band is enabled.

## 5.3 What you will see

| Build | Switches in the app |
|---|---|
| 2 relays | Light, Fan |
| 4 relays | Switch 1, Switch 2, Fan, Light |
| 8 relays | Switch 1 to Switch 6, Fan, Light |

They all belong to one node. Rename them in the app, or change the default
names in `relay_config.h`.

## 5.4 Organise

- **Rooms/Homes:** group devices (for example a "Living room" containing the Light and Fan).
- **Rename:** open a device and edit its name.

## 5.5 Schedules

```
Schedules → create new → name it
   ↓
Tap + in Actions → choose devices → set ON or OFF for each
   ↓
Set time and repeat days → Save
```

If you select no repeat days, the schedule runs once. For schedules to fire at
your local time, the phone app sets the node's timezone; check the unit shows
the right time behaviour after setup.

## 5.6 Sharing with another person

Use the app's **Sharing** option: open the sharing section, choose **Add User**,
and enter the other person's RainMaker username (their sign-up email). They
accept from the notification centre in their app.

Do **not** share the setup QR code: it is a one-time provisioning code, not a key.

Account ownership matters for support: as far as the RainMaker docs indicate,
the primary user has full control and shared users have limited access. If you
build units for other people, decide who the primary user is before you provision.

## 5.7 Reset

| Action | How |
|---|---|
| Forget WiFi only (keeps everything else) | Hold the BOOT button about 3 s, then provision again |
| Factory reset | Hold BOOT about 10 s |
| Remove a device from your account | Open the device in the app and delete it |

After reflashing or erasing the ESP32, delete the old device in the app first,
then provision again.
