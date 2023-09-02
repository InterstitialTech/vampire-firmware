#include "common.h"

void delay_ms(unsigned long ms) {

    vTaskDelay(ms / portTICK_PERIOD_MS);

}

void reset_system(void) {

    printf("Resetting the system!\n");
    esp_restart();

}

