#include <WiFi.h>
#include <esp_now.h>
#include <ArduinoOTA.h>

/******** WiFi ********/
const char *ssid = "Acer";
const char *password = "Sarthak@017";

/******** BELT MAC ********/
uint8_t beltMAC[] = {0x80, 0xF3, 0xDA, 0x4A, 0xA7, 0xDC};

/******** Packet ********/
typedef struct {
  uint8_t direction;
  uint8_t risk;
} DataPacket;

DataPacket pkt;

void setup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(300);

  esp_now_init();

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, beltMAC, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  ArduinoOTA.setHostname("Cap-ESP32");
  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();

  // 🔍 Test data
  pkt.direction = 10;  // any number
  pkt.risk = 2;

  esp_now_send(beltMAC, (uint8_t *)&pkt, sizeof(pkt));
  delay(1000);
}
