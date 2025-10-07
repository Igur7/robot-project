#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40); 

#define CH_X     0
#define CH_Y     1
#define CH_BTN   2
#define CH_BASE  3

int angleToPulse(byte angle) {
  int pulselen = map(angle, 0, 180, 150, 600);
  return pulselen;
}

int joyX = A0;
int joyY = A1;
int joySW = 2;
int buttonRec = 3;
int buttonPlay = 4;
int buttonReset = 5;
int potBase = A2;

byte posX = 90;
byte posY = 90;
byte posBase = 90;
int deadZone = 100;

#define MAX_STEPS 128
byte seqX[MAX_STEPS];
byte seqY[MAX_STEPS];
byte seqBtn[MAX_STEPS];
byte seqBase[MAX_STEPS];
uint16_t seqDelay[MAX_STEPS];  

int stepCount = 0;
bool recording = false;
bool playing = false;

bool lastRecState = HIGH;
bool lastPlayState = HIGH;
bool lastResetState = HIGH;

bool btnState = false;
bool lastJoySWState = HIGH;

int playIndex = 0;
unsigned long lastPlayTime = 0;
unsigned long lastStepTime = 0;  
int playDelay = 100;

unsigned long lastMoveTime = 0;
int moveInterval = 30;

void setup() {
  Wire.begin();
  pwm.begin();
  pwm.setPWMFreq(60);

  pinMode(joySW, INPUT_PULLUP);
  pinMode(buttonRec, INPUT_PULLUP);
  pinMode(buttonPlay, INPUT_PULLUP);
  pinMode(buttonReset, INPUT_PULLUP);

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
      lastStepTime = currentTime;   
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
      lastPlayTime = currentTime;
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

    if (stepCount < MAX_STEPS && (currentTime - lastPlayTime >= playDelay)) {
      if (stepCount == 0 || 
          seqX[stepCount-1] != posX || 
          seqY[stepCount-1] != posY || 
          seqBtn[stepCount-1] != (btnState ? 0 : 90) || 
          seqBase[stepCount-1] != posBase) {
        
        // zapis pozycji
        seqX[stepCount] = posX;
        seqY[stepCount] = posY;
        seqBtn[stepCount] = (btnState ? 0 : 90);
        seqBase[stepCount] = posBase;

        // zapis czasu od poprzedniego kroku
        seqDelay[stepCount] = (uint16_t)(currentTime - lastStepTime);
        lastStepTime = currentTime;

        stepCount++;
      }
      lastPlayTime = currentTime;
    }
  }
  // --- Tryb odtwarzania ---
  else if (playing) {
    if (playIndex < stepCount && (currentTime - lastPlayTime >= seqDelay[playIndex])) {
      setServo(CH_X, seqX[playIndex]);
      setServo(CH_Y, seqY[playIndex]);
      setServo(CH_BTN, seqBtn[playIndex]);
      setServo(CH_BASE, seqBase[playIndex]);
      
      lastPlayTime = currentTime;
      playIndex++;
    } else if (playIndex >= stepCount) {
      playing = false;
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
    if (valX < (512 - deadZone)) { if (posX > 0) posX--; }
    else if (valX > (512 + deadZone)) { if (posX < 180) posX++; }
    setServo(CH_X, posX);

    int valY = analogRead(joyY);
    if (valY < (512 - deadZone)) { if (posY > 0) posY--; }
    else if (valY > (512 + deadZone)) { if (posY < 180) posY++; }
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

// --- Sterowanie serwem ---
void setServo(uint8_t channel, byte angle) {
  pwm.setPWM(channel, 0, angleToPulse(angle));
}
