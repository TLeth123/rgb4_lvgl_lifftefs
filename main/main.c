#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"
#include "lv_demos.h"
#include "lvgl_init.h"
#include "i2c_lcd.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_littlefs.h"
#include "esp_attr.h"
#include "../components/lvgl__lvgl/demos/benchmark/lv_demo_benchmark.h"
#include "../components/lvgl__lvgl/demos/widgets/lv_demo_widgets.h"
#include "../components/lv_lib_split_jpg/lv_sjpg.h"
#include "../components/lv_lib_split_jpg/tjpgd.h"

static const char *TAG = "app_main";

esp_err_t write_sector(const char *path, uint32_t sector, const uint8_t *buffer, size_t size)
{
    FILE *file = fopen(path, "wb+");
    if (file == NULL)
    {
        return ESP_FAIL;
    }

    printf("write_sector size=%d\n", size);
    fseek(file, sector * size, SEEK_SET);
    // fseek(file, 0, SEEK_SET);
    size_t bytes_written = fwrite(buffer, 1, size, file);
    fclose(file);

    if (bytes_written != size)
    {
        return ESP_FAIL;
    }

    return ESP_OK;
}

void lvgl_driver_init() // 初始化液晶驱动
{
    ESP_ERROR_CHECK(i2c_master_init());
    lv_init();
    lvgl_disp_init();
    // lvgl_touch();
    lv_port_tick_init();
}

void lv_tick_task(void *arg) // LVGL 时钟任务
{
    while (1)
    {
        vTaskDelay((10) / portTICK_PERIOD_MS);
        lv_task_handler();
    }
}

// extern const uint8_t *kiwik_map;

lv_img_dsc_t kiwik1 = {
    .header.cf = LV_IMG_CF_TRUE_COLOR,
    .header.always_zero = 0,
    .header.reserved = 0,
    .header.w = 480,
    .header.h = 480,
    .data_size = 230400 * LV_COLOR_SIZE / 8,
    .data = NULL,
};

void delay_callback(lv_timer_t *timer)
{
    lv_demo_benchmark();
}

void app_main(void)
{
    // 挂载 LittleFS 文件系统
    esp_vfs_littlefs_conf_t conf = {
        .base_path = "/littlefs",
        .partition_label = "my_littlefs",
        .format_if_mount_failed = true};
    esp_err_t err = esp_vfs_littlefs_register(&conf);
    if (err != ESP_OK)
    {
        printf("挂载 LittleFS 文件系统失败\n");
        return;
    }
    size_t total, used;
    esp_err_t ret = esp_littlefs_info(conf.partition_label, &total, &used); // 获取挂载点信息
    if (ret != ESP_OK)
    {
        printf("Failed to get LittleFS partition information: %d\n", ret);
        return;
    }
    else
    {
        printf("Partition size: total: %d, used: %d\n", total, used);
    }
    lvgl_driver_init(); // 初始化液晶驱动
    FILE *file = fopen("/littlefs/kiwik.bin", "rb+");
    if (file == NULL)
    {
        printf("open error\n");
        return;
    }
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    printf("fileSize = %ld\n", fileSize);
    unsigned char *data = (unsigned char *)malloc(fileSize);
    if (data == NULL)
    {
        printf("内存分配失败\n");
        fclose(file);
        return;
    }
    size_t readSize = fread(data, 1, fileSize, file);
    if (readSize != fileSize)
    {
        printf("读取文件失败\n");
        free(data);
        fclose(file);
        return;
    }
    fclose(file);
    printf("读取数据: 0x%02x\n", data[fileSize - 1]);

    /************************************************************/
    kiwik1.data_size = fileSize;
    kiwik1.data = malloc(sizeof(unsigned char) * fileSize);
    memcpy((uint8_t *)kiwik1.data, data, fileSize);
    lv_obj_t *img1;
    lv_split_jpeg_init();
    img1 = lv_img_create(lv_scr_act());
    lv_img_set_src(img1, &kiwik1);
    /************************************************************/
    lv_obj_del_delayed(img1, 5000);
    lv_timer_t *my_timer = lv_timer_create(delay_callback, 5000, NULL);
    xTaskCreate(lv_tick_task, "lv_tick_task", 4096, NULL, 1, NULL);
    esp_vfs_littlefs_unregister(conf.partition_label);
    ESP_LOGI(TAG, "LittleFS unmounted");
    vTaskDelay(6000 / portTICK_PERIOD_MS);
    lv_timer_del(my_timer);
    free(data);
    free(kiwik1.data);
}
