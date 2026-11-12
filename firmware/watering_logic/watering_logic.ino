// Watering logic for the M5Stack Fire.
// Earth unit on Port B (G36), servo valve on Port C (G17).
//
// The soil must read dry several times in a row before the valve opens.
// The valve stays open for a fixed time, closes, then a cooldown blocks
// another cycle. Button A forces a test cycle.
//
// The times below are short, for bench testing. For a real plot use a
// longer cooldown (30 minutes or more) so water can soak in.
// Written for the M5Stack board package 3.x and M5Unified.

#include <M5Unified.h>

const int EARTH_PIN = 36;
const int SERVO_PIN = 17;

const int CLOSED_DEG = 20;     // servo angle with the valve shut
const int OPEN_DEG   = 100;    // servo angle with the valve open

const int DRY_THRESHOLD = 3300;          // raw reading above this means dry
const int CONFIRM_READS = 5;             // dry readings in a row before watering
const unsigned long READ_EVERY_MS = 1000;
const unsigned long OPEN_MS       = 5000;    // valve open time
const unsigned long COOLDOWN_MS   = 60000;   // wait after watering

enum State { WAITING, WATERING, COOLDOWN };
State state = WAITING;
int dryCount = 0;
unsigned long stateStart = 0;
unsigned long lastRead = 0;
int raw = 0;

void setServoAngle(int deg) {
  int us = map(deg, 0, 180, 600, 2300);
  uint32_t duty = (uint32_t)us * 65536UL / 20000UL;
  ledcWrite(SERVO_PIN, duty);
}

int readEarth() {
  long sum = 0;
  for (int i = 0; i < 10; i++) sum += analogRead(EARTH_PIN);
  return sum / 10;
}

void startWatering() {
  setServoAngle(OPEN_DEG);
  state = WATERING;
  stateStart = millis();
  dryCount = 0;
  Serial.println("Valve OPEN");
}

void draw() {
  M5.Display.fillScreen(BLACK);
  M5.Display.setTextSize(3);
  M5.Display.setCursor(10, 10);
  M5.Display.printf("Soil: %d\n", raw);
  M5.Display.printf("Limit: %d\n", DRY_THRESHOLD);
  if (state == WAITING) {
    M5.Display.printf("Waiting\nDry reads: %d/%d", dryCount, CONFIRM_READS);
  } else if (state == WATERING) {
    M5.Display.printf("WATERING\n%lu s left", (OPEN_MS - (millis() - stateStart)) / 1000);
  } else {
    M5.Display.printf("Cooldown\n%lu s left", (COOLDOWN_MS - (millis() - stateStart)) / 1000);
  }
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  Serial.begin(115200);
  ledcAttach(SERVO_PIN, 50, 16);
  setServoAngle(CLOSED_DEG);
}

void loop() {
  M5.update();
  unsigned long now = millis();

  if (M5.BtnA.wasPressed() && state == WAITING) startWatering();

  if (now - lastRead >= READ_EVERY_MS) {
    lastRead = now;
    raw = readEarth();

    if (state == WAITING) {
      if (raw > DRY_THRESHOLD) dryCount++;
      else dryCount = 0;
      if (dryCount >= CONFIRM_READS) startWatering();
    } else if (state == WATERING && now - stateStart >= OPEN_MS) {
      setServoAngle(CLOSED_DEG);
      state = COOLDOWN;
      stateStart = now;
      Serial.println("Valve CLOSED, cooldown");
    } else if (state == COOLDOWN && now - stateStart >= COOLDOWN_MS) {
      state = WAITING;
      dryCount = 0;
      Serial.println("Ready");
    }

    Serial.printf("soil=%d state=%d dry=%d\n", raw, state, dryCount);
    draw();
  }
}
