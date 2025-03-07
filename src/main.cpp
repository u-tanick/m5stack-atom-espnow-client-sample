// ==================================
// 全体共通のヘッダファイルのinclude
#include <Arduino.h>                         // Arduinoフレームワークを使用する場合は必ず必要
#include <Update.h>                          // 定義しないとエラーが出るため追加。
#include <Ticker.h>                          // 定義しないとエラーが出るため追加。
#include <M5Unified.h>                       // M5Unifiedライブラリ
// ================================== End

// #define ARDUINO_M5STACK_ATOM
#define ARDUINO_M5STACK_ATOMS3

// ==================================
// for LED
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

// ブロードキャスト用のアドレス
uint8_t macAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// 送信データ（別の型とか構造体でもいい）
int send_data = 1;

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
    M5.Lcd.fillScreen(TFT_RED);
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

  // ESP-NOWの初期化(出来なければリセットして繰り返し)
  if (esp_now_init() != ESP_OK) {
    return;
  }

  // 送信先アドレスを設定(この場合はブロードキャストアドレス)
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, macAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    return;
  }
}
// ================================== End

void loop() {

  M5.update();

  // === ボタンAが押されたらデータを送信 ===
  if (M5.BtnA.wasPressed()) {
    #ifdef ARDUINO_M5STACK_ATOM
      setLed(CRGB::Blue);
    #endif
    #ifdef ARDUINO_M5STACK_ATOMS3
      M5.Lcd.fillScreen(TFT_GREEN);
    #endif

    // ESP-NOW送信
    esp_now_send(macAddress, (uint8_t *) &send_data, sizeof(send_data));
    delay(2000);

    #ifdef ARDUINO_M5STACK_ATOM
      setLed(CRGB::Black);
    #endif
    #ifdef ARDUINO_M5STACK_ATOMS3
      M5.Lcd.fillScreen(TFT_RED);
    #endif
  }
}