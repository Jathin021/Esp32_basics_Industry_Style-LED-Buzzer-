#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"

// MACROS
#define TAG "MAIN"
#define led_pin GPIO_NUM_2
void app_main(void)
{
    // Initialize the LED pin
    ESP_LOGI(TAG,"Starting LED blink example");
    gpio_reset_pin(led_pin);
    gpio_set_direction(led_pin,GPIO_MODE_OUTPUT);
    while (1)
    {
        // LED ON
        gpio_set_level(led_pin,1);
        ESP_LOGI(TAG,"LED ON");
        vTaskDelay(1000/portTICK_PERIOD_MS);
        // LED OFF
        gpio_set_level(led_pin, 0);
        ESP_LOGI(TAG,"LED OFF");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}