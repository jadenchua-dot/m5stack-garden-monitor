// Water level warning for the M5Stack Fire.
// The ToF unit sits above the tank, pointing down at the water (Port A).
// The farther the water surface, the lower the level.
//
// Set TANK_FULL_MM (distance with a full tank) and TANK_EMPTY_MM (distance
// with an empty tank) from your own measurements. The screen turns red and
// shows LOW WATER when the level falls below LOW_PERCENT.
//
// The screen uses M5GFX only, not M5Unified, so it does not touch the I2C
// bus that the ToF unit uses. Needs the "Adafruit VL53L0X" library.

#include <Wire.h>
#include <M5GFX.h>
#include <Adafruit_VL53L0X.h>

const int TANK_FULL_MM  = 50;    // distance to the water when the tank is full
const int TANK_EMPTY_MM = 200;   // distance to the water when the tank is empty
const int LOW_PERCENT   = 20;    // warn below this level

M5GFX display;
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
int lastMm = -1;

// Takes 5 readings and returns the middle one, to ignore a stray value.
// Returns -1 if the sensor reports out of range.
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

void setup() {
  Serial.begin(115200);
  display.init();
  display.setTextSize(3);
  Wire.begin(21, 22);
  delay(1000);
  if (!lox.begin(0x29, false, &Wire)) {
    Serial.println("ToF not found at 0x29");
    display.fillScreen(TFT_BLACK);
    display.setCursor(10, 10);
    display.print("ToF not found");
    while (true) delay(1000);
  }
  Serial.println("ToF ready");
}

void loop() {
  int mm = readDistanceMm();
  if (mm >= 0) lastMm = mm;

  int pct = 0;
  if (lastMm >= 0) {
    pct = map(constrain(lastMm, TANK_FULL_MM, TANK_EMPTY_MM),
              TANK_EMPTY_MM, TANK_FULL_MM, 0, 100);
  }
  bool low = (lastMm >= 0) && (pct < LOW_PERCENT);

  display.fillScreen(low ? TFT_RED : TFT_BLACK);
  display.setTextColor(TFT_WHITE, low ? TFT_RED : TFT_BLACK);
  display.setCursor(10, 10);
  display.printf("Distance: %d mm\n", lastMm);
  display.printf("Level: %d %%\n", pct);
  display.print(low ? "LOW WATER" : "Water OK");

  Serial.printf("distance=%d mm level=%d%% %s\n", lastMm, pct, low ? "LOW" : "ok");
  delay(500);
}
