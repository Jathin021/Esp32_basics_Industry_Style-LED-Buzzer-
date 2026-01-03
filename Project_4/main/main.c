/* SOS Morse Code Beacon - ESP32 ESP-IDF
 * Implements continuous SOS transmission with LED and buzzer synchronization
 * Uses ESP_LOG for structured logging
 */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
// Tag for logging [web:47][web:55]
static const char *TAG = "SOS_BEACON";
// Pin definitions
#define LED_PIN         GPIO_NUM_2      // High-intensity LED
#define BUZZER_PIN      GPIO_NUM_5      // Buzzer

// LEDC configuration for buzzer tone
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_BUZZER_CHANNEL     LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT
#define LEDC_DUTY               (4096)  // 50% duty cycle
#define BUZZER_FREQUENCY        1000    // 1 kHz beep tone

// Morse code timing constants (standard ITU timing)
#define TIME_UNIT       200             // Base time unit in milliseconds
#define DOT_DURATION    (TIME_UNIT * 1) // Dot = 1 unit
#define DASH_DURATION   (TIME_UNIT * 3) // Dash = 3 units
#define SYMBOL_SPACE    (TIME_UNIT * 1) // Space between dots/dashes = 1 unit
#define LETTER_SPACE    (TIME_UNIT * 3) // Space between letters = 3 units
#define WORD_SPACE      (TIME_UNIT * 7) // Space between words = 7 units

// Morse code symbols
typedef enum {
    DOT,
    DASH,
    SPACE_SYMBOL,
    SPACE_LETTER,
    SPACE_WORD
} MorseSymbol;

// SOS pattern: S(. . .) O(- - -) S(. . .)
MorseSymbol sos_pattern[] = {
    // Letter S: ...
    DOT, SYMBOL_SPACE, DOT, SYMBOL_SPACE, DOT,
    SPACE_LETTER,
    // Letter O: ---
    DASH, SYMBOL_SPACE, DASH, SYMBOL_SPACE, DASH,
    SPACE_LETTER,
    // Letter S: ...
    DOT, SYMBOL_SPACE, DOT, SYMBOL_SPACE, DOT,
    SPACE_WORD  // Pause before repeat
};

#define PATTERN_LENGTH (sizeof(sos_pattern) / sizeof(MorseSymbol))

// Buffer for building morse code output string
static char morse_buffer[128];
static int morse_pos = 0;

// Function prototypes
void init_gpio(void);
void init_buzzer(void);
void transmit_symbol(MorseSymbol symbol);
void signal_on(int duration_ms);
void signal_off(int duration_ms);
void transmit_sos(void);
void morse_buffer_add(const char* str);
void morse_buffer_clear(void);

void app_main(void)
{
    // Set log level [web:47][web:55]
    esp_log_level_set("*", ESP_LOG_INFO);           // Set all components to INFO
    esp_log_level_set(TAG, ESP_LOG_INFO);           // Set this application to INFO
    
    // Initialize hardware
    init_gpio();
    init_buzzer();
    
    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "SOS Morse Code Beacon - ESP32 ESP-IDF");
    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "Morse Code: ... --- ... (SOS)");
    ESP_LOGI(TAG, "Dot duration: %d ms", DOT_DURATION);
    ESP_LOGI(TAG, "Dash duration: %d ms", DASH_DURATION);
    ESP_LOGI(TAG, "Pattern length: %d symbols", PATTERN_LENGTH);
    ESP_LOGI(TAG, "Transmitting continuously...");
    ESP_LOGI(TAG, "===========================================");
    
    // Continuous transmission loop
    while(1) {
        transmit_sos();
    }
}
void init_gpio(void)
{
    // Configure LED pin as output
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);  // Start with LED off
    
    ESP_LOGI(TAG, "LED initialized on GPIO%d", LED_PIN);
}
void init_buzzer(void)
{
    // Configure LEDC timer for buzzer
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = BUZZER_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    esp_err_t ret = ledc_timer_config(&ledc_timer);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure LEDC timer: %s", esp_err_to_name(ret));
        return;
    }
    // Configure LEDC channel for buzzer
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_BUZZER_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = BUZZER_PIN,
        .duty           = 0,
        .hpoint         = 0
    };
    ret = ledc_channel_config(&ledc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure LEDC channel: %s", esp_err_to_name(ret));
        return;
    }   
    ESP_LOGI(TAG, "Buzzer initialized on GPIO%d at %d Hz", BUZZER_PIN, BUZZER_FREQUENCY);
}
void signal_on(int duration_ms)
{
    // Turn on LED
    gpio_set_level(LED_PIN, 1);
    // Turn on buzzer by setting duty cycle
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_BUZZER_CHANNEL, LEDC_DUTY));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_BUZZER_CHANNEL));   
    // Wait for specified duration
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
}
void signal_off(int duration_ms)
{
    // Turn off LED
    gpio_set_level(LED_PIN, 0);
    // Turn off buzzer by setting duty to 0
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_BUZZER_CHANNEL, 0));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_BUZZER_CHANNEL));   
    // Wait for specified duration
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
}
void morse_buffer_add(const char* str)
{
    int len = strlen(str);
    if (morse_pos + len < sizeof(morse_buffer) - 1) {
        strcpy(morse_buffer + morse_pos, str);
        morse_pos += len;
    }
}
void morse_buffer_clear(void)
{
    morse_buffer[0] = '\0';
    morse_pos = 0;
}
void transmit_symbol(MorseSymbol symbol)
{
    switch(symbol) {
        case DOT:
            morse_buffer_add(".");
            signal_on(DOT_DURATION);    // Short beep and LED flash
            break;
        case DASH:
            morse_buffer_add("-");
            signal_on(DASH_DURATION);   // Long beep and LED flash
            break;
        case SPACE_SYMBOL:
            // Space between dots/dashes within same letter
            signal_off(SPACE_SYMBOL);
            break;
        case SPACE_LETTER:
            // Space between letters
            morse_buffer_add(" ");
            signal_off(LETTER_SPACE);
            break;           
        case SPACE_WORD:
            // Space between words (end of SOS)
            signal_off(WORD_SPACE);
            break;
    }
}
void transmit_sos(void)
{
    morse_buffer_clear();
    // Iterate through the SOS pattern array
    for (int i = 0; i < PATTERN_LENGTH; i++) {
        transmit_symbol(sos_pattern[i]);
    }    
    // Log the complete morse code pattern [web:53]
    ESP_LOGI(TAG, "Transmitted SOS: %s", morse_buffer);
    ESP_LOGD(TAG, "Transmission complete. Repeating...");
}
