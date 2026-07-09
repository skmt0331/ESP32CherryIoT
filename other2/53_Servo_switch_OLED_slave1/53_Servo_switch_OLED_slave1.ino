#include <U8g2lib.h>  // OLEDディスプレイに文字や図形を描画するためのライブラリ
#include <Wire.h>     // OLEDと通信する「I2C」規格を制御するライブラリ
#include <esp_now.h>  // ルーターなしで子機同士が直接通信できる「ESP-NOW」用のライブラリ
#include <WiFi.h>     // ESP32のWi-Fiチップを起動・制御するためのライブラリ

//  【OLEDの設定】解像度128x64 / リセットピンなし / SCL＝3番ピン / SDA＝1番ピン を指定
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 3, /* data=*/ 1);

//  【LEDの設定】コントロールするLEDが接続されているピン番号（ConnectorB = GPIO4）
const int LED_PIN = 4; 

//  【データの設計図】親機から送られてくる手紙（パケット）のフォーマットを定義
typedef struct struct_message {
    int commandType;  // 命令の種類（1: LED点滅指示 / 2: OLED表示指示）
} struct_message;

//  【データの受け皿】受信データを一時保存する変数
struct_message recvData;

//  親機（マスター）から無線データを受信したときに、自動的に割り込んで実行される関数
void OnDataRecv(const esp_now_recv_info *recvInfo, const uint8_t *incomingData, int len) {
  
  //  ① 届いたばかりの最新データを、一旦一時的な変数（newData）にコピー
  struct_message newData;
  memcpy(&newData, incomingData, sizeof(newData));
  
  //  ② 【最優先判定】もし届いたのが「人間検出（コマンド2）」だった場合
  if (newData.commandType == 2) {
    // 現在実行中のLED処理があっても、強制的に「コマンド2」で上書きして優先させます
    recvData.commandType = 2; 
  } else {
    // それ以外（コマンド1など）なら、通常通りそのまま受け取ります
    recvData.commandType = newData.commandType;
  }
  
  // --------------------------------------------------------------------------------
  //  【コマンド1】：センサー検知時の処理 ➡️ 不気味な緩急（バースト明滅）のホラー演出
  // --------------------------------------------------------------------------------
  if (recvData.commandType == 1) {
    Serial.println("スレーブ1: ホラー風バースト点滅 開始");
    
    // 大きな波（塊）を2回繰り返す
    for (int wave = 0; wave < 2; wave++) {
      
      // ① 【バースト期】激しく、細かく、狂ったように連打点滅
      //  対策：ESP32の仕様バグを防ぐため、点滅もdigitalWriteではなくanalogWrite(255と0)に統一しました
      int burstCount = random(5, 20); 
      for (int i = 0; i < burstCount; i++) {
        analogWrite(LED_PIN, 255); delay(random(15, 40));  // 明るさMAXで超高速点灯
        analogWrite(LED_PIN, 0);   delay(random(15, 45));  // 完全消灯で超高速消灯
      }
      
      // ② 【静寂期】フッと完全に消えて、次の怪奇現象を待つ「不気味な間」
      analogWrite(LED_PIN, 0); 
      
      // 【割り込み監視つき待機】ランダムな待ち時間を10msずつ細かく刻んで監視しながら待つ
      int silentDelay = random(400, 800);
      int waited = 0;
      while (waited < silentDelay) {
        delay(10); 
        waited += 10;
        // 待っている間にコマンド2の電波が来ると、ESP32がこの関数を最初から再実行します
      }
    }
    
    // ③ 【最後のあがき】最後に長めにフワッと光って、じわ〜っと力尽きるように消える余韻
    analogWrite(LED_PIN, 255); 
    delay(random(300, 500)); // まずは0.3〜0.5秒間、しっかり明るく光る
    
    // Brightness(255)から消灯(0)まで、5ずつ細かく落としてアナログ調に消灯させる
    for (int brightness = 255; brightness >= 0; brightness -= 5) {
      analogWrite(LED_PIN, brightness); 
      delay(15);                        
    }
    
    analogWrite(LED_PIN, 0);  // 最後に確実に完全消灯
    Serial.println("スレーブ1: ホラー風バースト点滅 終了");
    recvData.commandType = 0; // 命令をリセット
  } 
  
  // --------------------------------------------------------------------------------
  //  【コマンド2】：人間のとき ➡️ OLEDに日本語を表示して3秒後に消す
  // --------------------------------------------------------------------------------
  if (recvData.commandType == 2) {
    Serial.println("スレーブ1: OLED表示開始（割り込み成功）");
    
    // 画面表示を最優先するため、LEDは強制消灯
    analogWrite(LED_PIN, 0); 

    u8g2.clearBuffer();         
    u8g2.setFont(u8g2_font_logisoso16_tf); // 英語用大フォント
    
    u8g2.setCursor(33, 21);
    u8g2.print("ALWAYS"); 
    u8g2.setCursor(8, 44);
    u8g2.print("WATCHING YOU"); 
    
    u8g2.setFont(u8g2_font_b16_t_japanese1);  // 日本語用フォント
    u8g2.setCursor(8, 61);
    u8g2.print("いつも見てるぞ"); 
    
    u8g2.sendBuffer();          // 画面に反映

    delay(45000); // 3秒間表示をキープ

    u8g2.clearBuffer(); 
    u8g2.sendBuffer();          // 画面消去
    Serial.println("スレーブ1: OLED表示終了");
    
    recvData.commandType = 0;   // 命令をリセット
  }
}

void setup() {
  Serial.begin(115200);   
  delay(3000);            

  // ランダム（乱数）の初期化（空中ノイズを拾う）
  randomSeed(analogRead(0));

  // LED用ピンの初期設定
  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, 0); // ここもanalogWriteでの初期化に変更

  // 無線（Wi-Fi）とESP-NOWの初期化
  WiFi.mode(WIFI_STA);   
  if (esp_now_init() != ESP_OK) return; 
  
  // 受信関数を登録
  esp_now_register_recv_cb(OnDataRecv);

  // OLEDディスプレイ（U8g2）の起動と設定
  u8g2.begin();            
  u8g2.enableUTF8Print();  // 日本語UTF-8の有効化
  
  u8g2.clearBuffer();
  u8g2.sendBuffer();
  
  Serial.println("【スレーブ1】日本語OLED・U8g2完全統一版 起動完了");
}

void loop() {
  // 受信割り込み（OnDataRecv）メインのため、ここは短い休憩のみ
  delay(10); 
}