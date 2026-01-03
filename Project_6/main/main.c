/* Automated Traffic Light System (Blind-Friendly Variant) - ESP32 ESP-IDF
 * Implements realistic traffic signal with audio cues for visually impaired
 * Uses Finite State Machine pattern for clean, maintainable code
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "TRAFFIC_LIGHT";

// Pin definitions
#define RED_LED_PIN     GPIO_NUM_2
#define YELLOW_LED_PIN  GPIO_NUM_4
#define GREEN_LED_PIN   GPIO_NUM_15
#define BUZZER_PIN      GPIO_NUM_5

// LEDC configuration for buzzer
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_BUZZER_CHANNEL     LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT
#define LEDC_DUTY               (4096)  // 50% duty cycle
#define BEEP_FREQUENCY          800     // Hz - pedestrian crossing beep

// Traffic light timing configuration (in milliseconds)
#define RED_DURATION            5000    // 5 seconds
#define RED_TO_YELLOW_DURATION  2000    // 2 seconds (prepare to go)
#define GREEN_DURATION          5000    // 5 seconds
#define GREEN_TO_YELLOW_DURATION 2000   // 2 seconds (prepare to stop)
#define BEEP_INTERVAL           1000    // Beep every 1 second during green
#define BEEP_DURATION           200     // 200ms beep

// Traffic light states using enum [web:67][web:69]
typedef enum {
    STATE_RED,                  // Stop - No buzzer
    STATE_RED_TO_YELLOW,        // Prepare to go
    STATE_GREEN,                // Go - Beeping for blind-friendly
    STATE_GREEN_TO_YELLOW,      // Prepare to stop
    STATE_MAX                   // Total number of states
} TrafficLightState;

// State names for logging [web:67]
static const char* state_names[] = {
    "RED (STOP)",
    "YELLOW (READY)",
    "GREEN (GO - Safe to Cross)",
    "YELLOW (CAUTION)"
};

// State duration mapping [web:67]
static const uint32_t state_durations[] = {
    RED_DURATION,
    RED_TO_YELLOW_DURATION,
    GREEN_DURATION,
    GREEN_TO_YELLOW_DURATION
};

// Traffic light context structure [web:69][web:71]
typedef struct {
    TrafficLightState current_state;
    uint32_t state_start_time;
    uint32_t last_beep_time;
    bool beep_active;
} TrafficLightContext;

// Global context
static TrafficLightContext traffic_context = {
    .current_state = STATE_RED,
    .state_start_time = 0,
    .last_beep_time = 0,
    .beep_active = false
};

// Function prototypes
void init_traffic_leds(void);
void init_buzzer(void);
void set_traffic_light(TrafficLightState state);
void turn_off_all_lights(void);
void beep_on(void);
void beep_off(void);
void handle_state_red(void);
void handle_state_red_to_yellow(void);
void handle_state_green(void);
void handle_state_green_to_yellow(void);
void state_machine_run(void);
void transition_to_state(TrafficLightState new_state);
uint32_t millis(void);

void app_main(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);
    
    ESP_LOGI(TAG, "================================================");
    ESP_LOGI(TAG, "  Automated Traffic Light System");
    ESP_LOGI(TAG, "  (Blind-Friendly with Audio Cues)");
    ESP_LOGI(TAG, "================================================");
    
    // Initialize hardware
    init_traffic_leds();
    init_buzzer();
    
    // Initialize state machine [web:67]
    traffic_context.current_state = STATE_RED;
    traffic_context.state_start_time = millis();
    
    ESP_LOGI(TAG, "Traffic light system started");
    ESP_LOGI(TAG, "Audio cues enabled during GREEN phase");
    ESP_LOGI(TAG, "================================================\n");
    
    // Main loop - run state machine continuously [web:67]
    while(1) {
        state_machine_run();
        vTaskDelay(pdMS_TO_TICKS(10));  // Small delay to prevent CPU hogging
    }
}

uint32_t millis(void)
{
    // ESP-IDF equivalent of Arduino millis() [web:59]
    return (uint32_t)(esp_timer_get_time() / 1000ULL);
}

void init_traffic_leds(void)
{
    // Configure all traffic light LEDs as outputs [web:44]
    gpio_config_t io_conf = {
        .pin_bit_mask = ((1ULL << RED_LED_PIN) | 
                        (1ULL << YELLOW_LED_PIN) | 
                        (1ULL << GREEN_LED_PIN)),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    
    turn_off_all_lights();
    ESP_LOGI(TAG, "Traffic LEDs initialized (R:%d, Y:%d, G:%d)", 
             RED_LED_PIN, YELLOW_LED_PIN, GREEN_LED_PIN);
}

void init_buzzer(void)
{
    // Configure LEDC timer for buzzer [web:18]
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = BEEP_FREQUENCY,
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
    
    ESP_LOGI(TAG, "Buzzer initialized on GPIO%d at %d Hz", BUZZER_PIN, BEEP_FREQUENCY);
}

void turn_off_all_lights(void)
{
    gpio_set_level(RED_LED_PIN, 0);
    gpio_set_level(YELLOW_LED_PIN, 0);
    gpio_set_level(GREEN_LED_PIN, 0);
}

void set_traffic_light(TrafficLightState state)
{
    // Turn off all lights first [web:44]
    turn_off_all_lights();
    
    // Turn on appropriate light based on state [web:67]
    switch(state) {
        case STATE_RED:
            gpio_set_level(RED_LED_PIN, 1);
            break;
            
        case STATE_RED_TO_YELLOW:
        case STATE_GREEN_TO_YELLOW:
            gpio_set_level(YELLOW_LED_PIN, 1);
            break;
            
        case STATE_GREEN:
            gpio_set_level(GREEN_LED_PIN, 1);
            break;
            
        default:
            break;
    }
}

void beep_on(void)
{
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_BUZZER_CHANNEL, LEDC_DUTY));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_BUZZER_CHANNEL));
    traffic_context.beep_active = true;
}

void beep_off(void)
{
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_BUZZER_CHANNEL, 0));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_BUZZER_CHANNEL));
    traffic_context.beep_active = false;
}

void transition_to_state(TrafficLightState new_state)
{
    // Log state transition [web:67]
    ESP_LOGI(TAG, "State Transition: %s -> %s", 
             state_names[traffic_context.current_state],
             state_names[new_state]);
    
    // Update state [web:67]
    traffic_context.current_state = new_state;
    traffic_context.state_start_time = millis();
    traffic_context.last_beep_time = 0;
    
    // Turn off buzzer during state transition
    beep_off();
    
    // Set appropriate traffic light
    set_traffic_light(new_state);
}

void handle_state_red(void)
{
    // RED state: STOP - No buzzer [web:67]
    // Simply wait for duration to expire
    uint32_t elapsed = millis() - traffic_context.state_start_time;
    
    if (elapsed >= state_durations[STATE_RED]) {
        transition_to_state(STATE_RED_TO_YELLOW);
    }
}

void handle_state_red_to_yellow(void)
{
    // YELLOW (after RED): READY - Prepare to go [web:67]
    uint32_t elapsed = millis() - traffic_context.state_start_time;
    
    if (elapsed >= state_durations[STATE_RED_TO_YELLOW]) {
        transition_to_state(STATE_GREEN);
    }
}

void handle_state_green(void)
{
    // GREEN state: GO - Safe to cross with periodic beeps [web:67]
    uint32_t current_time = millis();
    uint32_t elapsed = current_time - traffic_context.state_start_time;
    
    // Handle periodic beeping for blind-friendly assistance
    if (traffic_context.last_beep_time == 0 || 
        (current_time - traffic_context.last_beep_time >= BEEP_INTERVAL)) {
        
        // Start new beep
        beep_on();
        traffic_context.last_beep_time = current_time;
        ESP_LOGD(TAG, "Beep: Safe to cross");
    }
    
    // Turn off beep after BEEP_DURATION
    if (traffic_context.beep_active && 
        (current_time - traffic_context.last_beep_time >= BEEP_DURATION)) {
        beep_off();
    }
    
    // Check if state duration expired
    if (elapsed >= state_durations[STATE_GREEN]) {
        beep_off();  // Ensure buzzer is off before transition
        transition_to_state(STATE_GREEN_TO_YELLOW);
    }
}

void handle_state_green_to_yellow(void)
{
    // YELLOW (after GREEN): CAUTION - Prepare to stop [web:67]
    uint32_t elapsed = millis() - traffic_context.state_start_time;
    
    if (elapsed >= state_durations[STATE_GREEN_TO_YELLOW]) {
        transition_to_state(STATE_RED);
    }
}

void state_machine_run(void)
{
    // Main state machine dispatcher using switch-case [web:67][web:69][page:7]
    switch(traffic_context.current_state) {
        case STATE_RED:
            handle_state_red();
            break;
            
        case STATE_RED_TO_YELLOW:
            handle_state_red_to_yellow();
            break;
            
        case STATE_GREEN:
            handle_state_green();
            break;
            
        case STATE_GREEN_TO_YELLOW:
            handle_state_green_to_yellow();
            break;
            
        default:
            ESP_LOGE(TAG, "Unknown state: %d", traffic_context.current_state);
            transition_to_state(STATE_RED);
            break;
    }
}
