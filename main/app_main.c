/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_at.h"
#include "esp_at_internal.h"
#include "esp_at_init.h"

/* Status LED: 2 Hz blink from boot (GPIO5). Period 500 ms → toggle every 250 ms. */
#define AT_STATUS_LED_GPIO      6
#define AT_STATUS_LED_HALF_MS   250

static void at_status_led_task(void *arg)
{
    (void)arg;
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << AT_STATUS_LED_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io);
    gpio_set_level(AT_STATUS_LED_GPIO, 0);

    while (1) {
        gpio_set_level(AT_STATUS_LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(AT_STATUS_LED_HALF_MS));
        gpio_set_level(AT_STATUS_LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(AT_STATUS_LED_HALF_MS));
    }
}

void app_main(void)
{
    xTaskCreate(at_status_led_task, "status_led", 2048, NULL, 1, NULL);

    esp_at_main_preprocess();

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_at_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_at_init();
}
