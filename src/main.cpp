
#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include "secrets.h"
#include <ArduinoJson.h>


// -------- LG TV --------
// const char* tv_ip = "192.168.1.100";  // CHANGE THIS

String clientKey = "";
bool registered = false;

WebSocketsClient webSocket;

// -------- IR --------
const uint16_t IR_LED_PIN = 5;
IRsend irsend(IR_LED_PIN);

// Vol- toggle pair
const uint16_t VOL_DOWN_A = 0xC21;
const uint16_t VOL_DOWN_B = 0x421;

bool toggle = false;

// -------- SEND LG COMMAND --------
void sendLG(const char* uri) {
  StaticJsonDocument<200> doc;

  doc["id"] = "1";
  doc["type"] = "request";
  doc["uri"] = uri;

  String msg;
  serializeJson(doc, msg);

  webSocket.sendTXT(msg);
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

  if (type == WStype_CONNECTED) {
    Serial.println("Connected to TV");

    // // STEP 1: initial register (NO key)
    // webSocket.sendTXT(R"({
    //   "id":"reg1",
    //   "type":"register",
    //   "payload":{
    //     "pairingType":"PROMPT"
    //   }
    // })");
  }

  if (type == WStype_TEXT) {

    Serial.println("RAW MSG:");
    Serial.println((char*)payload);

    JsonDocument doc;
    deserializeJson(doc, payload);

    const char* msgType = doc["type"];

    // -------- STEP A: capture client-key --------
    if (strcmp(msgType, "registered") == 0) {

      clientKey = doc["payload"]["client-key"].as<String>();

      Serial.print("Got client-key: ");
      Serial.println(clientKey);

      // STEP 2: re-register WITH key + permissions
      String msg =
        String("{\"id\":\"reg2\",\"type\":\"register\",\"payload\":{")
        + "\"pairingType\":\"PROMPT\","
        + "\"client-key\":\"" + clientKey + "\","
        + "\"manifest\":{"
        + "\"manifestVersion\":1,"
        + "\"appVersion\":\"1.0\","
        + "\"permissions\":[\"CONTROL_AUDIO\",\"READ_AUDIO_STATUS\"]"
        + "}}}";

      webSocket.sendTXT(msg);
      return;
    }

    // -------- STEP B: registration confirmed --------
    if (strcmp(msgType, "response") == 0 && doc["id"] == "reg2") {

      Serial.println("Registration complete");

      registered = true;

      // STEP 3: NOW subscribe (IMPORTANT FIX)
      webSocket.sendTXT(R"({
        "id":"sub_1",
        "type":"subscribe",
        "uri":"ssap://audio/getVolume"
      })");

      return;
    }

    // -------- STEP C: subscription events --------
    if (registered) {

      JsonObject p = doc["payload"];

      if (!p.containsKey("cause")) return;

      const char* cause = p["cause"] | "";
      bool muted = p["muted"] | false;

      Serial.print("EVENT: ");
      Serial.println(cause);

      // ===== IR MIRROR =====

      if (muted) {
        irsend.sendRC5(0x1419, 12);
        return;
      }

      if (strcmp(cause, "volumeUp") == 0) {
        irsend.sendRC5(0x422, 12);
      }
      else if (strcmp(cause, "volumeDown") == 0) {
        irsend.sendRC5(0xC21, 12);
      }
    }
  }
}

// -------- SETUP --------
void setup() {
  Serial.begin(115200);

  irsend.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASS);


  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");

  webSocket.begin(TV_IP, 3000, "/");
  webSocket.onEvent(webSocketEvent);
}

// -------- LOOP --------
void loop() {
  webSocket.loop();

  // Example test: alternate IR + TV control
  static unsigned long last = 0;

  static unsigned long lastIR = 0;
  if (millis() - lastIR < 80) return;
  lastIR = millis();
}


    // Serial.println("IR Vol-");
    // uint16_t code = toggle ? VOL_DOWN_A : VOL_DOWN_B;
    // irsend.sendRC5(code, 12);
    // toggle = !toggle;

    // Serial.println("TV Vol Down");
    // sendLG("ssap://audio/volumeDown");























// #include <Arduino.h>
// #include <IRrecv.h>
// #include <IRsend.h>
// #include <IRremoteESP8266.h>
// #include <IRutils.h>

// const uint16_t IR_RECEIVE_PIN = 4;  // change if needed

// IRrecv irrecv(IR_RECEIVE_PIN);
// decode_results results;

// const uint16_t IR_LED_PIN = 5;  // change if needed
// IRsend irsend(IR_LED_PIN);

// // Your captured values
// const uint16_t ADDRESS = 0x10;
// const uint16_t VOL_UP   = 0x22;
// const uint16_t VOL_DOWN = 0x21;

// //const uint16_t RC5_BITS = 12;

// // Your verified toggle pair
// const uint16_t VOL_DOWN_A = 0xC21;
// const uint16_t VOL_DOWN_B = 0x421;

// bool toggle = false;

// void setup() {
//   Serial.begin(115200);
//   delay(1000);

//   Serial.println("\nIR Receiver Ready...");
//   Serial.printf("Listening on GPIO %d\n", IR_RECEIVE_PIN);

// Serial.printf("RC5_BITS %d\n", RC5_BITS);

//   irrecv.enableIRIn();  // start receiver
//   irsend.begin();
//   Serial.println("IR Blaster Ready - Volume loop starting...");
// }

// void loop() {

//   uint16_t code = toggle ? VOL_DOWN_A : VOL_DOWN_B;

//   Serial.print("Sending: 0x");
//   Serial.println(code, HEX);

//   irsend.sendRC5(code, 12);

//   toggle = !toggle;

//   delay(400);  // adjust speed if needed


//   //  Serial.println("Volume UP");
//   // irsend.sendRC5(0x422, 12);
//   // irsend.sendRC5(ADDRESS, VOL_UP, RC5_BITS);
//   // delay(300);  // adjust speed

//   // Serial.println("Volume DOWN");
//   // irsend.sendRC5(ADDRESS, VOL_DOWN, RC5_BITS);
//   // delay(300);


//   if (irrecv.decode(&results)) {

//   Serial.println("----- CLEAN IR DATA -----");

//   Serial.print("Protocol: ");
//   Serial.println(typeToString(results.decode_type));

//   Serial.print("Address: ");
//   Serial.println(results.address, HEX);

//   Serial.print("Command: ");
//   Serial.println(results.command, HEX);

//   Serial.print("Raw HEX: ");
//   Serial.println(results.value, HEX);

//   irrecv.resume();
// }
// }


/*
Power on/off
----- CLEAN IR DATA -----
Protocol: RC5
Address: 10
Command: 22
Raw HEX: 422, C12

Mute on/off
----- CLEAN IR DATA -----
Protocol: RC5X
Address: 10
Command: 59
Raw HEX: 1419, 1C19

Vol -
----- CLEAN IR DATA -----
Protocol: RC5
Address: 10
Command: 21
Raw HEX: C21, 421

Vol +
----- CLEAN IR DATA -----
Protocol: RC5
Address: 10
Command: 22
Raw HEX: 422, C22




*/