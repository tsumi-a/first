#include <Arduino.h>

const int BUTTON_1 = 5;
const int BUTTON_2 = 14;
const int BUTTON_3 = 27;
const int BUTTON_4 = 26;
const int RED_LIGHT = 4;
const int RELAY = 17;
const int ADC_PIN = 34;

int code[] = {1, 2, 3, 4};
int enteredCode[4];
int codePosition = 0;

void resetCode() {
  codePosition = 0;
}

void checkCode() {
  bool correct = true;
  
  for (int i = 0; i < 4; i++) {
    if (enteredCode[i] != code[i]) {
      correct = false;
      break;
    }
  }
  
  if (correct) {
    Serial.println("\nCorrect!.");
    digitalWrite(RELAY, HIGH);
    for (int i = 0; i < 10; i++) {
      digitalWrite(RED_LIGHT, LOW);
      delay(500);
      digitalWrite(RED_LIGHT, HIGH);
      delay(500);
    }
    digitalWrite(RELAY, LOW);
    delay(10000);
  } else {
    Serial.println("\nEnter 4 code:");
    delay(3000);
  }

  resetCode();
}

void handleButtonPress(int buttonNumber) {
  enteredCode[codePosition] = buttonNumber;
  codePosition++;
  
  Serial.print("* ");
  
  if (codePosition >= 4) {
    checkCode();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_1, INPUT);
  pinMode(BUTTON_2, INPUT);
  pinMode(BUTTON_3, INPUT);
  pinMode(BUTTON_4, INPUT);
  pinMode(RED_LIGHT, OUTPUT);
  pinMode(RELAY, OUTPUT);
    pinMode(ADC_PIN, INPUT);
  digitalWrite(RELAY, LOW);
  Serial.println("Enter 4 code");
  analogReadResolution(12);
}

void loop() {
  int adcValue = analogRead(ADC_PIN);
//   Serial.println(adcValue);
  if(adcValue > 1500) {
    digitalWrite(RED_LIGHT, HIGH);
    digitalWrite(RELAY, LOW);
    
    if (digitalRead(BUTTON_1) == LOW) {
      handleButtonPress(1);
      delay(300);
    }
    else if (digitalRead(BUTTON_2) == LOW) {
      handleButtonPress(2);
      delay(300);
    }
    else if (digitalRead(BUTTON_3) == LOW) {
      handleButtonPress(3);
      delay(300);
    }
    else if (digitalRead(BUTTON_4) == LOW) {
      handleButtonPress(4);
      delay(300);
    }
  } else {
    digitalWrite(RED_LIGHT, LOW);
    digitalWrite(RELAY, HIGH);
  }
  
  delay(50);
}
