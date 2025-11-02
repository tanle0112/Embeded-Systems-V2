// ===== Task 4: ESP32-S3 AP Web Control (UI cải tiến, in IP chắc chắn) =====
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

// ==== PIN (YOLO-UNO): D3,D5 cho 2 strip; D7 cho relay ====
#define NEO_PIN_1 3
#define NEO_PIN_2 5
#define RELAY_PIN 7
#define NEO_COUNT 8

// ==== Wi-Fi AP ====
#define SSID_AP  "ESP32S3_AP"
#define PASS_AP  "12345678"

// ==== LED/Relay HAL ====
Adafruit_NeoPixel neo1(NEO_COUNT, NEO_PIN_1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel neo2(NEO_COUNT, NEO_PIN_2, NEO_GRB + NEO_KHZ800);

static void fillStrip(Adafruit_NeoPixel &s, uint8_t r, uint8_t g, uint8_t b) {
  for (uint16_t i = 0; i < s.numPixels(); i++) s.setPixelColor(i, s.Color(r,g,b));
  s.show();
}
static void ledsBegin() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  neo1.begin(); neo1.clear(); neo1.show();
  neo2.begin(); neo2.clear(); neo2.show();
}
static void LED1(uint8_t r,uint8_t g,uint8_t b){ fillStrip(neo1,r,g,b); }
static void LED2(uint8_t r,uint8_t g,uint8_t b){ fillStrip(neo2,r,g,b); }
static void Relay(bool on){ digitalWrite(RELAY_PIN, on ? HIGH : LOW); }

// ==== WEB ====
WebServer server(80);

// UI gọn, nhãn rõ (đáp ứng yêu cầu “ít nhất 2 nút & 2 thiết bị”)
static const char HTML[] PROGMEM = R"HTML(
<!doctype html><html><head><meta name=viewport content="width=device-width,initial-scale=1">
<title>ESP32-S3 Control (AP)</title>
<style>
 body{font-family:sans-serif;max-width:560px;margin:20px auto;padding:8px}
 h2{margin:0 0 12px}
 section{border:1px solid #ddd;border-radius:10px;padding:12px;margin:10px 0}
 button{padding:10px 14px;margin:6px 4px;border-radius:10px;border:1px solid #ccc}
 .row{display:flex;flex-wrap:wrap;gap:6px}
</style></head><body>
<h2>ESP32-S3 Control (AP)</h2>
<section><h3>LED1</h3>
<div class=row>
 <button onclick="fetch('/led1?mode=on')">ON (White)</button>
 <button onclick="fetch('/led1?mode=off')">OFF</button>
 <button onclick="fetch('/led1?r=255&g=0&b=0')">Red</button>
 <button onclick="fetch('/led1?r=0&g=255&b=0')">Green</button>
 <button onclick="fetch('/led1?r=0&g=0&b=255')">Blue</button>
</div></section>

<section><h3>LED2</h3>
<div class=row>
 <button onclick="fetch('/led2?mode=on')">ON (White)</button>
 <button onclick="fetch('/led2?mode=off')">OFF</button>
 <button onclick="fetch('/led2?r=255&g=255&b=0')">Yellow</button>
 <button onclick="fetch('/led2?r=255&g=0&b=255')">Magenta</button>
 <button onclick="fetch('/led2?r=0&g=255&b=255')">Cyan</button>
</div></section>

<section><h3>Relay</h3>
<div class=row>
 <button onclick="fetch('/relay?state=on')">Relay ON</button>
 <button onclick="fetch('/relay?state=off')">Relay OFF</button>
</div></section>
</body></html>
)HTML";

static void httpRoot(){ server.send_P(200,"text/html",HTML); }
static void httpLED1(){
  if (server.hasArg("mode")){
    String m = server.arg("mode");
    if (m=="on") LED1(255,255,255); else LED1(0,0,0);
  } else {
    LED1(server.arg("r").toInt(), server.arg("g").toInt(), server.arg("b").toInt());
  }
  server.send(200,"text/plain","ok");
}
static void httpLED2(){
  if (server.hasArg("mode")){
    String m = server.arg("mode");
    if (m=="on") LED2(255,255,255); else LED2(0,0,0);
  } else {
    LED2(server.arg("r").toInt(), server.arg("g").toInt(), server.arg("b").toInt());
  }
  server.send(200,"text/plain","ok");
}
static void httpRelay(){
  Relay(server.arg("state")=="on");
  server.send(200,"text/plain","ok");
}

void setup() {
  Serial.begin(115200);
  // Chờ Serial sẵn sàng để chắc chắn thấy log
  delay(500);
#if defined(ARDUINO_USB_CDC_ON_BOOT)
  uint32_t t0 = millis(); while(!Serial && millis()-t0 < 2000) { delay(10); }
#endif

  ledsBegin();

  WiFi.mode(WIFI_AP);
  bool ok = WiFi.softAP(SSID_AP, PASS_AP);
  delay(100); // cho DHCP stack lên
  IPAddress ip = WiFi.softAPIP();

  Serial.println();
  Serial.println("==== AP STARTED ====");
  Serial.print("SSID : "); Serial.println(SSID_AP);
  Serial.print("PASS : "); Serial.println(PASS_AP);
  Serial.print("AP IP: "); Serial.println(ip);       // Thường là 192.168.4.1
  Serial.println("Open http://192.168.4.1 in your browser after joining Wi-Fi.");

  server.on("/",   httpRoot);
  server.on("/led1",  httpLED1);
  server.on("/led2",  httpLED2);
  server.on("/relay", httpRelay);
  server.begin();
}

void loop() {
  server.handleClient();
  delay(10);
}
