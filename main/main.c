#include "freertos/FreeRTOS.h"

#include "string.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_rom_sys.h"        // Correct inclusion for esp_rom_delay_us()

#include "nvs_flash.h"          // Required for NVS flash memory operations
#include "esp_bt.h"             // Bluetooth specific APIs
#include "esp_bt_main.h"        // Main BT stack initialization and enable/disable APIs
#include "esp_gap_ble_api.h"    // GAP (Generic Access Profile) BLE API
#include "esp_gattc_api.h"      // GATT Client functionality
#include "esp_gatt_defs.h"      // GATT definitions and structures
#include "esp_log.h"            // ESP32 logging


// Configuration
#define WHEEL_TICK_PIN GPIO_NUM_2 // The GPIO pin for outputting the wheel tick signal
#define TICK_INTERVAL_MS 1000      // Time interval between ticks in milliseconds
#define PULSE_WIDTH_US 50         // Pulse width of the wheel tick in microseconds


// ESP Logging Tag
static const char *TAG = "WheelTickGenerator";

//static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
//static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);


// BLE initialization function
void ble_init() {
    esp_err_t ret;

    // Initialize NVS — required for BLE storage operations (e.g., bonding devices)
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Release the memory dedicated to the classic Bluetooth stack
    // as we won't be using classic Bluetooth, only BLE.
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    // Initialize the Bluetooth controller with default settings
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "%s initialize controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    // Enable the Bluetooth controller in BLE mode
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    // Initialize Bluedroid Bluetooth stack. This API initializes and enables the Bluedroid stack,
    // making it ready to use the BLE and classic Bluetooth functionalities.
    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(TAG, "%s initialize bluedroid failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    // Enable Bluedroid. The Bluedroid stack needs to be enabled before any Bluetooth operations can be performed.
    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(TAG, "%s enable bluedroid failed: %s", __func__, esp_err_to_name(ret));
        return;
    }
}


// GAP callback function
static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    switch (event) {
        case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
            // Handle the completion of setting scan parameters
            break;

        case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
            // Handle the start of the scanning process
            break;

        case ESP_GAP_BLE_SCAN_RESULT_EVT:
            // Handle a scan result, which may include advertisement data from BLE devices
            break;

        case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
            // Handle the stop of the scanning process
            break;

        default:
            break;
    }
}

// GATT Client callback function
static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param) {
    switch (event) {
        case ESP_GATTC_REG_EVT:
            // Handle registration event
            break;

        case ESP_GATTC_CONNECT_EVT:
            // Handle connect event
            break;

        case ESP_GATTC_DISCONNECT_EVT:
            // Handle disconnect event
            break;

        // Add other cases to handle other GATT client events, such as ESP_GATTC_SEARCH_CMPL_EVT for service discovery completion.
        case ESP_GAP_BLE_SCAN_RESULT_EVT: {
            esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
            uint8_t *adv_name = NULL;
            uint8_t adv_name_len = 0;
            adv_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv,
                                        ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);
            // Check if the advertised name matches the Wahoo sensor's name
            if (adv_name != NULL && strncmp((char *)adv_name, "Wahoo Sensor Name", adv_name_len) == 0) {
                ESP_LOGI(TAG, "Wahoo sensor found, initiating connection...");
                // Stop scanning
                esp_ble_gap_stop_scanning();
                // Initiate connection
                esp_ble_gattc_open(gattc_if, scan_result->scan_rst.bda, scan_result->scan_rst.ble_addr_type, true);
                }
            break;
        }
        default:
            break;
    }
}


void ble_scan() {
    // Ensure ble_scan_params is defined somewhere in your code
    static esp_ble_scan_params_t ble_scan_params = {
        .scan_type = BLE_SCAN_TYPE_ACTIVE,
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
        .scan_interval = 0x50,
        .scan_window = 0x30,
        .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE
    };

    // Register the GAP callback function
    esp_ble_gap_register_callback(esp_gap_cb);

    // Set scan parameters and start scanning
    esp_ble_gap_set_scan_params(&ble_scan_params);
}


// Function to generate a single wheel tick
void generate_wheel_tick(void *params) {
    while (1) {
        // Set pin high to start the pulse
        gpio_set_level(WHEEL_TICK_PIN, 1);
        // Wait for the pulse width duration
        esp_rom_delay_us(PULSE_WIDTH_US);
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

    esp_err_t ret;

    // Initialize NVS required for BLE operations
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Release the memory dedicated to classic Bluetooth as we're focusing on BLE
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    // Initialize and enable the BLE controller with the default configuration
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    ESP_ERROR_CHECK(ret);
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    ESP_ERROR_CHECK(ret);

    // Initialize and enable Bluedroid
    ret = esp_bluedroid_init();
    ESP_ERROR_CHECK(ret);
    ret = esp_bluedroid_enable();
    ESP_ERROR_CHECK(ret);

    // Register GAP and GATT client callbacks
    ret = esp_ble_gap_register_callback(esp_gap_cb);
    if (ret) {
        ESP_LOGE(TAG, "GAP callback registration failed: %s", esp_err_to_name(ret));
    }
    ret = esp_ble_gattc_register_callback(esp_gattc_cb);
    if (ret) {
        ESP_LOGE(TAG, "GATT client callback registration failed: %s", esp_err_to_name(ret));
    }


    // Initialize the GPIO pin for the wheel tick output
    gpio_reset_pin(WHEEL_TICK_PIN); // Reset the pin to a known state
    gpio_set_direction(WHEEL_TICK_PIN, GPIO_MODE_OUTPUT);

    // Start a FreeRTOS task to generate wheel ticks indefinitely
    xTaskCreate(generate_wheel_tick, "generate_wheel_tick", 2048, NULL, 10, NULL);
}
