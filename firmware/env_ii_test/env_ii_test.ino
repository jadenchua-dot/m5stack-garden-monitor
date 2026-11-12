// ENV II test for the M5Stack Fire, Port A (SDA G21, SCL G22).
// The SHT30 (0x44) is read directly over I2C. The BMP280 (0x76) uses the
// Adafruit BMP280 library. Readings print to Serial Monitor at 115200 baud.

#include <Wire.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp(&Wire);
bool bmpOk = false;

bool readSHT30(float &t, float &h) {
  Wire.beginTransmission(0x44);
  Wire.write(0x2C);
  Wire.write(0x06);
  if (Wire.endTransmission() != 0) return false;
  delay(20);
  if (Wire.requestFrom(0x44, 6) != 6) return false;
  uint8_t b[6];
  for (int i = 0; i < 6; i++) b[i] = Wire.read();
  uint16_t rawT = (b[0] << 8) | b[1];
  uint16_t rawH = (b[3] << 8) | b[4];
  t = -45.0 + 175.0 * rawT / 65535.0;
  h = 100.0 * rawH / 65535.0;
  return true;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  bmpOk = bmp.begin(0x76);
  Serial.println(bmpOk ? "BMP280 ready" : "BMP280 not found");
}

void loop() {
  float t, h;
  if (readSHT30(t, h)) {
    Serial.printf("Temp: %.1f C   Humidity: %.1f %%", t, h);
  } else {
    Serial.print("SHT30 read failed");
  }
  if (bmpOk) {
    Serial.printf("   Pressure: %.1f hPa", bmp.readPressure() / 100.0);
  }
  Serial.println();
  delay(2000);
}
