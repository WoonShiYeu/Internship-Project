
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

 
void setup() {
  Serial.begin(9600);
  Serial.setTimeout(2000);

 // Wait for serial to initialize.
  while(!Serial) { }

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

 display.clearDisplay();
 display.setTextColor(WHITE);
 display.setCursor(0, 16);
 display.println("Awoke!Sleep in:");
 display.display();
 Serial.println("Awoke!Sleep in:");
  
  for (int i = 1; i <= 15; i++) {
    display.setCursor(0, 8);
    display.clearDisplay();
    display.printf("%d",i);
     display.display();
    delay(1000);
  }

 
  
  // Deep sleep mode for 30 seconds, the ESP8266 wakes up by itself when GPIO 16 (D0 in NodeMCU board) is connected to the RESET pin
  Serial.println("I'm awake, but I'm going into deep sleep mode for 5 seconds");
  display.setCursor(0, 12);
  display.clearDisplay();
  display.print("Sleep for 5 seconds");
  display.display();
  ESP.deepSleep(5e6); 
  
  // Deep sleep mode until RESET pin is connected to a LOW signal (for example pushbutton or magnetic reed switch)
  //Serial.println("I'm awake, but I'm going into deep sleep mode until RESET pin is connected to a LOW signal");
  //ESP.deepSleep(0); 
}

void loop() {
}
