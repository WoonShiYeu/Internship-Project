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
 display.setCursor(0,1);
 display.print("Going to sleep in:");
 display.display();
 delay(1000);
 Serial.println("Going to sleep in:");

for (int i = 5; i >= 0; i--) {
    display.setCursor(0,10);
display.clearDisplay();
    display.printf("%d",i);
     display.display();
     Serial.printf("%d\n",i);
    delay(1000);
  }
  display.clearDisplay();
  display.setCursor(0,1);
  display.print("I'm asleep, press the button to wake me up");
  display.display();
  Serial.println("I'm asleep, press the button to wake me up");
  ESP.deepSleep(0); 
}

void loop() {
}
