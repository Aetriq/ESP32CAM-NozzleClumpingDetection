/* CEG4195 Project */
/* Author: Alex Gordon */
/* Last revision: 3-29-26 */

/* ===== 1.0 DEPENDENCIES & MACROS ===== */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_camera.h"

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

#include "edge-impulse-sdk/classifier/ei_run_classifier.h"

#define FLASH_LED_PIN GPIO_NUM_4
#define UART_PORT_NUM UART_NUM_0
#define BUF_SIZE 256
#define CROP_SIZE 96

/* ===== 2.0 GLOBALS & CALLBACKS ===== */
float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];

int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    memcpy(out_ptr, features + offset, length * sizeof(float));
    return 0;
}

/* ===== 3.0 HARDWARE INITIALIZATION ===== */
void init_hardware() {
    gpio_reset_pin(FLASH_LED_PIN);
    gpio_set_direction(FLASH_LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(FLASH_LED_PIN, 0);

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_param_config(UART_PORT_NUM, &uart_config);
    uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_RGB565; 
    config.frame_size = FRAMESIZE_240X240;
    config.fb_count = 1;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        printf("CAM_FAIL\n");
        vTaskDelay(portMAX_DELAY); 
    }
}

/* ===== 4.0 INFERENCE PIPELINE ===== */
void perform_scan() {
    gpio_set_level(FLASH_LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(200)); 

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        printf("CAP_FAIL\n");
        gpio_set_level(FLASH_LED_PIN, 0);
        return;
    }

    int start_x = (fb->width - CROP_SIZE) / 2;
    int start_y = (fb->height - CROP_SIZE) / 2;
    uint16_t *pixels = (uint16_t *)fb->buf;
    int dst_idx = 0;

    for (int y = start_y; y < start_y + CROP_SIZE; y++) {
        for (int x = start_x; x < start_x + CROP_SIZE; x++) {
            uint16_t pixel = pixels[y * fb->width + x];
            uint8_t r = (pixel & 0xF800) >> 8;
            uint8_t g = (pixel & 0x07E0) >> 3;
            uint8_t b = (pixel & 0x001F) << 3;
            
            features[dst_idx++] = (r << 16) | (g << 8) | b;
        }
    }

    esp_camera_fb_return(fb); 
    gpio_set_level(FLASH_LED_PIN, 0);

    ei_impulse_result_t result = { 0 };
    signal_t features_signal;
    features_signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
    features_signal.get_data = &raw_feature_get_data;

    EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false);
    if (res != 0) {
        printf("ML_ERR\n");
        return;
    }

    float max_val = 0.0;
    int best_idx = -1;
    for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        if (result.classification[i].value > max_val) {
            max_val = result.classification[i].value;
            best_idx = i;
        }
    }

    const char* best_class = result.classification[best_idx].label;
    if (strcmp(best_class, "blob") == 0) {
        printf("BLOB\n");
    } else if (strcmp(best_class, "extruded") == 0) {
        printf("EXTRUDED\n");
    } else {
        printf("CLEAN\n");
    }
}

/* ===== 5.0 MAIN RTOS TASK ===== */
extern "C" void app_main() {
    init_hardware();
    printf("READY\n");

    uint8_t data[BUF_SIZE + 1];
    while (1) {
        int len = uart_read_bytes(UART_PORT_NUM, data, BUF_SIZE, pdMS_TO_TICKS(100));
        if (len > 0) {
            data[len] = '\0'; 
            
            if (strstr((char*)data, "CHECK_NOZZLE") != NULL) {
                perform_scan();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}