#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40); // domyślny adres PCA9685

// --- Kanały PCA9685 ---
#define CH_X     0
#define CH_Y     1
#define CH_BTN   2
#define CH_BASE  3

// --- Funkcja do zamiany kąta (0–180°) na wartość PWM ---
int angleToPulse(int angle) {
  int pulselen = map(angle, 0, 180, 150, 600); 
  return pulselen;
}

// --- Piny wejściowe ---
int joyX = A0;
int joyY = A1;
int joySW = 2;       // joystick przycisk
int buttonRec = 3;   // guzik nagrywania
int buttonPlay = 4;  // guzik odtwarzania
int buttonReset = 5; // guzik resetu pozycji
int potBase = A2;    // potencjometr dla serwa bazowego

// --- Pozycje ---
int posX = 90;
int posY = 90;
int posBase = 90;
int deadZone = 100;

// --- Tablice do zapisu sekwencji ---
int seqX[200];
int seqY[200];
int seqBtn[200];
int seqBase[200];
int stepCount = 0;
bool recording = false;
bool playing = false;

// --- obsługa toggle przycisków ---
bool lastRecState = HIGH;
bool lastPlayState = HIGH;
bool lastResetState = HIGH;

// --- toggle joystick button ---
bool btnState = false;        
bool lastJoySWState = HIGH;

// --- odtwarzanie sekwencji ---
int playIndex = 0;
unsigned long lastPlayTime = 0;
int playDelay = 100;

// --- sterowanie serwami w trybie joysticka ---
unsigned long lastMoveTime = 0;
int moveInterval = 5;

void setup() {
  Wire.begin();
  pwm.begin();
  pwm.setPWMFreq(60); // serwa standard 50–60Hz

  pinMode(joySW, INPUT_PULLUP);
  pinMode(buttonRec, INPUT_PULLUP);
  pinMode(buttonPlay, INPUT_PULLUP);
  pinMode(buttonReset, INPUT_PULLUP);

  // ustaw pozycje startowe
  setServo(CH_X, posX);
  setServo(CH_Y, posY);
  setServo(CH_BTN, 90);
  setServo(CH_BASE, posBase);
}

void loop() {
  unsigned long currentTime = millis();

  // --- JOYSTICK BUTTON toggle ---
  bool currentJoySW = digitalRead(joySW);
  if (lastJoySWState == HIGH && currentJoySW == LOW) {
    btnState = !btnState;
    setServo(CH_BTN, btnState ? 0 : 90);
    delay(200);
  }
  lastJoySWState = currentJoySW;

  // --- RECORD toggle ---
  bool currentRecState = digitalRead(buttonRec);
  if (lastRecState == HIGH && currentRecState == LOW) {
    if (!recording) {
      stepCount = 0;
      recording = true;
    } else {
      recording = false;
    }
    delay(200);
  }
  lastRecState = currentRecState;

  // --- PLAY toggle ---
  bool currentPlayState = digitalRead(buttonPlay);
  if (lastPlayState == HIGH && currentPlayState == LOW) {
    if (!playing) {
      playing = true;
      playIndex = 0;
    } else {
      playing = false;
    }
    delay(200);
  }
  lastPlayState = currentPlayState;

  // --- RESET pozycji ---
  bool currentResetState = digitalRead(buttonReset);
  if (lastResetState == HIGH && currentResetState == LOW) {
    posX = 90;
    posY = 90;
    posBase = 90;
    btnState = false;
    setServo(CH_X, posX);
    setServo(CH_Y, posY);
    setServo(CH_BTN, 90);
    setServo(CH_BASE, posBase);
    delay(200);
  }
  lastResetState = currentResetState;

  // --- Tryb nagrywania ---
  if (recording) {
    sterowanieJoystickiem(currentTime);
    sterowanieBaza();

    if (stepCount < 200 && (currentTime - lastPlayTime >= playDelay)) {
      seqX[stepCount] = posX;
      seqY[stepCount] = posY;
      seqBtn[stepCount] = (btnState ? 0 : 90);
      seqBase[stepCount] = posBase;
      stepCount++;
      lastPlayTime = currentTime;
    }
  }
  // --- Tryb odtwarzania ---
  else if (playing) {
    if (currentTime - lastPlayTime >= playDelay) {
      if (playIndex < stepCount) {
        setServo(CH_X, seqX[playIndex]);
        setServo(CH_Y, seqY[playIndex]);
        setServo(CH_BTN, seqBtn[playIndex]);
        setServo(CH_BASE, seqBase[playIndex]);
        playIndex++;
      } else {
        playing = false;
      }
      lastPlayTime = currentTime;
    }
  }
  // --- Tryb normalny ---
  else {
    sterowanieJoystickiem(currentTime);
    sterowanieBaza();
  }
}

// --- Sterowanie joystickiem ---
void sterowanieJoystickiem(unsigned long currentTime) {
  if (currentTime - lastMoveTime >= moveInterval) {
    int valX = analogRead(joyX);
    if (valX < (512 - deadZone)) { posX--; if (posX < 0) posX = 0; }
    else if (valX > (512 + deadZone)) { posX++; if (posX > 180) posX = 180; }
    setServo(CH_X, posX);

    int valY = analogRead(joyY);
    if (valY < (512 - deadZone)) { posY--; if (posY < 0) posY = 0; }
    else if (valY > (512 + deadZone)) { posY++; if (posY > 180) posY = 180; }
    setServo(CH_Y, posY);

    lastMoveTime = currentTime;
  }
}

// --- Sterowanie bazą potencjometrem ---
void sterowanieBaza() {
  int potValue = analogRead(potBase);
  posBase = map(potValue, 0, 1023, 0, 180);
  setServo(CH_BASE, posBase);
}

// --- Funkcja sterująca serwem przez PCA9685 ---
void setServo(uint8_t channel, int angle) {
  pwm.setPWM(channel, 0, angleToPulse(angle));
}
