#include <stdio.h>
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"


#define I2C_NUM_0 (0)
#define GPIO_I2C_SCL (GPIO_NUM_45)
#define GPIO_I2C_SDA (GPIO_NUM_47)

esp_err_t i2c_master_init(void);
void lvgl_touch();


