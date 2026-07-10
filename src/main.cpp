#include <Arduino.h>

uint8_t PWM1 = 5;
uint8_t DIR1 = 4;
uint8_t PWM2 = 6;
uint8_t DIR2 = 7;
uint8_t leftSensor = 8;
uint8_t middleSensor = 9;
uint8_t rightSensor = 10;
uint8_t sensorValues[3] = {leftSensor, middleSensor, rightSensor};
void readLineSensor(uint8_t sensorPin);

void setup() {
  Serial.begin(115200);
}

void loop() {
  for (uint8_t i = 0; i < sizeof(sensorValues); i++) {
    readLineSensor(sensorValues[i]);
    delay(10);
  }
  Serial.println();
}

void readLineSensor(uint8_t sensorPin) {
  pinMode(sensorPin, OUTPUT);
  digitalWrite(sensorPin, HIGH);
  delayMicroseconds(10);
  pinMode(sensorPin, INPUT);
  unsigned long start = micros();
  while (digitalRead(sensorPin) == HIGH) { 
  }
  unsigned long totalTime = micros() - start;
  if (totalTime > 2500) {
    totalTime = 2500;
  }
  Serial.print(totalTime);
  Serial.print("\t");
}