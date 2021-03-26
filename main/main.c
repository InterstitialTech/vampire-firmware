#include <stdio.h>

#include "common.h"

void task_heartBeat(void*);
void task_adc(void*);
void task_modem(void*);

void app_main(void) {

    xTaskCreate(task_heartBeat, "Heart Beat", 4096, NULL, 1, NULL);
    xTaskCreate(task_adc, "Measure Battery", 2048, NULL, 1, NULL);
    xTaskCreate(task_modem, "Do Modem", 4096, NULL, 1, NULL);

}

