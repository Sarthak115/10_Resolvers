#include <WiFi.h>
#include <esp_now.h>
#include <WebServer.h>
#include <ArduinoOTA.h>

/******** WiFi ********/
const char *ssid = "Acer";
const char *password = "Sarthak@017";

/******** BELT MAC ********/
uint8_t beltMAC[] = {0x80,0xF3,0xDA,0x4A,0xA7,0xDC};

WebServer server(80);

/******** PINS ********/
#define FRONT_TRIG 5
#define FRONT_ECHO 18
#define LEFT_TRIG  17
#define LEFT_ECHO  16
#define RIGHT_TRIG 4
#define RIGHT_ECHO 2
#define DOWN_TRIG  19
#define DOWN_ECHO  21
#define BACK_TRIG  23
#define BACK_ECHO  22

/******** PACKET ********/
typedef struct {
  uint8_t direction;  // 0–7, 255 = none
  uint8_t risk;       // 0=low,1=medium,2=high
  uint8_t height;     // 0=floor,1=mid,2=high
  uint8_t motion;     // 0=static,1=approaching
} NavPacket;

NavPacket pkt;

/******** DISTANCE STORAGE ********/
long d_front = 999, d_left = 999, d_right = 999, d_down = 999, d_back = 999;
long prevFront = 999, prevBack = 999;

/******** DISTANCE FUNCTION ********/
long getDistance(int trig, int echo) {
  digitalWrite(trig, LOW); delayMicroseconds(2);
  digitalWrite(trig, HIGH); delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long d = pulseIn(echo, HIGH, 25000);
  if (d <= 0) return 999;
  return d * 0.034 / 2;
}

/******** CORE LOGIC ********/
void updateLogic() {
  d_front = getDistance(FRONT_TRIG, FRONT_ECHO);
  d_left  = getDistance(LEFT_TRIG, LEFT_ECHO);
  d_right = getDistance(RIGHT_TRIG, RIGHT_ECHO);
  d_down  = getDistance(DOWN_TRIG, DOWN_ECHO);
  d_back  = getDistance(BACK_TRIG, BACK_ECHO);

  // Defaults
  pkt.direction = 255;
  pkt.risk = 0;
  pkt.height = 1;
  pkt.motion = 0;

  // Motion detection (front)
  if (d_front < prevFront - 10) pkt.motion = 1;
  prevFront = d_front;

  // Following detection (back)
  if (d_back < prevBack - 10) {
    pkt.direction = 4;   // BACK
    pkt.risk = 1;
    pkt.motion = 1;
  }
  prevBack = d_back;

  // PRIORITY ORDER
  if (d_down > 80) {                // Floor hazard
    pkt.direction = 0;              // Front
    pkt.risk = 2;
    pkt.height = 0;
  }
  else if (d_front < 60) {           // Front obstacle
    pkt.direction = 0;
    pkt.risk = (d_front < 30) ? 2 : 1;
    pkt.height = 1;
  }
  else if (d_left < 50) {            // Left
    pkt.direction = 6;
    pkt.risk = 1;
  }
  else if (d_right < 50) {           // Right
    pkt.direction = 2;
    pkt.risk = 1;
  }
}

/******** HTTP: PACKET ONLY ********/
void handleCap() {
  String json = "{";
  json += "\"direction\":" + String(pkt.direction) + ",";
  json += "\"risk\":" + String(pkt.risk) + ",";
  json += "\"height\":" + String(pkt.height) + ",";
  json += "\"motion\":" + String(pkt.motion);
  json += "}";
  server.send(200, "application/json", json);
}

/******** HTTP: FULL DEBUG ********/
void handleDebug() {
  String json = "{";
  json += "\"ultrasonic\":{";
  json += "\"front\":" + String(d_front) + ",";
  json += "\"left\":" + String(d_left) + ",";
  json += "\"right\":" + String(d_right) + ",";
  json += "\"back\":" + String(d_back) + ",";
  json += "\"down\":" + String(d_down);
  json += "},";
  json += "\"packet\":{";
  json += "\"direction\":" + String(pkt.direction) + ",";
  json += "\"risk\":" + String(pkt.risk) + ",";
  json += "\"height\":" + String(pkt.height) + ",";
  json += "\"motion\":" + String(pkt.motion);
  json += "}";
  json += "}";
  server.send(200, "application/json", json);
}

/******** SETUP ********/
void setup() {
  pinMode(FRONT_TRIG, OUTPUT);
  pinMode(LEFT_TRIG, OUTPUT);
  pinMode(RIGHT_TRIG, OUTPUT);
  pinMode(DOWN_TRIG, OUTPUT);
  pinMode(BACK_TRIG, OUTPUT);

  pinMode(FRONT_ECHO, INPUT_PULLDOWN);
  pinMode(LEFT_ECHO, INPUT_PULLDOWN);
  pinMode(RIGHT_ECHO, INPUT_PULLDOWN);
  pinMode(DOWN_ECHO, INPUT_PULLDOWN);
  pinMode(BACK_ECHO, INPUT_PULLDOWN);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(300);

  esp_now_init();
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, beltMAC, 6);
  esp_now_add_peer(&peer);

  ArduinoOTA.setHostname("Cap-ESP32");
  ArduinoOTA.begin();

  server.on("/cap", handleCap);
  server.on("/debug", handleDebug);
  server.begin();
}

/******** LOOP ********/
void loop() {
  updateLogic();
  esp_now_send(beltMAC, (uint8_t*)&pkt, sizeof(pkt));
  ArduinoOTA.handle();
  server.handleClient();
  delay(200);
}
