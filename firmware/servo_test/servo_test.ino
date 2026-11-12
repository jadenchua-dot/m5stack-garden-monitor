// Servo test for the M5Stack Fire, Port C (signal on G17).
// Sweeps the servo between two angles every 1.5 seconds.
// Written for the M5Stack board package 3.x (ledcAttach and ledcWrite take the pin).

const int SERVO_PIN = 17;

void setAngle(int deg) {
  int us = map(deg, 0, 180, 600, 2300);
  uint32_t duty = (uint32_t)us * 65536UL / 20000UL;
  ledcWrite(SERVO_PIN, duty);
}

void setup() {
  ledcAttach(SERVO_PIN, 50, 16);
  setAngle(20);
}

void loop() {
  setAngle(20);  delay(1500);
  setAngle(100); delay(1500);
}
