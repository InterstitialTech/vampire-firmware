#include "driver/gpio.h"
#include "common.h"

#define PIN_LED                 12 

// TODO handle volatile
uint16_t NBLINKS = NBLINKS_NORMAL;

extern float VRAIL;
extern float VLIPO;
extern float TEMP;
extern float HUMI;

void task_heartBeat(void *arg) {

    uint16_t nblinks;

    gpio_pad_select_gpio(PIN_LED);
    gpio_set_direction(PIN_LED, GPIO_MODE_OUTPUT);

    while(1) {

        /* Blink NBLINKS times */
        nblinks = NBLINKS;  // store global async value locally
        for (int i=0; i<nblinks; i++) {
            gpio_set_level(PIN_LED, 0);
            delay_ms(200);
            gpio_set_level(PIN_LED, 1);
            delay_ms(200 );
        }

        NBLINKS = NBLINKS_NORMAL;   // give other threads a chance to reset this

        /* Off for ~10 seconds */
        gpio_set_level(PIN_LED, 1);
        delay_ms(9*1000);

    }

}

