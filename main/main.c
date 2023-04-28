#include <stdio.h>
#include "common.h"

void task_heartBeat(void*);
void task_adc(void*);
void task_modem(void*);
void task_shtc3(void*);

void app_main(void) {

    xTaskCreate(task_heartBeat, "Heart Beat", 2048, NULL, 1, NULL);
    xTaskCreate(task_adc, "Measure Battery", 2048, NULL, 1, NULL);
    xTaskCreate(task_modem, "Upload via Modem", 4096, NULL, 1, NULL);
    xTaskCreate(task_shtc3, "Sample SHTC3", 4096, NULL, 1, NULL);

}

