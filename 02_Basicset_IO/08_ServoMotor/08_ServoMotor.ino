#include <ESP32Servo.h> //ESP32Servo by Kevin Harrington

const int motorPin = 3; //3:ConnectorA 4:ConnectorB

Servo myservo;

void setup() {
  myservo.attach(motorPin);
}

void loop() {
  // 0-180 right
  for(int angle = 50; angle <= 130; angle++) {                        
    myservo.write(angle);
    delay(50);
  }
  // 180-0  left 
  for(int angle = 130; angle >= 50; angle--) {                 
    myservo.write(angle);  
    delay(50);      
  }
}
