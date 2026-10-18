# 7. Safety and installation

**This is an educational prototype. It is not a certified electrical product.**
Mains voltage (220-240 V AC) can kill and can start fires. Build and test the
low-voltage part first. If you are not experienced with mains wiring, have a
licensed electrician build or at least inspect the mains side, and follow your
local electrical code (for example, BIS standards in India).

## 7.1 Rules

1. **Switch off at the breaker** before touching any mains wire. Never work on a live circuit.
2. Keep every mains connection inside a **closed, fire-retardant enclosure**. Nothing live may be touchable.
3. **Fuse the Live input.** Do not exceed the rating of the cord, wire or relay.
4. **Never switch Neutral or Earth with the relay.** Relays switch Live only.
5. Use **properly rated wire and screw terminals**. Strip cleanly, use ferrules, tighten fully, and tug-test every wire.
6. Provide **strain relief** where the cord enters the box.
7. Keep **mains and low-voltage wiring physically separated** inside the box, with clear air gaps.
8. Use an **isolated, certified AC-DC module** for the 5 V supply. Do not use a transformerless (capacitor-dropper) supply.
9. Respect the **relay rating** (typically 10 A / 250 V AC for resistive loads). Motors and other inductive loads draw far more on start-up.
10. **Do not use heavy loads** (geysers, ACs, heaters) with these small relays. Use a properly rated contactor.
11. Test with a **lamp** first, on an **RCD/GFCI-protected** socket.
12. If anything smells, buzzes, gets hot or sparks: unplug it immediately.

## 7.2 Recommended build order

```
1. Low-voltage bench build on USB, no mains   → every relay clicks from the app
2. Add the fuse, MOV, AC-DC module (power OFF) → check wiring twice, with a multimeter
3. Power up with a lamp as the only load       → test each output alone, then together
4. Burn-in for 24-48 hours                      → check for heat, buzzing, resets
5. Have it inspected                            → before it goes into a home
```

## 7.3 Version 1 for a home: a plug-in box

For a first installation, build it like a smart power strip: a mains cord in,
2/4/8 sockets out, and the appliances plug into the sockets. Nobody has to open
a switchboard or rewire the house. Fixed in-wall installation should be done by
a licensed electrician.

## 7.4 Installing it in a home

```
Pick a dry, ventilated spot within WiFi range, away from water and heat
   ↓
Plug the box into a socket (ideally RCD/ELCB protected)
   ↓
Check total load against the fuse, cord and relay ratings
   ↓
Plug in the appliances
   ↓
Provision on the home WiFi (docs/05) and test remotely
   ↓
Leave a short instruction sheet: how to reset WiFi (hold BOOT about 3 s)
```

## 7.5 Behaviour to be aware of

- **Power cut:** with `RESTORE_LAST_STATE 1`, relays return to their last state when power comes back, so an appliance may switch on by itself. Choose per appliance whether that is acceptable, or set `RESTORE_LAST_STATE 0`.
- **No internet:** the cloud commands stop working; schedules already stored on the unit are designed to keep running. Test this before you rely on it.
- **No manual control yet:** if the app, WiFi or cloud is down, there is no physical switch per relay. Keep a manual way to power the appliance.

## 7.6 Maintenance

Check each installed unit after 1 week, 1 month and 3 months for heat marks,
loose terminals, dust and WiFi drops. Keep a log: build date, variant, MAC
address, location, firmware version.

## 7.7 Disclaimer

You are responsible for what you build and where you install it. The authors
give no warranty (see LICENSE) and accept no liability for damage or injury.
