#include "common.h"
#include "i2c.h"
#include "shtc3.h"

#define PERIOD_MS_SHTC3 5020
#define SHTC3_REG_ID 0xEFC8

// TODO handle volatile
float TEMP = 0.;
float HUMI = 0.;

void task_shtc3(void *arg) {

    i2c_init();

    const uint8_t cmd[] = "LED_ON";

    uint8_t buf[64];

    uint16_t temp_raw, humi_raw;

    while (1) {

        shtc3_wakeup();
        delay_ms(1);
        shtc3_GetTempAndHumidity(&temp_raw, &humi_raw);
        shtc3_sleep();

        TEMP = shtc3_convert_temp(temp_raw);
        HUMI = shtc3_convert_humd(humi_raw);

        delay_ms(1000);

    }

}
