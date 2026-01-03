/* "Ticking Time Bomb" Countdown Timer - ESP32 ESP-IDF
 * Dramatic countdown with accelerating ticks and explosion effect
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "TIME_BOMB";

// Pin definitions - Array of 5 LEDs [web:44][page:6]
#define NUM_LEDS 5
static const gpio_num_t led_pins[NUM_LEDS] = {
    GPIO_NUM_2,   // LED 1
    GPIO_NUM_4,   // LED 2
    GPIO_NUM_15,  // LED 3
    GPIO_NUM_18,  // LED 4
    GPIO_NUM_19   // LED 5
};

#define BUZZER_PIN      GPIO_NUM_5

// LEDC configuration for buzzer
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_BUZZER_CHANNEL     LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT
#define LEDC_DUTY               (4096)  // 50% duty cycle

// Countdown timing configuration
#define INITIAL_TICK_INTERVAL   1000    // 1 second per tick initially
#define ACCELERATED_TICK_INTERVAL 200   // 200ms per tick when urgent
#define TICK_FREQUENCY          1000    // Tick beep frequency (Hz)
#define EXPLOSION_FREQUENCY     100     // Low rumbling explosion sound (Hz)
#define EXPLOSION_DURATION      2000    // Explosion effect duration (ms)
#define FLASH_INTERVAL          100     // LED flash interval during explosion (ms)

// Countdown phases
typedef enum {
    PHASE_SETUP,
    PHASE_NORMAL_COUNTDOWN,
    PHASE_ACCELERATED_COUNTDOWN,
    PHASE_EXPLOSION,
    PHASE_RESET
} CountdownPhase;

// Function prototypes
void init_leds(void);
void init_buzzer(void);
void setup_phase(void);
void countdown_phase(int start_led, int end_led, int tick_interval);
void explosion_phase(void);
void turn_on_leds(int count);
void turn_off_all_leds(void);
void flash_all_leds(int times, int interval_ms);
void beep(int frequency, int duration_ms);
void tick_sound(void);
void explosion_sound(void);

void app_main(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);
    
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  Ticking Time Bomb - Countdown Timer");
    ESP_LOGI(TAG, "========================================");
    
    // Initialize hardware
    init_leds();
    init_buzzer();
    
    while(1) {
        // Phase 1: Setup - All LEDs ON
        ESP_LOGI(TAG, "PHASE: Setup");
        setup_phase();
        
        // Phase 2: Normal countdown (5 LEDs → 2 LEDs)
        ESP_LOGI(TAG, "PHASE: Normal Countdown");
        countdown_phase(NUM_LEDS - 1, 1, INITIAL_TICK_INTERVAL);
        
        // Phase 3: Accelerated countdown (Last LED)
        ESP_LOGI(TAG, "PHASE: CRITICAL - Accelerated Ticking!");
        countdown_phase(0, 0, ACCELERATED_TICK_INTERVAL);
        
        // Phase 4: Explosion
        ESP_LOGI(TAG, "PHASE: EXPLOSION!");
        explosion_phase();
        
        // Wait before restarting
        ESP_LOGI(TAG, "Resetting in 3 seconds...\n");
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void init_leds(void)
{
    // Configure all LED pins as outputs using array iteration [web:44][page:6]
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = 0
    };
    
    // Build pin bit mask from array [page:6]
    for (int i = 0; i < NUM_LEDS; i++) {
        io_conf.pin_bit_mask |= (1ULL << led_pins[i]);
    }
    
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    turn_off_all_leds();
    
    ESP_LOGI(TAG, "Initialized %d LEDs", NUM_LEDS);
}

void init_buzzer(void)
{
    // Configure LEDC timer for buzzer [web:18]
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = TICK_FREQUENCY,
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
    
    ESP_LOGI(TAG, "Buzzer initialized on GPIO%d", BUZZER_PIN);
}

void turn_on_leds(int count)
{
    // Turn on specified number of LEDs from the array [web:44]
    for (int i = 0; i < NUM_LEDS; i++) {
        if (i < count) {
            gpio_set_level(led_pins[i], 1);
        } else {
            gpio_set_level(led_pins[i], 0);
        }
    }
}

void turn_off_all_leds(void)
{
    // Turn off all LEDs using array iteration [web:44]
    for (int i = 0; i < NUM_LEDS; i++) {
        gpio_set_level(led_pins[i], 0);
    }
}

void flash_all_leds(int times, int interval_ms)
{
    // Flash all LEDs specified number of times [web:44]
    for (int i = 0; i < times; i++) {
        for (int j = 0; j < NUM_LEDS; j++) {
            gpio_set_level(led_pins[j], 1);
        }
        vTaskDelay(pdMS_TO_TICKS(interval_ms));
        
        turn_off_all_leds();
        vTaskDelay(pdMS_TO_TICKS(interval_ms));
    }
}

void beep(int frequency, int duration_ms)
{
    // Generate beep at specified frequency [web:18]
    ESP_ERROR_CHECK(ledc_set_freq(LEDC_MODE, LEDC_TIMER, frequency));
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_BUZZER_CHANNEL, LEDC_DUTY));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_BUZZER_CHANNEL));
    
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    
    // Turn off buzzer
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_BUZZER_CHANNEL, 0));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_BUZZER_CHANNEL));
}

void tick_sound(void)
{
    // Short tick beep (100ms) [web:39]
    beep(TICK_FREQUENCY, 100);
}

void explosion_sound(void)
{
    // Low frequency rumbling explosion sound [web:62]
    beep(EXPLOSION_FREQUENCY, EXPLOSION_DURATION);
}

void setup_phase(void)
{
    // Turn on all LEDs to show full countdown [web:44]
    turn_on_leds(NUM_LEDS);
    ESP_LOGI(TAG, "All %d LEDs ON - Timer Armed", NUM_LEDS);
    vTaskDelay(pdMS_TO_TICKS(2000));  // Display for 2 seconds
}

void countdown_phase(int start_led, int end_led, int tick_interval)
{
    // Countdown from start_led to end_led with specified interval
    int direction = (start_led > end_led) ? -1 : 1;
    int current_led = start_led;
    
    while (true) {
        // Display current LED count
        turn_on_leds(current_led + 1);
        ESP_LOGI(TAG, "LEDs remaining: %d", current_led + 1);
        
        // Emit tick sound
        tick_sound();
        
        // Check if we've reached the end
        if (current_led == end_led) {
            // For the last LED in accelerated phase, do multiple rapid ticks
            if (tick_interval == ACCELERATED_TICK_INTERVAL && end_led == 0) {
                for (int i = 0; i < 5; i++) {
                    vTaskDelay(pdMS_TO_TICKS(ACCELERATED_TICK_INTERVAL));
                    tick_sound();
                }
            }
            break;
        }
        
        // Wait for next tick (variable delay for acceleration effect)
        vTaskDelay(pdMS_TO_TICKS(tick_interval));
        
        // Move to next LED
        current_led += direction;
    }
}

void explosion_phase(void)
{
    // Rapid LED flashing with explosion sound [web:44][web:62]
    ESP_LOGI(TAG, "*** BOOM! ***");
    
    // Start explosion sound in parallel with LED effects
    ESP_ERROR_CHECK(ledc_set_freq(LEDC_MODE, LEDC_TIMER, EXPLOSION_FREQUENCY));
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_BUZZER_CHANNEL, LEDC_DUTY));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_BUZZER_CHANNEL));
    
    // Flash all LEDs rapidly during explosion
    int num_flashes = EXPLOSION_DURATION / (FLASH_INTERVAL * 2);
    flash_all_leds(num_flashes, FLASH_INTERVAL);
    
    // Turn off buzzer
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_BUZZER_CHANNEL, 0));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_BUZZER_CHANNEL));
    
    ESP_LOGI(TAG, "Explosion complete");
}
