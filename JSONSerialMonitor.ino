#include <ArduinoJson.h>
#include <M5Core2.h>

void setup() {
  Serial.begin(9600);
  M5.begin();

}

void loop() {
  if (Serial.read() == 'j'){
  DynamicJsonBuffer jBuffer;
  JsonObject& root = jBuffer.createObject();
  
  root["day"] = "Monday";

  int temp = 21;
  int humidity = 53;

  root["temperature"] = temp;
  root["humidity"] = humidity;

  root.prettyPrintTo(Serial);
  Serial.println();
  }

}
