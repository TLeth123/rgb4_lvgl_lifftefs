set(I2C_INIT ./i2c_init)
file(GLOB_RECURSE I2C_INIT_RECURSE ${I2C_INIT}/*.c)

set(LVGL_LCD ./lvgl_lcd)
file(GLOB_RECURSE LVGL_LCD_RECURSE ${LVGL_LCD}/*.c)

set(ST7701 ./st7701)
file(GLOB_RECURSE ST7701_RECURSE ${ST7701}/*.c)

set(MY_LVGL ../components/lvgl__lvgl)
file(GLOB_RECURSE MY_LVGL_RECURSE ${MY_LVGL}/*.c)

# set(MY_PNG ../components/lv_lib_png)
# file(GLOB_RECURSE MY_PNG_RECURSE ${MY_PNG}/*.c)

set(BEN ../components/lvgl__lvgl/demos/benchmark)
file(GLOB_RECURSE BEN_RECURSE ${BEN}/*.c)


set(WID ../components/lvgl__lvgl/demos/widgets)
file(GLOB_RECURSE WID_RECURSE ${WID}/*.c)

idf_component_register(SRCS 
                            "main.c" 
                            "lvgl_demo_ui.c"
                            "kiwik.c"
                            ${I2C_INIT_RECURSE} 
                            ${LVGL_LCD_RECURSE} 
                            ${ST7701_RECURSE} 
                            ${BEN_RECURSE} 
                            ${WID_RECURSE} 
                           #  ${MY_PNG_RECURSE} 
                         #    ${MY_LVGL_RECURSE} 


                INCLUDE_DIRS 
                            "."
                            ${I2C_INIT} 
                            ${LVGL_LCD}
                            ${ST7701}
                            ${BEN}
                            ${WID}
                           #  ${MY_PNG} 
                         #    ${MY_LVGL}

                       
                       )
