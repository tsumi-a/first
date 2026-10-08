#include <Arduino.h>

// пины кнопок (вторая нога кнопки на GND)
const int BUTTON_1 = 5;
const int BUTTON_2 = 14;
const int BUTTON_3 = 27;
const int BUTTON_4 = 26;

const int RED_LIGHT = 4;   // светодиод
const int RELAY = 17;      // реле
const int ADC_PIN = 34;    // фоторезистор

const int ADC_LIMIT = 1500;           // порог датчика, АЦП 12 бит (0-4095)
const int CODE_LEN = 4;
const int MAX_ERRORS = 3;             // сколько ошибок можно сделать подряд

const unsigned long DEBOUNCE_TIME = 30;     // антидребезг, мс
const unsigned long OPEN_TIME = 10000;      // сколько открыт замок
const unsigned long BLINK_TIME = 500;       // период мигания
const unsigned long PAUSE_AFTER_OPEN = 10000;
const unsigned long PAUSE_AFTER_ERROR = 3000;
const unsigned long PAUSE_BLOCK = 30000;    // блок после 3 ошибок

int code[CODE_LEN] = {1, 2, 3, 4};   // правильный код
int enteredCode[CODE_LEN];
int codePosition = 0;
int errors = 0;

// состояния программы
const int STATE_INPUT = 0;   // ждём ввод кода
const int STATE_OPEN = 1;    // замок открыт
const int STATE_PAUSE = 2;   // пауза

int state = STATE_INPUT;
unsigned long stateStart = 0;
unsigned long pauseTime = 0;

// для антидребезга кнопок
int buttonPins[4] = {BUTTON_1, BUTTON_2, BUTTON_3, BUTTON_4};
bool lastReading[4] = {HIGH, HIGH, HIGH, HIGH};
bool buttonState[4] = {HIGH, HIGH, HIGH, HIGH};
unsigned long lastChange[4] = {0, 0, 0, 0};

void setState(int newState) {
  state = newState;
  stateStart = millis();
}

void startPause(unsigned long time) {
  pauseTime = time;
  setState(STATE_PAUSE);
}

// возвращает true один раз, когда кнопку нажали
bool isPressed(int i) {
  bool reading = digitalRead(buttonPins[i]);
  if (reading != lastReading[i]) {
    lastReading[i] = reading;
    lastChange[i] = millis();
  }
  if (millis() - lastChange[i] > DEBOUNCE_TIME && reading != buttonState[i]) {
    buttonState[i] = reading;
    if (reading == LOW) {
      return true;
    }
  }
  return false;
}

bool checkCode() {
  for (int i = 0; i < CODE_LEN; i++) {
    if (enteredCode[i] != code[i]) {
      return false;
    }
  }
  return true;
}

void codeEntered() {
  codePosition = 0;
  if (checkCode()) {
    Serial.println("\nCorrect!");
    errors = 0;
    digitalWrite(RELAY, HIGH);
    setState(STATE_OPEN);
  } else {
    errors++;
    Serial.print("\nWrong code, errors: ");
    Serial.println(errors);
    if (errors >= MAX_ERRORS) {
      Serial.println("Too many errors, wait 30 sec");
      errors = 0;
      startPause(PAUSE_BLOCK);
    } else {
      startPause(PAUSE_AFTER_ERROR);
    }
  }
}

void inputMode() {
  digitalWrite(RED_LIGHT, HIGH);
  digitalWrite(RELAY, LOW);
  for (int i = 0; i < 4; i++) {
    if (isPressed(i)) {
      enteredCode[codePosition] = i + 1;
      codePosition++;
      Serial.print("* ");
      if (codePosition >= CODE_LEN) {
        codeEntered();
      }
      break;
    }
  }
}

void openMode() {
  unsigned long t = millis() - stateStart;
  if (t >= OPEN_TIME) {
    digitalWrite(RELAY, LOW);
    digitalWrite(RED_LIGHT, HIGH);
    startPause(PAUSE_AFTER_OPEN);
    return;
  }
  // мигаем светодиодом, без delay чтобы плата не вставала
  if ((t / BLINK_TIME) % 2 == 0) {
    digitalWrite(RED_LIGHT, LOW);
  } else {
    digitalWrite(RED_LIGHT, HIGH);
  }
}

void pauseMode() {
  if (millis() - stateStart >= pauseTime) {
    codePosition = 0;
    Serial.println("Enter 4 code:");
    setState(STATE_INPUT);
  }
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 4; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  pinMode(RED_LIGHT, OUTPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(ADC_PIN, INPUT);
  digitalWrite(RELAY, LOW);
  analogReadResolution(12);
  Serial.println("Enter 4 code:");
}

void loop() {
  int adcValue = analogRead(ADC_PIN);

  // если датчик ниже порога - реле включено, кнопки не работают
  if (adcValue <= ADC_LIMIT) {
    digitalWrite(RED_LIGHT, LOW);
    digitalWrite(RELAY, HIGH);
    codePosition = 0;
    return;
  }

  if (state == STATE_INPUT) {
    inputMode();
  } else if (state == STATE_OPEN) {
    openMode();
  } else {
    pauseMode();
  }
}
