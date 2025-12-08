#include <R503Lib.h>

#define fpsSerial Serial1
R503Lib fps(&fpsSerial, 44, 43, 0xFFFFFFFF);

const int UNLOCK_PIN0 = 10;  // FeatherS2 pin 10 (GPIO10) orange -> ATMega PC1
const int UNLOCK_PIN1 = 11;  // FeatherS2 pin 11 (GPIO11) yellow -> ATMega PC2
const int RESET_PIN = 7;   // FeatherS2 pin 7 (GPIO11) white -> ATMega PC0
// rx = yellow
// tx = purple


void setup() {
  Serial.begin(115200);

  pinMode(UNLOCK_PIN0, OUTPUT);
  digitalWrite(UNLOCK_PIN0, LOW);
  pinMode(UNLOCK_PIN1, OUTPUT);
  digitalWrite(UNLOCK_PIN1, LOW);
  pinMode(RESET_PIN, INPUT);


  Serial1.begin(57600, SERIAL_8N1, 44, 43);
  delay(200);

  if (fps.begin(57600, 0x0) != R503_OK) {
    Serial.println("sensor error");
    while (1) delay(10);
  }

  fps.setAuraLED(aLEDBreathing, aLEDBlue, 50, 255);
  Serial.println("ready");
}

void loop() {
  // checks reset pin
  if (digitalRead(RESET_PIN) == HIGH) {
      digitalWrite(UNLOCK_PIN0, LOW);
      digitalWrite(UNLOCK_PIN1, LOW);

      // reset LED to idle
      fps.setAuraLED(aLEDBreathing, aLEDBlue, 50, 255);

      // soft-reset internal R503 state machine
      // fps.softReset();
      // delay(200);

      Serial.println("RESET received -> returning to idle state");
      
      // Wait for reset line to go LOW again
      while (digitalRead(RESET_PIN) == HIGH) {
        delay(10);
        }
      
      return;
    }

  int ret = fps.takeImage();

  if (ret == R503_NO_FINGER) {
    delay(50);
    return;
  }

  if (ret != R503_OK) {
    Serial.printf("takeImage err 0x%02X\n", ret);
    return;
  }

  Serial.println("finger detected");
  fps.setAuraLED(aLEDBreathing, aLEDYellow, 120, 255);

  ret = fps.extractFeatures(1);
  if (ret != R503_OK) {
    Serial.printf("extract err 0x%02X\n", ret);
    fps.setAuraLED(aLEDRed, aLEDBreathing, 100, 255);
    return;
  }

  uint16_t id, conf;
  ret = fps.searchFinger(1, id, conf);
  // id0 = 01
  if (ret == R503_OK && id == 0) {
    Serial.printf("AUTHORIZED: ID %d\n", id);
    fps.setAuraLED(aLEDBreathing, aLEDGreen, 255, 255);
    digitalWrite(UNLOCK_PIN0, LOW);  // stay low
    digitalWrite(UNLOCK_PIN1, HIGH);   // stay high
  }
  // id1 = 10
  else if (ret == R503_OK && id == 1) {
    Serial.printf("AUTHORIZED: ID %d\n", id);
    fps.setAuraLED(aLEDBreathing, aLEDGreen, 255, 255);
    digitalWrite(UNLOCK_PIN0, HIGH);   // stay high
    digitalWrite(UNLOCK_PIN1, LOW);  // stay low
  }
  // id2 = 11
  else if (ret == R503_OK && id == 2) {
    Serial.printf("AUTHORIZED: ID %d\n", id);
    fps.setAuraLED(aLEDBreathing, aLEDGreen, 255, 255);
    digitalWrite(UNLOCK_PIN0, HIGH);  // stay high
    digitalWrite(UNLOCK_PIN1, HIGH);  // stay high
  }
  // no match = 00
  else {
    Serial.println("unauthorized or no match");
    fps.setAuraLED(aLEDBreathing, aLEDRed, 255, 255);
  }

  delay(300);
}
