/**************** OTA HEADERS ****************/
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

/**************** WEB SERVER ****************/
#include <WebServer.h>

/************* WiFi Credentials *************/
const char *ssid = "Acer";
const char *password = "Sarthak@017";

/**************** ULTRASONIC PINS ************/
#define FRONT_TRIG 5
#define FRONT_ECHO 18

#define LEFT_TRIG 17
#define LEFT_ECHO 16

#define RIGHT_TRIG 4
#define RIGHT_ECHO 2

#define DOWN_TRIG 19
#define DOWN_ECHO 21

/**************** WEB SERVER ****************/
WebServer server(80);

/**************** DEBUG TIMERS ***************/
unsigned long lastDebugPrint = 0;
const unsigned long debugIntervalMs = 500;

/**************** DISTANCE FUNCTION **********/
long getDistanceCM(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long duration = pulseIn(echo, HIGH, 25000); // 25ms timeout

  if (duration <= 0) return 999;

  long distance = duration * 0.034 / 2;

  // Sanity bounds
  if (distance < 2 || distance > 400) return 999;

  return distance;
}

/**************** CAP STATE ******************/
String cap_direction = "none";
String cap_hazard = "none";
String cap_risk = "low";
String cap_motion = "static";

/**************** DEBUG VALUES ***************/
long dbg_front = 999;
long dbg_left  = 999;
long dbg_right = 999;
long dbg_down  = 999;

/**************** CAP LOGIC ******************/
void updateCapLogic() {

  dbg_front = getDistanceCM(FRONT_TRIG, FRONT_ECHO);
  dbg_left  = getDistanceCM(LEFT_TRIG, LEFT_ECHO);
  dbg_right = getDistanceCM(RIGHT_TRIG, RIGHT_ECHO);
  dbg_down  = getDistanceCM(DOWN_TRIG, DOWN_ECHO);

  // Reset state
  cap_direction = "none";
  cap_hazard = "none";
  cap_risk = "low";

  // PRIORITY 1: FLOOR (stairs / drop)
  if (dbg_down > 80) {
    cap_direction = "front";
    cap_hazard = "bottom";
    cap_risk = "high";
  }

  // PRIORITY 2: FRONT
 else if (dbg_front < 60) {
    cap_direction = "front";
    cap_hazard = "middle";
    cap_risk = (dbg_front < 30) ? "high" : "medium";
  }

  // PRIORITY 3: LEFT
  else if (dbg_left < 50) {
    cap_direction = "left";
    cap_hazard = "middle";
    cap_risk = "medium";
  }

  // PRIORITY 4: RIGHT
  else if (dbg_right < 50) {
    cap_direction = "right";
    cap_hazard = "middle";
    cap_risk = "medium";
  }
}

/**************** /cap ENDPOINT **************/
void handleCap() {
  updateCapLogic();

  String json = "{";
  json += "\"direction\":\"" + cap_direction + "\",";
  json += "\"hazard\":\"" + cap_hazard + "\",";
  json += "\"risk\":\"" + cap_risk + "\",";
  json += "\"motion\":\"" + cap_motion + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

/**************** /cap/raw ENDPOINT **********/
void handleCapRaw() {
  dbg_front = getDistanceCM(FRONT_TRIG, FRONT_ECHO);
  dbg_left  = getDistanceCM(LEFT_TRIG, LEFT_ECHO);
  dbg_right = getDistanceCM(RIGHT_TRIG, RIGHT_ECHO);
  dbg_down  = getDistanceCM(DOWN_TRIG, DOWN_ECHO);

  String json = "{";
  json += "\"front_cm\":" + String(dbg_front) + ",";
  json += "\"left_cm\":"  + String(dbg_left)  + ",";
  json += "\"right_cm\":" + String(dbg_right) + ",";
  json += "\"down_cm\":"  + String(dbg_down);
  json += "}";

  server.send(200, "application/json", json);
}

/**************** SETUP **********************/
void setup() {
  Serial.begin(115200);
  Serial.println("\n[BOOT] Starting ESP32 Cap Module");

  pinMode(FRONT_TRIG, OUTPUT);
  pinMode(FRONT_ECHO, INPUT_PULLDOWN);

  pinMode(LEFT_TRIG, OUTPUT);
  pinMode(LEFT_ECHO, INPUT_PULLDOWN);

  pinMode(RIGHT_TRIG, OUTPUT);
  pinMode(RIGHT_ECHO, INPUT_PULLDOWN);

  pinMode(DOWN_TRIG, OUTPUT);
  pinMode(DOWN_ECHO, INPUT_PULLDOWN);

  WiFi.begin(ssid, password);
  Serial.print("[WiFi] Connecting");

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\n[WiFi] Connected");
  Serial.print("[WiFi] IP: ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.begin();
  Serial.println("[OTA] Ready");

  server.on("/cap", handleCap);
  server.on("/cap/raw", handleCapRaw);
  server.begin();
  Serial.println("[HTTP] Server started");
}

/**************** LOOP ***********************/
void loop() {
  ArduinoOTA.handle();
  server.handleClient();

  // 🔍 ALWAYS-ON DEBUG
  if (millis() - lastDebugPrint >= debugIntervalMs) {
    lastDebugPrint = millis();

    updateCapLogic();

    Serial.print("[ULTRASONIC] ");
    Serial.print("F: "); Serial.print(dbg_front);
    Serial.print(" | L: "); Serial.print(dbg_left);
    Serial.print(" | R: "); Serial.print(dbg_right);
    Serial.print(" | D: "); Serial.println(dbg_down);

    Serial.print("[CAP] ");
    Serial.print("Dir: "); Serial.print(cap_direction);
    Serial.print(" | Hazard: "); Serial.print(cap_hazard);
    Serial.print(" | Risk: "); Serial.println(cap_risk);

    Serial.println("-------------------------------------------");
  }
}