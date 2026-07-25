#include <Arduino.h>
#include <QTRSensors.h>

QTRSensors qtr; 
uint8_t E1 = 5;
uint8_t M1 = 4;
uint8_t E2 = 6;
uint8_t M2 = 7;
constexpr uint8_t leftMSensor = 8;
constexpr uint8_t middleSensor = 9;
constexpr uint8_t rightMSensor = 10;
constexpr uint8_t leftSensor = 11;
constexpr uint8_t rightSensor = 12;
constexpr uint8_t sensorCount = 3;
int defaultSpeed = 105;
int32_t targetSensorReading = 1000;
int32_t sensorList[sensorCount] = {leftMSensor, middleSensor, rightMSensor};
uint16_t sensorValues[sensorCount] = {leftMSensor, middleSensor, rightMSensor};
int32_t lastError = 0;
int previouslyCheckedSensor = 0;
uint8_t controlInterval = 10;
float pTerm = 0;
float iTerm = 0;
int32_t setPoint = 250;
bool readLineSensor(int32_t sensorPin);
void motorController(int32_t pwmValue);
void moveLeft(int32_t pwmValue);
void moveForward(int32_t pwmValue);
void moveRight(int32_t pwmValue);
void motorPidController();


class Motor {
  public:
    uint8_t pwmPin;
    uint8_t dirPin;
    Motor(uint8_t pwmPin_, uint8_t dirPin_) {
      pwmPin = pwmPin_;
      dirPin = dirPin_;
    }
    void begin() {
      pinMode(pwmPin, OUTPUT);
      pinMode(dirPin, OUTPUT);
      digitalWrite(pwmPin, LOW);
      digitalWrite(dirPin, LOW);
    }
    void move(int pwmValue, bool direction) {
      digitalWrite(pwmPin, LOW);
      digitalWrite(dirPin, direction);
      analogWrite(pwmPin, constrain((defaultSpeed + pwmValue), 0, 255));
    }
    void stop() {
      digitalWrite(pwmPin, LOW);
      digitalWrite(dirPin, LOW);
      analogWrite(pwmPin, 0);
    }
};

Motor leftMotor(E2, M2);
Motor rightMotor(E1, M1);

void setup() {
  // Set Timer3 (controls pin 5 / E1) prescaler to 1 → ~31kHz PWM
  TCCR3B = (TCCR3B & 0b11111000) | 0x01;

  // Set Timer4 (controls pin 6 / E2) prescaler to 1 → ~31kHz PWM
  TCCR4B = (TCCR4B & 0b11111000) | 0x01;
  Serial.begin(115200);
  leftMotor.begin();
  rightMotor.begin();
  qtr.setTypeRC();
  qtr.setSensorPins((const uint8_t[]){leftMSensor, middleSensor, rightMSensor}, 3);
  qtr.setEmitterPin(2);
  delay(500);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // turn on Arduino's LED to indicate we are in calibration mode

  // 2.5 ms RC read timeout (default) * 10 reads per calibrate() call
  // = ~25 ms per calibrate() call.
  // Call calibrate() 400 times to make calibration take about 10 seconds.
  Serial.println("begin calibration");
  for (uint16_t i = 0; i < 200; i++)

  {
    Serial.println(i);
    qtr.calibrate();
  }
  digitalWrite(LED_BUILTIN, LOW); // turn off Arduino's LED to indicate we are through with calibration

  // print the calibration minimum values measured when emitters were on
  for (uint8_t i = 0; i < sensorCount; i++)
  {
    Serial.print(qtr.calibrationOn.minimum[i]);
    Serial.print(' ');
  }
  Serial.println();

  // print the calibration maximum values measured when emitters were on
  for (uint8_t i = 0; i < sensorCount; i++)
  {
    Serial.print(qtr.calibrationOn.maximum[i]);
    Serial.print(' ');
  }
  Serial.println();
  Serial.println();
  delay(1000);
}

void loop()
{
  qtr.readCalibrated(sensorValues);
  for (int i = 0; i < sizeof(sensorValues); i++) {
    //Serial.print(sensorValues[i]);
    //Serial.print("\t");
  }
  if (sensorValues[0] == 1000 || sensorValues[1] == 1000 || sensorValues[2] == 1000) {
    motorPidController();
  }
  else {
    leftMotor.stop();
    rightMotor.stop(); 
    // left or right sensors
  }
  // for (uint8_t i = 0; i < sizeof(sensorValues); i++) {
  //   bool isBlackLine =  readLineSensor(sensorValues[i]);
  //   if (isBlackLine) {
  //     if (sensorList[i] = lastCheckedSensor)
  //     motorController(sensorValues[i], 100); //150 is pwm value
  //   } else {
  //     motorController(sensorValues[i], 0);
  //   }
  //   delay(10);
  //   lastCheckedSensor = sensorList[i];
  // }
  Serial.println();
  delay(10);
}

// bool readLineSensor(uint8_t sensorPin) {
//   pinMode(sensorPin, OUTPUT);
//   digitalWrite(sensorPin, HIGH);
//   delayMicroseconds(10);
//   pinMode(sensorPin, INPUT);
//   unsigned long start = micros();
//   while (digitalRead(sensorPin) == HIGH) { 
//   }
//   unsigned long totalTime = micros() - start;
//   if (totalTime > 2500) {
//     //return 1;
//   }
//   return 0;
  
// }

void motorController(int32_t pwmValue) {
    leftMotor.move(-pwmValue, 1);
    rightMotor.move(pwmValue, 1);

  // switch(sensorPosition) {
  //   case leftMSensor: 
  //     moveLeft(sensorPosition, pwmValue);
  //     break;
  //   case middleSensor:
  //     moveForward(sensorPosition, pwmValue);
  //     break;
  //   case rightMSensor:
  //     moveRight(sensorPosition, pwmValue);
  //     break;
  //   default:
  //     break;
  // }
}

void moveForward(int32_t pwmValue) {
    digitalWrite(E1, LOW);
    digitalWrite(M1, HIGH);
    digitalWrite(E2, LOW);
    digitalWrite(M2, HIGH);
    Serial.println(defaultSpeed);
    analogWrite(E1, defaultSpeed);
    analogWrite(E2, defaultSpeed);

}

void moveRight(int32_t pwmValue) {
  
  digitalWrite(E1, LOW);
  digitalWrite(M1, LOW);
  digitalWrite(E2, LOW);
  digitalWrite(M2, HIGH);
  analogWrite(E1, 0);
  analogWrite(E2, constrain((defaultSpeed + pwmValue), 0, 255));
}

void moveLeft(int32_t pwmValue) {
  digitalWrite(E1, LOW);
  digitalWrite(M1, HIGH);
  digitalWrite(E2, LOW);
  digitalWrite(M2, LOW);
  //Serial.println(defaultSpeed + pwmValue);
  analogWrite(E1, constrain((defaultSpeed + pwmValue), 0, 255));
  analogWrite(E2, 0);

}

void motorPidController() {
  // Controller gains.
  float kp = 0.05;
  float ki = 0.00;
  float kd = 0.2;
  uint16_t middleSensorReading = sensorValues[1];
  // Compute the error (in encoder counts). 
  int error = sensorValues[0] - sensorValues[2];
  // Compute the P term.
  float pTerm = kp * error;
  // Compute and constrain I term.
  iTerm += ki * (error + lastError) / 2;
  iTerm = constrain(iTerm, -255, 255);
  // Compute D term.
  float dTerm = kd * (error - lastError);
  // Compute final output.
  int32_t controllerOutput = pTerm + iTerm + dTerm;
  controllerOutput = constrain(controllerOutput, -255, 255);
  // Update last_error for the next iteration.
  lastError = error;
  Serial.println(controllerOutput);
  motorController(controllerOutput);
}