// =======================
// Task 4 - Single file
// ESP32-S3 AP WebServer: control BOTH RGB strips together + Relay
// =======================

#include <Arduino.h>              // Core Arduino functions
#include <WiFi.h>                 // Wi-Fi library for ESP32
#include <WebServer.h>            // Lightweight HTTP server
#include <Adafruit_NeoPixel.h>    // Library for RGB LED control

// ---------- PIN MAP (according to YOLO-UNO / previous setup) ----------
#define LED_PIN_1   3     // Data pin for RGB strip 1
#define LED_PIN_2   5     // Data pin for RGB strip 2
#define RELAY_PIN   7     // Relay control pin
#define NUM_PIXELS  1     // Number of LEDs per strip

// ---------- Wi-Fi Access Point configuration ----------
#define SSID_AP     "ESP32S3_RTOS"   // SSID name for ESP32 AP
#define PASS_AP     "12345678"       // Password for AP

// ---------- Global objects ----------
WebServer server(80);   // Create a web server on port 80
Adafruit_NeoPixel rgb1(NUM_PIXELS, LED_PIN_1, NEO_GRB + NEO_KHZ800);  // RGB strip 1
Adafruit_NeoPixel rgb2(NUM_PIXELS, LED_PIN_2, NEO_GRB + NEO_KHZ800);  // RGB strip 2

// =======================
// Helper functions
// =======================

// Set both RGB LEDs to the same color (r,g,b)
static void setBothRGB(uint8_t r, uint8_t g, uint8_t b) {
  rgb1.setPixelColor(0, rgb1.Color(r, g, b));   // Set color for first LED
  rgb2.setPixelColor(0, rgb2.Color(r, g, b));   // Set color for second LED
  rgb1.show();  // Apply color to strip 1
  rgb2.show();  // Apply color to strip 2
  Serial.printf("[RGB] Both LEDs -> R=%u G=%u B=%u\n", r, g, b);  // Log color values
}

// Turn the relay ON or OFF
static void setRelay(bool on) {
  digitalWrite(RELAY_PIN, on ? HIGH : LOW);  // Set pin HIGH or LOW based on boolean
  Serial.printf("[Relay] %s\n", on ? "ON" : "OFF");  // Log relay state
}

// =======================
// HTTP Request Handlers
// =======================

// HTML page served to clients connecting to ESP32 AP
static const char HTML[] PROGMEM = R"HTML(
<!doctype html><html><head><meta name=viewport content="width=device-width,initial-scale=1">
<title>ESP32-S3 Control</title>
<style>
body{font-family:system-ui,Arial,sans-serif;max-width:560px;margin:20px auto;padding:0 10px}
h2{margin:.2em 0}
.card{border:1px solid #ddd;border-radius:12px;padding:14px;margin:10px 0;box-shadow:0 2px 8px rgba(0,0,0,.06)}
.btn{padding:10px 14px;margin:6px 4px;border-radius:10px;border:1px solid #ccc;background:#f7f7f7;cursor:pointer}
.row{display:flex;gap:6px;flex-wrap:wrap}
label{display:block;margin:6px 0 4px}
input[type=range]{width:100%}
.small{opacity:.7;font-size:12px}
</style></head><body>
<h2>ESP32-S3 Control Panel</h2>
<div class=card>
  <div class=small id=net></div>
</div>

<div class=card>
  <h3>Both RGB (Linked)</h3>
  <!-- Preset color buttons -->
  <div class=row>
    <button class=btn onclick="preset(255,0,0)">Red</button>
    <button class=btn onclick="preset(0,255,0)">Green</button>
    <button class=btn onclick="preset(0,0,255)">Blue</button>
    <button class=btn onclick="preset(255,255,0)">Yellow</button>
    <button class=btn onclick="preset(255,0,255)">Magenta</button>
    <button class=btn onclick="preset(0,255,255)">Cyan</button>
    <button class=btn onclick="preset(0,0,0)">Off</button>
  </div>

  <!-- RGB sliders -->
  <label>R <input id=r type=range min=0 max=255 value=0 oninput="send()"></label>
  <label>G <input id=g type=range min=0 max=255 value=0 oninput="send()"></label>
  <label>B <input id=b type=range min=0 max=255 value=0 oninput="send()"></label>
</div>

<div class=card>
  <h3>Relay</h3>
  <!-- Relay ON/OFF buttons -->
  <button class=btn onclick="fetch('/relay?state=on')">ON</button>
  <button class=btn onclick="fetch('/relay?state=off')">OFF</button>
</div>

<script>
// Fetch JSON data from the server
async function fetchJSON(u){const r=await fetch(u); try{return await r.json()}catch(e){return null}}

// Display SSID and IP info
async function info(){
  const j = await fetchJSON('/info');
  if(j) document.getElementById('net').textContent =
    `AP SSID: ${j.ssid} | IP: ${j.ip}`;
}

// Set preset RGB color
function preset(R,G,B){ r.value=R; g.value=G; b.value=B; send(); }

let t=null;
// Send RGB values to server with slight delay (debounce)
function send(){
  clearTimeout(t);
  t=setTimeout(()=>{
    const u=`/rgb?r=${r.value}&g=${g.value}&b=${b.value}`;
    fetch(u);
  },120);
}

info();  // Initialize network info when page loads
</script>
</body></html>
)HTML";

// Serve the main control webpage
static void handleRoot() {
  server.send_P(200, "text/html", HTML);
}

// Send Wi-Fi AP info as JSON
static void handleInfo() {
  String ip = WiFi.softAPIP().toString();
  String json = "{\"ssid\":\"" + String(SSID_AP) + "\",\"ip\":\"" + ip + "\"}";
  server.send(200, "application/json", json);
}

// Handle RGB color change requests
static void handleRGB() {
  int r = server.hasArg("r") ? server.arg("r").toInt() : 0;
  int g = server.hasArg("g") ? server.arg("g").toInt() : 0;
  int b = server.hasArg("b") ? server.arg("b").toInt() : 0;

  // Clamp RGB values to 0–255
  r = constrain(r,0,255);
  g = constrain(g,0,255);
  b = constrain(b,0,255);

  // Apply new color to both LEDs
  setBothRGB((uint8_t)r,(uint8_t)g,(uint8_t)b);
  server.send(200, "text/plain", "ok");
}

// Handle relay ON/OFF requests
static void handleRelay() {
  bool on = (server.arg("state") == "on");
  setRelay(on);
  server.send(200, "text/plain", "ok");
}

// =======================
// Arduino lifecycle functions
// =======================
void setup() {
  Serial.begin(115200);
  delay(1500);

  // Initialize NeoPixel strips
  rgb1.begin();
  rgb2.begin();
  rgb1.setBrightness(60);
  rgb2.setBrightness(60);
  setBothRGB(0,0,0);  // Turn off initially

  // Initialize relay output pin
  pinMode(RELAY_PIN, OUTPUT);
  setRelay(false);  // Relay OFF by default

  // Configure ESP32 as Wi-Fi Access Point
  WiFi.mode(WIFI_AP);
  bool ok = WiFi.softAP(SSID_AP, PASS_AP);  // Start AP
  delay(200);
  IPAddress ip = WiFi.softAPIP();           // Get IP address of AP

  // Print AP details to Serial
  Serial.println();
  Serial.println(F("===== AP STARTED ====="));
  Serial.printf("SSID : %s\n", SSID_AP);
  Serial.printf("PASS : %s\n", PASS_AP);
  Serial.printf("AP IP: %s\n", ip.toString().c_str());

  // Register HTTP routes
  server.on("/", handleRoot);     // Serve control panel
  server.on("/info", handleInfo); // Return AP info as JSON
  server.on("/rgb", handleRGB);   // Control both RGB LEDs
  server.on("/relay", handleRelay);// Control relay

  server.begin();                 // Start HTTP server
  Serial.println(F("HTTP server: started"));
}

void loop() {
  server.handleClient();  // Continuously handle incoming HTTP requests
  delay(10);              // Small delay to reduce CPU load
}
