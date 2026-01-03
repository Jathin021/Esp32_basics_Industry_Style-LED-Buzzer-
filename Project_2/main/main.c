#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_timer.h"
#include "esp_log.h"

/* TAG for logging */
static const char *TAG = "POLICE_SIREN";

/* GPIO pins */
#define RED_LED     18
#define BLUE_LED    19
#define BUZZER      21

/* Timing */
#define LED_TIME_MS     200
#define BUZZER_TIME_MS   5

/* Buzzer frequency */
#define FREQ_MIN   600
#define FREQ_MAX  1200
#define FREQ_STEP    5

bool led_on = false;
bool freq_up = true;

int buzzer_freq = FREQ_MIN;

uint64_t last_led_time = 0;
uint64_t last_buzzer_time = 0;

/* Buzzer setup */
void buzzer_start(void)
{
    ESP_LOGI(TAG, "Initializing buzzer PWM");

    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = buzzer_freq,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer);

    ledc_channel_config_t channel = {
        .gpio_num = BUZZER,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 950,
        .hpoint = 0
    };
    ledc_channel_config(&channel);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 950);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    ESP_LOGI(TAG, "Buzzer started at %d Hz", buzzer_freq);
}

void app_main(void)
{
    ESP_LOGI(TAG, "Police Siren Project Started");

    /* LED pins setup */
    gpio_config_t led_cfg = {
        .pin_bit_mask = (1ULL << RED_LED) | (1ULL << BLUE_LED),
        .mode = GPIO_MODE_OUTPUT
    };
    gpio_config(&led_cfg);

    ESP_LOGI(TAG, "LED GPIO configured");

    buzzer_start();

    while (1)
    {
        uint64_t now = esp_timer_get_time() / 1000;

        /* LED blinking */
        if (now - last_led_time >= LED_TIME_MS)
        {
            led_on = !led_on;
            gpio_set_level(RED_LED, led_on);
            gpio_set_level(BLUE_LED, !led_on);

            ESP_LOGI(TAG, "LED switched: RED=%d BLUE=%d",
                     led_on, !led_on);

            last_led_time = now;
        }

        /* Buzzer smooth siren */
        if (now - last_buzzer_time >= BUZZER_TIME_MS)
        {
            if (freq_up)
            {
                buzzer_freq += FREQ_STEP;
                if (buzzer_freq >= FREQ_MAX)
                {
                    freq_up = false;
                    ESP_LOGI(TAG, "Reached MAX frequency");
                }
            }
            else
            {
                buzzer_freq -= FREQ_STEP;
                if (buzzer_freq <= FREQ_MIN)
                {
                    freq_up = true;
                    ESP_LOGI(TAG, "Reached MIN frequency");
                }
            }
            ledc_timer_config_t timer = {
                .speed_mode = LEDC_LOW_SPEED_MODE,
                .timer_num = LEDC_TIMER_0,
                .duty_resolution = LEDC_TIMER_10_BIT,
                .freq_hz = buzzer_freq,
                .clk_cfg = LEDC_AUTO_CLK
            };
            ledc_timer_config(&timer);
            ESP_LOGI(TAG, "Buzzer frequency: %d Hz", buzzer_freq);
            last_buzzer_time = now;
        }
        vTaskDelay(1);
    }
}
