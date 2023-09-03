#ifndef MODEM_H
#define MODEM_H

#include "common.h"

#define MODEM_BUF_SIZE 1024
#define MODEM_NTRY_MAX 5

void modem_setup(void);
bool modem_init(void);

void modem_power_up(void);
void modem_power_down(void);
void modem_reset(void);

bool modem_get_network_registration(uint8_t*);
bool modem_get_network_system_mode(uint8_t*);
bool modem_get_functionality(uint8_t*);
bool modem_get_available_networks(void);
bool modem_get_imsi(void);
bool modem_get_imei(void);
bool modem_get_firmware_version(void);
char *modem_imei_str(void);

uint8_t *modem_get_buffer_data(void);
char *modem_get_buffer_string(size_t, size_t);

bool modem_connect_bearer(void);
bool modem_post_data(char*);
bool modem_query_bearer(void);

bool modem_get_rssi_ber(uint8_t*, uint8_t*);

bool modem_gps_enable(void);
bool modem_gps_get_nav(void);
bool modem_gps_parse_nav(void);

void modem_reset_ipstatus(void);

bool modem_ip_shut(void);
bool modem_ip_is_initial(void);
bool modem_ip_is_status(void);
void modem_ip_print(void);
bool modem_ip_is_gprsact(void);

#endif
