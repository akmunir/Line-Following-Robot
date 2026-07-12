#include <Arduino.h>

uint8_t PWM1 = 5;
uint8_t DIR1 = 4;
uint8_t PWM2 = 6;
uint8_t DIR2 = 7;
constexpr uint8_t leftSensor = 8;
constexpr uint8_t middleSensor = 9;
constexpr uint8_t rightSensor = 10;
uint8_t sensorValues[3] = {leftSensor, middleSensor, rightSensor};
bool readLineSensor(uint8_t sensorPin);
void motorController(uint8_t sensorPosition, uint8_t pwmValue);
void moveLeft(uint8_t sensorPosition, uint8_t pwmValue);
void moveForward(uint8_t sensorPosition, uint8_t pwmValue);
void moveRight(uint8_t sensorPosition, uint8_t pwmValue);

void setup() {
  Serial.begin(115200);
}

void loop() {
  for (uint8_t i = 0; i < sizeof(sensorValues); i++) {
    bool isBlackLine =  readLineSensor(sensorValues[i]);
    if (isBlackLine) {
      //Serial.println("black line detected");
      motorController(sensorValues[i], 100); //150 is pwm value
    } else {
      motorController(sensorValues[i], 0);
    }
    delay(10);
  }
  Serial.println();
}

bool readLineSensor(uint8_t sensorPin) {
  pinMode(sensorPin, OUTPUT);
  digitalWrite(sensorPin, HIGH);
  delayMicroseconds(10);
  pinMode(sensorPin, INPUT);
  unsigned long start = micros();
  while (digitalRead(sensorPin) == HIGH) { 
  }
  unsigned long totalTime = micros() - start;
  Serial.print(totalTime);
  Serial.print("\t");
  if (totalTime > 2500) {
    //return 1;
  }
  return 0;
  
}

void motorController(uint8_t sensorPosition, uint8_t pwmValue) {
  switch(sensorPosition) {
    case leftSensor: 
      moveLeft(sensorPosition, pwmValue);
      break;
    case middleSensor:
      moveForward(sensorPosition, pwmValue);
      break;
    case rightSensor:
      moveRight(sensorPosition, pwmValue);
      break;
  }
}

void moveForward(uint8_t sensorPosition, uint8_t pwmValue) {
    digitalWrite(PWM1, LOW);
    digitalWrite(DIR1, HIGH);
    digitalWrite(PWM2, LOW);
    digitalWrite(DIR2, HIGH);
    analogWrite(PWM1, pwmValue);
    analogWrite(PWM2, pwmValue);

}

void moveLeft(uint8_t sensorPosition, uint8_t pwmValue) {


}

void moveRight(uint8_t sensorPosition, uint8_t pwmValue) {


}