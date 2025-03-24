#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "misty";
const char* password = "CHIYASEEDS";


const int leds[] = {2, 4, 16, 17, 5};  // LED Pins
const int ledCount = 5;

WebServer server(80);
bool runAlternating = false;
bool runCenterToBottom = false;
void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<title>ESP32 LED Control</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; text-align: center; background-color: #f4f4f4; padding: 20px; }";
  html += ".container { background: white; padding: 20px; border-radius: 10px; box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.1); max-width: 400px; margin: auto; }";
  html += "h1 { color: #333; }";
  html += "button { width: 100%; padding: 10px; margin: 10px 0; border: none; border-radius: 5px; font-size: 16px; acursor: pointer; }";
  html += "button:hover { opacity: 0.8; }";
  html += ".btn-green { background-color: #4CAF50; color: white; }";
  html += ".btn-blue { background-color: #008CBA; color: white; }";
  html += ".btn-red { background-color: #f44336; color: white; }";
  html += ".btn-orange { background-color: #ff9800; color: white; }";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<h1>ESP32 LED Control</h1>";
  html += "<button class='btn-green' onclick=\"fetch('/press_all').then(response => alert('Toggled All LEDs'))\">Press All</button>";
  html += "<button class='btn-blue' onclick=\"fetch('/alternating_start').then(response => alert('Alternating Started'))\">Start Alternating Light</button>";
  html += "<button class='btn-red' onclick=\"fetch('/center_to_bottom_start').then(response => alert('Center to Bottom Started'))\">Start Center to Bottom</button>";
  html += "<button class='btn-orange' onclick=\"fetch('/stop').then(response => alert('Patterns Stopped'))\">Stop</button>";
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}
void toggleAll() {
  static bool state = false;
  state = !state;
  for (int i = 0; i < ledCount; i++) {
    digitalWrite(leds[i], state ? HIGH : LOW);
  }
  server.send(200, "text/plain", "All LEDs toggled");
}

void startAlternating() {
  runAlternating = true;
  server.send(200, "text/plain", "Alternating LEDs Started");
}
void startCenterToBottom() {
  runCenterToBottom = true;
  server.send(200, "text/plain", "Center to Bottom Started");
}
void stopPatterns() {
  runAlternating = false;
  runCenterToBottom = false;
  for (int i = 0; i < ledCount; i++) {
    digitalWrite(leds[i], LOW);
  }
  server.send(200, "text/plain", "Patterns Stopped");
}
void loopAlternating() {
  static unsigned long lastTime = 0;
  static int step = 0;
  const int pattern[] = {0, 2, 4, 1, 3}; 
  const int patternLength = sizeof(pattern) / sizeof(pattern[0]);

  if (runAlternating && millis() - lastTime > 500) {
    lastTime = millis();
    for (int i = 0; i < ledCount; i++) {
      digitalWrite(leds[i], LOW);
    }
    digitalWrite(leds[pattern[step]], HIGH);
    step = (step + 1) % patternLength;
  }
}
void loopCenterToBottom() {
  static unsigned long lastTime = 0;
  static int step = 0;
  static bool forward = true;

  if (runCenterToBottom && millis() - lastTime > 300) {
    lastTime = millis();
    
    for (int i = 0; i < ledCount; i++) {
      digitalWrite(leds[i], LOW);
    }
    
    int mid = ledCount / 2;
    for (int i = 0; i <= step; i++) {
      if (mid - i >= 0) digitalWrite(leds[mid - i], HIGH);
      if (mid + i < ledCount) digitalWrite(leds[mid + i], HIGH);
    }

    if (forward) {
      step++;
      if (step > mid) forward = false;
    } else {
      step--;
      if (step < 0) forward = true;
    }
  }
}
void setup() {
  Serial.begin(115200);
  for (int i = 0; i < ledCount; i++) {
    pinMode(leds[i], OUTPUT);
    digitalWrite(leds[i], LOW);
  }
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/press_all", toggleAll);
  server.on("/alternating_start", startAlternating);
  server.on("/center_to_bottom_start", startCenterToBottom);
  server.on("/stop", stopPatterns);
  
  server.begin();
  Serial.println("HTTP server started");
}
void loop() {
  server.handleClient();
  loopAlternating();
  loopCenterToBottom();
}
