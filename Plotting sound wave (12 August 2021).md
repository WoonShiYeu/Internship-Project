```
#include <M5Core2.h>
#include <driver/i2s.h>

#define PIN_CLK  0
#define PIN_DATA 34
#define READ_LEN (2 * 256)
#define GAIN_FACTOR 3
uint8_t BUFFER[READ_LEN] = {0};

uint16_t oldy[320];
int16_t *adcBuffer = NULL;

void i2sInit()
{
   i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
    .sample_rate =  44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // is fixed at 12bit, stereo, MSB
    .channel_format = I2S_CHANNEL_FMT_ALL_RIGHT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 2,
    .dma_buf_len = 128,
   };

   i2s_pin_config_t pin_config;
   pin_config.bck_io_num   = I2S_PIN_NO_CHANGE;
   pin_config.ws_io_num    = PIN_CLK;
   pin_config.data_out_num = I2S_PIN_NO_CHANGE;
   pin_config.data_in_num  = PIN_DATA;
  
   
   i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
   i2s_set_pin(I2S_NUM_0, &pin_config);
   i2s_set_clk(I2S_NUM_0, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}



void mic_record_task (void* arg)
{   
  size_t bytesread;
  while(1){
    i2s_read(I2S_NUM_0,(char*) BUFFER, READ_LEN, &bytesread, (100 / portTICK_RATE_MS));
    adcBuffer = (int16_t *)BUFFER;
    showSignal();
    vTaskDelay(100 / portTICK_RATE_MS);
  }
}

void setup() {
  M5.begin();
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextDatum(TC_DATUM);
  M5.Lcd.setTextColor(YELLOW,BLACK);
  M5.Lcd.drawString("Mic Test",160,0,2);

  i2sInit();
  xTaskCreate(mic_record_task, "mic_record_task", 2048, NULL, 1, NULL);
}


void showSignal(){
  int y;
  M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(WHITE,BLACK);
    M5.Lcd.drawString("     Volume:",0,240,2);
  for (int n = 0; n < 320; n++){
    y = adcBuffer[n] * GAIN_FACTOR;
    y = map(y, INT16_MIN, INT16_MAX, 90, 150);
    M5.Lcd.drawPixel(n, oldy[n],BLACK);
    M5.Lcd.drawPixel(n,y,WHITE);
    
    
    if ((y>=135) || (y<=105))
    {
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.setTextColor(RED,BLACK);
    M5.Lcd.setTextPadding(160);
    M5.Lcd.drawString("HIGH    ",320,240,2);}
    
    else if ((y>118) && (y<122))
    {
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.setTextColor(GREEN,BLACK);
    M5.Lcd.setTextPadding(160);
    M5.Lcd.drawString("Low    ",320,240,2);}
    
    else
    {
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.setTextColor(YELLOW,BLACK);
    M5.Lcd.setTextPadding(160);
    M5.Lcd.drawString("Medium    ",320,240,2);}
    
    oldy[n] = y; }

  
}




void loop() {
  printf("loop cycling\n");
  vTaskDelay(1000 / portTICK_RATE_MS); // otherwise the main task wastes half of the cpu cycles
}
```

The demonstration video is posted here: https://youtu.be/GI2WQ5xhyEg
