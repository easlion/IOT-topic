/*
  本範例為HX711簡單示範，會把重量顯示在監控視窗中
  
  需先安裝Bogdan Necula的HX711函式庫
  https://github.com/bogde/HX711
  
  傑森創工製作
  
  粉絲團：
  https://www.facebook.com/jasonshow
  
  傑森創工部落格：
  https://blog.jmaker.com.tw/

  傑森創工賣場：https://goo.gl/EWoPQ4

 */
#include "HX711.h"

// 接線設定
const int DT_PIN = 16;
const int SCK_PIN = 4;

const int scale_factor = -216; //比例參數，從校正程式中取得

HX711 scale;

void setup() {
  Serial.begin(9600);
  Serial.println("Initializing the scale");

  scale.begin(DT_PIN, SCK_PIN);

  Serial.println("Before setting up the scale:"); 
  
  Serial.println(scale.get_units(5), 0);  //未設定比例參數前的數值

  scale.set_scale(scale_factor);       // 設定比例參數
  scale.tare();               // 歸零

  Serial.println("After setting up the scale:"); 

  Serial.println(scale.get_units(5), 0);  //設定比例參數後的數值

  Serial.println("Readings:");  //在這個訊息之前都不要放東西在電子稱上
}

void loop() {
  
  Serial.println(scale.get_units(10), 0); 

  scale.power_down();             // 進入睡眠模式
  delay(1000);
  scale.power_up();               // 結束睡眠模式
}
