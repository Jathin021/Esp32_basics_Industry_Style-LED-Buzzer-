/* Digital Melody Player - Star Wars Imperial March
 * ESP32 ESP-IDF Implementation
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_err.h"
// Pin definitions
#define BUZZER_PIN      GPIO_NUM_5
#define LED1_PIN        GPIO_NUM_2   // Low notes
#define LED2_PIN        GPIO_NUM_4   // Mid notes
#define LED3_PIN        GPIO_NUM_15  // High notes
// LEDC configuration
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_BUZZER_CHANNEL     LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT
#define LEDC_DUTY               (4096) // 50% duty cycle
// Musical note structure
typedef struct {
    int frequency;      // Note frequency in Hz (0 = rest)
    int duration;       // Duration in milliseconds
} Note;
// Note frequency definitions (in Hz) [web:31][page:3]
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define REST     0
// Tempo setting (120 BPM) [page:3]
#define TEMPO 120
#define WHOLE_NOTE ((60000 * 4) / TEMPO)
// Helper function to calculate note duration
int calc_duration(int divider) {
    if (divider > 0) {
        return WHOLE_NOTE / divider;
    } else {
        // Dotted notes (negative values) [page:3]
        return (WHOLE_NOTE / abs(divider)) * 1.5;
    }
}
// Imperial March melody - complete version [web:31][page:3]
// Format: {frequency, duration_divider}
// Negative durations represent dotted notes
int imperial_march_raw[][2] = {
    // Main theme
    {NOTE_A4, -4}, {NOTE_A4, -4}, {NOTE_A4, 16}, {NOTE_A4, 16}, 
    {NOTE_A4, 16}, {NOTE_A4, 16}, {NOTE_F4, 8}, {REST, 8},
    {NOTE_A4, -4}, {NOTE_A4, -4}, {NOTE_A4, 16}, {NOTE_A4, 16}, 
    {NOTE_A4, 16}, {NOTE_A4, 16}, {NOTE_F4, 8}, {REST, 8},
    {NOTE_A4, 4}, {NOTE_A4, 4}, {NOTE_A4, 4}, {NOTE_F4, -8}, {NOTE_C5, 16},    
    // Section 1
    {NOTE_A4, 4}, {NOTE_F4, -8}, {NOTE_C5, 16}, {NOTE_A4, 2},
    {NOTE_E5, 4}, {NOTE_E5, 4}, {NOTE_E5, 4}, {NOTE_F5, -8}, {NOTE_C5, 16},
    {NOTE_A4, 4}, {NOTE_F4, -8}, {NOTE_C5, 16}, {NOTE_A4, 2},   
    // Section 2
    {NOTE_A5, 4}, {NOTE_A4, -8}, {NOTE_A4, 16}, {NOTE_A5, 4}, 
    {NOTE_GS5, -8}, {NOTE_G5, 16},
    {NOTE_DS5, 16}, {NOTE_D5, 16}, {NOTE_DS5, 8}, {REST, 8}, 
    {NOTE_A4, 8}, {NOTE_DS5, 4}, {NOTE_D5, -8}, {NOTE_CS5, 16},
    
    {NOTE_C5, 16}, {NOTE_B4, 16}, {NOTE_C5, 16}, {REST, 8}, 
    {NOTE_F4, 8}, {NOTE_GS4, 4}, {NOTE_F4, -8}, {NOTE_A4, -16},
    {NOTE_C5, 4}, {NOTE_A4, -8}, {NOTE_C5, 16}, {NOTE_E5, 2},  
    // Section 3 (repeat of section 2)
    {NOTE_A5, 4}, {NOTE_A4, -8}, {NOTE_A4, 16}, {NOTE_A5, 4}, 
    {NOTE_GS5, -8}, {NOTE_G5, 16},
    {NOTE_DS5, 16}, {NOTE_D5, 16}, {NOTE_DS5, 8}, {REST, 8}, 
    {NOTE_A4, 8}, {NOTE_DS5, 4}, {NOTE_D5, -8}, {NOTE_CS5, 16},  
    {NOTE_C5, 16}, {NOTE_B4, 16}, {NOTE_C5, 16}, {REST, 8}, 
    {NOTE_F4, 8}, {NOTE_GS4, 4}, {NOTE_F4, -8}, {NOTE_A4, -16},
    {NOTE_A4, 4}, {NOTE_F4, -8}, {NOTE_C5, 16}, {NOTE_A4, 2},
};
#define MELODY_LENGTH (sizeof(imperial_march_raw) / sizeof(imperial_march_raw[0]))
// Function prototypes
void init_buzzer(void);
void init_leds(void);
void play_note(int frequency, int duration);
void play_melody(void);
void update_leds(int frequency);
void leds_off(void);

void app_main(void)
{
    // Initialize hardware
    init_buzzer();
    init_leds();
    
    printf("Digital Jukebox - Star Wars Imperial March\n");
    printf("Melody Length: %d notes\n", MELODY_LENGTH);
    
    while(1) {
        printf("Playing: Star Wars Imperial March\n");
        play_melody();
        printf("Melody complete. Restarting in 5 seconds...\n");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void init_buzzer(void)
{
    // Configure LEDC timer for buzzer [web:18]
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = 1000,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
    
    // Configure LEDC channel for buzzer [web:18]
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_BUZZER_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = BUZZER_PIN,
        .duty           = 0,
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void init_leds(void)
{
    // Configure LED GPIO pins as outputs [web:17]
    gpio_config_t io_conf = {
        .pin_bit_mask = ((1ULL << LED1_PIN) | (1ULL << LED2_PIN) | (1ULL << LED3_PIN)),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    leds_off();
}

void play_note(int frequency, int duration)
{
    if (frequency == 0) {
        // Rest - silence [page:3]
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_BUZZER_CHANNEL, 0));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_BUZZER_CHANNEL));
        leds_off();
    } else {
        // Play note with specified frequency [web:18]
        ESP_ERROR_CHECK(ledc_set_freq(LEDC_MODE, LEDC_TIMER, frequency));
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_BUZZER_CHANNEL, LEDC_DUTY));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_BUZZER_CHANNEL));
        
        // Update LED pattern based on frequency [web:37]
        update_leds(frequency);
    }
    
    // Play for 90% of duration (10% pause between notes) [page:3]
    vTaskDelay(pdMS_TO_TICKS(duration * 0.9));
    
    // Brief silence between notes
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_BUZZER_CHANNEL, 0));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_BUZZER_CHANNEL));
    leds_off();
    vTaskDelay(pdMS_TO_TICKS(duration * 0.1));
}

void play_melody(void)
{
    // Iterate through all notes in the Imperial March [page:3]
    for (int i = 0; i < MELODY_LENGTH; i++) {
        int frequency = imperial_march_raw[i][0];
        int divider = imperial_march_raw[i][1];
        int duration = calc_duration(divider);
        
        play_note(frequency, duration);
    }
}

void update_leds(int frequency)
{
    // Turn on different LEDs based on note frequency range [web:37]
    leds_off();
    
    if (frequency < 400) {
        // Low notes (A4 and below) - LED1
        gpio_set_level(LED1_PIN, 1);
    } else if (frequency < 650) {
        // Mid notes (A4-E5) - LED2
        gpio_set_level(LED2_PIN, 1);
    } else {
        // High notes (E5 and above) - LED3
        gpio_set_level(LED3_PIN, 1);
    }
}

void leds_off(void)
{
    gpio_set_level(LED1_PIN, 0);
    gpio_set_level(LED2_PIN, 0);
    gpio_set_level(LED3_PIN, 0);
}
