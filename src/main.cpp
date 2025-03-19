// ==================================
// 全体共通のヘッダファイルのinclude
#include <Arduino.h>                         // Arduinoフレームワークを使用する場合は必ず必要
#include <Update.h>                          // 定義しないとエラーが出るため追加。
#include <Ticker.h>                          // 定義しないとエラーが出るため追加。
#include <M5Unified.h>                       // M5Unifiedライブラリ
// ================================== End

#define ARDUINO_M5STACK_ATOMS3
// #define ARDUINO_M5STACK_ATOM

// ==================================
// for LED (ATOM Lite)
#ifdef ARDUINO_M5STACK_ATOM
  #include <FastLED.h>
  #define ATOM_LED_PIN  27      // ATOM Lite本体のLED用
  #define ATOM_NUM_LEDS 1       // Atom LED
  static CRGB atom_leds[ATOM_NUM_LEDS];
 
  // Atom本体のLEDを光らせる用の関数
  void setLed(CRGB color)
  {
    // change RGB to GRB
    uint8_t t = color.r;
    color.r = color.g;
    color.g = t;
    atom_leds[0] = color;
    FastLED.show();
  }
  
#endif
// ================================== End

// ==================================
// for ESPNow
#include "WiFi.h"
#include <esp_now.h>

// ブロードキャスト用のアドレス（テスト用）
// uint8_t macAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// 複数のMACアドレスを定義
uint8_t macAddresses[][6] = {
  {0x2c, 0xbc, 0xbb, 0x82, 0xbb, 0xe0},  // Basic
  {0x90, 0x15, 0x06, 0xfa, 0x05, 0xe8},  // AtomLite
  {0x2c, 0xbc, 0xbb, 0x94, 0x32, 0x0c},  // Core2 AWS
  {0x78, 0x21, 0x84, 0xaa, 0xbb, 0xa8}   // Fire
};

// 登録するデバイス数
const int numPeers = sizeof(macAddresses) / sizeof(macAddresses[0]);

// 送信データ（別の型とか構造体でもいい）
// int send_data = 1;
// ================================== End

void setup() {
  // 設定用の情報を抽出
  auto cfg = M5.config();
  #ifdef ARDUINO_M5STACK_ATOMS3
    // Groveポートの出力をしない（m5atomS3用）
    cfg.output_power = true;
  #endif
  // M5Stackをcfgの設定で初期化
  M5.begin(cfg);

  #ifdef ARDUINO_M5STACK_ATOMS3
    // 液晶初期化
    M5.Lcd.init();                         // 初期化
    M5.Lcd.setTextWrap(false);             // テキストが画面からはみ出した時の折り返し無し
    M5.Lcd.setTextColor(TFT_WHITE);        // 文字色
    M5.Lcd.fillScreen(TFT_BLACK);
  #endif

  #ifdef ARDUINO_M5STACK_ATOM
    FastLED.addLeds<WS2811, ATOM_LED_PIN, RGB>(atom_leds, ATOM_NUM_LEDS);
    FastLED.setBrightness(5);
    for (int i = 0; i < 3; i++)
    {
      setLed(CRGB::Red);
      delay(500);
      setLed(CRGB::Black);
      delay(500);
    }
  #endif

  // WiFi初期化
  WiFi.mode(WIFI_STA);

  // ESP-NOWの初期化
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  Serial.println("ESP-NOW Initialized");

  for (int i = 0; i < numPeers; i++) {
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, macAddresses[i], 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.printf("Failed to add peer %d\n", i);
    } else {
        Serial.printf("Peer %d added\n", i);
    }
  }
}

void loop() {

  M5.update();

  // === ボタンAが押されたらデータを送信 ===
  if (M5.BtnA.wasPressed()) {
    #ifdef ARDUINO_M5STACK_ATOMS3
      M5.Lcd.fillScreen(TFT_GREEN);
    #endif
    #ifdef ARDUINO_M5STACK_ATOM
      setLed(CRGB::Green);
    #endif

    // 送信データ（別の型とか構造体でもいい）
    uint8_t sendData[1] = {1};

    // 順番にESP-NOWデータ送信
    for (int i = 0; i < numPeers; i++) {
        esp_err_t result = esp_now_send(macAddresses[i], sendData, sizeof(sendData));

        if (result == ESP_OK) {
            Serial.printf("Data sent to Peer %d\n", i);
        } else {
            Serial.printf("Failed to send data to Peer %d, Error: %d\n", i, result);
        }
    }

    delay(1000);
    #ifdef ARDUINO_M5STACK_ATOMS3
      M5.Lcd.fillScreen(TFT_BLACK);
    #endif
    #ifdef ARDUINO_M5STACK_ATOM
      setLed(CRGB::Black);
    #endif
  }
}