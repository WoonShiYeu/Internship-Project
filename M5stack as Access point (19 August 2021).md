```
#include <M5Core2.h>
#include "WiFi.h"

char ssid[] = "M5StackCore2";
char password[] = "12345678";

WiFiServer server(80);
IPAddress ip(192, 168, 0, 1);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

extern unsigned char lightOn[];
extern unsigned char lightOff[];

String M5STACK_DATA;
uint8_t led_count = 8;

void setup() {
M5.begin();
WiFi.mode(WIFI_AP);
WiFi.softAP(ssid, password);
WiFi.softAPConfig(ip, gateway, subnet);
WiFi.begin();
server.begin();
//M5.Lcd.setBrightness(100);
//M5.Lcd.setTextSize(3);
//M5.Lcd.setCursor(50, 20);
M5.Lcd.print("Wi-Fi Server");
//M5.Lcd.setTextSize(2);
//M5.Lcd.setCursor(50, 60);
M5.Lcd.print("SSID: ");
M5.Lcd.print(ssid);
//M5.Lcd.setCursor(50, 80);
//M5.Lcd.print("PSWD: ");
//M5.Lcd.print(password);
//M5.Lcd.setCursor(50, 100);
M5.Lcd.print("URL: ");
M5.Lcd.print(ip);

pinMode(M5_LED, OUTPUT);
digitalWrite(M5_LED, HIGH);

pinMode(M5_BUTTON_HOME, INPUT);

M5.Axp.ScreenBreath(led_count);
}

String parseGET(String str) {
String tmp = "";
for (int i = 0, j = 0; i < str.length(); i++)
{
if (str[i] == ' ') j++;
if (j == 1)
{
if (str[i] != ' ') tmp += str[i];
}
if (j == 2) break;
}
return tmp;
}

bool button(int contactNumber) {
if (digitalRead(contactNumber) == LOW)
{
delay(10);
if (digitalRead(contactNumber) == LOW) return true;
}
return false;
}

void loop() {
if(digitalRead(M5_BUTTON_HOME) == LOW){
led_count++;
if(led_count > 12)
led_count = 7;
while(digitalRead(M5_BUTTON_HOME) == LOW);
M5.Axp.ScreenBreath(led_count);
}

String currentString = "";
bool readyResponse = false;
WiFiClient client = server.available();
while (client.connected())
{
if (client.available())
{
if (button(button_A))
{
M5.Lcd.fillRect(45, 230, 230, 5, BLACK);
M5.Lcd.fillRect(45, 230, 50, 5, 0xf980);
M5.Lcd.drawLine(135, 235, 184, 235, 0x4c0);
M5.Lcd.drawLine(225, 235, 274, 235, 0x013);
M5STACK_DATA = "hello from red button A!";
}

  if (button(button_B))
  {
    M5.Lcd.fillRect(45, 230, 230, 5, BLACK);
    M5.Lcd.drawLine(45, 235, 94, 235, 0xf980);
    M5.Lcd.fillRect(135, 230, 50, 5, 0x37e0);
    M5.Lcd.drawLine(225, 235, 274, 235, 0x013);
    M5STACK_DATA = "hello from green button B!";
  }
  
  if (button(button_C))
  {
    M5.Lcd.fillRect(45, 230, 230, 5, BLACK);
    M5.Lcd.drawLine(45, 235, 94, 235, 0xf980);
    M5.Lcd.drawLine(135, 235, 184, 235, 0x4c0);
    M5.Lcd.fillRect(225, 230, 50, 5, 0x19f);
    M5STACK_DATA = "hello from blue button C!";
  }*/
  
  char c = client.read();
  if (c != '\r')
  {
    if (c != '\n')
    {
      currentString += c;
    }
  }
  else
  {
    M5.Lcd.print(currentString);
    M5.Lcd.print("\n");
    readyResponse = true; // готов отдать браузеру
  }

  if (readyResponse) // отдать браузеру
  {
    String GET = parseGET(currentString);

    //client.flush();
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println();

    if (GET == "/update")
    {
      client.print("<html>");
      client.print("<head>");
      client.print("<meta http-equiv=\"Refresh\" content=\"1\"/>");
      client.print("<script type=\"text/javascript\">");
      client.print("var M5STACK_DATA = \"");
      client.print(M5STACK_DATA);
      client.print("\";");
      client.print("</script>");
      client.print("</head>");
      client.print("</html>");
    }
    else
    {
      client.print("<html>");
      client.print("<head>");
      client.print("<meta name=\"viewport\" content=\"width=device-width; initial-scale=1.0; maximum-scale=1.0; user-scalable=0;\" />");
      client.print("<script type=\"text/javascript\">");
      client.print("setInterval(function(){document.getElementById(\"M5STACK_DATA\").innerHTML = window.frames[0].M5STACK_DATA;}, 1000);");
      client.print("</script>");
      client.print("</head>");
      client.print("<body>");
      //client.print("<h1>M5StickC</h1>");
      //client.print("<h2>Wi-Fi Server</h2>");
      //client.print("<hr>");
      client.print("<h3>Remote control:</h3>");
      //client.print("<span>Note: please click on the button and look at the screen of the device</span>");
      //client.print("<br><br>");

      if ((GET == "/off?") || (GET == "/"))
      {
        digitalWrite(M5_LED, HIGH);
        //M5.Lcd.drawBitmap(126, 120, 100, 100, (uint16_t *)lightOff);
        client.print("<form action=\"/on\">");
        client.print("<button style=\"width: 200px; height: 200px;\" type=\"submit\">Turn ON</button>");
      }
      else if (GET == "/on?")
      {
        digitalWrite(M5_LED, LOW);
        //M5.Lcd.drawBitmap(126, 120, 100, 100, (uint16_t *)lightOn);
        client.print("<form action=\"/off\">");
        client.print("<button style=\"width: 200px; height: 200px;\" type=\"submit\">Turn OFF</button>");
      }
      
      client.print("</form>");
      client.print("<hr>");
      client.print("<h3>Remote monitor:</h3>");
      //client.print("<span>Note: please press and hold any three buttons on the device</span>");
      //client.print("<br>");
      client.print("<span id=\"M5STACK_DATA\"></span>");
      client.print("<iframe src=\"/update\" hidden=\"true\"></iframe>");
      client.print("</body>");
      client.print("</html>");
    }

    client.println();
    client.println();
    readyResponse = false;
    currentString = "";
    break;
  }
}
}
}
```
The demonstration video is posted here: https://youtu.be/KCnCXKHKzcc
