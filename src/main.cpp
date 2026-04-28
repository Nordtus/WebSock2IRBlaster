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
bool volToggle = false;
// bool muteToggle = false;

// -------- SEND LG COMMAND --------
void sendLG(const char *uri)
{
  JsonDocument doc;

  doc["id"] = "1";
  doc["type"] = "request";
  doc["uri"] = uri;

  String msg;
  serializeJson(doc, msg);

  webSocket.sendTXT(msg);
  Serial.print("> ");
  Serial.println(msg);
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{

  if (type == WStype_DISCONNECTED)
  {
    Serial.println("Disconnected from TV");
    registered = false;
  }

  if (type == WStype_CONNECTED)
  {
    Serial.println("Connected to TV");

    registered = false;
    clientKey = "";

    webSocket.sendTXT(R"({"id":"reg1","type":"register","payload":{"pairingType":"PROMPT"}})");
    Serial.print("> ");
    Serial.println(R"({"id":"reg1","type":"register","payload":{"pairingType":"PROMPT"}})");
  }

  if (type == WStype_TEXT)
  {

    Serial.print("< ");
    Serial.println((char *)payload);

    JsonDocument doc;
    deserializeJson(doc, payload);

    const char *msgType = doc["type"];

    // -------- STEP A: capture client-key --------
    const char *msgId = doc["id"] | "";

    if (strcmp(msgType, "registered") == 0 && strcmp(msgId, "reg1") == 0)
    {

      clientKey = doc["payload"]["client-key"].as<String>();

      Serial.print("Got client-key: ");
      Serial.println(clientKey);

      // STEP 2: re-register WITH key + permissions
      String msg =
          String("{\"id\":\"reg2\",\"type\":\"register\",\"payload\":{") + "\"pairingType\":\"PROMPT\"," + "\"client-key\":\"" + clientKey + "\"," + "\"manifest\":{" + "\"manifestVersion\":1," + "\"appVersion\":\"1.0\"," + "\"permissions\":[\"CONTROL_AUDIO\",\"READ_AUDIO_STATUS\"]" + "}}}";

      webSocket.sendTXT(msg);
      Serial.print("> ");
      Serial.println(msg);
      return;
    }

    // -------- STEP B: registration confirmed --------
    if (strcmp(msgType, "registered") == 0 && strcmp(msgId, "reg2") == 0)
    {
      Serial.println("Registration complete");

      registered = true;

      webSocket.sendTXT(R"({"id":"sub_1","type":"subscribe","uri":"ssap://audio/getVolume"})");
      Serial.print("> ");
      Serial.println(R"({"id":"sub_1","type":"subscribe","uri":"ssap://audio/getVolume"})");

      return;
    }

    // -------- STEP C: subscription events --------
    if (registered)
    {
      if (!doc["payload"].is<JsonObject>())
        return;

      JsonObject p = doc["payload"];

      bool muted = p["muted"] | false;

      const char *cause = p["cause"] | ""; // safe even if missing

      Serial.print("EVENT at ");
      Serial.print(millis());
      Serial.print(": ");
      Serial.println(cause);

      static unsigned long lastIR = 0;
      static bool lastMuted = false;

      if (millis() - lastIR > 80)
      {
        lastIR = millis();

        // -------- MUTE HANDLING --------
        if (p.containsKey("muted") && muted != lastMuted)
        {
          lastMuted = muted;

          Serial.print("MUTE CHANGE: ");
          Serial.println(muted ? "ON" : "OFF");

          // irsend.sendRC5(0x1419, 12);

          uint16_t code = muted ? 0x419 : 0xC19; // alternating pair
          // muteToggle = !muteToggle;

          irsend.sendRC5(code, 12);

          return;
        }

        // -------- VOLUME --------
        if (strcmp(cause, "volumeUp") == 0)
        {
          for (int i = 0; i < 3; i++)
          {
            uint16_t code = volToggle ? 0x422 : 0xC22;
            volToggle = !volToggle;

            irsend.sendRC5(code, 12);
            delay(35); // important: prevents IR collapse
          }
        }
        else if (strcmp(cause, "volumeDown") == 0)
        {
          for (int i = 0; i < 3; i++)
          {
            uint16_t code = volToggle ? 0xC21 : 0x421;
            volToggle = !volToggle;

            irsend.sendRC5(code, 12);
            delay(35);
          }
        }
      }
    }
  }
}

// -------- SETUP --------
void setup()
{
  Serial.begin(115200);

  irsend.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  WiFi.setSleep(false);

  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\r\nWiFi connected");

  // webSocket.begin(TV_IP, 3000, "/");
  webSocket.beginSSL(TV_IP, 3001, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000); // retry every 5s
  webSocket.enableHeartbeat(15000, 3000, 2);
}

// -------- LOOP --------
void loop()
{
  webSocket.loop();
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