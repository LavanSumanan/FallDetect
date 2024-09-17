#include <Wire.h>
#include <LSM6.h>
#include <SoftwareSerial.h>

#define ledPin 13
#define fallLedPin 2
#define btLedPin 3
#define velostatPin A0
#define tiltPin 7

#define safe 1
#define fall 2
#define notconnected 3

SoftwareSerial bt(10, 11); /* (Tx,Rx) */

int maxRoc = 0;

// test setup
unsigned long ledPrevTime = 0;

// velostat setup
const int velostatThreshold = 300;
const int velostatHoldTimeMs = 1000;
unsigned long velostatPrevTime = 0;

// gyroscope setup
LSM6 imu;
const int bufferSize = 10;
float gyroSamples[bufferSize] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const int rocBufferSize = 20;
bool rocAboveThreshold[rocBufferSize];
int rocIndex = 0;
const long rocThreshold = 100000;

// input states
bool btConnected = 0;
bool held = 0;
bool tilted = 0;
bool falling = 0;
// derived input states
bool fallen = 0;
bool stuck = 0;
// pending alert states
unsigned long alertPrevTime = 0;
bool done = 0;
// system state
int state = 0; // 0: safe, 1: pending, 2: alert
int prevState = 0;

// delays
const int MsPerS = 1000;
const int pendingDelayInMs = 5 * MsPerS; // TODO: Change from 5 to 30 for final product
const int delayInMs = 50;

bool containsOne(bool arr[], int size)
{
  for (int i = 0; i < size; i++)
  {
    if (arr[i])
      return true;
  }
  return false;
}

void shiftArray(float arr[], int size, float newVal)
{
  for (int i = size - 1; i > 0; i--)
  {
    arr[i] = arr[i - 1];
  }
  arr[0] = newVal;
}

void setup()
{
  // testing
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(fallLedPin, OUTPUT);
  pinMode(btLedPin, OUTPUT);
  // velostat
  pinMode(velostatPin, INPUT);
  // tilt
  pinMode(tiltPin, INPUT);
  // gyroscopes
  Wire.begin();
  if (!imu.init())
  {
    Serial.println("Failed to detect and initialize IMU!");
    while (1)
      ;
  }
  imu.enableDefault();
  // bluetooth
  bt.begin(9600); /* Define baud rate for software serial communication */
  Serial.println("Program Started");
}

void loop()
{
  // ######## GET INPUT ########
  while (bt.available())
  {
    int btRead = bt.read();
    btConnected = btRead == 0;
    Serial.print("Received:");
    Serial.print(btRead);
    if (btConnected)
    {
      bt.write(safe);
    }
    else
    {
      bt.write(notconnected);
    }
  }

  // velostat
  int velostatReading = analogRead(velostatPin);
  // tilt
  int tilt_val = digitalRead(tiltPin);
  // gyroscopes
  imu.read();
  float gyro_magnitude = abs(imu.g.x) + abs(imu.g.z);
  shiftArray(gyroSamples, bufferSize, gyro_magnitude);
  float a_avg = 0;
  float b_avg = 0;
  float sliding_roc = 0;
  for (int i = 0; i < bufferSize / 2; i++)
  {
    a_avg += gyroSamples[i];
  }
  a_avg /= bufferSize;
  for (int i = bufferSize / 2; i < bufferSize; i++)
  {
    b_avg += gyroSamples[i];
  }
  b_avg /= bufferSize;
  sliding_roc = abs(a_avg - b_avg) / delayInMs * MsPerS * (bufferSize / 2);

  if (sliding_roc > maxRoc)
    maxRoc = sliding_roc; // TESTING

  rocAboveThreshold[rocIndex] = sliding_roc > rocThreshold;
  rocIndex = (rocIndex + 1) % rocBufferSize;

  // ######## DETERMINE STATES ########
  if (!held && velostatReading > velostatThreshold)
  {
    held = 1;
    velostatPrevTime = millis();
  }
  else if (held && millis() - velostatPrevTime > velostatHoldTimeMs && velostatReading < velostatThreshold)
  {
    held = 0;
  }
  tilted = tilt_val == 0;
  falling = containsOne(rocAboveThreshold, rocBufferSize);
  // derived states
  fallen = !held && tilted && falling;
  stuck = !held && tilted;

  switch (state)
  {
  case 0:
    prevState = 0;
    if (!fallen)
      break;
    else
    {
      state = 1;
      alertPrevTime = millis();
    }
    break;
  case 1:
    prevState = 1;
    if (!done)
      break;
    else
    {
      if (stuck)
        state = 2;
      else
        state = 0;
    }
    break;
  case 2:
    prevState = 2;
    if (stuck)
      break;
    state = 0;
    break;
  }

  done = millis() - alertPrevTime > pendingDelayInMs;

  // ######## TRIGGER OUTPUTS ########
  if (state == 0 && prevState != 0)
  {
    bt.write(safe);
  }

  if (state == 2 && prevState != 2)
  {
    ledPrevTime = millis();
    bt.write(fall);
  }

  digitalWrite(ledPin, state == 1);
  digitalWrite(fallLedPin, state == 1);
  digitalWrite(btLedPin, btConnected);

  // ######## CALIBRATION ########
  Serial.print("sliding_roc:");
  Serial.print(sliding_roc);
  Serial.print("maxRoc:");
  Serial.print(maxRoc);

  Serial.print("velostatReading:");
  Serial.print(velostatReading);
  // Serial.print("gyro_magnitude:");
  // Serial.print(gyro_magnitude);

  // ######## TEST OUTPUT ########
  Serial.print("held:");
  Serial.print(held);
  Serial.print("tilted:");
  Serial.print(tilted);
  Serial.print("falling:");
  Serial.print(falling);
  Serial.print("done:");
  Serial.print(done);
  Serial.print("state:");
  Serial.println(state);

  delay(delayInMs);
}
