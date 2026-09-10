/* Ultrasonic RFID Security Gate System - Combined Code with Gate Control - FIXED */

#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

// Ultrasonic Sensor Pins
#define trigPin 2
#define echoPin 3

// LED Pins
#define LEDlampRed 4
#define LEDlampYellow 5
#define LEDlampGreen 6

// Buzzer Pin
#define soundbuzzer 7

// Servo Pin
#define SERVO_PIN 8

// RFID Pins
#define SS_PIN 10
#define RST_PIN 9


String authorizedUID = "43 BA FC 0A";

// System variables
String lastStatus = "";
bool accessGranted = false;
unsigned long accessGrantedTime = 0;
const unsigned long ACCESS_TIMEOUT = 15000; // 15 seconds to pass through after RFID scan
bool buzzerActive = false;

// Gate/Servo variables
bool gateOpen = false;
bool servoAttached = false;
unsigned long gateOpenTime = 0;
const unsigned long GATE_OPEN_DURATION = 2000; // 2 seconds gate open time (reduced from 3 seconds)

// Initialize components
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 rfid(SS_PIN, RST_PIN);
Servo gateServo;

void setup() {
  Serial.begin(9600);
  
  // Initialize ultrasonic sensor pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // Initialize LED pins
  pinMode(LEDlampRed, OUTPUT);
  pinMode(LEDlampYellow, OUTPUT);
  pinMode(LEDlampGreen, OUTPUT);
  
  // Initialize buzzer pin
  pinMode(soundbuzzer, OUTPUT);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Access Control");
  lcd.setCursor(0, 1);
  lcd.print("System Ready");
  
  // Initialize RFID
  SPI.begin();
  rfid.PCD_Init();
  
  delay(2000);
  lcd.clear();
  
  Serial.println("Smart Access Control System with Gate Ready");
  Serial.println("Monitoring area...");
  Serial.println("Zone definitions:");
  Serial.println("- DANGER: <= 9 cm");
  Serial.println("- WARNING: 10-14 cm");
  Serial.println("- SAFE: >= 15 cm");
  
  // Initialize gate in closed position
  closeGate();
}

// Function to open gate (NOISE ELIMINATED) - OPENS FROM 6 O'CLOCK TO 9 O'CLOCK
void openGate() {
  if (!servoAttached) {
    gateServo.attach(SERVO_PIN);
    servoAttached = true;
    delay(50);
  }
  
  gateServo.write(180); // Open position (from 6 o'clock to 9 o'clock - 180 degrees)
  delay(600); // Wait for movement
  
  // Detach to eliminate noise
  gateServo.detach();
  servoAttached = false;
  
  gateOpen = true;
  gateOpenTime = millis();
  
  Serial.println("Gate opened - will auto-close in " + String(GATE_OPEN_DURATION/1000) + " seconds");
}

// Function to close gate (NOISE ELIMINATED) - DEFAULT POSITION AT 6 O'CLOCK
void closeGate() {
  if (!servoAttached) {
    gateServo.attach(SERVO_PIN);
    servoAttached = true;
    delay(50);
  }
  
  gateServo.write(0); // Default/Closed position (6 o'clock - 0 degrees)
  delay(600); // Wait for movement
  
  // Detach to eliminate noise
  gateServo.detach();
  servoAttached = false;
  
  gateOpen = false;
  
  Serial.println("Gate closed");
}

String scanRFID() {
  // Check for new RFID cards
  if (!rfid.PICC_IsNewCardPresent()) {
    return "";
  }
  
  if (!rfid.PICC_ReadCardSerial()) {
    return "";
  }
  
  // Read and format UID
  String scannedUID = "";
  Serial.print("RFID Scanned: ");
  
  for (byte i = 0; i < rfid.uid.size; i++) {
    // Format UID properly
    if (rfid.uid.uidByte[i] < 0x10) {
      scannedUID += " 0";
    } else {
      scannedUID += " ";
    }
    scannedUID += String(rfid.uid.uidByte[i], HEX);
    
    // Print to Serial for debugging
    if (rfid.uid.uidByte[i] < 0x10) Serial.print(" 0");
    else Serial.print(" ");
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  
  // Convert to uppercase and remove leading space
  scannedUID.toUpperCase();
  scannedUID = scannedUID.substring(1);
  
  Serial.println();
  Serial.println("Formatted UID: " + scannedUID);
  
  // Halt PICC and stop encryption
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  
  return scannedUID;
}

long getDistance() {
  // Ultrasonic sensor reading
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long durationindigit = pulseIn(echoPin, HIGH);
  long distanceincm = (durationindigit / 5) / 29.1;
  
  return distanceincm;
}

void updateLEDs(String status) {
  // Turn off all LEDs first
  digitalWrite(LEDlampRed, LOW);
  digitalWrite(LEDlampYellow, LOW);
  digitalWrite(LEDlampGreen, LOW);
  
  // Turn on appropriate LED
  if (status == "DANGER") {
    digitalWrite(LEDlampRed, HIGH);
  } else if (status == "WARNING") {
    digitalWrite(LEDlampYellow, HIGH);
  } else { // SAFE or NO_OBJECT
    digitalWrite(LEDlampGreen, HIGH);
  }
}

void loop() {
  // Check if gate should auto-close
  if (gateOpen && (millis() - gateOpenTime > GATE_OPEN_DURATION)) {
    closeGate();
    
    // Reset system for next person
    accessGranted = false;
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Thank you!");
    lcd.setCursor(0, 1);
    lcd.print("System Ready");
    
    Serial.println("Gate closed - System reset for next person");
    delay(2000);
    
    // Clear screen and show ready message
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Access Control");
    lcd.setCursor(0, 1);
    lcd.print("Ready for scan");
  }
  
  // Check if access grant has expired
  if (accessGranted && (millis() - accessGrantedTime > ACCESS_TIMEOUT)) {
    accessGranted = false;
    Serial.println("Access permission expired");
  }
  
  // Get distance reading
  long distanceincm = getDistance();
  String currentStatus = "";
  
 
  if (distanceincm <= 5) {  
    currentStatus = "DANGER";
  } else if (distanceincm <= 10) { 
    currentStatus = "WARNING";
  } else if (distanceincm >= 20) {  
    currentStatus = "SAFE";
  } else {
    
    currentStatus = "NO_OBJECT";
  }
  
  // Check for RFID scan
  String scannedUID = scanRFID();
  if (scannedUID != "") {
    if (scannedUID == authorizedUID) {
      // Valid card scanned
      accessGranted = true;
      accessGrantedTime = millis();
      noTone(soundbuzzer); // Stop buzzer immediately
      buzzerActive = false;
      
      // Open gate when valid RFID is scanned
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Access Granted");
      lcd.setCursor(0, 1);
      lcd.print("Opening Gate...");
      
      delay(1000);
      openGate();
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Gate Open");
      lcd.setCursor(0, 1);
      lcd.print("Please pass");
      
      Serial.println("Access granted - gate opened");
      delay(2000);
    } else {
      // Invalid card
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Access Denied");
      lcd.setCursor(0, 1);
      lcd.print("Invalid Card");
      
      Serial.println("Access denied - invalid card");
      delay(2000);
    }
  }
  
  // Handle different zones
  if (currentStatus == "NO_OBJECT" || currentStatus == "SAFE") {
    // Green Zone - Safe area, nothing happens
    updateLEDs("SAFE");
    noTone(soundbuzzer);
    buzzerActive = false;
    
    if (currentStatus != lastStatus) {
      lcd.clear();
      lcd.setCursor(0, 0);
      if (currentStatus == "NO_OBJECT") {
        lcd.print("Area Clear");
      } else {
        lcd.print("Distance: ");
        lcd.print(distanceincm);
        lcd.print("cm");
      }
      lcd.setCursor(0, 1);
      lcd.print("    SAFE    ");
      
      Serial.print(distanceincm);
      Serial.println(" cm (SAFE)");
    }
    
  } else if (currentStatus == "WARNING") {
    // Yellow Zone - Warning, ask for RFID
    updateLEDs("WARNING");
    noTone(soundbuzzer); // No buzzer in yellow zone
    buzzerActive = false;
    
    if (currentStatus != lastStatus && !gateOpen) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Please scan");
      lcd.setCursor(0, 1);
      lcd.print("your key card");
      
      Serial.print(distanceincm);
      Serial.println(" cm (WARNING - Please scan RFID)");
    } else if (gateOpen) {
      // Show gate status while open with proper countdown
      unsigned long timeElapsed = millis() - gateOpenTime;
      if (timeElapsed < GATE_OPEN_DURATION) {
        unsigned long timeLeft = (GATE_OPEN_DURATION - timeElapsed) / 1000;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Gate Open");
        lcd.setCursor(0, 1);
        lcd.print("Closing in: ");
        lcd.print(timeLeft);
        lcd.print("s");
      }
    }
    
  } else if (currentStatus == "DANGER") {
    // Red Zone - Danger area
    updateLEDs("DANGER");
    
    if (accessGranted) {
      // Person has valid access - no alarm
      noTone(soundbuzzer);
      buzzerActive = false;
      
      if (currentStatus != lastStatus) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Authorized");
        lcd.setCursor(0, 1);
        lcd.print("Access");
        
        Serial.print(distanceincm);
        Serial.println(" cm (AUTHORIZED PASSAGE)");
      }
    } else {
      // Unauthorized person in danger zone - ALARM!
      if (!buzzerActive) {
        tone(soundbuzzer, 1000); // Start alarm
        buzzerActive = true;
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("UNAUTHORIZED!");
        lcd.setCursor(0, 1);
        lcd.print("Scan key card");
        
        Serial.print(distanceincm);
        Serial.println(" cm (DANGER - UNAUTHORIZED ACCESS!)");
      }
    }
  }
  
  // Update last status
  lastStatus = currentStatus;
  
  delay(300); // Main loop delay
}