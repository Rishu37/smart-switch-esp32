# Learning Guide: How This Smart Switch Actually Works

This document explains every concept behind the Automated Switch project —
written for self-study, so you understand *why* each piece exists, not just
*that* it works. Read it top to bottom, or jump to any section.

---

## 1. The Hardware Side

### What is an ESP32?
A small microcontroller chip with built-in WiFi and Bluetooth. Unlike a
regular microcontroller (like a basic Arduino), it can connect to networks
on its own — no separate WiFi module needed. It runs your firmware (the
program you write/flash onto it) and controls everything: reading buttons,
driving LEDs, and switching the relay.

### What is a relay, and why do you need one?
The ESP32's GPIO pins output a weak 3.3V signal — nowhere near enough to
power a lamp, fan, or appliance running on mains voltage. A relay is
essentially an electrically-controlled switch: a small signal from the
ESP32 triggers an internal electromagnet, which physically closes a much
larger switch capable of handling higher voltage/current (your 12V load, or
mains AC in a real deployment). This is also a safety boundary — it
electrically isolates your low-voltage control circuit from the
higher-power load circuit.

### GPIO pins
"General Purpose Input/Output" — the ESP32's pins that can be individually
programmed to either read a signal (input, e.g. a button press) or send one
(output, e.g. triggering the relay or lighting an LED). Your firmware maps
specific logical actions ("turn relay on") to specific physical pin numbers.

---

## 2. WiFi Concepts

### STA mode vs AP mode
- **STA (Station) mode** — the ESP32 acts like a normal WiFi client, joining
  your home router just like a phone or laptop does.
- **AP (Access Point) mode**, also called SoftAP — the ESP32 creates its
  *own* WiFi network that other devices can join, the same way your home
  router creates one. This is how the device lets your phone talk to it
  directly before it has any home WiFi credentials.

### Why hybrid provisioning uses both
On first boot (or after a reset), the device has no home WiFi to join yet —
so it starts in AP mode, so your phone can connect directly to *it* and hand
over your home WiFi's name/password. Once it has that, it switches to STA
mode and joins your real network like any other device. This two-step dance
is called **provisioning**.

### BLE (Bluetooth Low Energy)
An alternative to SoftAP for the same handoff — instead of the phone joining
a temporary WiFi network, it exchanges the setup data over Bluetooth
instead. Some provisioning flows offer both; the underlying goal (get WiFi
credentials from phone to device) is identical either way.

---

## 3. Talking to the Cloud

### What is MQTT, and why not just use normal web requests (HTTP)?
MQTT is a lightweight messaging protocol designed for devices with weak,
unreliable, or low-power connections — exactly the situation for most IoT
hardware. Instead of the device constantly asking "did anything change?"
(which HTTP polling would require), MQTT works on a **publish/subscribe**
model:

- The device **subscribes** to a "topic" (like a mailbox address) and just
  waits — the connection stays open, and the broker pushes messages to it
  the instant something is published, with no polling needed.
- To send a command, the cloud **publishes** a message to that same topic,
  and the device receives it immediately.

This is why your switch reacts instantly to a toggle in the app, rather than
checking in every few seconds.

### What is a "broker"?
The MQTT broker is the server in the middle that receives published messages
and routes them to whoever is subscribed to that topic. In this project,
that's **AWS IoT Core** — a broker Espressif runs and manages for you as
part of ESP RainMaker.

### TLS and certificates — why does the device need its own certificate?
TLS is the same encryption technology behind HTTPS — it makes sure no one
can eavesdrop on or tamper with the data in transit. On top of that, this
project uses **per-device certificates**: each ESP32 has its own unique
private key and matching public certificate, registered with the cloud.
This means:
- Only a device holding the correct private key can connect and claim to be
  that specific device — you can't impersonate another device.
- The cloud can restrict what each device is allowed to publish/subscribe
  to, based on its identity — device A cannot see or control device B's
  topics.

---

## 4. The Cloud Backend, Piece by Piece

ESP RainMaker's backend runs on AWS, using several purpose-built services
working together. Think of each one as doing exactly one job well:

| Service | Job | Analogy |
|---|---|---|
| **AWS IoT Core** | MQTT broker — receives/routes device messages | The post office sorting and delivering mail |
| **API Gateway** | Front door for app requests (REST API) | The reception desk that takes your request |
| **Lambda** | Runs the actual logic when triggered (e.g. "process a toggle command") | The staff member who processes your request |
| **DynamoDB** | Stores structured data — device info, current state, user-device links | A filing cabinet of records |
| **S3** | Stores files — specifically firmware update files here | A warehouse for large files |
| **Cognito** | Manages user accounts, login, and issues secure tokens | The ID-checker at the entrance |

### Walking through a single button tap
1. You tap "ON" in the app.
2. The app sends an HTTPS request to **API Gateway**, with your login token
   attached (issued earlier by **Cognito**).
3. API Gateway triggers a **Lambda** function to handle "set device state."
4. Lambda looks up the device's info in **DynamoDB**, confirms you have
   permission to control it, then publishes an MQTT message to that
   device's specific topic on **AWS IoT Core**.
5. Your ESP32, which has been sitting connected and subscribed to that
   topic, receives the message instantly and flips the relay's GPIO pin.
6. The ESP32 publishes its new state back to another topic, which flows
   back through IoT Core, gets stored in DynamoDB, and the app updates its
   display to match.

This is also why toggling from mobile data (no shared WiFi with the device)
works — the phone and device both just talk to the same cloud services
independently; neither needs to reach the other directly.

### What is a "serverless" backend, and why does it matter here?
"Serverless" doesn't mean there are no servers — it means Espressif doesn't
have to keep a server running 24/7 waiting for requests. Lambda functions
only run (and only cost money) when actually triggered by an event, and
scale automatically if many devices are active at once. This is part of why
a hobbyist project like yours can use RainMaker for free — it costs
Espressif very little to run your one device's occasional messages through
this architecture.

---

## 5. Provisioning, Step by Step

1. Device boots with no saved WiFi credentials → starts its own SoftAP
   hotspot + prints a QR code (which encodes the connection info).
2. You open the RainMaker Home app and scan that QR code.
3. The app uses the QR data to connect directly to the device's temporary
   hotspot.
4. Over that direct connection, the app sends your **home** WiFi's SSID and
   password to the device (this local exchange is also encrypted).
5. The device saves those credentials to flash memory, closes its hotspot,
   and switches to STA mode using the new credentials.
6. Once connected to your home WiFi, the device connects out to AWS IoT
   Core and starts normal cloud communication.
7. The QR code has now served its purpose — it doesn't grant ongoing
   access, it only kicked off this one-time handoff.

---

## 6. OTA (Over-The-Air) Updates

Instead of physically reconnecting the ESP32 to a PC to update its
firmware, OTA lets new firmware be pushed remotely:

1. A new firmware file is uploaded to **S3**.
2. The cloud generates a temporary, secure "signed URL" pointing to that
   file — this URL only works for a limited time and can't be freely shared
   or reused indefinitely.
3. The device is notified an update is available (or checks periodically).
4. It downloads the firmware from the signed URL, verifies it, and installs
   it, typically rebooting once done.

This is the same basic idea used by phone/app updates — code lives in cloud
storage, and devices pull it down through a secure, temporary link rather
than a permanently open download path.

---

## 7. Key Terms Glossary

- **Firmware** — the program running directly on the ESP32's hardware
  (different from an "app," which runs on your phone).
- **Flashing** — the process of uploading firmware onto the chip.
- **Provisioning** — the one-time setup process of handing WiFi credentials
  (and other config) to a new device.
- **Topic** — an MQTT "channel" or address that messages are published to
  and subscribed from.
- **Token** — a temporary, signed piece of data proving you're logged in,
  used instead of sending your password on every request.
- **Shadow / device state** — the cloud's stored copy of a device's current
  status, kept in sync with the real device so the app can show up-to-date
  info even if the device is briefly offline.

---

## 8. Why This Matters for the "Fully Self-Hosted" Version Later

Everything described here — the MQTT broker, the certificate-based auth,
the REST API layer, the token-based login — are exactly the pieces you'd
need to build yourself for a fully self-owned version (e.g. running
Mosquitto on your own server instead of AWS IoT Core, writing your own
Lambda-equivalent logic, managing your own certificates). Understanding how
RainMaker does it under the hood is effectively a working blueprint for
that future self-hosted build.
