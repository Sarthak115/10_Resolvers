#include <WiFi.h>
#include <esp_now.h>
#include <WebServer.h>
#include <ArduinoOTA.h>

const char *ssid = "Acer";
const char *password = "Sarthak@017";

// BELT MAC
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

typedef struct {
  uint8_t direction;
  uint8_t risk;
  uint8_t height;
  uint8_t motion;
} NavPacket;

NavPacket pkt;

long prevFront = 999;
long prevBack  = 999;

/******** Distance ********/
long getDistance(int trig, int echo) {
  digitalWrite(trig, LOW); delayMicroseconds(2);
  digitalWrite(trig, HIGH); delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long d = pulseIn(echo, HIGH, 25000);
  if (d <= 0) return 999;
  return d * 0.034 / 2;
}

/******** Core Logic ********/
void updateLogic() {
  long front = getDistance(FRONT_TRIG, FRONT_ECHO);
  long left  = getDistance(LEFT_TRIG, LEFT_ECHO);
  long right = getDistance(RIGHT_TRIG, RIGHT_ECHO);
  long down  = getDistance(DOWN_TRIG, DOWN_ECHO);
  long back  = getDistance(BACK_TRIG, BACK_ECHO);

  // Defaults
  pkt.direction = 255;
  pkt.risk = 0;
  pkt.height = 1;
  pkt.motion = 0;

  // Motion detection (front)
  if (front < prevFront - 10) pkt.motion = 1;
  prevFront = front;

  // FOLLOWING detection (back)
  if (back < prevBack - 10) {
    pkt.direction = 4;   // BACK
    pkt.risk = 1;
    pkt.motion = 1;
  }
  prevBack = back;

  // Floor priority
  if (down > 80) {
    pkt.direction = 0;
    pkt.risk = 2;
    pkt.height = 0;
  }
  else if (front < 60) {
    pkt.direction = 0;
    pkt.risk = (front < 30) ? 2 : 1;
    pkt.height = 1;
  }
  else if (left < 50) {
    pkt.direction = 6;
    pkt.risk = 1;
  }
  else if (right < 50) {
    pkt.direction = 2;
    pkt.risk = 1;
  }
}

/******** HTTP DEBUG ********/
void handleCap() {
  String json = "{";
  json += "\"dir\":" + String(pkt.direction) + ",";
  json += "\"risk\":" + String(pkt.risk) + ",";
  json += "\"height\":" + String(pkt.height) + ",";
  json += "\"motion\":" + String(pkt.motion);
  json += "}";
  server.send(200, "application/json", json);
}

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

  ArduinoOTA.begin();
  server.on("/cap", handleCap);
  server.begin();
}

void loop() {
  updateLogic();
  esp_now_send(beltMAC, (uint8_t*)&pkt, sizeof(pkt));
  ArduinoOTA.handle();
  server.handleClient();
  delay(200);
}
