# M5Stack Garden Monitor

A self-watering garden monitor for community and urban farming, built on the M5Stack Fire (ESP32). It reads soil moisture, air conditions and water tank level, then runs a small pump only when the plants need it.

## Problem

Singapore imports around 90% of its food, and community garden plots and balcony farms are one way residents grow some of their own. Many of these plots fail for a simple reason: uneven watering. New gardeners over-water in the first week, then forget in the second. Water gets wasted, plants die, and people give up.

## Proposed solution

A low-cost node that takes the guesswork out of watering:

1. Soil moisture sensors measure how dry each plot is.
2. An environment sensor reads air temperature, humidity and pressure, so the system can skip a cycle when the air is already humid.
3. A distance sensor above the water tank measures the water level and warns before the pump runs dry.
4. A pump switches on for a short, fixed time when the soil stays dry across several readings, then waits through a cooldown before it can run again.
5. The Fire's screen shows live readings, and status lights show plot health at a glance.

## Hardware

| Part | Role |
|---|---|
| M5Stack Fire (ESP32) | Controller, screen, buttons, WiFi |
| Earth unit | Soil moisture |
| ENV II unit | Air temperature, humidity, pressure |
| ToF unit | Water tank level |
| RGB units | Status lights |
| RFID unit | Identify a plot by tag (stretch goal) |
| Light unit | Sunlight logging (stretch goal) |
| HUB / Pb.HUB units | Share ports between units |
| DC motor or small pump | Waters the plot |
| 2N3904 transistor, 1 kΩ resistor, 1N4007 diode | Switch for the motor |
| Separate battery pack | Motor power |
| Lego blocks | Planter frame and tank mount |

## Wiring plan

| Port | Connected to |
|---|---|
| Port A (red, I2C) | HUB unit with ENV II and ToF |
| Port B (black) | Earth unit, analog output on GPIO36 |
| Port C (blue) | Pump switch signal on GPIO17, through a cut Grove cable to the breadboard |

The motor never connects to a Fire pin directly. GPIO17 drives the base of the 2N3904 through the 1 kΩ resistor. The motor runs from its own supply, the diode sits across the motor terminals with the stripe toward the positive side, and the motor supply ground joins the Fire's GND.

Pin assignments follow the standard Fire pinout and still need checking against M5Stack's Fire documentation and a blink test before the pump is powered.

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
- [ ] ENV II readings on screen
- [ ] Earth unit calibrated (dry and wet values)
- [ ] Pump switch built and tested without the Fire attached
- [ ] Watering logic with confirm reads and cooldown
- [ ] ToF water level warning
- [ ] WiFi logging to a dashboard
- [ ] Lego frame built

## Repository layout

```
firmware/
  hello_fire/        first test sketch
docs/
  setup-windows.md   setup notes and fixes
  wiring.md          wiring notes
```

## Notes

- AC motors are out of scope. This project uses low-voltage DC only.
- WiFi credentials, when added, go in a `secrets.h` file that is listed in `.gitignore`.

Built by Jad.
