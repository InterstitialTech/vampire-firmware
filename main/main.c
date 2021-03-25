#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/adc.h"

#include "modem.h"

#define NBLINKS_NORMAL 1
#define NBLINKS_ADC_FAILURE 2
#define NBLINKS_MODEM_FAILURE 3

#define PIN_LED                 12 

volatile uint16_t VBAT = 0;
volatile uint16_t NBLINKS = NBLINKS_NORMAL;
volatile float LAT = 0.0;
volatile float LON = 0.0;

void delay_ms(unsigned long ms) {

    vTaskDelay(ms / portTICK_PERIOD_MS);

}

void heartBeat(void *arg) {

    gpio_pad_select_gpio(PIN_LED);
    gpio_set_direction(PIN_LED, GPIO_MODE_OUTPUT);

    while(1) {

        /* Blink NBLINKS times */
        for (int i=0; i<NBLINKS; i++) {
            gpio_set_level(PIN_LED, 0);
            delay_ms(100);
            gpio_set_level(PIN_LED, 1);
            delay_ms(100 );
        }

        /* Off for one second */
        gpio_set_level(PIN_LED, 1);
        delay_ms(900 );

    }

}

void measureBattery(void *arg) {

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL_0, ADC_ATTEN_DB_0);

    while (1) {
        VBAT = adc1_get_raw(ADC_CHANNEL_0);
        printf("VBAT = %d\n", VBAT);
        delay_ms(1200 );
    }

}

void doModem(void *arg) {

    // bring up modem

    modem_setup();
    printf("modem setup\n");
    modem_power_up();
    printf("modem powered up\n");
    modem_reset();  // in case it was already powered on
    printf("modem reset\n");

    while(1) {
        if (modem_init()) {
            printf("modem_init succeeded\n");
            break;
        } else {
            printf("modem_init failed\n");
        }
        delay_ms(1000 );
    }


    // enter loop

    uint8_t fun, reg, mode;  // modem vitals

    while (1) {


        // modem vitals
        fun = 255;
        if (!modem_get_functionality(&fun)) {
            printf("[ERROR] modem_get_functionality failed\n");
        } else {
            printf("Modem Functionality: %d\n", fun);
        }

        reg = 255;
        if (!modem_get_network_registration(&reg)) {
            printf("[ERROR] modem_get_network_registration failed\n");
        } else {
            printf("Network Registration Status: %d\n", reg);
        }

        mode = 255;
        if (!modem_get_network_system_mode(&mode)) {
            printf("[ERROR] modem_get_network_system_mode failed\n");
        } else {
            printf("Network System Mode: %d\n", mode);
        }

        // try post
        if ((fun==1) && (reg==5) && (mode==7)) {

            if (!modem_ip_is_gprsact()) {
                while (!modem_ip_is_initial()) {
                    modem_ip_shut();
                    delay_ms(100);
                }
                while (!modem_connect_bearer()) {
                    printf("[ERROR] modem_connect_bearer failed\n");
                }
            }

            modem_query_bearer();

            if (!modem_post_temperature(3.14159)) {
                printf("[ERROR] modem_post_temperature failed\n");
            } else {
                printf("HTTP POST succeeded\n");
            }

        }

        delay_ms(3000);

    }

}

void app_main(void) {

    xTaskCreate(heartBeat, "Heart Beat", 4096, NULL, 1, NULL);
    xTaskCreate(measureBattery, "Measure Battery", 2048, NULL, 1, NULL);
    xTaskCreate(doModem, "Do Modem", 4096, NULL, 1, NULL);

}

