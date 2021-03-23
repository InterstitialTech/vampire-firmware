#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#define BLINK_GPIO 12

void heartBeat(void *arg) {

    gpio_pad_select_gpio(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    while(1) {

        /* Blink on (output low) */
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(100 / portTICK_PERIOD_MS);

        /* Blink off (output high) */
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(900 / portTICK_PERIOD_MS);

    }

}

void app_main(void) {

    xTaskCreate(heartBeat, "Heart Beat", 2048, NULL, 1, NULL);

}

