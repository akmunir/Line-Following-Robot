#include <Arduino.h>
#include <QTRSensors.h>
#include <EEPROM.h>

QTRSensors qtr;
uint8_t E1 = 5;
uint8_t M1 = 4;
uint8_t E2 = 6;
uint8_t M2 = 7;
constexpr uint8_t leftSensor = 9;
constexpr uint8_t leftMSensor = 10;
constexpr uint8_t rightMSensor = 11;
constexpr uint8_t rightSensor = 12;
constexpr uint8_t sensorCount = 4;
int defaultSpeed = 190;
int32_t targetSensorReading = 1000;
int32_t sensorList[sensorCount] = {leftSensor, leftMSensor, rightMSensor, rightSensor};
uint16_t sensorValues[sensorCount] = {leftSensor, leftMSensor, rightMSensor, rightSensor};
int32_t lastError = 0;
int32_t lastTurnError = 0;
uint8_t controlInterval = 10;
int lastControlinterval = 0;
float pTerm = 0;
float iTerm = 0;
int leftSensorOffset = 53;
int middleLeftSensorOffset = 13;
int middleRightSensorOffset = -61;
int rightSensorOffset = 6;
bool leftTurnFlag = false;
bool readLineSensor(int32_t sensorPin);
void motorController(int32_t pwmValue, int32_t directionL, int32_t directionR, bool isTurn);
void moveLeft(int32_t pwmValue);
void moveForward(int32_t pwmValue);
void moveRight(int32_t pwmValue);
void motorPidController();

class Motor
{
public:
  uint8_t pwmPin;
  uint8_t dirPin;
  Motor(uint8_t pwmPin_, uint8_t dirPin_)
  {
    pwmPin = pwmPin_;
    dirPin = dirPin_;
  }
  void begin()
  {
    pinMode(pwmPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
    digitalWrite(pwmPin, LOW);
    digitalWrite(dirPin, LOW);
  }
  void move(int pwmValue, bool direction)
  {
    digitalWrite(pwmPin, LOW);
    digitalWrite(dirPin, direction);
    analogWrite(pwmPin, constrain(abs(pwmValue), 0, 255));
  }
  void stop()
  {
    digitalWrite(pwmPin, LOW);
    digitalWrite(dirPin, LOW);
    analogWrite(pwmPin, 0);
  }
};

Motor leftMotor(E2, M2);
Motor rightMotor(E1, M1);

void setup()
{
  // Set Timer3 (controls pin 5 / E1) prescaler to 1 → ~31kHz PWM
  // TCCR3B = (TCCR3B & 0b11111000) | 0x01;

  // Set Timer4 (controls pin 6 / E2) prescaler to 1 → ~31kHz PWM
  // TCCR4B = (TCCR4B & 0b11111000) | 0x01;
  Serial.begin(115200);
  leftMotor.begin();
  rightMotor.begin();
  qtr.setTypeRC();
  qtr.setSensorPins((const uint8_t[]){leftSensor, leftMSensor, rightMSensor, rightSensor}, 4);
  qtr.setEmitterPin(2);
  delay(500);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // turn on Arduino's LED to indicate we are in calibration mode
  // 2.5 ms RC read timeout (default) * 10 reads per calibrate() call
  // = ~25 ms per calibrate() call.
//   Serial.println("begin calibration");
//   delay(5000);
//   for (uint16_t i = 0; i < 400; i++)

//   {
//     Serial.println(i);
//     qtr.calibrate();
//   }
//   digitalWrite(LED_BUILTIN, LOW); // turn off Arduino's LED to indicate we are through with calibration

//  for (uint8_t i = 0; i < sensorCount; i++) {
//   EEPROM.put(i * 2, qtr.calibrationOn.minimum[i]);
//   Serial.print(qtr.calibrationOn.minimum[i]);
//   Serial.print(' ');
// }
// Serial.println();

// for (uint8_t i = 0; i < sensorCount; i++) {
//   EEPROM.put(sensorCount * 2 + i * 2, qtr.calibrationOn.maximum[i]);
//   Serial.print(qtr.calibrationOn.maximum[i]);
//   Serial.print(' ');
// }
// Serial.println();
// delay(1000);
Serial.println("print EPROM");
//delay(3000);
  qtr.calibrate();
  // MAX
  for (uint8_t i = 0; i < sensorCount; i++)
  {
    uint16_t val;
    EEPROM.get(i * 2, val);
    qtr.calibrationOn.minimum[i] = val;
    Serial.print(val);
    Serial.print(' ');
  }
  Serial.println();
  // MIN
  for (uint8_t i = 0; i < sensorCount; i++)
  {
    uint16_t val;
    EEPROM.get(sensorCount * 2 + i * 2, val);
    qtr.calibrationOn.maximum[i] = val;
    Serial.print(val);
    Serial.print(' ');
  }
  Serial.println();
  //delay(3000);
}

void loop()
{
  if(millis() - lastControlinterval >= controlInterval) {
    lastControlinterval += controlInterval;
  }
  qtr.readCalibrated(sensorValues);
  for (int i = 0; i < sizeof(sensorValues); i++)
  {
    // Serial.print(sensorValues[i]);
    // Serial.print("\t");
  }
  if (sensorValues[0] == 1000 || sensorValues[1] == 1000 || sensorValues[2] == 1000 || sensorValues[3] > 900)
  {
    motorPidController();
  }
  else
  {
    leftMotor.stop();
    rightMotor.stop();
 
  }
  Serial.println();
}



void motorController(int32_t pwmValue, int32_t directionL, int32_t directionR, bool isTurn)
{
  if (!isTurn) {
    leftMotor.move(-pwmValue + defaultSpeed, directionL);
    rightMotor.move(pwmValue + defaultSpeed, directionR);
  } else {
    leftMotor.move(-pwmValue, directionL);
    rightMotor.move(pwmValue, directionR);
  }
  
}


void motorPidController()
{
  uint32_t leftSensorAdj = sensorValues[0];
  uint32_t leftMSensorAdj = sensorValues[1];
  uint32_t rightMSensorAdj = sensorValues[2];
  uint32_t rightSensorAdj = sensorValues[3];
  
  float kp;
  float ki;
  float kd;
  int error;
  int tempError;
  int turnError;

  if (sensorValues[0] >= 900 || sensorValues[3] >= 900)
  {
    //Serial.println("turning");
    kp = 0.3;
    ki = 0;
    kd = 0;
    //Serial.println(sensorValues[3]);
    turnError = leftSensorAdj - rightSensorAdj;
    float pTerm = kp * turnError;
    iTerm += ki * (turnError + lastTurnError) / 2;
    iTerm = constrain(iTerm, -255, 255);
    float dTerm = kd * (turnError - lastTurnError);
    int32_t controllerOutput = pTerm + iTerm + dTerm;
    controllerOutput = constrain(controllerOutput, -255, 255);
    lastTurnError = turnError;
    //Serial.println(controllerOutput);
    if (leftMSensorAdj >= 900) {
      motorController(controllerOutput, 0, 1, true);
      //Serial.println("moving left");
      Serial.println(turnError);
    } else if (rightSensorAdj >= 900) {
      motorController(controllerOutput, 1, 0, true);
      Serial.println("moving right");
      Serial.println(turnError);
      
    } else {

    }
  }
  else
  {
    //Serial.println("moving straight");
    kp = 0.4;
    ki = 0;
    kd = 0.5;
    error = sensorValues[1] - sensorValues[2];
    float pTerm = kp * error;
    iTerm += ki * (error + lastError) / 2;
    iTerm = constrain(iTerm, -255, 255);
    float dTerm = kd * (error - lastError) / controlInterval;
    int32_t controllerOutput = pTerm + iTerm + dTerm;
    controllerOutput = constrain(controllerOutput, -255, 255);
    lastError = error;
    //Serial.println(error);
    motorController(controllerOutput, 1, 1, false);
  }
}