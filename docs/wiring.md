# Wiring notes

How the garden monitor is connected. Anything marked TODO is not built or verified yet. Only mark a row as verified after testing it on the real hardware.

## Overview

TODO: add a photo of the whole setup, saved as `docs/images/overview.jpg`.

![Overview of the setup](images/overview.jpg)

## Fire port map

| Port | Colour | Pins | Connected to | Status |
|---|---|---|---|---|
| Port A | Red | G21 (SDA), G22 (SCL) | HUB unit with ENV II and ToF | TODO |
| Port B | Black | G26, G36 | Earth unit, analog output on G36 | TODO |
| Port C | Blue | G16, G17 | Cut Grove cable to breadboard, pump switch signal on G17 | TODO |

Pin numbers follow the standard Fire pinout. Confirm them against M5Stack's Fire documentation, and confirm the Port C signal wire colour with a blink test (LED and resistor between the wire and GND, sketch toggling G17).

## I2C devices

| Unit | Typical address | Verified with scan |
|---|---|---|
| ENV II (SHT30 and BMP280) | 0x44 and 0x76 | Yes, both found (2026-11-12) |
| ToF | 0x29 | Yes, found (2026-11-12) |

The scan (`firmware/i2c_scan`) also lists 0x10, 0x68 and 0x75. These sit on the Fire's own internal bus, not on the external units.

## Sensor behaviour

| Unit | Output | Reading | Calibration |
|---|---|---|---|
| Earth | Analog voltage on G36 | Raw 0 to 4095 | Dry in air: 4095. In water: about 2020 (hovers 2010 to 2030). Damp soil: TODO |
| ENV II | I2C | Temperature, humidity, pressure | None |
| ToF | I2C | Distance in mm to water surface | Empty tank: TODO. Full tank: TODO |

## Data flow

<img width="1650" height="780" alt="data-flow" src="https://github.com/user-attachments/assets/ff8d21d9-1171-45c5-8c9c-a5306338c45d" />


## Pump switch circuit

Parts: 2N3904, 1 kΩ resistor, 1N4007 diode, DC motor or small pump, separate battery pack.

| From | To |
|---|---|
| Fire G17 (Port C signal) | 1 kΩ resistor, then 2N3904 base |
| 2N3904 emitter | Common GND rail |
| 2N3904 collector | Motor wire 1 |
| Motor wire 2 | Battery pack positive |
| 1N4007 | Across the motor, stripe toward positive |
| Fire GND (Port C black wire) | Common GND rail |
| Battery pack negative | Common GND rail |

Notes:

- The motor never connects to a Fire pin directly.
- The motor supply is separate from the Fire's USB power.
- Motor voltage and current: TODO (check the label).
- 2N3904 limit is about 200 mA. Temperature check after 10 seconds of running: TODO.

TODO: photo of the breadboard, saved as `docs/images/breadboard.jpg`.

![Breadboard pump switch](images/breadboard.jpg)

## Test log

| Date | What I tested | Result |
|---|---|---|
| 2026-11-12 | hello_fire sketch: screen and buttons | TODO |
| 2026-11-13 | env_ii_test: ENV II through the HUB on Port A | Works: 23.7 C, 57.6 %, 1007.5 hPa |

## Safety

- Low-voltage DC only. No AC motor in this project.
- Connect the motor supply last, after checking every other wire.
- Keep water and electronics apart. The Fire, breadboard and battery pack sit above and away from the plot and tank.

## Lessons

TODO: add what went wrong and how it was fixed.
