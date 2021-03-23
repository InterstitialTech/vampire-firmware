#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/adc.h"

#define LED_GPIO 12

#define NBLINKS_NORMAL 1
#define NBLINKS_ADC_FAILURE 2
#define NBLINKS_MODEM_FAILURE 3

volatile uint16_t VBAT = 0;
volatile uint16_t NBLINKS = 0;

void heartBeat(void *arg) {

    gpio_pad_select_gpio(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    while(1) {

        /* Blink on (output low) */
        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(100 / portTICK_PERIOD_MS);

        /* Blink off (output high) */
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(900 / portTICK_PERIOD_MS);

    }

}

void measureBattery(void *arg) {

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL_0, ADC_ATTEN_DB_0);

    while (1) {
        VBAT = adc1_get_raw(ADC_CHANNEL_0);
        printf("VBAT = %d\n", VBAT);
        vTaskDelay(1200 / portTICK_PERIOD_MS);
    }

}

void app_main(void) {

    xTaskCreate(heartBeat, "Heart Beat", 2048, NULL, 1, NULL);
    xTaskCreate(measureBattery, "Measure Battery", 2048, NULL, 1, NULL);

}

