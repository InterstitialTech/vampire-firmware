#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "driver/uart.h"
#include "driver/gpio.h"

#include "modem.h"

#define PIN_MODEM_PWR           4                                                                                         
#define PIN_MODEM_RESET         5                                                                                         
#define PIN_MODEM_RX            26                                                                                        
#define PIN_MODEM_TX            27                                                                                        
#define PIN_MODEM_DTR           25                                                                                        

#define ACCESS_TOKEN "TOKEN"    // replace TOKEN with device-specific Access Token

// global buffers
uint8_t MODEM_BUF[MODEM_BUF_SIZE];

// TODO handle volatile
bool FIX = false;
char LAT[16];
char LON[16];

// forward declarations
static void _send_command(const char*);
static bool _confirm_response(const char*, uint64_t);
static bool _confirm_ok(uint64_t);
static bool _send_confirm(const char*, const char*, uint64_t);
static bool _get_data(size_t, uint64_t);
static bool _get_variable(uint64_t);

void modem_setup(void) {

    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1, .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    uart_driver_install(UART_NUM_1, MODEM_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, PIN_MODEM_TX, PIN_MODEM_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    ////

    gpio_pad_select_gpio(PIN_MODEM_PWR);
    gpio_set_direction(PIN_MODEM_PWR, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_MODEM_PWR, 1);

    gpio_pad_select_gpio(PIN_MODEM_RESET);
    gpio_set_direction(PIN_MODEM_RESET, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_MODEM_RESET, 1);

    gpio_pad_select_gpio(PIN_MODEM_DTR);
    gpio_set_direction(PIN_MODEM_DTR, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_MODEM_DTR, 0);

    MODEM_BUF[0] = '\0';

}

void modem_power_up(void) {

    // pulse low for 100 ms
    gpio_set_level(PIN_MODEM_PWR, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_MODEM_PWR, 1);

}

void modem_power_down(void) {

    // pulse low for 1200 ms
    gpio_set_level(PIN_MODEM_PWR, 0);
    vTaskDelay(1200 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_MODEM_PWR, 1);

}

void modem_reset(void) {

    // pulse low for >= 252 ms
    gpio_set_level(PIN_MODEM_RESET, 0);
    vTaskDelay(300 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_MODEM_RESET, 1);

}

bool modem_init(void) {

    // disable echo
    if (!_send_confirm("ATE0", "OK", 100)) return false;

    // TODO figure out these preferred modes

    // preferred mode
    //if (!_send_confirm("AT+CNMP=2", "OK", 1000)) return false;  // automatic
    //if (!_send_confirm("AT+CNMP=13", "OK", 1000)) return false; // GSM only
    if (!_send_confirm("AT+CNMP=38", "OK", 1000)) return false; // LTE only
    //if (!_send_confirm("AT+CNMP=51", "OK", 1000)) return false; // GSM and LTE only

    // preferred selection between cat-m and nb-iot
    if (!_send_confirm("AT+CMNB=1", "OK", 1000)) return false;  // cat-M
    //if (!_send_confirm("AT+CMNB=2", "OK", 1000)) return false;  // NB-IOT
    //if (!_send_confirm("AT+CMNB=3", "OK", 1000)) return false;  // cat-M and NB-IOT

    // not necessary?
    if (!_send_confirm("AT+CGDCONT=1,\"IP\",\"soracom.io\"", "OK", 1000)) {printf("blarg\n"); return false;}
    
    return true;

}

bool modem_get_imsi(void) {

    // result is stored in MODEM_BUF

    _send_command("AT+CIMI");

    if (!_get_data(15, 1000)) {return false;}
    if (!_confirm_ok(1000)) {return false;}

    return true;

}

bool modem_get_imei(void) {

    // result is stored in MODEM_BUF

    _send_command("AT+GSN");

    if (!_get_data(15, 1000)) return false;
    if (!_confirm_ok(1000)) return false;

    return true;

}

bool modem_get_firmware_version(void) {

    // result is stored in MODEM_BUF

    _send_command("AT+CGMR");

    if (!_get_data(24, 1000)) return false;
    if (!_confirm_ok(1000)) return false;

    return true;

}

char *modem_get_buffer_string(size_t start, size_t len) {

    MODEM_BUF[start + len] = '\0';

    return (char *) (MODEM_BUF + start);

}

//// internets

bool modem_connect_bearer(void) {

    // attach service
    if (!_send_confirm("AT+CGATT=1", "OK", 1000)) {printf("cb1\n"); return false;}
    if (!_send_confirm("AT+CSTT=\"soracom.io\",\"sora\",\"sora\"", "OK", 1000)) {printf("cb2\n");return false;} 
    if (!_send_confirm("AT+CIICR", "OK", 2000)) {printf("cb3\n");return false;} 

    // IP bearer
    if (!_send_confirm("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"", "OK", 1000)) {printf("cb3.5\n");return false;} 
    if (!_send_confirm("AT+SAPBR=3,1,\"APN\",\"soracom.io\"", "OK", 1000)) {printf("cb4\n");return false;} 
    if (!_send_confirm("AT+SAPBR=3,1,\"USER\",\"sora\"", "OK", 1000)) {printf("cb5\n");return false;} 
    if (!_send_confirm("AT+SAPBR=3,1,\"PWD\",\"sora\"", "OK", 1000)) {printf("cb6\n");return false;} 
    if (!_send_confirm("AT+SAPBR=1,1", "OK", 1000)) {printf("cb7\n");return false;} 

    // this will provide your IP address, if desired
    //if (!_send_confirm("AT+CIFSR", "OK", 1000)) return false;

    return true;

}

bool modem_post_data(char *payload) {

    // HTTP POST

    char ATstring[30];

    _send_confirm("AT+HTTPTERM", "OK", 1000);   // just in case
    if (!_send_confirm("AT+HTTPINIT", "OK", 1000)) {printf("cp1\n"); return false;}

    if (!_send_confirm("AT+HTTPPARA=\"CID\",1", "OK", 1000)) {printf("cp2\n"); return false;}

    if (!_send_confirm("AT+HTTPPARA=\"URL\",\"http://things.interstitial.coop/api/v1/"
                        ACCESS_TOKEN "/telemetry\"", "OK", 1000))
                        {printf("cp3\n"); return false;}

    if (!_send_confirm("AT+HTTPPARA=\"CONTENT\",\"application/json\"", "OK", 1000))
        {printf("cp3.1\n"); return false;}

    sprintf(ATstring, "AT+HTTPDATA=%d,10000", strlen(payload));  // do i need *any* delay here?
    if (!_send_confirm(ATstring, "DOWNLOAD", 5000)) {printf("cp4\n"); return false;}
    vTaskDelay(500 / portTICK_PERIOD_MS); // how short can this be?
    if (!_send_confirm(payload, "OK", 5000)) {printf("cp5\n"); return false;}

    if (!_send_confirm("AT+HTTPACTION=1", "OK", 1000)) {printf("cp6\n"); return false;}
    if (!_confirm_response("+HTTPACTION: 1,200,0", 2000)) {printf("cp7\n"); return false;}

    if (!_send_confirm("AT+HTTPTERM", "OK", 1000)) {printf("cp8\n"); return false;}

    return true;

}

bool modem_query_bearer(void) {

    // result is stored in MODEM_BUF

    // response:
    //  +SAPBR: <CID>,<status>,<ip_addr>
    // where status =
    //  0 Bearer is connecting
    //  1 Bearer is connected
    //  2 Bearer is closing
    //  3 Bearer is closed
    // and ip_addr is the address of the *bearer*

    _send_command("AT+SAPBR=2,1");
    int len_received;
    len_received = uart_read_bytes(UART_NUM_1, MODEM_BUF, MODEM_BUF_SIZE, 100/portTICK_PERIOD_MS);
    MODEM_BUF[len_received] = '\0';
    //printf("BEARER STATUS = %s\n", MODEM_BUF);

    return true;

}

bool modem_get_rssi_ber(uint8_t *rssi, uint8_t *ber) { 

    // RSSI is encoded as a number from 0 to 31 (inclusive) representing dBm
    // from -115 to -52.
    // BER (bit error rate) is encoded as a number from 0 to 7 (inclusive),
    // representing "RxQual" (https://en.wikipedia.org/wiki/Rxqual).
    // Both values will rail to 99 if they are "not known or detectable".
    // We return these numbers as a uint8_t's via the output parameters

    _send_command("AT+CSQ");

    if (!_get_data(11, 1000)) return false;

    if (!_confirm_ok(1000)) return false;

    // validate message
    if (strncmp((const char*) (MODEM_BUF + 2), "+CSQ:", 5)) return false;

    // extract data
    *rssi = strtol((const char*) (MODEM_BUF + 7), NULL, 10);
    *ber = strtol((const char*) (MODEM_BUF + 11), NULL, 10);

    return true;

}

bool modem_get_network_registration(uint8_t *netstat) {

    _send_command("AT+CGREG?");

    if (!_get_data(11, 1000)) return false;
    if (!_confirm_ok(1000)) return false;

    // validate message
    if (strncmp((const char*) (MODEM_BUF+2), "+CGREG: ", 8)) return false;
    if (MODEM_BUF[11] != ',') return false;

    // extract data
    *netstat = MODEM_BUF[12] - '0';

    return true;

}

bool modem_get_network_system_mode(uint8_t *status) {

    // status =  0 (no service)
    //           1 (GSM)
    //           3 (EGPRS)
    //           7 (LTE M1)
    //           9 (LTE NB)

    _send_command("AT+CNSMOD?");

    if (!_get_data(12, 1000)) return false;
    if (!_confirm_ok(1000)) return false;

    // validate message
    if (strncmp((const char*) (MODEM_BUF+2), "+CNSMOD: ", 9)) return false;
    if (MODEM_BUF[12] != ',') return false;

    *status = MODEM_BUF[13] - '0';

    return true;

}

bool modem_get_functionality(uint8_t *status) {

    //  status =    0 (Minimum functionality)
    //              1 (Full functionality (Default))
    //              4 (Disable phone both transmit and receive RF circuits)
    //              5 (Factory Test Mode)
    //              6 (Reset)
    //              7 (Offline Mode)

    _send_command("AT+CFUN?");

    if (!_get_data(8, 1000)) return false;
    if (!_confirm_ok(1000)) return false;

    // validate message
    if (strncmp((const char*)MODEM_BUF + 2, "+CFUN: ", 7)) return false;

    *status = MODEM_BUF[9] - '0';

    return true;

}

bool modem_ip_shut(void) {

    _send_command("AT+CIPSHUT");
    if (!_confirm_response("SHUT OK", 100)) return false;

    return true;

}

void modem_ip_print(void) {

    int len_received;

    _send_command("AT+CIPSTATUS");
    _confirm_ok(100);
    len_received = uart_read_bytes(UART_NUM_1, MODEM_BUF, MODEM_BUF_SIZE, 100/portTICK_PERIOD_MS);
    MODEM_BUF[len_received] = '\0';
    printf("IP = %s\n", MODEM_BUF);

}


bool modem_ip_is_initial(void) {
    _send_command("AT+CIPSTATUS");
    if (!_confirm_ok(100)) return false;
    if (!_get_variable(100)) return false;
    if (strncmp((char *) (MODEM_BUF + 2), "STATE: IP INITIAL", 17)) {return false;}
    return true;
}

bool modem_ip_is_status(void) {
    _send_command("AT+CIPSTATUS");
    if (!_confirm_ok(100)) return false;
    if (!_get_variable(100)) return false;
    if (strncmp((char *) (MODEM_BUF + 2), "STATE: IP STATUS", 16)) {return false;}
    return true;
}

bool modem_ip_is_gprsact(void) {
    _send_command("AT+CIPSTATUS");
    if (!_confirm_ok(100)) return false;
    if (!_get_variable(100)) return false;
    if (strncmp((char *) (MODEM_BUF + 2), "STATE: IP GPRSACT", 17)) {return false;}
    return true;
}

//// GPS

bool modem_gps_enable(void) {

    if (!_send_confirm("AT+SGPIO=0,4,1,1", "OK", 1000)) {return false;}
    if (!_send_confirm("AT+CGNSPWR=1", "OK", 1000)) {return false;}

    return true;

}

bool modem_gps_get_nav(void) {

    // upon success, nav info is stored in MODEM_BUF

    int len_received;

    _send_command("AT+CGNSINF");    // nav info parsed from NMEA sentence
    len_received = uart_read_bytes(UART_NUM_1, MODEM_BUF, MODEM_BUF_SIZE, 500/portTICK_PERIOD_MS);
    MODEM_BUF[len_received] = '\0';

    //printf("CGNSINF = %s\n", MODEM_BUF);

    return (len_received > 0);
}

bool modem_gps_parse_nav(void) {

    char *run, *fix;
    char *dt, *lat, *lon;
    char *msl, *speed, *course, *fixmode;
    char *reserved;
    char *hdop, *pdop, *vdeop;
    char *nsats, *nused, *nglonass;
    char *cnomax, *hpa, *vpa;

    char *ptr = (char *) MODEM_BUF;

    char * redflag;

    run = strsep(&ptr, ","); // token=\r\n+CGNSINF: 1
    fix = strsep(&ptr, ","); // token=1
    dt = strsep(&ptr, ","); // token=20210310063505.000
    lat = strsep(&ptr, ","); // token=47.630245
    lon = strsep(&ptr, ","); // token=-122.310556
    msl = strsep(&ptr, ","); // token=121.400
    speed = strsep(&ptr, ","); // token=0.00
    course = strsep(&ptr, ","); // token=356.7
    fixmode = strsep(&ptr, ","); // token=1
    reserved = strsep(&ptr, ","); // token=
    hdop = strsep(&ptr, ","); // token=0.9
    pdop = strsep(&ptr, ","); // token=1.2
    vdeop = strsep(&ptr, ","); // token=0.8
    reserved = strsep(&ptr, ","); // token=
    nsats = strsep(&ptr, ","); // token=20
    nused = strsep(&ptr, ","); // token=9
    nglonass = strsep(&ptr, ","); // token=2
    reserved = strsep(&ptr, ","); // token=
    cnomax = strsep(&ptr, ","); // token=37
    hpa = strsep(&ptr, ","); // token=
    vpa = strsep(&ptr, ","); // token=

    printf("\nGPS Report:\n");
    printf("\tfix = %s\n", fix);
    printf("\tnsats = %s\n", nsats);
    printf("\tlat = %s\n", lat);
    printf("\tlon = %s\n\n", lon);

    FIX = !strcmp(fix, "1");
    if (FIX) {
        strcpy(LAT, lat);
        strcpy(LON, lon);
    }

    return true;

}

//// static functions

static void _send_command(const char *cmd) {

    // handles \r and \n, no need to include them in the argument

    uart_flush_input(UART_NUM_1);
    uart_write_bytes(UART_NUM_1, cmd, strlen(cmd));
    uart_write_bytes(UART_NUM_1, "\r\n", 2);

}

static bool _confirm_response(const char *resp, uint64_t timeout_ms) {

    // handles \r and \n, no need to include them in the argument

    int len_resp, len_expected, len_received;

    len_resp = strlen(resp);
    len_expected = len_resp + 4;    // \r\n<data>\r\n

    len_received = uart_read_bytes(UART_NUM_1, MODEM_BUF, len_expected, timeout_ms / portTICK_PERIOD_MS);

    if (len_received != len_expected) {return false;}
    if (strncmp((char*)MODEM_BUF, "\r\n", 2)) {return false;}
    if (strncmp((char*)MODEM_BUF + 2, resp, len_resp)) {return false;}
    if (strncmp((char*)MODEM_BUF + 2 + len_resp, "\r\n", 2)) {return false;}

    return true;

}

static bool _confirm_ok(uint64_t timeout_ms) {

    // same as _confirm_response("OK", timeout_ms), but uses its own buffer

    uint8_t localbuf[6];
    int len_received;

    len_received = uart_read_bytes(UART_NUM_1, localbuf, 6, timeout_ms / portTICK_PERIOD_MS);

    if (len_received != 6) {return false;}
    if (strncmp((char *) localbuf + 0, "\r\n", 2)) {return false;}
    if (strncmp((char *) localbuf + 2, "OK", 2)) {return false;}
    if (strncmp((char *) localbuf + 4, "\r\n", 2)) {return false;}

    return true;

}

static bool _send_confirm(const char *cmd, const char *resp, uint64_t timeout_ms) {

    // convenience function

    _send_command(cmd);
    return _confirm_response(resp, timeout_ms);

}

static bool _get_data(size_t len, uint64_t timeout_ms) {

    // does NOT add a null termination, because it could be used for binary
    // data as well as strings

    if (len > MODEM_BUF_SIZE) {
        printf("[ERROR] _get_data: len is greater than MODEM_BUF_SIZE\n");
        return false;
    }

    int len_expected, len_received;

    len_expected = len + 4;    // \r\n<data>\r\n

    len_received = uart_read_bytes(UART_NUM_1, MODEM_BUF, len_expected, timeout_ms / portTICK_PERIOD_MS);

    if (len_received != len_expected) {return false;}
    if (strncmp((char*)MODEM_BUF, "\r\n", 2)) {return false;}
    if (strncmp((char*)MODEM_BUF + 2 + len, "\r\n", 2)) {return false;}

    return true;

}

static bool _get_variable(uint64_t timeout_ms) {

    // short for "get variable length response"
    // result is stored in MODEM_BUF (including \r\n's)

    int len_received;

    len_received = uart_read_bytes(UART_NUM_1, MODEM_BUF, MODEM_BUF_SIZE, timeout_ms / portTICK_PERIOD_MS);
    if (strncmp((char *) MODEM_BUF, "\r\n", 2)) {return false;}
    if (strncmp((char *) (MODEM_BUF + len_received - 2), "\r\n", 2)) {return false;}

    return true;

}

