/**************** OTA HEADERS ****************/
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

/************* WiFi Credentials *************/
const char *ssid = "Acer";
const char *password = "Sarthak@017";

/************* MOTOR PIN ********************/
#define MOTOR_PIN 25   // Change if needed

/**************** SETUP *********************/
void setup() {
  Serial.begin(115200);
  Serial.println("\n[BOOT] ESP32 Belt Motor OTA Test");

  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);

  WiFi.begin(ssid, password);
  Serial.print("[WiFi] Connecting");

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\n[WiFi] Connected");
  Serial.print("[WiFi] IP: ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.setHostname("Belt-Motor-Test");
  ArduinoOTA.begin();
  Serial.println("[OTA] Ready");
}

/**************** LOOP **********************/
void loop() {
  ArduinoOTA.handle();

  // Motor ON
  digitalWrite(MOTOR_PIN, HIGH);
  Serial.println("[MOTOR] ON");
  delay(300);

  // Motor OFF
  digitalWrite(MOTOR_PIN, LOW);
  Serial.println("[MOTOR] OFF");
  delay(700);
}