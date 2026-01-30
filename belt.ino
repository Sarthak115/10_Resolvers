#include <WiFi.h>
#include <esp_now.h>
#include <WebServer.h>
#include <ArduinoOTA.h>

/******** WiFi ********/
const char *ssid = "Acer";
const char *password = "Sarthak@017";

/******** HTTP ********/
WebServer server(80);

/******** Packet ********/
typedef struct {
  uint8_t direction;
  uint8_t risk;
} DataPacket;

volatile bool packetReceived = false;
DataPacket lastPacket;

/******** ESP-NOW RX CALLBACK (ESP32 CORE 3.x) ********/
void onReceive(const esp_now_recv_info *info,
               const uint8_t *data,
               int len) {
  if (len == sizeof(DataPacket)) {
    memcpy(&lastPacket, data, sizeof(DataPacket));
    packetReceived = true;
  }
}

/******** HTTP STATUS ********/
void handleStatus() {
  String json = "{";
  json += "\"received\":" + String(packetReceived ? "true" : "false") + ",";
  json += "\"direction\":" + String(lastPacket.direction) + ",";
  json += "\"risk\":" + String(lastPacket.risk);
  json += "}";
  server.send(200, "application/json", json);
}

void setup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(300);

  esp_now_init();
  esp_now_register_recv_cb(onReceive);

  ArduinoOTA.setHostname("Belt-ESP32");
  ArduinoOTA.begin();

  server.on("/status", handleStatus);
  server.begin();
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
}
