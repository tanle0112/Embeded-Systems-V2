// ===== Task 4: ESP32-S3 AP Web Control =====
// Goal: Create a local Web Server running in Access Point (AP) mode.
// The ESP32 hosts its own Wi-Fi network so users can connect directly
// and control two NeoPixel RGB LED strips and one relay via a simple web interface.

#include <Arduino.h>
#include <WiFi.h>              // Wi-Fi functions for ESP32
#include <WebServer.h>         // Lightweight synchronous web server
#include <Adafruit_NeoPixel.h> // Library to drive NeoPixel RGB LEDs

// ==== Hardware pin configuration (YOLO-UNO board) ====
#define NEO_PIN_1 3            // D3 → first RGB LED strip
#define NEO_PIN_2 5            // D5 → second RGB LED strip
#define RELAY_PIN 7            // D7 → relay control pin
#define NEO_COUNT 8            // number of LEDs per strip

// ==== Wi-Fi Access Point credentials ====
#define SSID_AP  "ESP32S3_AP"  // Wi-Fi SSID that the ESP32 will broadcast
#define PASS_AP  "12345678"    // Wi-Fi password for connecting to the AP

// ==== LED and relay control objects ====
Adafruit_NeoPixel neo1(NEO_COUNT, NEO_PIN_1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel neo2(NEO_COUNT, NEO_PIN_2, NEO_GRB + NEO_KHZ800);

// ======= LED & RELAY HANDLERS =======

// Fill the entire LED strip with one RGB color
static void fillStrip(Adafruit_NeoPixel &s, uint8_t r, uint8_t g, uint8_t b) {
  for (uint16_t i = 0; i < s.numPixels(); i++)
    s.setPixelColor(i, s.Color(r,g,b));
  s.show(); // send data to LEDs
}

// Initialize relay and LED strips
static void ledsBegin() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);   // ensure relay starts off
  neo1.begin(); neo1.clear(); neo1.show();
  neo2.begin(); neo2.clear(); neo2.show();
}

// Simple helper functions to set colors for LED1, LED2, and toggle relay
static void LED1(uint8_t r,uint8_t g,uint8_t b){ fillStrip(neo1,r,g,b); }
static void LED2(uint8_t r,uint8_t g,uint8_t b){ fillStrip(neo2,r,g,b); }
static void Relay(bool on){ digitalWrite(RELAY_PIN, on ? HIGH : LOW); }

// ==== Create the web server listening on port 80 ====
WebServer server(80);

// ==== Embedded HTML UI served to clients ====
// Includes control sections for LED1, LED2, and Relay with multiple buttons
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

<!-- Section: LED1 control -->
<section><h3>LED1</h3>
<div class=row>
 <button onclick="fetch('/led1?mode=on')">ON (White)</button>
 <button onclick="fetch('/led1?mode=off')">OFF</button>
 <button onclick="fetch('/led1?r=255&g=0&b=0')">Red</button>
 <button onclick="fetch('/led1?r=0&g=255&b=0')">Green</button>
 <button onclick="fetch('/led1?r=0&g=0&b=255')">Blue</button>
</div></section>

<!-- Section: LED2 control -->
<section><h3>LED2</h3>
<div class=row>
 <button onclick="fetch('/led2?mode=on')">ON (White)</button>
 <button onclick="fetch('/led2?mode=off')">OFF</button>
 <button onclick="fetch('/led2?r=255&g=255&b=0')">Yellow</button>
 <button onclick="fetch('/led2?r=255&g=0&b=255')">Magenta</button>
 <button onclick="fetch('/led2?r=0&g=255&b=255')">Cyan</button>
</div></section>

<!-- Section: Relay control -->
<section><h3>Relay</h3>
<div class=row>
 <button onclick="fetch('/relay?state=on')">Relay ON</button>
 <button onclick="fetch('/relay?state=off')">Relay OFF</button>
</div></section>
</body></html>
)HTML";

// ==== HTTP request handlers ====

// Serve the HTML UI when visiting the root URL
static void httpRoot(){ server.send_P(200,"text/html",HTML); }

// Handle requests to /led1 → turn LED1 on/off or change its color
static void httpLED1(){
  if (server.hasArg("mode")){        // check if "mode" (on/off) argument exists
    String m = server.arg("mode");
    if (m=="on") LED1(255,255,255);  // white ON
    else LED1(0,0,0);                // turn OFF
  } else {                           // if no "mode", expect RGB values
    LED1(server.arg("r").toInt(), server.arg("g").toInt(), server.arg("b").toInt());
  }
  server.send(200,"text/plain","ok"); // send simple OK response
}

// Handle /led2 → same logic for the second LED strip
static void httpLED2(){
  if (server.hasArg("mode")){
    String m = server.arg("mode");
    if (m=="on") LED2(255,255,255);
    else LED2(0,0,0);
  } else {
    LED2(server.arg("r").toInt(), server.arg("g").toInt(), server.arg("b").toInt());
  }
  server.send(200,"text/plain","ok");
}

// Handle /relay → switch relay ON or OFF
static void httpRelay(){
  Relay(server.arg("state")=="on");
  server.send(200,"text/plain","ok");
}

// ==== Setup runs once on boot ====
void setup() {
  Serial.begin(115200);
  delay(500);
#if defined(ARDUINO_USB_CDC_ON_BOOT)
  // Wait for Serial to become ready so logs appear properly
  uint32_t t0 = millis();
  while(!Serial && millis()-t0 < 2000) { delay(10); }
#endif

  ledsBegin();  // initialize hardware

  // Configure ESP32 as Access Point (creates its own Wi-Fi network)
  WiFi.mode(WIFI_AP);
  bool ok = WiFi.softAP(SSID_AP, PASS_AP);  // start AP with SSID and password
  delay(100);                               // wait a bit for DHCP stack
  IPAddress ip = WiFi.softAPIP();           // get local AP IP (usually 192.168.4.1)

  // Print connection info to Serial monitor
  Serial.println();
  Serial.println("==== AP STARTED ====");
  Serial.print("SSID : "); Serial.println(SSID_AP);
  Serial.print("PASS : "); Serial.println(PASS_AP);
  Serial.print("AP IP: "); Serial.println(ip);
  Serial.println("Open http://192.168.4.1 in your browser after joining Wi-Fi.");

  // Register routes (URL endpoints) with the web server
  server.on("/",   httpRoot);   // root page
  server.on("/led1",  httpLED1);
  server.on("/led2",  httpLED2);
  server.on("/relay", httpRelay);
  server.begin(); // start listening for HTTP requests
}

// ==== Main loop runs continuously ====
void loop() {
  server.handleClient();  // process any incoming HTTP requests
  delay(10);              // small delay to yield CPU to other tasks
}
