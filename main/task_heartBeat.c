#include "driver/gpio.h"

#include "common.h"

#define PIN_LED                 12 

// TODO handle volatile
uint16_t NBLINKS = NBLINKS_NORMAL;

void task_heartBeat(void *arg) {

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
        delay_ms(1900 );

    }

}

