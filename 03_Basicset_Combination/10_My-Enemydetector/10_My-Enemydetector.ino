const int trigPin = 3; //3:ConnectorA 4:ConnectorB
const int echoPin = 1; //1:ConnectorA 5:ConnectorB
const int spkrPin = 4; //3:ConnectorA 4:ConnectorB

float Duration = 0; //Received Interval Time
float Distance = 0; //Calculation results from Duration

#define BEAT 230
#define DO 261.6
#define _DO 277.18
#define RE 293.665
#define _RE 311.127
#define MI 329.63
#define FA 349.228
#define _FA 369.994
#define SO 391.995
#define _SO 415.305
#define RA 440
#define _RA 466.164
#define TI 493.883
#define octDO 523.251
#define octMI 658
#define octRA 880

void doremi0() {
  ledcWriteTone(spkrPin, RA);
  delay(300);
  ledcWriteTone(spkrPin, DO);
  delay(300);
  ledcWriteTone(spkrPin, 0); // no sound
  delay(250);
}

void doremi1() {
  ledcWriteTone(spkrPin, octDO);
  delay(90);
  ledcWriteTone(spkrPin, MI);
  delay(90);
  ledcWriteTone(spkrPin, 0); // no sound
  delay(100);
}

void doremi2() {
  ledcWriteTone(spkrPin, octRA);
  delay(20);
  ledcWriteTone(spkrPin, octMI);
  delay(20);
  ledcWriteTone(spkrPin, 0); // no sound
  delay(10);
}

void doremi3() {
  ledcWriteTone(spkrPin, 0); // no sound
  delay(250);

}

  
void setup() {
  Serial.begin(115200);
  pinMode(echoPin,INPUT);
  pinMode(trigPin,OUTPUT);
  pinMode(spkrPin, OUTPUT);
  ledcAttach(spkrPin, 12000, 8); //Pin setting(Pin num, Max frequency, Resolution)

}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); //Ultrasonic output
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  Duration = pulseIn(echoPin, HIGH); //Input from sensor
  Duration = Duration/2; //Half the round trip time
  Distance = Duration*340*100/1000000; //Set sonic speed to 340 m/s

  if (Distance <= 50 && Distance > 30) {
      doremi0();
    delay(50);
  } else if (Distance <= 30 && Distance >10) {
      doremi1();
    delay(50);
  } else if (Distance <= 10) {
      doremi2();
    delay(10);
  
  }

  if (Distance >= 20) {
      doremi3();
    delay(50);
  }

  //Look at the serial monitor
  Serial.print("きょり:");
  Serial.print(Distance);
  Serial.println("cm");
  delay(50);

}
