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

    gpio_pad_select_gpio(PIN_LED);
    gpio_set_direction(PIN_LED, GPIO_MODE_OUTPUT);

    while(1) {

        printf("\nHeartbeat Report:\n");
        printf("\tVRAIL = %.3f\n", VRAIL);
        printf("\tVLIPO = %.3f\n", VLIPO);
        printf("\tTEMP = %.3f\n", TEMP);
        printf("\tHUMI = %.3f\n\n", HUMI);

        /* Blink NBLINKS times */
        for (int i=0; i<NBLINKS; i++) {
            gpio_set_level(PIN_LED, 0);
            delay_ms(100);
            gpio_set_level(PIN_LED, 1);
            delay_ms(100 );
        }

        /* Off for two seconds */
        gpio_set_level(PIN_LED, 1);
        delay_ms(1900 );

    }

}

