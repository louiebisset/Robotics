// Ultrasonic radar: sweeps an HC-SR04 on an SG90 servo and prints "angle,distance_cm"
#include <Servo.h>

const int TRIG_PIN  = 9;
const int ECHO_PIN  = 10;
const int SERVO_PIN = 6;

const int MIN_ANGLE = 15;    // avoid the servo's end stops
const int MAX_ANGLE = 165;
const int STEP_DEG  = 2;     // smaller = finer map, slower sweep
const int SETTLE_MS = 30;    // let the servo stop moving before measuring

Servo servo;

// Returns distance in cm, or -1 if no echo was received
float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 25000UL);  // timeout about 4 m
  if (duration == 0) return -1;
  return duration * 0.0343 / 2.0;                              // speed of sound, out and back
}

// Median of 3 readings removes most single-reading spikes
float readMedian() {
  float a[3];
  for (int i = 0; i < 3; i++) {
    a[i] = readDistanceCm();
    delay(10);
  }
  if (a[0] > a[1]) { float t = a[0]; a[0] = a[1]; a[1] = t; }
  if (a[1] > a[2]) { float t = a[1]; a[1] = a[2]; a[2] = t; }
  if (a[0] > a[1]) { float t = a[0]; a[0] = a[1]; a[1] = t; }
  return a[1];
}

void scanAt(int angle) {
  servo.write(angle);
  delay(SETTLE_MS);
  float d = readMedian();
  Serial.print(angle);
  Serial.print(',');
  Serial.println(d, 1);
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  servo.attach(SERVO_PIN);
  servo.write(MIN_ANGLE);
  delay(500);
}

void loop() {
  for (int a = MIN_ANGLE; a <= MAX_ANGLE; a += STEP_DEG) scanAt(a);
  for (int a = MAX_ANGLE; a >= MIN_ANGLE; a -= STEP_DEG) scanAt(a);
}
