#include "common.h"

#include "driver/adc.h"

// TODO handle volatile
float VLIPO = 0.;
float VRAIL = 0.;

void task_adc(void *arg) {

    uint16_t counts_lipo, counts_rail;

    adc1_config_width(ADC_WIDTH_BIT_12);

    // LIPO on ADC1, Channel 0
    adc1_config_channel_atten(ADC_CHANNEL_0, ADC_ATTEN_DB_0);

    // RAIL on ADC1, Channel 5
    adc1_config_channel_atten(ADC_CHANNEL_5, ADC_ATTEN_DB_0);

    while (1) {

        counts_lipo = adc1_get_raw(ADC_CHANNEL_0);
        VLIPO = counts_lipo * 0.0010586798822092942;    // 26.8 * 1.1 / 6.8 / 4095

        counts_rail = adc1_get_raw(ADC_CHANNEL_5);
        VRAIL = counts_rail * 0.015490435490435493;     // 51900. * 1.1 / 900. / 4095

        printf("\ncounts_lipo = %d\n", counts_lipo);
        printf("counts_rail = %d\n", counts_rail);
        printf("VLIPO = %.3f\n", VLIPO);
        printf("VRAIL = %.3f\n\n", VRAIL);

        delay_ms(1000);

    }

}

