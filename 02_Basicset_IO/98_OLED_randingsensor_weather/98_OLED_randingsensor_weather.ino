#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h> //by Adafruit
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C

const char* ssid = "xxxxxxx";
const char* password =  "xxxxxxxx";

const String endpoint = "http://api.openweathermap.org/data/2.5/weather?q=xxxxxx,jp&APPID=";
const String key = "xxxxxxxx"; // Your API key

const int trigPin = 3; //3:ConnectorA 4:ConnectorB 距離センサー
const int echoPin = 1; //1:ConnectorA 5:ConnectorB 距離センサー

float Duration = 0; //Received Interval Time
float Distance = 0; //Calculation results from Duration

bool Caution = false; // 今cautionモードならtrue、平時ならfalse


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long lastWeatherCheck = 0; 
const unsigned long weatherInterval = 30000; // 30秒ごとに天気を更新
String weatherMain = ""; // 天気のテキストを保存する変数

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  pinMode(echoPin,INPUT);
  pinMode(trigPin,OUTPUT);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network");

  Wire.begin(5,4); //(SDA, SCL) 1,3:ConnectorA 5,4:ConnectorB ディスプレイ

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    for(;;); //Infinite loop when screen initialization fails　ディスプレイ起動
  }

  display.setTextWrap(false);  // Disable wrapping

}

void loop()
{
    if (millis() - lastWeatherCheck >= weatherInterval) {
    lastWeatherCheck = millis(); // タイマーをリセット

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(endpoint + key); 
      int httpCode = http.GET(); 
   
      if (httpCode > 0) { 
        String payload = http.getString();  
        Serial.println(httpCode);
        Serial.println(payload);

        DynamicJsonDocument forecaseDoc(1024);
        deserializeJson(forecaseDoc, payload);

        // データを抽出して変数に保存
        weatherMain = forecaseDoc["weather"][0]["main"].as<String>();        
        Serial.printf("main:%s\n", weatherMain.c_str());
      } 
      else {
        Serial.println("Error on HTTP request");
      }
      http.end(); 
    }
  }
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); //Ultrasonic output
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  Duration = pulseIn(echoPin, HIGH); //Input from sensor
  Duration = Duration/2; //Half the round trip time
  Distance = Duration*340*100/1000000; //Set sonic speed to 340 m/s

  if(Distance <= 50){
    Caution = true;
  }else if(Distance >= 50) {
    Caution = false;
  }


  if(Caution == true){
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20, 20);
    display.print(F("caution"));
    display.display();
    delay(10);

  }else{
    if (Caution == false) { 
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20, 20);
    }
    if (weatherMain != "") {
      display.print(weatherMain); 
    } else {
      display.print(F("Loading..."));
    }
  }

  display.display();

  Serial.print("きょり:");
  Serial.print(Distance);
  Serial.println("cm");
  delay(500);

}
  // for(int j=0; j<128; j++){
  //   display.clearDisplay();

  //   display.setTextSize(2);
  //   display.setTextColor(SSD1306_WHITE);
  //   display.setCursor(10, j);
  //   display.print(F("CherryIoT"));

  //   display.display();
  //   delay(20);
  // }