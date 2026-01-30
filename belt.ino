#include <WiFi.h>
#include <esp_now.h>
#include <WebServer.h>
#include <ArduinoOTA.h>

const char *ssid = "Acer";
const char *password = "Sarthak@017";

WebServer server(80);

/******** PACKET ********/
typedef struct {
  uint8_t direction;
  uint8_t risk;
  uint8_t height;
  uint8_t motion;
} NavPacket;

NavPacket pkt;
bool received = false;

/******** MOTOR PINS ********/
int motorPins[8] = {13,12,14,27,26,25,33,32};

/******** MOTOR CONTROL ********/
void stopAllMotors() {
  for(int i=0;i<8;i++) digitalWrite(motorPins[i], LOW);
}

void vibrateMotor(int idx, int risk) {
  stopAllMotors();
  if (idx > 7) return;

  if (risk == 2) {
    digitalWrite(motorPins[idx], HIGH);   // continuous
  } 
  else {
    digitalWrite(motorPins[idx], HIGH);
    delay(150);
    digitalWrite(motorPins[idx], LOW);
  }
}

/******** ESP-NOW RECEIVE ********/
void onReceive(const esp_now_recv_info *info,
               const uint8_t *data, int len) {
  if (len == sizeof(NavPacket)) {
    memcpy(&pkt, data, sizeof(pkt));
    received = true;
  }
}

/******** HTTP: STATUS ********/
void handleBelt() {
  String json = "{";
  json += "\"dir\":" + String(pkt.direction) + ",";
  json += "\"risk\":" + String(pkt.risk) + ",";
  json += "\"height\":" + String(pkt.height) + ",";
  json += "\"motion\":" + String(pkt.motion);
  json += "}";
  server.send(200, "application/json", json);
}

/******** HTTP: MANUAL MOTOR TEST ********/
void handleTest() {
  if (!server.hasArg("m")) {
    server.send(400, "text/plain", "Missing motor index (?m=0-7)");
    return;
  }

  int m = server.arg("m").toInt();
  int r = server.hasArg("r") ? server.arg("r").toInt() : 1;

  vibrateMotor(m, r);

  String msg = "Motor ";
  msg += m;
  msg += " activated with risk ";
  msg += r;

  server.send(200, "text/plain", msg);
}

/******** SETUP ********/
void setup() {
  for(int i=0;i<8;i++) {
    pinMode(motorPins[i], OUTPUT);
    digitalWrite(motorPins[i], LOW);
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(300);

  esp_now_init();
  esp_now_register_recv_cb(onReceive);

  ArduinoOTA.setHostname("Belt-ESP32");
  ArduinoOTA.begin();

  server.on("/belt", handleBelt);
  server.on("/test", handleTest);
  server.begin();
}

/******** LOOP ********/
void loop() {
  if (received) {
    vibrateMotor(pkt.direction, pkt.risk);
    received = false;
  }

  ArduinoOTA.handle();
  server.handleClient();
}
