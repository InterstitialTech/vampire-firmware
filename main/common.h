#ifndef COMMON_H
#define COMMON_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define NBLINKS_NORMAL 1
#define NBLINKS_ADC_FAILURE 2
#define NBLINKS_MODEM_FAILURE 3

void delay_ms(unsigned long);

#endif
