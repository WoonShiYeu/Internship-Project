```
#include <M5Core2.h>

// the setup routine runs once when M5Stack starts up
void setup(){

  // Initialize the M5Stack object
  M5.begin();

  // LCD display

  M5.Lcd.setTextColor(TFT_GREEN,TFT_BLACK);
  M5.Lcd.setTextSize(3);
  M5.Lcd.println("Hello World!");
  


  M5.Lcd.setTextColor(TFT_YELLOW); 
  M5.Lcd.setTextSize(1);
  M5.Lcd.println("M5Stack is running successfully!");
}

// the loop routine runs over and over again forever
void loop() {

}
```

![2](https://user-images.githubusercontent.com/87056506/131205186-ea4677be-5192-4929-8bfe-7cebc0950296.jpeg)
