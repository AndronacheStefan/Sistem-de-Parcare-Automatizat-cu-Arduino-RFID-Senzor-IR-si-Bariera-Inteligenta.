#include <SPI.h>
#include <Servo.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Buzzer.h>

#define SS_PIN 10 // Pinul SS pentru modulul RFID
#define RST_PIN 7 // Pinul de reset pentru modulul RFID
#define servoPin 9 // Pinul pentru servomotor
#define IR 3 // Pinul pentru senzorul IR de intrare
#define GREEN 2 // Pinul pentru LED-ul verde
#define RED 6 // Pinul pentru LED-ul roșu
#define YELLOW 5 // Pinul pentru LED-ul galben
#define IR1 8 // Pinul pentru senzorul IR de ieșire
#define buzzer A3 // Pinul pentru buzzer

bool exitRequested = false;
int flag1 = 0;
int flag2 = 0;
int unghi = 0;

Servo servo;
MFRC522 mfrc522(SS_PIN, RST_PIN); // Instance of MFRC522
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522::MIFARE_Key key;

void setup() {
  checkExitSensor();
  Serial.begin(9600);
  SPI.begin(); // Initialize SPI
  mfrc522.PCD_Init(); // Initialize RC522
  lcd.init();
  servo.attach(servoPin);
  pinMode(IR, INPUT);
  pinMode(IR1, INPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(RED, OUTPUT);
}

void loop() {
  checkExitSensor();
  lcd.setCursor(0, 0);
  lcd.print("Put card on rdr ");
  lcd.backlight();
  
  // Turn yellow LED on to indicate waiting for a card
  digitalWrite(YELLOW, HIGH);
  
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Display UID on serial monitor
  Serial.print("UID tag :");
  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  
  // Verify UID for authorized access
  content.toUpperCase();
  if (content.substring(1) == "83 D2 7F 35") { // Change to your authorized UID
    lcd.clear();
    Serial.println("Authorized access");
    tone(buzzer, 4000);
    lcd.setCursor(0, 0);
    lcd.print("Access Granted");
    
    // Turn yellow LED off, green LED on for access granted
    digitalWrite(YELLOW, LOW);
    delay(100); // Short delay to ensure LED state change
    digitalWrite(GREEN, HIGH);

    // Move servo to open position
    for (unghi = 90; unghi >= 0; unghi--) {
      servo.write(unghi);
      delay(5);
    }

    // Wait until IR sensor detects object passing through
    while (digitalRead(IR) != 0) {
      // Waiting
    }
    delay(5000); // Additional delay for security
    noTone(buzzer);
    
    // Close the servo
    for (unghi = 0; unghi <= 90; unghi++) {
      servo.write(unghi);
      delay(5);
    }
    
    // Reset LED states
    digitalWrite(GREEN, LOW);
    digitalWrite(YELLOW, HIGH);
  } else {
    lcd.clear();
    Serial.println("Access denied");
    
    // Turn yellow LED off, red LED on for access denied
    digitalWrite(YELLOW, LOW);
    delay(100); // Short delay to ensure LED state change
    digitalWrite(RED, HIGH);

    lcd.setCursor(0, 0);
    lcd.print("Access denied.");
    delay(3000);
    
    // Reset LED states
    digitalWrite(RED, LOW);
    digitalWrite(YELLOW, HIGH);
  }
  lcd.clear();
}

void checkExitSensor() {
  if (digitalRead(IR1) == LOW && !exitRequested) {
    exitRequested = true; // Set flag to avoid repeated triggering
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Exit Open");
    tone(buzzer, 4000);
    digitalWrite(YELLOW, LOW);
    delay(100); // Short delay to ensure LED state change
    digitalWrite(GREEN, HIGH);

    for (unghi = 90; unghi >= 0; unghi--) {
      servo.write(unghi);
      delay(5);
    }

    while (digitalRead(IR) != 0) {
      // Waiting
    }
    delay(3000);
    noTone(buzzer);

    for (unghi = 0; unghi <= 90; unghi++) {
      servo.write(unghi);
      delay(5);
    }
    digitalWrite(GREEN, LOW);
    delay(100); // Short delay to ensure LED state change
    digitalWrite(YELLOW, HIGH);
    lcd.clear();
    exitRequested = false; // Reset flag for next exit detection
  }
}
