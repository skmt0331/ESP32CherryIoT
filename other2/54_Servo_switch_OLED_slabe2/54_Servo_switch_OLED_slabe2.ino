#include <esp_now.h>
#include <WiFi.h>

const int sensorPin = 3; // 3:ConnectorA

// 🔴【重要】子機（スレーブ）のMACアドレスを正しく入れてください
uint8_t mainMacAddress[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

typedef struct struct_message {
  int commandType; // 4 = センサー検知
} struct_message;

struct_message sendData;

void setup() {
  Serial.begin(115200);
  pinMode(sensorPin, INPUT);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) return;

  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, mainMacAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

void loop() {
  int sensorState = digitalRead(sensorPin);
  Serial.println(sensorState); // デバッグ用

  // センサーが反応したら（1 or 0、センサーの仕様に合わせてHigh/Lowを調整してください）
  if (sensorState == HIGH) { 
    Serial.println("センサー検知！親機へ送信します。");
    sendData.commandType = 4; // コマンド4：センサー検知
    esp_now_send(mainMacAddress, (uint8_t *) &sendData, sizeof(sendData));
    delay(2000); // 連続送信を防ぐためのチャタリング防止（2秒待つ）
  }
  delay(200);
}