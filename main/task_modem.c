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

static int NTRY_RESET = 0;

void handle_failure() {

    if (++NTRY_RESET >= NTRY_RESET_MAX) reset_system();
    NBLINKS = NBLINKS_MODEM_FAILURE;
    delay_ms(1000);

}

void task_modem(void *arg) {

    uint8_t fun, reg, mode;  // modem vitals
    char payload[128];

    // bring up modem

    NBLINKS = NBLINKS_MODEM_FAILURE;

    modem_setup();
    printf("setup: modem_setup\n");
    modem_power_up();
    printf("setup: modem_power_up\n");
    modem_reset();  // in case it was already powered on
    printf("setup: modem_reset\n");

    // setup, loop through attempts
    NTRY_RESET = 0;
    while (1) {

        if (!modem_init()) {
            printf("modem_init failed\n");
            handle_failure();
            continue;
        }
        printf("setup: modem_init succeeded\n");

        if (!modem_gps_enable()) {
            printf("modem_gps_enable failed\n");
            handle_failure();
            continue;
        }
        printf("setup: modem_gps_enable succeeded\n");

        break;

    }

    // post loop
    while (1) {

        if (MODEM_DEBUG) {
            if (modem_get_imei()) {
                printf("\nIMEI: %s\n", modem_get_buffer_string(2, 15));
            }
            if (modem_get_imsi()) {
                printf("\nIMSI: %s\n", modem_get_buffer_string(2, 15));
            }
        }

        // mini try loop
        /*
        int ntry_fun;
        for (ntry_fun=0; ntry_fun<5; ntry_fun++) {
            fun = 255;
            if (!modem_get_functionality(&fun)) {
                printf("[ERROR] modem_get_functionality failed\n");
            } else {    
                printf("Modem Functionality: %d\n", fun);
                break;
            }
            delay_ms(1000);
        }
        if (ntry_fun == 5) {
            handle_failure();
            continue;
        }
        */
        // asdf
        fun = 255;
        if (!modem_get_functionality(&fun)) {
            printf("[ERROR] modem_get_functionality failed\n");
            handle_failure();
            continue;
        }
        printf("modem_get_functionality succeeded. fun = %d\n", fun);

        reg = 255;
        if (!modem_get_network_registration(&reg)) {
            printf("[ERROR] modem_get_network_registration failed\n");
            handle_failure();
            continue;
        }
        printf("Network Registration Status: %d\n", reg);

        mode = 255;
        if (!modem_get_network_system_mode(&mode)) {
            printf("[ERROR] modem_get_network_system_mode failed\n");
            handle_failure();
            continue;
        }
        printf("Network System Mode: %d\n", mode);

        // assert functionality, registration, and mode
        if (!((fun==1) && (reg==5) && (mode==7))){
            printf("assertion failed\n");
            handle_failure();
            continue;
        }
        printf("assertion succeeded\n");

        // GPS Stuff
        if (modem_gps_get_nav()) {
            modem_gps_parse_nav();
        } else {
            printf("gps_get_nav failed\n");
            FIX = false; // not a failure
        }

        if (!modem_ip_is_gprsact()) {

            // lost the GPRS connect, need to repair

            // mini try loop, is_initial
            int ntry_init;
            for (ntry_init=0; ntry_init<5; ntry_init++) {
                if (!modem_ip_is_initial()) {
                    printf("modem_ip_is_initial failed\n");
                    modem_ip_shut();
                    delay_ms(500);
                } else {
                    break;
                }
            }
            if (ntry_init == 5) {
                handle_failure();
                continue;
            }

            // mini try loop, connect_bearer
            int ntry_bearer;
            for (ntry_bearer=0; ntry_bearer<10; ntry_bearer++) {
                if (!modem_connect_bearer()) {
                    printf("[ERROR] modem_connect_bearer failed\n");
                    delay_ms(100);
                } else {
                    break;
                }
            }
            if (ntry_bearer == 5) {
                handle_failure();
                continue;
            }

        }

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
            handle_failure();
            continue;
        }
        printf("modem_post_data succeeded\n");

        printf("task_modem succeeded!\n");
        NBLINKS = NBLINKS_NORMAL;
        NTRY_RESET = 0;

        delay_ms(PERIOD_MS_UPLOAD);

    }

}

