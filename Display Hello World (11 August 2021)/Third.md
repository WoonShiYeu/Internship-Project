```
#include <M5Core2.h>

void setup() {
  M5.begin();  //Initialize M5Core2
  M5.Lcd.setTextDatum(MC_DATUM);    //Set text alignment to center-aligned
  M5.Lcd.drawString("hello world", 160, 120, 2);    //Print the string 'hello' in font 2 at (160, 120)
}

void loop(){
}
```
![3](https://user-images.githubusercontent.com/87056506/131205197-1be6f724-8663-4f37-a6fa-b785c3983862.jpeg)
