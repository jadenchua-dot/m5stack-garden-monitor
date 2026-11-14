// Garden monitor: all parts in one sketch, for the M5Stack Fire.
//
//   Earth unit -> Port B (G36)   soil moisture
//   HUB on Port A (G21, G22)     ENV II (0x44) and ToF (0x29)
//   Servo      -> Port C (G17)   water valve
//
// Rules: the soil must read dry several times in a row. The valve then opens
// for a fixed time, closes, and a cooldown blocks the next cycle. Watering is
// skipped when the tank is low or the air is very humid. Button A forces a
// test cycle (still blocked when the tank is low).
//
// The screen uses M5GFX only, and the I2C units use raw Wire, so the two do
// not clash. Needs the "Adafruit VL53L0X" library.

#include <Wire.h>
#include <M5GFX.h>
#include <Adafruit_VL53L0X.h>

// Pins
const int EARTH_PIN = 36;
const int SERVO_PIN = 17;
const int BTN_A_PIN = 39;

// Servo valve
const int CLOSED_DEG = 20;
const int OPEN_DEG   = 100;

// true flips the servo's turning direction (a higher angle then turns the
// arm clockwise instead of anticlockwise). All angles below stay the same.
const bool REVERSE_DIRECTION = true;

// Watering rules (short times for bench testing)
const int DRY_THRESHOLD = 3300;
const int CONFIRM_READS = 5;
const int HUMID_SKIP_PERCENT = 90;
const unsigned long READ_EVERY_MS = 1000;
const unsigned long OPEN_MS       = 5000;
const unsigned long COOLDOWN_MS   = 60000;

// Tank level from the ToF distance
const int TANK_FULL_MM  = 50;
const int TANK_EMPTY_MM = 200;
const int LOW_PERCENT   = 20;

M5GFX display;
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
bool tofOk = false;

enum State { WAITING, WATERING, COOLDOWN };
State state = WAITING;
int dryCount = 0;
unsigned long stateStart = 0;
unsigned long lastRead = 0;
int currentDeg = CLOSED_DEG;

int soil = 0;
float tempC = 0, hum = -1;
int distMm = -1;
int levelPct = -1;
const char *note = "";

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
    delay(12);
  }
}

int readEarth() {
  long sum = 0;
  for (int i = 0; i < 10; i++) sum += analogRead(EARTH_PIN);
  return sum / 10;
}

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

// Middle of 5 readings. Returns -1 if the ToF is out of range.
int readDistanceMm() {
  int v[5];
  int n = 0;
  for (int i = 0; i < 5; i++) {
    VL53L0X_RangingMeasurementData_t m;
    lox.rangingTest(&m, false);
    if (m.RangeStatus != 4) v[n++] = m.RangeMilliMeter;
    delay(30);
  }
  if (n < 3) return -1;
  for (int i = 0; i < n - 1; i++)
    for (int j = i + 1; j < n; j++)
      if (v[j] < v[i]) { int t = v[i]; v[i] = v[j]; v[j] = t; }
  return v[n / 2];
}

bool tankLow() { return levelPct >= 0 && levelPct < LOW_PERCENT; }

void startWatering() {
  moveServoTo(OPEN_DEG);
  state = WATERING;
  stateStart = millis();
  dryCount = 0;
  Serial.println("Valve OPEN");
}

void draw() {
  bool low = tankLow();
  uint16_t bg = low ? TFT_RED : TFT_BLACK;
  display.fillScreen(bg);
  display.setTextColor(TFT_WHITE, bg);
  display.setCursor(10, 10);
  display.printf("Soil: %d (limit %d)\n", soil, DRY_THRESHOLD);
  if (hum >= 0) display.printf("Air: %.1f C  %.0f %%\n", tempC, hum);
  else display.print("Air: no reading\n");
  if (levelPct >= 0) display.printf("Water: %d %%%s\n", levelPct, low ? "  LOW" : "");
  else display.print("Water: no reading\n");
  if (state == WAITING) display.printf("Waiting, dry %d/%d\n", dryCount, CONFIRM_READS);
  else if (state == WATERING)
    display.printf("WATERING %lu s\n", (OPEN_MS - (millis() - stateStart)) / 1000);
  else
    display.printf("Cooldown %lu s\n", (COOLDOWN_MS - (millis() - stateStart)) / 1000);
  display.print(note);
}

void setup() {
  Serial.begin(115200);
  pinMode(BTN_A_PIN, INPUT);
  display.init();
  display.setTextSize(2);
  Wire.begin(21, 22);
  ledcAttach(SERVO_PIN, 50, 16);
  setServoAngle(CLOSED_DEG);
  delay(500);
  tofOk = lox.begin(0x29, false, &Wire);
  Serial.println(tofOk ? "ToF ready" : "ToF not found, water level unknown");
}

void loop() {
  unsigned long now = millis();
  bool btnA = (digitalRead(BTN_A_PIN) == LOW);

  if (now - lastRead < READ_EVERY_MS && !btnA) return;
  lastRead = now;

  soil = readEarth();
  float t, h;
  if (readSHT30(t, h)) { tempC = t; hum = h; } else { hum = -1; }
  if (tofOk) {
    int mm = readDistanceMm();
    if (mm >= 0) distMm = mm;
    if (distMm >= 0) {
      levelPct = map(constrain(distMm, TANK_FULL_MM, TANK_EMPTY_MM),
                     TANK_EMPTY_MM, TANK_FULL_MM, 0, 100);
    }
  }

  note = "";
  if (state == WAITING) {
    if (soil > DRY_THRESHOLD) dryCount++; else dryCount = 0;
    bool wantWater = (dryCount >= CONFIRM_READS) || btnA;
    if (wantWater) {
      if (tankLow()) {
        note = "Tank low: skipped";
      } else if (!btnA && hum >= HUMID_SKIP_PERCENT) {
        note = "Humid air: skipped";
      } else {
        startWatering();
      }
    }
  } else if (state == WATERING && now - stateStart >= OPEN_MS) {
    moveServoTo(CLOSED_DEG);
    state = COOLDOWN;
    stateStart = millis();
    Serial.println("Valve CLOSED, cooldown");
  } else if (state == COOLDOWN && now - stateStart >= COOLDOWN_MS) {
    state = WAITING;
    dryCount = 0;
    Serial.println("Ready");
  }

  Serial.printf("soil=%d temp=%.1f hum=%.0f dist=%d level=%d state=%d dry=%d %s\n",
                soil, tempC, hum, distMm, levelPct, state, dryCount, note);
  draw();
}
