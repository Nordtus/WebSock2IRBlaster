
#include <Arduino.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRremoteESP8266.h>
#include <IRutils.h>

const uint16_t IR_RECEIVE_PIN = 4;  // change if needed

IRrecv irrecv(IR_RECEIVE_PIN);
decode_results results;

const uint16_t IR_LED_PIN = 5;  // change if needed
IRsend irsend(IR_LED_PIN);

// Your captured values
const uint16_t ADDRESS = 0x10;
const uint16_t VOL_UP   = 0x22;
const uint16_t VOL_DOWN = 0x21;

//const uint16_t RC5_BITS = 12;

// Your verified toggle pair
const uint16_t VOL_DOWN_A = 0xC21;
const uint16_t VOL_DOWN_B = 0x421;

bool toggle = false;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\nIR Receiver Ready...");
  Serial.printf("Listening on GPIO %d\n", IR_RECEIVE_PIN);

Serial.printf("RC5_BITS %d\n", RC5_BITS);

  irrecv.enableIRIn();  // start receiver
  irsend.begin();
  Serial.println("IR Blaster Ready - Volume loop starting...");
}

void loop() {

  uint16_t code = toggle ? VOL_DOWN_A : VOL_DOWN_B;

  Serial.print("Sending: 0x");
  Serial.println(code, HEX);

  irsend.sendRC5(code, 12);

  toggle = !toggle;

  delay(400);  // adjust speed if needed


  //  Serial.println("Volume UP");
  // irsend.sendRC5(0x422, 12);
  // irsend.sendRC5(ADDRESS, VOL_UP, RC5_BITS);
  // delay(300);  // adjust speed

  // Serial.println("Volume DOWN");
  // irsend.sendRC5(ADDRESS, VOL_DOWN, RC5_BITS);
  // delay(300);


  if (irrecv.decode(&results)) {

  Serial.println("----- CLEAN IR DATA -----");

  Serial.print("Protocol: ");
  Serial.println(typeToString(results.decode_type));

  Serial.print("Address: ");
  Serial.println(results.address, HEX);

  Serial.print("Command: ");
  Serial.println(results.command, HEX);

  Serial.print("Raw HEX: ");
  Serial.println(results.value, HEX);

  irrecv.resume();
}
}


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