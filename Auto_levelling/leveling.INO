// Self-levelling platform: an MPU6050 (GY-521) measures tilt and a PID controller
// moves an SG90 servo to keep the platform level.
#include <Wire.h>
#include <Servo.h>

const uint8_t MPU_ADDR  = 0x68;
const int     SERVO_PIN = 9;

// PID gains: tune these 
float Kp = 5.0;
float Ki = 1.0;
float Kd = 0.15;

const float SETPOINT   = 0.0;   // target tilt in degrees (0 = level)
const int   DIRECTION  = 1;     // change to -1 if the platform corrects the wrong way
const int   CENTER_DEG = 90;    // servo position when the platform is level
const int   MAX_OFFSET = 60;    // servo limited to CENTER_DEG +/- this
const float ALPHA      = 0.98;  // complementary filter: 98% gyro, 2% accelerometer
const float I_LIMIT    = 30.0;  // anti-windup limit on the integral term

Servo servo;
float gyroOffset = 0;           // deg/s, measured at startup
float angle = 0;                // filtered tilt in degrees
float integral = 0;
unsigned long lastMicros;

int16_t read16() {
  int16_t hi = Wire.read();
  int16_t lo = Wire.read();
  return (hi << 8) | lo;
}

// Reads the accelerometer tilt angle and the gyro rate about the X axis
bool readIMU(float &accAngle, float &gyroDps) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);                                    // first accel register
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((int)MPU_ADDR, 14) != 14) return false;

  read16();                                            // accel X (not used)
  int16_t ay = read16();
  int16_t az = read16();
  read16();                                            // temperature
  int16_t gx = read16();
  read16();                                            // gyro Y
  read16();                                            // gyro Z

  accAngle = atan2((float)ay, (float)az) * 180.0 / PI; // tilt about the X axis
  gyroDps  = gx / 131.0;                               // +/-250 deg/s range
  return true;
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);                                    // power management register
  Wire.write(0);                                       // wake the MPU6050
  if (Wire.endTransmission() != 0) {
    Serial.println("MPU6050 not found - check wiring");
    while (1);
  }

  servo.attach(SERVO_PIN);
  servo.write(CENTER_DEG);
  delay(1000);

  // Calibrate the gyro: keep the platform still for about 2 seconds
  float sum = 0, a, g;
  int n = 0;
  for (int i = 0; i < 300; i++) {
    if (readIMU(a, g)) { sum += g; n++; }
    delay(5);
  }
  if (n > 0) gyroOffset = sum / n;

  readIMU(angle, g);                                   // start the filter from the accelerometer angle
  lastMicros = micros();
}

void loop() {
  float accAngle, gyroDps;
  if (!readIMU(accAngle, gyroDps)) return;
  gyroDps -= gyroOffset;

  unsigned long now = micros();
  float dt = (now - lastMicros) / 1000000.0;
  lastMicros = now;

  // Complementary filter: gyro is smooth but drifts, accelerometer is noisy but stable
  angle = ALPHA * (angle + gyroDps * dt) + (1.0 - ALPHA) * accAngle;

  // PID: the derivative term uses the gyro rate directly, which is less noisy
  float error = SETPOINT - angle;
  integral += error * dt;
  integral = constrain(integral, -I_LIMIT, I_LIMIT);
  float output = Kp * error + Ki * integral - Kd * gyroDps;
  output = constrain(output, -MAX_OFFSET, MAX_OFFSET);

  int servoCmd = CENTER_DEG + DIRECTION * (int)round(output);
  servo.write(servoCmd);

  // Open Tools > Serial Plotter (115200 baud) to see these as live graphs
  Serial.print("Angle:");    Serial.print(angle, 1);
  Serial.print(",Setpoint:"); Serial.print(SETPOINT, 1);
  Serial.print(",Output:");   Serial.println(output, 1);

  delay(10);
}
