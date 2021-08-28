```
#include <ThingSpeak.h>
#include <M5Core2.h>  // Including library for dht
#include <WiFi.h>
#include <WiFiClient.h>
#include <Arduino.h>


 int i; 
 
const char *ssid =  "Woon family@tplink";     // replace with your wifi ssid and wpa2 key
const char *pass =  "987654321";
const char* server = "api.thingspeak.com";
 
WiFiClient client;

unsigned long myChannelNumber = 1478800l;

const char * apiKey = "EDXOQ0AUAQT94RN0"; 

void setup(){
  Serial.begin(9600);
 M5.begin();
  WiFi.begin(ssid, pass);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  ThingSpeak.begin (client);

 
  M5.Lcd.setTextColor(YELLOW);  //Set the font color to yellow.  设置字体颜色为黄色
  M5.Lcd.setTextSize(2);  //Set the font size.  设置字体大小为2
  M5.Lcd.setCursor(65, 10); //Move the cursor position to (x, y).  移动光标位置到 (x, y)处
  M5.Lcd.println("Press A to add 1");
  M5.Lcd.setCursor(3, 35);
  M5.Lcd.setTextColor(RED);
}
 
void loop() 
{

  M5.update(); //Read the press state of the key.  读取按键 A 的状态
  if (M5.BtnA.wasReleased() || M5.BtnA.pressedFor(1000, 200)) {
    M5.Lcd.print('A');
    i = i+1;}
      
  ThingSpeak.writeField(myChannelNumber,1,i, apiKey);
  delay(100);
}
```
