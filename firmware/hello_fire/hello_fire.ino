#include <M5Unified.h>

int presses = 0;

void show(const char* name, uint16_t color) {
  presses++;
  M5.Display.fillScreen(color);
  M5.Display.setTextColor(WHITE, color);
  M5.Display.setCursor(10, 10);
  M5.Display.println(name);
  M5.Display.printf("Presses: %d\n", presses);
  Serial.printf("%s pressed, total %d\n", name, presses);
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  Serial.begin(115200);
  M5.Display.setTextSize(2);
  M5.Display.fillScreen(BLACK);
  M5.Display.setCursor(10, 10);
  M5.Display.println("Hello Jad!");
  M5.Display.println("Fire is working");
  M5.Display.println("Press A, B or C");
  Serial.println("Fire booted");
}

void loop() {
  M5.update();
  if (M5.BtnA.wasPressed()) show("Button A", RED);
  if (M5.BtnB.wasPressed()) show("Button B", BLUE);
  if (M5.BtnC.wasPressed()) show("Button C", DARKGREEN);
}
