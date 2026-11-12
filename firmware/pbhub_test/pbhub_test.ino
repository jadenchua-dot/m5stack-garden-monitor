// Pb.HUB test for the M5Stack Fire. The Pb.HUB is an I2C device (default
// address 0x61) on Port A (SDA G21, SCL G22), so it can sit on the same HUB
// unit as the ENV II and ToF. Each of its 6 channels reads one analog sensor.
// Prints the analog value of all 6 channels to Serial Monitor at 115200 baud.
// Plug the Earth unit into one channel and the Angle unit into another, then
// watch which numbers change when you touch the sensors or turn the dial.
//
// Analog command per channel (from M5Stack docs): 0x46, 0x56, 0x66, 0x76,
// 0x86, 0xA6. Each read returns 2 bytes, low byte first (assumed from the
// M5Stack library, to be confirmed on the real unit).

#include <Wire.h>

const uint8_t PBHUB_ADDR = 0x61;
const uint8_t ANALOG_CMD[6] = {0x46, 0x56, 0x66, 0x76, 0x86, 0xA6};

int readAnalog(uint8_t ch) {
  Wire.beginTransmission(PBHUB_ADDR);
  Wire.write(ANALOG_CMD[ch]);
  if (Wire.endTransmission() != 0) return -1;
  if (Wire.requestFrom(PBHUB_ADDR, (uint8_t)2) != 2) return -1;
  uint8_t lo = Wire.read();
  uint8_t hi = Wire.read();
  return (hi << 8) | lo;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  delay(500);
  Wire.beginTransmission(PBHUB_ADDR);
  bool found = (Wire.endTransmission() == 0);
  Serial.println(found ? "Pb.HUB found at 0x61" : "Pb.HUB NOT found at 0x61");
}

void loop() {
  for (int ch = 0; ch < 6; ch++) {
    Serial.printf("ch%d=%d  ", ch, readAnalog(ch));
  }
  Serial.println();
  delay(500);
}
