// ToF distance test for the M5Stack Fire, Port A (SDA G21, SCL G22).
// The ToF unit (VL53L0X, address 0x29) measures the distance to the water
// surface. Readings print to Serial Monitor at 115200 baud.
// Needs the "Adafruit VL53L0X" library (Library Manager).

#include <Wire.h>
#include <Adafruit_VL53L0X.h>

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  if (!lox.begin(0x29, false, &Wire)) {
    Serial.println("ToF not found at 0x29");
    while (true) delay(1000);
  }
  Serial.println("ToF ready");
}

void loop() {
  VL53L0X_RangingMeasurementData_t m;
  lox.rangingTest(&m, false);
  if (m.RangeStatus != 4) {
    Serial.printf("Distance: %d mm\n", m.RangeMilliMeter);
  } else {
    Serial.println("Out of range");
  }
  delay(500);
}
