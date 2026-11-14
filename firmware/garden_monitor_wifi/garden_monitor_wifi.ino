// Garden monitor with WiFi dashboard, for the M5Stack Fire.
//
//   Earth unit -> Port B (G36)   soil moisture
//   HUB on Port A (G21, G22)     ENV II (0x44) and ToF (0x29)
//   Servo      -> Port C (G17)   water valve
//
// Same watering rules as garden_monitor. On top, the Fire joins your WiFi and
// runs a tiny web server. Open the Fire's IP address (shown on its screen) in
// a browser on the same network to see a live dashboard of the readings.
//
// The screen uses M5GFX only, and the I2C units use raw Wire, so the two do
// not clash. Needs the "Adafruit VL53L0X" library.
//
// IMPORTANT: put your own WiFi name and password below before uploading.
// Do not commit your real password to a public repo. Use your home WiFi or a
// phone hotspot; school and office networks often block device-to-device.

#include <Wire.h>
#include <M5GFX.h>
#include <Adafruit_VL53L0X.h>
#include <WiFi.h>
#include <WebServer.h>

// ---- WiFi: fill these in ----
const char *WIFI_SSID = "YOUR_WIFI_NAME";
const char *WIFI_PASS = "YOUR_WIFI_PASSWORD";

// Pins
const int EARTH_PIN = 36;
const int SERVO_PIN = 17;
const int BTN_A_PIN = 39;

// Servo valve
const int CLOSED_DEG = 25;   // tube shut (resting position, not watering)
const int OPEN_DEG   = 130;  // tube open (watering)
const bool REVERSE_DIRECTION = true;   // higher angle turns the arm clockwise

// Watering rules (short times for bench testing)
const int DRY_THRESHOLD = 3300;
const int CONFIRM_READS = 5;
const int HUMID_SKIP_PERCENT = 90;
const unsigned long READ_EVERY_MS = 1000;
const unsigned long OPEN_MS       = 5000;
const unsigned long COOLDOWN_MS   = 60000;

// Tank level from the ToF distance (9 cm bottle, ToF about 2 cm above the rim)
const int TANK_FULL_MM  = 40;
const int TANK_EMPTY_MM = 100;
const int LOW_PERCENT   = 20;

M5GFX display;
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
WebServer server(80);
bool tofOk = false;
String ipText = "WiFi: ...";

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

// ---- the web page the Fire serves ----
const char PAGE[] = R"HTML(
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Garden Monitor</title>
<style>
  :root { color-scheme: dark; }
  body { margin:0; font-family: system-ui, sans-serif; background:#10140f; color:#e8efe3; }
  header { padding:22px 18px; border-bottom:1px solid #2a3326; }
  h1 { margin:0; font-size:20px; }
  header p { margin:4px 0 0; color:#8aa17d; font-size:13px; }
  main { display:grid; gap:12px; padding:18px; max-width:520px; margin:0 auto;
         grid-template-columns:1fr 1fr; }
  .card { background:#19200f3b; background:#1a2115; border:1px solid #2a3326;
          border-radius:12px; padding:16px; }
  .card.wide { grid-column:1 / -1; }
  .label { color:#8aa17d; font-size:12px; text-transform:uppercase; letter-spacing:.04em; }
  .value { font-size:30px; font-weight:650; margin-top:6px; }
  .unit { font-size:15px; color:#8aa17d; font-weight:400; }
  .state { font-size:22px; font-weight:650; margin-top:6px; }
  .bar { height:10px; border-radius:6px; background:#2a3326; margin-top:10px; overflow:hidden; }
  .bar span { display:block; height:100%; background:#5fa85f; }
  .low .value, .low .bar span { color:#ff6b6b; background:#ff6b6b; }
  .note { color:#f2c14e; min-height:18px; margin-top:8px; font-size:14px; }
  footer { text-align:center; color:#5f6f55; font-size:12px; padding:14px; }
</style>
</head>
<body>
<header>
  <h1>Garden Monitor</h1>
  <p id="conn">Connecting to the Fire...</p>
</header>
<main>
  <div class="card">
    <div class="label">Soil</div>
    <div class="value" id="soil">--</div>
    <div class="note" id="soilnote"></div>
  </div>
  <div class="card">
    <div class="label">Air</div>
    <div class="value" id="temp">--<span class="unit"> C</span></div>
    <div class="note" id="hum"></div>
  </div>
  <div class="card wide" id="watercard">
    <div class="label">Water tank</div>
    <div class="value" id="level">--<span class="unit"> %</span></div>
    <div class="bar"><span id="bar" style="width:0%"></span></div>
  </div>
  <div class="card wide">
    <div class="label">Valve</div>
    <div class="state" id="state">--</div>
    <div class="note" id="note"></div>
  </div>
</main>
<footer>Updates every 2 seconds &middot; served by the M5Stack Fire</footer>
<script>
async function tick() {
  try {
    const r = await fetch('/data');
    const d = await r.json();
    document.getElementById('conn').textContent = 'Live';
    document.getElementById('soil').textContent = d.soil;
    document.getElementById('soilnote').textContent =
      d.soil > d.threshold ? 'Dry (limit ' + d.threshold + ')' : 'Wet enough';
    document.getElementById('temp').innerHTML =
      (d.temp < -100 ? '--' : d.temp) + '<span class="unit"> C</span>';
    document.getElementById('hum').textContent =
      d.hum < 0 ? 'Humidity: --' : 'Humidity: ' + d.hum + ' %';
    document.getElementById('level').innerHTML =
      (d.level < 0 ? '--' : d.level) + '<span class="unit"> %</span>';
    const lvl = d.level < 0 ? 0 : d.level;
    document.getElementById('bar').style.width = lvl + '%';
    document.getElementById('watercard').className =
      'card wide' + (d.level >= 0 && d.level < 20 ? ' low' : '');
    document.getElementById('state').textContent = d.state;
    document.getElementById('note').textContent = d.note;
  } catch (e) {
    document.getElementById('conn').textContent = 'Lost connection, retrying...';
  }
}
tick();
setInterval(tick, 2000);
</script>
</body>
</html>
)HTML";

String stateName() {
  if (state == WATERING) return "Watering";
  if (state == COOLDOWN) return "Cooldown";
  return "Waiting";
}

void handleRoot() { server.send(200, "text/html", PAGE); }

void handleData() {
  String j = "{";
  j += "\"soil\":" + String(soil);
  j += ",\"temp\":" + String(hum >= 0 ? tempC : -999, 1);
  j += ",\"hum\":" + String((int)hum);
  j += ",\"level\":" + String(levelPct);
  j += ",\"threshold\":" + String(DRY_THRESHOLD);
  j += ",\"state\":\"" + stateName() + "\"";
  j += ",\"note\":\"" + String(note) + "\"";
  j += "}";
  server.send(200, "application/json", j);
}

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
  display.printf("%s\n", note);
  display.setTextColor(TFT_CYAN, bg);
  display.print(ipText);
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

  // Short settle time before WiFi init — gives the power rail time to stabilise.
  delay(1000);

  // Join WiFi, with a timeout so the garden still runs if WiFi fails.
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("WiFi connecting");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(300);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    ipText = "http://" + WiFi.localIP().toString();
    Serial.println("\n" + ipText);
    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.begin();
  } else {
    ipText = "WiFi: not connected";
    Serial.println("\nWiFi failed, running without the dashboard");
  }
}

void loop() {
  server.handleClient();

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
