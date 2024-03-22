#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

// Configuration
#define WHEEL_TICK_PIN GPIO_NUM_2 // The GPIO pin for outputting the wheel tick signal
#define TICK_INTERVAL_MS 100      // Time interval between ticks in milliseconds
#define PULSE_WIDTH_US 50         // Pulse width of the wheel tick in microseconds

// Tag used for ESP log messages
static const char *TAG = "WheelTickGenerator";

// Function to generate a single wheel tick
void generate_wheel_tick(void *params) {
    while (1) {
        // Set pin high to start the pulse
        gpio_set_level(WHEEL_TICK_PIN, 1);
        // Wait for the pulse width duration
        ets_delay_us(PULSE_WIDTH_US);
        // Set pin low to end the pulse
        gpio_set_level(WHEEL_TICK_PIN, 0);

        // Log the tick for debugging purposes
        ESP_LOGI(TAG, "Generated wheel tick");

        // Wait for the next tick interval
        vTaskDelay(pdMS_TO_TICKS(TICK_INTERVAL_MS));
    }
}

// Main application entry point
void app_main(void) {
    // Configure the GPIO pin for the wheel tick output
    gpio_pad_select_gpio(WHEEL_TICK_PIN);
    gpio_set_direction(WHEEL_TICK_PIN, GPIO_MODE_OUTPUT);

    // Start a FreeRTOS task to generate wheel ticks indefinitely
    xTaskCreate(generate_wheel_tick, "generate_wheel_tick", 2048, NULL, 10, NULL);
}
