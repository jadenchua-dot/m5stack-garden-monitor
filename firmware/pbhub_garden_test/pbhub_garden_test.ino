// Pb.HUB test: Earth unit and Angle unit on one hub, servo on Port C.
// The dial sets how far the servo valve opens. The Earth reading prints
// beside it. Output goes to Serial Monitor at 115200 baud.
//
// Wiring:
//   Pb.HUB     -> Port A (I2C, SDA G21, SCL G22), address 0x61.
//                 Plug it into the HUB unit that holds the ENV II and ToF.
//   Angle unit -> a Pb.HUB channel, set ANGLE_CH below.
//   Earth unit -> a different Pb.HUB channel, set EARTH_CH below.
//   Servo      -> Port C (signal on G17).
//
// Run pbhub_test first to see which channel each unit is on.
// The I2C is raw Wire (no M5Unified), the same as env_ii_test.

#include <Wire.h>

const uint8_t PBHUB_ADDR = 0x61;
const uint8_t ANALOG_CMD[6] = {0x46, 0x56, 0x66, 0x76, 0x86, 0xA6};

const int ANGLE_CH = 0;   // channel the Angle unit is plugged into (0 to 5)
const int EARTH_CH = 1;   // channel the Earth unit is plugged into (0 to 5)

const int SERVO_PIN  = 17;
const int CLOSED_DEG = 20;    // servo angle with the valve shut
const int OPEN_DEG   = 100;   // servo angle with the valve fully open

// Dial reading at each end of its turn. Check on Serial Monitor and adjust.
// The Pb.HUB ADC may be 10-bit (max 1023), so start with a guess of 800.
const int DIAL_MIN = 0;
const int DIAL_MAX = 800;

int readAnalog(uint8_t ch) {
  Wire.beginTransmission(PBHUB_ADDR);
  Wire.write(ANALOG_CMD[ch]);
  if (Wire.endTransmission() != 0) return -1;
  if (Wire.requestFrom(PBHUB_ADDR, (uint8_t)2) != 2) return -1;
  uint8_t lo = Wire.read();
  uint8_t hi = Wire.read();
  return (hi << 8) | lo;
}

void setServoAngle(int deg) {
  int us = map(deg, 0, 180, 600, 2300);
  uint32_t duty = (uint32_t)us * 65536UL / 20000UL;
  ledcWrite(SERVO_PIN, duty);
}

int lastDeg = -100;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  ledcAttach(SERVO_PIN, 50, 16);
  setServoAngle(CLOSED_DEG);
  delay(500);
  Wire.beginTransmission(PBHUB_ADDR);
  Serial.println(Wire.endTransmission() == 0 ? "Pb.HUB found at 0x61"
                                             : "Pb.HUB NOT found at 0x61");
}

void loop() {
  int dial  = readAnalog(ANGLE_CH);
  int earth = readAnalog(EARTH_CH);

  if (dial >= 0) {
    int deg = map(constrain(dial, DIAL_MIN, DIAL_MAX), DIAL_MIN, DIAL_MAX,
                  CLOSED_DEG, OPEN_DEG);
    if (abs(deg - lastDeg) >= 2) {   // ignore tiny changes to stop jitter
      setServoAngle(deg);
      lastDeg = deg;
    }
  }

  Serial.printf("dial=%d  servo=%d deg  earth=%d\n", dial, lastDeg, earth);
  delay(200);
}
