#include "common.h"
#include "modem.h"

#define PERIOD_MS_UPLOAD 5000

extern volatile uint16_t NBLINKS;

extern bool FIX;
extern char LAT[16];
extern char LON[16];

extern float VRAIL;
extern float VLIPO;
extern float TEMP;
extern float HUMI;

#define MODEM_DEBUG 0

void task_modem(void *arg) {

    // bring up modem

    NBLINKS = NBLINKS_MODEM_FAILURE;

    modem_setup();
    printf("modem setup\n");
    modem_power_up();
    printf("modem powered up\n");
    modem_reset();  // in case it was already powered on
    printf("modem reset\n");

    while (!modem_init()) {
        printf("modem_init failed\n");
        delay_ms(1000 );
    }
    printf("modem_init succeeded\n");

    while (!modem_gps_enable()) {
        printf("modem_gps_enable failed\n");
        delay_ms(1000 );
    }
    printf("modem_gps_enable succeeded\n");

    // enter loop

    uint8_t fun, reg, mode;  // modem vitals
    char payload[128];

    while (1) {

        if (MODEM_DEBUG) {
            if (modem_get_imei()) {
                printf("\nIMEI: %s\n", modem_get_buffer_string(2, 15));
            }
            if (modem_get_imsi()) {
                printf("\nIMSI: %s\n", modem_get_buffer_string(2, 15));
            }
        }

        // modem vitals
        fun = 255;
        if (!modem_get_functionality(&fun)) {
            printf("[ERROR] modem_get_functionality failed\n");
        } else if (MODEM_DEBUG) {
            printf("Modem Functionality: %d\n", fun);
        }

        reg = 255;
        if (!modem_get_network_registration(&reg)) {
            printf("[ERROR] modem_get_network_registration failed\n");
        } else if (MODEM_DEBUG) {
            printf("Network Registration Status: %d\n", reg);
        }

        mode = 255;
        if (!modem_get_network_system_mode(&mode)) {
            printf("[ERROR] modem_get_network_system_mode failed\n");
        } else if (MODEM_DEBUG) {
            printf("Network System Mode: %d\n", mode);
        }

        // try post
        if ((fun==1) && (reg==5) && (mode==7)) {


            // GPS Stuff
            if (modem_gps_get_nav()) {
                modem_gps_parse_nav();
            } else {
                printf("gps_get_nav failed\n");
            }

            if (!modem_ip_is_gprsact()) {
                while (!modem_ip_is_initial()) {
                    printf("modem_ip_is_initial failed\n");
                    modem_ip_shut();
                    delay_ms(100);
                }
                while (!modem_connect_bearer()) {
                    printf("[ERROR] modem_connect_bearer failed\n");
                    delay_ms(100);
                }
            }

            modem_query_bearer();

            // set payload
            if (FIX) {
                sprintf(payload, "{\"vlipo\":%.3f, \"vrail\":%.3f, "
                                    "\"temperature\":%.3f, \"humidity\":%.3f, "
                                    "\"latitude\":%s, \"longitude\":%s}",
                        VLIPO, VRAIL, TEMP, HUMI, LAT, LON);
            } else {
                sprintf(payload, "{\"vlipo\":%.3f, \"vrail\":%.3f, "
                                    "\"temperature\":%.3f, \"humidity\":%.3f}",
                        VLIPO, VRAIL, TEMP, HUMI);
            }

            // do HTTP POST
            if (!modem_post_data(payload)) {
                printf("[ERROR] modem_post_data failed\n");
                NBLINKS = NBLINKS_MODEM_FAILURE;
            } else {
                printf("modem_post_data succeeded\n");
                NBLINKS = NBLINKS_NORMAL;
            }

            delay_ms(PERIOD_MS_UPLOAD);

        } else {
            NBLINKS = NBLINKS_MODEM_FAILURE;
        }

    }

}

