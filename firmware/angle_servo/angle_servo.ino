// Angle unit controls the servo valve.
// Turn the dial up and the servo opens wider, so more water flows.
//
// Wiring for this test:
//   Angle unit -> Port B (analog output on G36). Unplug the Earth unit first,
//                 because it uses the same pin.
//   Servo      -> Port C (signal on G17).
//
// Set CLOSED_DEG and OPEN_DEG to match your Lego valve arm. Start with a
// small range and widen it, so the servo never strains against the frame.
// Written for the M5Stack board package 3.x and M5Unified.

#include <M5Unified.h>

const int ANGLE_PIN = 36;
const int SERVO_PIN = 17;

const int CLOSED_DEG = 20;    // servo angle with the valve shut
const int OPEN_DEG   = 100;   // servo angle with the valve fully open

// Raw reading at each end of the dial. Check these on the screen and adjust.
const int DIAL_MIN = 0;
const int DIAL_MAX = 3100;

void setServoAngle(int deg) {
  int us = map(deg, 0, 180, 600, 2300);
  uint32_t duty = (uint32_t)us * 65536UL / 20000UL;
  ledcWrite(SERVO_PIN, duty);
}

int readDial() {
  long sum = 0;
  for (int i = 0; i < 10; i++) sum += analogRead(ANGLE_PIN);
  return sum / 10;
}

int lastDeg = -100;

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  Serial.begin(115200);
  ledcAttach(SERVO_PIN, 50, 16);
  setServoAngle(CLOSED_DEG);
  M5.Display.setTextSize(3);
}

void loop() {
  int raw = readDial();
  int deg = map(constrain(raw, DIAL_MIN, DIAL_MAX), DIAL_MIN, DIAL_MAX,
                CLOSED_DEG, OPEN_DEG);

  // Move only when the dial changes by 2 degrees or more, to stop jitter.
  if (abs(deg - lastDeg) >= 2) {
    setServoAngle(deg);
    lastDeg = deg;
  }

  int openPct = (lastDeg - CLOSED_DEG) * 100 / (OPEN_DEG - CLOSED_DEG);
  M5.Display.setCursor(10, 20);
  M5.Display.printf("Dial: %4d  \nServo: %3d deg \nOpen: %3d %%  ", raw, lastDeg, openPct);
  Serial.printf("dial=%d servo=%d open=%d%%\n", raw, lastDeg, openPct);
  delay(50);
}
