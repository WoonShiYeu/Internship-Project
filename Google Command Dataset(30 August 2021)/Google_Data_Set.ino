/* Edge Impulse Arduino examples
 * Copyright (c) 2021 EdgeImpulse Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// If your target is limited in memory remove this macro to save 10K RAM
#define EIDSP_QUANTIZE_FILTERBANK   0

/**
 * Define the number of slices per model window. E.g. a model window of 1000 ms
 * with slices per model window set to 4. Results in a slice size of 250 ms.
 * For more info: https://docs.edgeimpulse.com/docs/continuous-audio-sampling
 */
#define EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW 3

/* Includes ---------------------------------------------------------------- */

#include <Dominic-Google-Data-Command-Set_inferencing.h>
#include <M5Core2.h>
#include <driver/i2s.h>

#define CONFIG_I2S_BCK_PIN 12 //Define I2S related ports.  定义I2S相关端口
#define CONFIG_I2S_LRCK_PIN 0
#define CONFIG_I2S_DATA_PIN 2
#define CONFIG_I2S_DATA_IN_PIN 34

#define Speak_I2S_NUMBER I2S_NUM_0  // Define the speaker port.  定义扬声器端口

#define MODE_MIC 0  // Define the working mode.  定义工作模式
#define MODE_SPK 1
#define DATA_SIZE 1024

uint8_t microphonedata0[1024 * 100];
int data_offset = 0;

bool InitI2SSpeakOrMic(int mode){  //Init I2S.  初始化I2S
    esp_err_t err = ESP_OK;

    i2s_driver_uninstall(Speak_I2S_NUMBER); // Uninstall the I2S driver.  卸载I2S驱动
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER| I2S_MODE_RX | I2S_MODE_PDM),  // Set the I2S operating mode.  设置I2S工作模式
        .sample_rate = 16000, // Set the I2S sampling rate.  设置I2S采样率
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // Fixed 12-bit stereo MSB.  固定为12位立体声MSB
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT, // Set the channel format.  设置频道格式
        .communication_format = I2S_COMM_FORMAT_I2S,  // Set the format of the communication.  设置通讯格式
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Set the interrupt flag.  设置中断的标志
        .dma_buf_count = 2, //DMA buffer count.  DMA缓冲区计数
        .dma_buf_len = 1024, //DMA buffer length.  DMA缓冲区长度
    };
    if (mode == MODE_MIC){
        i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM);
    }else{
        i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
        i2s_config.use_apll = false;  //I2S clock setup.  I2S时钟设置
        i2s_config.tx_desc_auto_clear = true; // Enables auto-cleanup descriptors for understreams.  开启欠流自动清除描述符
    }
    // Install and drive I2S.  安装并驱动I2S
    err += i2s_driver_install(Speak_I2S_NUMBER, &i2s_config, 0, NULL);

    i2s_pin_config_t tx_pin_config;
    tx_pin_config.bck_io_num = CONFIG_I2S_BCK_PIN;  // Link the BCK to the CONFIG_I2S_BCK_PIN pin. 将BCK链接至CONFIG_I2S_BCK_PIN引脚
    tx_pin_config.ws_io_num = CONFIG_I2S_LRCK_PIN;  //          ...
    tx_pin_config.data_out_num = CONFIG_I2S_DATA_PIN;  //       ...
    tx_pin_config.data_in_num = CONFIG_I2S_DATA_IN_PIN; //      ...
    err += i2s_set_pin(Speak_I2S_NUMBER, &tx_pin_config); // Set the I2S pin number.  设置I2S引脚编号
    err += i2s_set_clk(Speak_I2S_NUMBER, 16000, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO); // Set the clock and bitwidth used by I2S Rx and Tx. 设置I2S RX、Tx使用的时钟和位宽
    return true;
}

/** Audio buffers, pointers and selectors */
typedef struct {
    signed short *buffers[2];
    unsigned char buf_select;
    unsigned char buf_ready;
    unsigned int buf_count;
    unsigned int n_samples;
} inference_t;

static inference_t inference;
static bool record_ready = false;
static signed short *sampleBuffer;
static bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal
static int print_results = -(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW);

/**
 * @brief      Arduino setup function
 */
void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);

    Serial.println("Edge Impulse Inferencing Demo");

    M5.begin();
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(YELLOW,BLACK);
    InitI2SSpeakOrMic(MODE_MIC);

    // summary of inferencing settings (from model_metadata.h)
    ei_printf("Inferencing settings:\n");
    ei_printf("\tInterval: %.2f ms.\n", (float)EI_CLASSIFIER_INTERVAL_MS);
    ei_printf("\tFrame size: %d\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    ei_printf("\tSample length: %d ms.\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT / 16);
    ei_printf("\tNo. of classes: %d\n", sizeof(ei_classifier_inferencing_categories) /
                                            sizeof(ei_classifier_inferencing_categories[0]));

    run_classifier_init();
    if (microphone_inference_start(EI_CLASSIFIER_SLICE_SIZE) == false) {
        ei_printf("ERR: Failed to setup audio sampling\r\n");
        return;
    }
}

/**
 * @brief      Arduino main function. Runs the inferencing loop.
 */
void loop()
{
    bool m = microphone_inference_record();
    if (!m) {
        ei_printf("ERR: Failed to record audio...\n");
        return;
    }

    signal_t signal;
    signal.total_length = EI_CLASSIFIER_SLICE_SIZE;
    signal.get_data = &microphone_audio_signal_get_data;
    ei_impulse_result_t result = {0};

    EI_IMPULSE_ERROR r = run_classifier_continuous(&signal, &result, debug_nn);
    if (r != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", r);
        return;
    }

    if (++print_results >= (EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW)) {
        // print the predictions
        ei_printf("Predictions ");
        ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
            result.timing.dsp, result.timing.classification, result.timing.anomaly);
        ei_printf(": \n");

        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextSize(2);
        M5.Lcd.setTextColor(YELLOW,BLACK);
        
        for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
            ei_printf("    %s: %.5f\n", result.classification[ix].label,
                      result.classification[ix].value);
                      M5.Lcd.setTextPadding(320);
        M5.Lcd.printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
        }


        M5.Lcd.setCursor(160, 220);
  M5.Lcd.setTextColor(WHITE,BLACK);
  M5.Lcd.setTextSize(3);
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    if(result.classification[ix].value>=0.4){
                   if(ix==0){M5.Lcd.setCursor(160, 220);M5.Lcd.setTextPadding(320);M5.Lcd.print("  DOWN "); delay(500);}
              else if(ix==1){M5.Lcd.setCursor(160, 220);M5.Lcd.setTextPadding(320);M5.Lcd.print("   GO  "); delay(500);}
              else if(ix==2){M5.Lcd.setCursor(160, 220);M5.Lcd.setTextPadding(320);M5.Lcd.print("  LEFT ");  delay(500);}
              else if(ix==3){M5.Lcd.setCursor(160, 220);M5.Lcd.setTextPadding(320);M5.Lcd.print("   NO  "); delay(500);} 
              else if(ix==4){M5.Lcd.setCursor(160, 220);M5.Lcd.setTextPadding(320);M5.Lcd.print(" NOISE ");}
              else if(ix==5){M5.Lcd.setCursor(160, 220);M5.Lcd.setTextPadding(320);M5.Lcd.print("  OFF  "); delay(500);}
              else if(ix==6){M5.Lcd.setCursor(160, 220);M5.Lcd.setTextPadding(320);M5.Lcd.print("   ON  "); delay(500);}
              else if(ix==7){M5.Lcd.setCursor(160, 220);M5.Lcd.setTextPadding(320);M5.Lcd.print(" RIGHT "); delay(500);}
              else if(ix==8){M5.Lcd.setCursor(160, 220);M5.Lcd.setTextPadding(320);M5.Lcd.print("  STOP "); delay(500);}
              else if(ix==9){M5.Lcd.setCursor(160, 220);M5.Lcd.setTextPadding(320);M5.Lcd.print("   UP  "); delay(500);}
              else if(ix==10){M5.Lcd.setCursor(160,220);M5.Lcd.setTextPadding(320);M5.Lcd.print("  YES  "); delay(500);}
              }
  }          
#if EI_CLASSIFIER_HAS_ANOMALY == 1
        ei_printf("    anomaly score: %.3f\n", result.anomaly);
#endif

        print_results = 0;
    }
}

/**
 * @brief      Printf function uses vsnprintf and output using Arduino Serial
 *
 * @param[in]  format     Variable argument list
 */
void ei_printf(const char *format, ...) {
    static char print_buf[1024] = { 0 };

    va_list args;
    va_start(args, format);
    int r = vsnprintf(print_buf, sizeof(print_buf), format, args);
    va_end(args);

    if (r > 0) {
        Serial.write(print_buf);
    }
}

/**
 * @brief      PDM buffer full callback
 *             Get data and call audio thread callback
 */
static void pdm_data_ready_inference_callback(void)
{
    size_t bytesRead;
    i2s_read(Speak_I2S_NUMBER, (char *)(sampleBuffer), DATA_SIZE, &bytesRead, (100 / portTICK_RATE_MS));

    if (record_ready == true) {
        for (int i = 0; i<bytesRead>> 1; i++) {
            inference.buffers[inference.buf_select][inference.buf_count++] = sampleBuffer[i];

            if (inference.buf_count >= inference.n_samples) {
                inference.buf_select ^= 1;
                inference.buf_count = 0;
                inference.buf_ready = 1;
            }
        }
    }
}

/**
 * @brief      Init inferencing struct and setup/start PDM
 *
 * @param[in]  n_samples  The n samples
 *
 * @return     { description_of_the_return_value }
 */
static bool microphone_inference_start(uint32_t n_samples)
{
    inference.buffers[0] = (signed short *)malloc(n_samples * sizeof(signed short));

    if (inference.buffers[0] == NULL) {
        return false;
    }

    inference.buffers[1] = (signed short *)malloc(n_samples * sizeof(signed short));

    if (inference.buffers[0] == NULL) {
        free(inference.buffers[0]);
        return false;
    }

    sampleBuffer = (signed short *)malloc((n_samples >> 1) * sizeof(signed short));

    if (sampleBuffer == NULL) {
        free(inference.buffers[0]);
        free(inference.buffers[1]);
        return false;
    }

    inference.buf_select = 0;
    inference.buf_count = 0;
    inference.n_samples = n_samples;
    inference.buf_ready = 0;
    
    record_ready = true;

    return true;
}

/**
 * @brief      Wait on new data
 *
 * @return     True when finished
 */
static bool microphone_inference_record(void)
{
    bool ret = true;

    if (inference.buf_ready == 1) {
        ei_printf(
            "Error sample buffer overrun. Decrease the number of slices per model window "
            "(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW)\n");
        ret = false;
    }

    while (inference.buf_ready == 0) {
        pdm_data_ready_inference_callback();
    }

    inference.buf_ready = 0;

    return ret;
}

/**
 * Get raw audio signal data
 */
static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr)
{
    numpy::int16_to_float(&inference.buffers[inference.buf_select ^ 1][offset], out_ptr, length);

    return 0;
}

/**
 * @brief      Stop PDM and release buffers
 */
static void microphone_inference_end(void)
{
    
    free(inference.buffers[0]);
    free(inference.buffers[1]);
    free(sampleBuffer);
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_MICROPHONE
#error "Invalid model for current sensor."
#endif
