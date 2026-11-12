# M5Stack Garden Monitor

A self-watering garden monitor for community and urban farming, built on the M5Stack Fire (ESP32). It reads soil moisture, air conditions and water tank level, then opens a servo-driven valve only when the plants need it.

## Problem

Singapore imports around 90% of its food, and community garden plots and balcony farms are one way residents grow some of their own. Many of these plots fail for a simple reason: uneven watering. New gardeners over-water in the first week, then forget in the second. Water gets wasted, plants die, and people give up.

## Proposed solution

A low-cost node that takes the guesswork out of watering:

1. Soil moisture sensors measure how dry each plot is.
2. An environment sensor reads air temperature, humidity and pressure, so the system can skip a cycle when the air is already humid.
3. A distance sensor above the water tank measures the water level and warns before the tank runs empty.
4. A servo releases a soft tube on a gravity-fed tank for a short, fixed time when the soil stays dry across several readings, then pinches it shut and waits through a cooldown before it can open again.
5. The Fire's screen shows live readings, and status lights show plot health at a glance.

## Hardware

| Part | Role |
|---|---|
| M5Stack Fire (ESP32) | Controller, screen, buttons, WiFi |
| Earth unit | Soil moisture |
| ENV II unit | Air temperature, humidity, pressure |
| ToF unit | Water tank level |
| Analog servo | Opens and closes the water valve |
| RGB units | Status lights |
| RFID unit | Identify a plot by tag (stretch goal) |
| Light unit | Sunlight logging (stretch goal) |
| HUB / Pb.HUB units | Share ports between units |
| Lego blocks | Planter frame, tank mount and valve arm |

An optional pump version (DC motor or small pump, 2N3904 transistor, 1 kΩ resistor, 1N4007 diode and a separate battery pack) is described in `docs/wiring.md`. It is not used in the current build.

## Wiring plan

| Port | Connected to |
|---|---|
| Port A (red, I2C) | HUB unit with ENV II and ToF |
| Port B (black) | Earth unit, analog output on GPIO36 |
| Port C (blue) | Servo, signal on GPIO17 |

The servo sweep test works on Port C. If the Fire ever resets when the servo moves, give the servo its own 5 V supply and join its ground to the Fire's GND. Pin numbers and wire colours come from M5Stack's Fire documentation, and the details are in `docs/wiring.md`.

## Setup (Windows)

1. Install Arduino IDE 2.x from arduino.cc.
2. Install the CP210x USB driver from Silicon Labs. The Fire shows up as "CP2104 USB to UART Bridge" in Device Manager.
3. In Arduino IDE open Preferences and add this URL under Additional boards manager URLs: `https://static-cdn.m5stack.com/resource/arduino/package_m5stack_index.json`
4. Install the M5Stack board package from Boards Manager, and the M5Unified library from Library Manager.
5. Select the M5Fire board and the Fire's COM port, then press Upload (the right arrow, not the Debug button).

Details and fixes for the problems I hit are in `docs/setup-windows.md`.

## Progress

- [x] Arduino IDE and M5Stack board package installed
- [x] USB driver installed, Fire detected on a COM port
- [x] `hello_fire` test sketch uploaded (screen and buttons)
- [x] I2C scan on Port A finds the ENV II and ToF units
- [x] ENV II readings in Serial Monitor (temperature, humidity, pressure)
- [x] Earth unit calibrated (dry and wet values)
- [x] Servo sweep test works on Port C
- [ ] Watering logic with confirm reads and cooldown
- [ ] ToF water level warning
- [ ] Lego frame, tank and valve arm built
- [ ] WiFi logging to a dashboard

## Repository layout

```
firmware/
  hello_fire/          first test sketch
  i2c_scan/            lists I2C devices on Port A
  env_ii_test/         ENV II temperature, humidity and pressure
  earth_calibration/   Earth unit raw readings
  servo_test/          servo sweep on Port C
docs/
  setup-windows.md     setup notes and fixes
  wiring.md            wiring notes
  build-log.md         dated notes on each stage
  images/              diagrams
```

## Notes

- AC motors are out of scope. This project uses low-voltage DC only.
- WiFi credentials, when added, go in a `secrets.h` file that is listed in `.gitignore`.

Built by Jad.
