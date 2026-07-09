#include <ESP32Servo.h> 
#include <esp_now.h>
#include <WiFi.h>

const int SERVO_PIN = 4;  // サーボのピン番号
Servo myServo;            // サーボを動かすための名前

//【重要】以前のコードの「slaveMacAddress」から「slave1MacAddress」に名前を統一します
// ここにスレーブ1（LED・OLED側）のMACアドレスを正しく記入してください
uint8_t slave1MacAddress[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

typedef struct struct_message {
    int commandType; 
} struct_message;

struct_message sendData;
struct_message recvData;

// クマ（ID:23）が来たらサーボ【のみ】動かす
void executeServoSequence() {
  Serial.println("親機: クマ(ID:23)検出 ➡️ サーボ駆動（LED/OLEDは連動しません）");
  myServo.detach(); delay(10);
  myServo.attach(SERVO_PIN, 700, 2300); delay(10); //電源を接続　理由は後述
  myServo.write(1600);   
  delay(1200); // 回転時間調整はここ
  myServo.write(1500); delay(200); //停止　1500から離れるほどサーボは早く動く
  myServo.detach(); //電源を切断　切断しないと勝手にじわじわ動く
  pinMode(SERVO_PIN, OUTPUT);
  digitalWrite(SERVO_PIN, LOW);
}

// スレーブ2（センサー）やPCからの受信処理
void OnDataRecv(const esp_now_recv_info *recvInfo, const uint8_t *incomingData, int len) {
  memcpy(&recvData, incomingData, sizeof(recvData));
  
  // スレーブ2から「センサー反応（コマンド4）」が届いたら、スレーブ1へ「点灯（コマンド1）」を中継
  if (recvData.commandType == 4) {
    sendData.commandType = 1; // 点灯せよ
    esp_now_send(slave1MacAddress, (uint8_t *) &sendData, sizeof(sendData));
  }
  // もしスレーブ2から「センサー反応終了（例：コマンド5など）」が届く仕様なら、ここで消灯を送る
  else if (recvData.commandType == 5) {
    sendData.commandType = 0; // 消灯せよ
    esp_now_send(slave1MacAddress, (uint8_t *) &sendData, sizeof(sendData));
  }
}

void setup() {
  Serial.begin(115200);
  delay(3000); 
  Serial.setTimeout(100);

  pinMode(SERVO_PIN, OUTPUT);
  digitalWrite(SERVO_PIN, LOW);

  WiFi.mode(WIFI_STA); 
  if (esp_now_init() != ESP_OK) return;
  
  esp_now_register_recv_cb(OnDataRecv); // 受信許可

  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, slave1MacAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  // 作成した設定情報（peerInfo）を使って、正式に通信相手としてシステムに登録
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("スレーブ1の登録に失敗しました");
  } else {
    Serial.println("スレーブ1の登録に成功、準備完了");
  }
}

void loop() {
  if (Serial.available() > 0) {
    String inputString = Serial.readStringUntil('\n');
    inputString.trim();
    if (inputString.length() == 0) return;

    if (inputString.indexOf("(ID:23)") != -1) {
      executeServoSequence(); // クマならサーボのみ
    } 
    else if (inputString.indexOf("(ID:1)") != -1) {
      // 人間ならスレーブ1へOLED表示指示（コマンド2）を送る
      sendData.commandType = 2;
      esp_now_send(slave1MacAddress, (uint8_t *) &sendData, sizeof(sendData));
    }
  }
  delay(10);
}