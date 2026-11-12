// Earth unit calibration for the M5Stack Fire, Port B (analog output on G36).
// Averages 10 readings and shows the raw value on the screen and in Serial Monitor.
// Note the value in air (dry) and in water (wet), then use them as RAW_DRY and RAW_WET.

#include <M5Unified.h>

const int SOIL_PIN = 36;

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  Serial.begin(115200);
  M5.Display.setTextSize(3);
}

void loop() {
  long sum = 0;
  for (int i = 0; i < 10; i++) { sum += analogRead(SOIL_PIN); delay(10); }
  int raw = sum / 10;
  M5.Display.fillScreen(BLACK);
  M5.Display.setCursor(10, 10);
  M5.Display.printf("Raw: %d", raw);
  Serial.printf("raw=%d\n", raw);
  delay(500);
}
