#include <Servo.h>

const int IN1 = 2;
const int IN2 = 3;
const int ENA = 5;
const int SERVO_STEER_PIN = 9;
const int SERVO_SCAN_PIN  = 10;
const int TRIG_PIN = 11;
const int ECHO_PIN = 12;

const int S0 = 4;
const int S1 = 6;
const int S2 = 7;
const int S3 = 8;
const int TCS_OUT = 13;

const int STEER_CENTER = 90;
const int STEER_LEFT   = 65;
const int STEER_RIGHT  = 115;

const int SCAN_LEFT   = 150;
const int SCAN_RIGHT  = 30;
const int SCAN_CENTER = 90;

const uint8_t SPEED_CRUISE = 150;
const int OBSTACLE_CM = 20;
const int COLOR_DELTA = 20;

Servo servoSteer, servoScan;

void motorStop() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
}

void motorForward(uint8_t p) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, p);
}

long readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long us = pulseIn(ECHO_PIN, HIGH, 30000UL);
  if (us == 0) return 999;
  return (long)(us * 0.0343 / 2.0);
}

unsigned long tcsRead(char ch) {
  if (ch == 'r') {
    digitalWrite(S2, LOW); digitalWrite(S3, LOW);
  } else {
    digitalWrite(S2, HIGH); digitalWrite(S3, HIGH);
  }
  unsigned long t = pulseIn(TCS_OUT, LOW, 50000UL);
  if (t == 0) t = 50000UL;
  return t;
}

bool handleColorIfAny() {
  unsigned long r = tcsRead('r');
  unsigned long g = tcsRead('g');
  bool greenDominant = ((long)r - (long)g) > COLOR_DELTA;
  bool redDominant   = ((long)g - (long)r) > COLOR_DELTA;

  if (greenDominant) {
    motorStop();
    servoSteer.write(STEER_LEFT);  delay(400);
    motorForward(SPEED_CRUISE);    delay(700);
    motorStop();                   delay(150);
    servoSteer.write(STEER_CENTER);
    return true;
  } else if (redDominant) {
    motorStop();
    servoSteer.write(STEER_RIGHT); delay(400);
    motorForward(SPEED_CRUISE);    delay(700);
    motorStop();                   delay(150);
    servoSteer.write(STEER_CENTER);
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(9600);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(S0, OUTPUT); pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT); pinMode(S3, OUTPUT);
  pinMode(TCS_OUT, INPUT);
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  servoSteer.attach(SERVO_STEER_PIN);
  servoScan.attach(SERVO_SCAN_PIN);
  servoSteer.write(STEER_CENTER);
  servoScan.write(SCAN_CENTER);
  motorStop();
}

void loop() {
  if (handleColorIfAny()) {
    delay(50);
    return;
  }

  long d = readDistanceCM();
  if (d < OBSTACLE_CM) {
    motorStop(); delay(120);

    servoScan.write(SCAN_LEFT);  delay(220);
    long L = readDistanceCM();

    servoScan.write(SCAN_RIGHT); delay(220);
    long R = readDistanceCM();

    servoScan.write(SCAN_CENTER);

    if (L > R) servoSteer.write(STEER_LEFT);
    else       servoSteer.write(STEER_RIGHT);

    motorForward(SPEED_CRUISE);  delay(500);
    motorStop();                 delay(120);
    servoSteer.write(STEER_CENTER);
  } else {
    servoSteer.write(STEER_CENTER);
    motorForward(SPEED_CRUISE);
  }

  delay(20);
}
