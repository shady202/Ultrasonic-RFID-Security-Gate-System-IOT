# Ultrasonic RFID Security Gate System

An Arduino-based smart access control system that combines ultrasonic proximity sensing with RFID authentication to guard a restricted area. As a person approaches, a three-zone LED system (green/yellow/red) gives instant visual feedback, while an RFID reader controls a servo-driven gate — only opening for authorized cards. Unauthorized presence in the danger zone triggers an audible alarm.

Built as part of the *Internet of Things: Concepts and Applications* module, this project demonstrates how low-cost IoT components can be combined into a practical, real-time access control solution with applications in physical security.

## Features

- **Proximity-based zone detection** using an ultrasonic sensor (Danger / Warning / Safe)
- **RFID authentication** via MFRC522 — only recognized card UIDs unlock the gate
- **Servo-controlled gate** that opens automatically on valid access and auto-closes after a timeout
- **Real-time LCD feedback** (e.g. "Access Granted", "Access Denied", "Please scan your key card")
- **Traffic-light LED indicators** (Red = danger/unauthorized, Yellow = warning, Green = safe)
- **Audible alarm** (passive buzzer) for unauthorized presence in the danger zone

## How It Works

1. The ultrasonic sensor continuously measures distance to the gate and classifies it into a zone:
   - **Safe** (≥ 20 cm) — green LED, no action
   - **Warning** (10–14 cm) — yellow LED, LCD prompts the user to scan their card
   - **Danger** (≤ 5 cm) — red LED; if the person hasn't scanned a valid card, the buzzer sounds an alarm
2. When an RFID card is scanned, its UID is compared against the stored authorized UID.
3. If valid, the LCD shows "Access Granted," the buzzer stops, and the servo opens the gate for a few seconds before auto-closing.
4. If invalid, the LCD shows "Access Denied" and the gate stays locked.

## Components Used

| Component | Quantity |
|---|---|
| Maker UNO (Arduino Uno-compatible, ATmega328P) | 1 |
| Ultrasonic Sensor (HC-SR04) | 1 |
| RFID Reader Module (MFRC522) | 1 |
| RFID Card / Key Fob (MIFARE) | 1 |
| Servo Motor (SG90) | 1 |
| Passive Buzzer | 1 |
| LEDs (Red, Yellow, Green) | 3 |
| Resistors (220Ω) | 4 |
| 16x2 LCD with I2C Module | 1 |
| Breadboard | 1 |
| Jumper Wires (M–M, M–F) | 23 |
| Power Supply (USB or 9V adapter) | 1 |

## Circuit / Pin Connections

| Arduino Pin | Connected To |
|---|---|
| D2 | Ultrasonic Sensor — Trig |
| D3 | Ultrasonic Sensor — Echo |
| D4 | Red LED (via resistor) |
| D5 | Yellow LED (via resistor) |
| D6 | Green LED (via resistor) |
| D7 | Passive Buzzer |
| D8 | Servo Motor — Signal |
| D9 | RFID RC522 — RST |
| D10 | RFID RC522 — SDA (SS) |
| D11 | RFID RC522 — MOSI |
| D12 | RFID RC522 — MISO |
| D13 | RFID RC522 — SCK |
| A4 (SDA) | I2C LCD — SDA |
| A5 (SCL) | I2C LCD — SCL |
| 5V / GND | Power rails for LCD, Servo, LEDs, Buzzer |
| 3.3V / GND | RFID RC522 power |

The circuit was first prototyped and tested in Tinkercad before being built on physical hardware.

## Getting Started

### Requirements
- [Arduino IDE](https://www.arduino.cc/en/software)
- Libraries: `LiquidCrystal_I2C`, `SPI` (built-in), `MFRC522`, `Servo` (built-in)

### Setup
1. Wire the components according to the pin table above.
2. Install the required libraries via the Arduino IDE Library Manager.
3. Open the `.ino` sketch and update `authorizedUID` with your own RFID card's UID (printed to Serial Monitor when you scan a card).
4. Upload the sketch to the Maker UNO / Arduino Uno.
5. Open the Serial Monitor (9600 baud) to view live status logs.

## Limitations

- Ultrasonic readings can be inaccurate in noisy environments or with irregular-shaped objects.
- The RFID module has a short read range and can be affected by damaged or blocked cards.
- The passive buzzer provides only basic audio feedback, which may be insufficient in loud environments.
- No encryption is used on the RFID communication, leaving it vulnerable to spoofing or card cloning.
- Single-controller design limits scalability for larger deployments.

## Future Enhancements

- Add encrypted/secure RFID cards to prevent cloning
- Integrate a stronger alarm/buzzer for better alerting
- Replace the ultrasonic sensor with infrared or camera-based detection for improved accuracy
- Add Wi-Fi/IoT connectivity for remote monitoring and access logging

## Author

Shady Ehab Assad Ibrahim — BSc (Hons) Computer Science with Cyber Security specialization
