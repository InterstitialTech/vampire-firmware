#include "common.h"
#include "driver/adc.h"

#define PERIOD_MS_ADC 5010

// TODO handle volatile
float VLIPO = 0.;
float VRAIL = 0.;

#define NUM_MEASUREMENTS 64

float get_avg_adc_count(adc1_channel_t chan) {

    float accumulator = 0.;

    for (int i=0; i<NUM_MEASUREMENTS; i++) {
        accumulator += adc1_get_raw(chan);
    }

    return accumulator / NUM_MEASUREMENTS;

}

void task_adc(void *arg) {

    float counts_lipo, counts_rail;

    adc1_config_width(ADC_WIDTH_BIT_12);

    // LIPO on ADC1, Channel 0
    adc1_config_channel_atten(ADC_CHANNEL_0, ADC_ATTEN_DB_0);

    // RAIL on ADC1, Channel 5
    adc1_config_channel_atten(ADC_CHANNEL_5, ADC_ATTEN_DB_0);

    while (1) {

        counts_lipo = get_avg_adc_count(ADC_CHANNEL_0);
        VLIPO = counts_lipo * 0.0010586798822092942;    // 26.8 * 1.1 / 6.8 / 4095

        counts_rail = get_avg_adc_count(ADC_CHANNEL_5);
        if (counts_rail > 1.0) {
            VRAIL = 0.013687839432180191 * counts_rail + 4.0;
        } else {
            VRAIL = 0.0;
        }

        printf("VLIPO = %.3f\n", VLIPO);
        printf("VRAIL = %.3f\n\n", VRAIL);

        delay_ms(PERIOD_MS_ADC);

    }

}

