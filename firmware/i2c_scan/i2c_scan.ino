// I2C scanner for the M5Stack Fire, Port A (SDA G21, SCL G22).
// Prints the address of every device it finds to Serial Monitor at 115200 baud.

#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
}

void loop() {
  Serial.println("Scanning...");
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("Found device at 0x%02X\n", addr);
    }
  }
  delay(3000);
}
