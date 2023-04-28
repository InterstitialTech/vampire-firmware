// from https://esp32tutorials.com/esp32-i2c-communication-tutorial-esp-idf/

#include "driver/i2c.h"

#define I2C_MASTER_SCL_IO 22               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 21               /*!< gpio number for I2C master data  */
#define I2C_MASTER_FREQ_HZ 100000        /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */

#define I2C_ADDR7_SHTC3 0x70

#define WRITE_BIT I2C_MASTER_WRITE              /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ                /*!< I2C master read */
#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                             /*!< I2C ack value */
#define NACK_VAL 0x1                            /*!< I2C nack value */

#define I2C_PORT 0


esp_err_t i2c_init(void)
{
  
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
        // .clk_flags = 0,          /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
    };
    esp_err_t err = i2c_param_config(I2C_PORT, &conf);
    if (err != ESP_OK) {
        return err;
    }
    return i2c_driver_install(I2C_PORT, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

esp_err_t i2c_write(uint8_t *buf, size_t len)
{
    esp_err_t ret; 

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();    
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, I2C_ADDR7_SHTC3 << 1 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write(cmd, buf, len, ACK_CHECK_DIS);
    i2c_master_stop(cmd);
    
    ret = i2c_master_cmd_begin(I2C_PORT, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

esp_err_t i2c_read(uint8_t *buf, size_t len)
{
    esp_err_t ret; 

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();    
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, I2C_ADDR7_SHTC3 << 1 | READ_BIT, ACK_CHECK_DIS);
    i2c_master_read(cmd, buf, len, ACK_CHECK_DIS);
    i2c_master_stop(cmd);
    
    ret = i2c_master_cmd_begin(I2C_PORT, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

esp_err_t i2c_read_reg(uint16_t reg_addr, uint8_t *buf, size_t len)
{
    esp_err_t ret; 
    i2c_cmd_handle_t cmd;

    cmd = i2c_cmd_link_create();    
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, I2C_ADDR7_SHTC3 << 1 | WRITE_BIT, ACK_CHECK_DIS);
    i2c_master_write_byte(cmd, (reg_addr >> 8) & 0xFF, ACK_CHECK_DIS);
    i2c_master_write_byte(cmd, reg_addr & 0xFF, ACK_CHECK_DIS);
    i2c_master_start(cmd); // repeated start
    i2c_master_write_byte(cmd, I2C_ADDR7_SHTC3 << 1 | READ_BIT, ACK_CHECK_DIS);
    i2c_master_read(cmd, buf, len, ACK_CHECK_DIS);
    i2c_master_stop(cmd);
    
    ret = i2c_master_cmd_begin(I2C_PORT, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

