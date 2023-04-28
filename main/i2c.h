#include "common.h"

esp_err_t i2c_init(void);
esp_err_t i2c_write(uint8_t *buf, size_t len);
esp_err_t i2c_read(uint8_t *buf, size_t len);
esp_err_t i2c_read_reg(uint16_t reg_addr, uint8_t *buf, size_t len);
