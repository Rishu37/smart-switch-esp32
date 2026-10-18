# 9. Limitations, roadmap and project ideas

## 9.1 Limitations of this project

### Hardware and firmware

- Relays are plain ON/OFF. **Fan speed control and light dimming** need different hardware (a triac dimmer or PWM driver).
- No status LEDs, no physical push-button per relay, no manual override yet.
- Relay states are saved to flash on every toggle (fine for humans, not for rapid automation).
- Not a certified electrical product. See [07](07-safety-and-installation.md).

### The public ESP RainMaker cloud (what this project uses)

| Limitation | What it means |
|---|---|
| **Evaluation service** | Espressif describes the public instance as meant for proof-of-concept and evaluation. Commercial products are expected to move to a private deployment |
| **Node quota** | An account can hold a limited number of nodes by default (5 per Espressif forum posts at the time of writing; raisable on request, but not without limit) |
| **Quota counting** | Forum answers indicate claims are counted per board and account combination, so re-claiming the same board from the same account does not use extra slots |
| **Limited fleet control** | The admin of a node is whoever provisions it. If other people provision their own units, you cannot see or update them remotely |
| **Voice assistants** | No custom Alexa skill or Google action on the public instance |
| **Custom iOS app** | Needs extra configuration to work reliably on the public instance |
| **Branding** | Users see the standard ESP RainMaker Home app, not your own |
| **Guarantees** | No uptime guarantee or SLA, and your data passes through Espressif's cloud |
| **Time limit** | None documented in the sources checked, but there is no written promise the free service stays the same. Check Espressif's current terms |

Limits can change: always check <https://docs.rainmaker.espressif.com/> before
you commit to a plan.

## 9.2 Giving units to other people

If you hand out units and each person provisions with their own account:

- You stay under the quota, because each account holds only its own node.
- You **lose remote visibility and OTA control** of those units.
- This suits a trial with friends. It is not a supported product.

If you provision units under **your** account and share them, you keep control,
but your account's node quota limits how many units you can manage.

## 9.3 Roadmap

| Step | Item |
|---|---|
| 1 | Test the multi-relay firmware on hardware and update the status table in the README |
| 2 | Remove the BOOT quick-tap behaviour from `app_driver.c` |
| 3 | Add status LEDs (connecting, connected, setup mode, error) |
| 4 | Add a physical push-button per relay (manual override), debounced |
| 5 | Add a startup policy per relay (always off, always on, last state) |
| 6 | Design a single PCB: ESP32-WROOM-32 module, isolated AC-DC supply, relay drivers, fuse, MOV, with mains and low-voltage sections separated |
| 7 | Enclosure design and a proper installation guide |
| 8 | Option A: move to a private RainMaker deployment. Option B: build a self-hosted stack (MQTT broker such as Mosquitto + your own backend and app) |

### Self-hosted version (long-term idea)

| Piece | RainMaker provides today | Self-hosted replacement |
|---|---|---|
| Message broker | Cloud MQTT broker | Mosquitto (your PC, VPS or Raspberry Pi) |
| Accounts and API | RainMaker cloud APIs | Your own backend (for example Node.js or Python) |
| Phone app | ESP RainMaker Home | Your own app (for example React Native) |
| Secure OTA | Cloud-managed OTA | Signed firmware images served from your own server |
| Remote access | Included | Public IP, a tunnel (for example Cloudflare Tunnel), or a VPS |

This is a large project of its own. The first step is understanding what
RainMaker does for you, which [10-learning-guide.md](10-learning-guide.md) explains.

## 9.4 What else can you build with this?

The same ESP32 + relay + RainMaker base can be reused for many projects. Rough
difficulty: **Easy** = this firmware with small changes, **Medium** = needs extra
parts and some coding, **Hard** = needs new hardware design.

| Project | Difficulty | Notes |
|---|---|---|
| Smart power strip (2/4/8 outlets) | Easy | This project as is |
| Fan + light controller for a room | Easy | Variant 2 or 4 |
| Water pump / tank filling timer | Easy | Use schedules; add a float sensor later |
| Garden irrigation with solenoid valves | Medium | Use 12/24 V valves on a separate supply, relays switch the valve supply |
| Aquarium lights and filter schedule | Easy | Schedules do the work |
| Workshop / shop lighting zones | Easy | One relay per zone |
| Temperature-based fan or heater switch | Medium | Add a DHT22 or DS18B20 sensor; start from the `multi_device` / `temperature_sensor` examples |
| Energy monitoring per outlet | Hard | Add a current/energy sensor module (needs careful mains-safe design) |
| Voice control (Alexa / Google) | Medium | Needs the platform's supported integration; limited on the public instance |
| IR remote blaster (TV, AC) | Medium | Add an IR LED and send codes from a device parameter |
| Motorised curtain or gate (open/stop/close) | Medium | Needs interlocked relays so both directions can never be on together |
| Door lock / strike control | Hard | Safety critical: never make a lock depend on WiFi alone |
| Greenhouse controller | Medium | Sensors + fan + pump + schedules |
| Scenes ("Good night" turns everything off) | Easy | Built into the RainMaker app |
| Self-hosted smart home hub | Hard | The long-term path above |
| Custom PCB product | Hard | Needs mains-safe layout and proper testing/certification before selling |

If you build something new, put it in a `projects/` folder in this repository
with its own README and wiring notes.
