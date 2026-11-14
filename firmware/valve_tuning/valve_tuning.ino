// Valve angle tuner for the M5Stack Fire (servo on Port C, signal on G17).
// Button A: angle down by 5 degrees. Button C: angle up by 5 degrees.
// Find the angle where the tube is shut and the angle where it is open,
// then put them in CLOSED_DEG and OPEN_DEG in garden_monitor.
// Move in small steps and stop if the servo hums or strains.

#include <M5Unified.h>

const int SERVO_PIN = 17;

// true flips the servo's turning direction (a higher angle then turns the
// arm clockwise instead of anticlockwise). All angles below stay the same.
const bool REVERSE_DIRECTION = true;
int currentDeg = 20;

void setServoAngle(int deg) {
  if (REVERSE_DIRECTION) deg = 180 - deg;
  int us = map(deg, 0, 180, 600, 2300);
  uint32_t duty = (uint32_t)us * 65536UL / 20000UL;
  ledcWrite(SERVO_PIN, duty);
}

void moveServoTo(int target) {
  while (currentDeg != target) {
    currentDeg += (target > currentDeg) ? 1 : -1;
    setServoAngle(currentDeg);
    delay(25);
  }
}

void draw() {
  M5.Display.fillScreen(BLACK);
  M5.Display.setTextSize(4);
  M5.Display.setCursor(10, 20);
  M5.Display.printf("Angle: %d", currentDeg);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(10, 120);
  M5.Display.print("A: -5    C: +5");
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  Serial.begin(115200);
  ledcAttach(SERVO_PIN, 50, 16);
  setServoAngle(currentDeg);
  draw();
}

void loop() {
  M5.update();
  if (M5.BtnA.wasPressed()) {
    moveServoTo(constrain(currentDeg - 5, 0, 180));
    Serial.printf("angle=%d\n", currentDeg);
    draw();
  }
  if (M5.BtnC.wasPressed()) {
    moveServoTo(constrain(currentDeg + 5, 0, 180));
    Serial.printf("angle=%d\n", currentDeg);
    draw();
  }
}
