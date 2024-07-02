#include "i2c_lcd.h"

static const char *TAG = "i2c_lcd";
// uint8_t GT911_ADDR[2] = {0x5d, 0x14};
// uint8_t GT911_ADDR = 0x5d;
lv_indev_t *indev_touchpad;
#define GT911_ADDR 0x5d 
#define WRITE_BIT I2C_MASTER_WRITE /*!< I2C master write 0---->写*/
#define READ_BIT I2C_MASTER_READ   /*!< I2C master read  1---->读*/
#define ACK_CHECK_EN 0x1           /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0          /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                /*!< I2C ack value */
#define NACK_VAL 0x1               /*!< I2C nack value */

/*******寄存器********/
#define GT911ID (0x8140)      // GT911的ID
#define GT_CTRL_REG (0x8040)  // 软件复位
#define GT_CFGS_REG (0x8047)  // 配置版本
#define GT_GSTID_REG (0x814E) // 状态寄存器,检测触摸点

/*******坐标寄存器********/
#define GT_TP1_REG (0x8150)
#define GT_TP2_REG (0x8158)
#define GT_TP3_REG (0x8160)
#define GT_TP4_REG (0x8168)
#define GT_TP5_REG (0x8170)

static void touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
static bool touchpad_is_pressed(void);
static void touchpad_get_xy(lv_coord_t *x, lv_coord_t *y);
static void gt911_init(void);

esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_NUM_0;
    static i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .scl_io_num = GPIO_I2C_SCL,
        .sda_io_num = GPIO_I2C_SDA,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };
    i2c_param_config(i2c_master_port, &conf);
    return i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);
}

// 主机写入8位数据

// 主机读取8位数据

//********************************主机写入16位数据********************************/
static int gt911_write_bytes_16bit(uint16_t reg_addr, size_t data_len, uint8_t *data)
{
    uint8_t regl = reg_addr & 0xff;
    uint8_t regh = (reg_addr >> 8) & 0xff;
    uint8_t wbuf[128] = {regh, regl};
    for (uint8_t i = 0; i < data_len; i++)
    {
        wbuf[i + 2] = data[i];
    }
    data_len += 2;
    return i2c_master_write_to_device(I2C_NUM_0, GT911_ADDR, wbuf, data_len, 1000 / portTICK_PERIOD_MS);
}

//********************************主机读取16位数据********************************/
static int gt911_read_bytes_16bit(uint16_t reg_addr, size_t data_len, uint8_t *data)
{
    uint8_t regl = reg_addr & 0xff;
    uint8_t regh = (reg_addr >> 8) & 0xff;
    uint8_t rbuf[2] = {regh, regl};

    return i2c_master_write_read_device(I2C_NUM_0, GT911_ADDR, rbuf, 2, data, data_len, 1000 / portTICK_RATE_MS);
}

void lvgl_touch()
{
    static lv_indev_drv_t indev_drv;
    gt911_init();
    /*注册触摸板输入设备*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read;
    indev_touchpad = lv_indev_drv_register(&indev_drv);
}

/*检测触摸板是否被按下，读取触摸板的坐标*/
static void touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    static lv_coord_t last_x = 0;
    static lv_coord_t last_y = 0;

    /*判断触摸屏是否有被按下，保存按下的坐标和状态*/
    if (touchpad_is_pressed())
    {
        touchpad_get_xy(&last_x, &last_y);
        data->state = LV_INDEV_STATE_PR;
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }

    /*设置最后按下的坐标*/
    data->point.x = last_x;
    data->point.y = last_y;
    // vTaskDelay(pdMS_TO_TICKS(500));
}

/*返回是否触摸板被按下*/
static bool touchpad_is_pressed(void)
{
    uint8_t temp = 0;
    gt911_read_bytes_16bit(GT_GSTID_REG, 1, &temp);
    // printf("GT_GSTID_REG:0x%x\n", temp);
    if (temp & 0x80)
    {
        if (temp & 0x0F)
        {
            return true;
        }
        temp = 0;
        gt911_write_bytes_16bit(GT_GSTID_REG, 1, &temp);
    }
    return false;
}

/*如果按下触摸板，则获取x和y坐标*/
static void touchpad_get_xy(lv_coord_t *x, lv_coord_t *y)
{
    uint8_t data[4];
    gt911_read_bytes_16bit(GT_TP1_REG, 4, data);
    *x = ((data[1] & 0x0f) << 8) + data[0];
    *y = ((data[3] & 0x0f) << 8) + data[2];
    // printf("触摸指标为————x: %3d-----y: %3d\n", *x, *y);

    data[0] = 0;
    // 把触摸状态改为0
    gt911_write_bytes_16bit(GT_GSTID_REG, 1, data);
}

static void gt911_init(void)
{
    uint8_t temp[5] = {0};
    ESP_ERROR_CHECK(gt911_read_bytes_16bit(GT911ID, 4, temp));
    ESP_LOGI(TAG, "tp driver ic:GT%s", temp);

    for (int i = 0; i < 5; i++)
    {
        temp[i] = 0;
    }
    temp[0] = 0x02;
    gt911_write_bytes_16bit(GT_CTRL_REG, 1, temp); // soft reset
    temp[0] = 0x00;
    vTaskDelay(pdMS_TO_TICKS(100));

    gt911_read_bytes_16bit(GT_CFGS_REG, 1, temp); // read config version
    ESP_LOGI(TAG, "tp config version:0x%x\n", temp[0]);
}
