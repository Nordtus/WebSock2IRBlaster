# ESP32 LG TV WebSocket → IR Volume Bridge

This project connects an ESP32 to an LG webOS TV via WebSocket and mirrors volume + mute events to an IR blaster. It effectively turns LG TV audio events into physical IR remote control signals for external devices (amps, receivers, etc.).

---

## Features

- Connects to LG webOS TV via WebSocket (SSL supported)
- Handles LG TV pairing + client-key registration automatically
- Subscribes to real-time audio events:
  - Volume up
  - Volume down
  - Mute toggle
- Converts LG audio events into IR commands
- Supports:
  - IR toggle encoding (prevents desync)
  - Burst transmission (3x IR pulses per event)
- Auto-reconnect on disconnect
- Stable state tracking (mute + volume toggling)

---

## How it works

1. ESP32 connects to WiFi
2. Opens WebSocket to LG TV (`wss://TV_IP:3001`)
3. Performs LG pairing handshake:
   - `reg1` → receive `client-key`
   - `reg2` → authenticated session
4. Subscribes to:
   - `ssap://audio/getVolume`
5. Receives real-time updates:
   - `volumeUp`
   - `volumeDown`
   - `muted`
6. Translates events into IR commands via IR LED

---

## Hardware Required

- ESP32 development board
- IR LED (with resistor/transistor driver recommended)
- LG webOS TV (2016+ recommended)
- Optional: external amplifier/receiver to control via IR

---

## Wiring (basic IR setup)

| ESP32 Pin | Component |
|-----------|----------|
| GPIO 5    | IR LED signal |
| GND       | IR LED GND |
| 3.3V/5V   | IR circuit power (depending on setup) |

> For best range, use a transistor driver circuit instead of driving IR LED directly.

---

## Dependencies

Install via PlatformIO:

- WebSocketsClient
- ArduinoJson (v7+)
- IRremoteESP8266
- WiFi (ESP32 core)

---

## Configuration

Create `secrets.h`:

```cpp
#define WIFI_SSID "your_wifi_name"
#define WIFI_PASS "your_wifi_password"
#define TV_IP "192.168.x.x"
```

## IR Code Detection (Learning Your Own Remote)

This project also includes a built-in IR receiver sketch (commented out at the bottom of `main.cpp`) that can be used to **capture and identify IR codes from your own remotes**.

This is useful if you want to:
- Replace the default RC5 codes with your own device’s IR codes
- Adapt the system to different TVs, amplifiers, or audio equipment
- Debug or verify IR signals

---

### IR Receiver Wiring

The IR receiver is connected to:

| ESP32 Pin | Function        |
|----------|----------------|
| GPIO 4   | IR Receiver OUT |
| GND      | Ground          |
| 3.3V     | Power           |

---

### How to Enable IR Detection Mode

At the bottom of `main.cpp`, there is a commented-out IR test sketch.

To use it:

1. Uncomment the IR receiver section in `main.cpp`
2. Comment out or disable the WebSocket + TV logic
3. Flash the ESP32
4. Open Serial Monitor (115200 baud)

---

### Example Output

When you press buttons on your remote, you will see output like:
----- CLEAN IR DATA -----
Protocol: RC5
Address: 10
Command: 22
Raw HEX: 422, C22

or

Protocol: RC5X
Address: 10
Command: 59
Raw HEX: 1419, 1C19

---

### How to Use the Data

Use the captured values directly in your main IR sending logic:

```cpp
irsend.sendRC5(0x422, 12);
irsend.sendRC5(0xC21, 12);
irsend.sendRC5(0x1419, 12);

Or replace the existing codes in:

Volume Up / Down handling
Mute toggle logic