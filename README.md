

这个是基于esp32s3的rgb4寸屏幕的基础上加入了lvgl和littlefs文件系统，先在esp32的partitions.csv分配出一个1M的空间，把烧有文件的的虚拟内存预先通过esp32官方的烧写软件烧写进去（和自己配置的地址空间要对应、最好大小也要对应）  
然后把内存里面图片的bin文件出来（.bin和.jpg的二进制是一样的），读出来的数据放在数组里面并且不能释放，在显示期间不可释放



### Hardware Connection

The connection between ESP Board and the LCD is as follows:

```
       ESP Board                           RGB  Panel
+-----------------------+              +-------------------+
|                   GND +--------------+GND                |
|                       |              |                   |
|                   3V3 +--------------+VCC                |
|                       |              |                   |
|                   PCLK+--------------+PCLK               |
|                       |              |                   |
|             DATA[15:0]+--------------+DATA[15:0]         |
|                       |              |                   |
|                  HSYNC+--------------+HSYNC              |
|                       |              |                   |
|                  VSYNC+--------------+VSYNC              |
|                       |              |                   |
|                     DE+--------------+DE                 |
|                       |              |                   |
|               BK_LIGHT+--------------+BLK                |
+-----------------------+              |                   |
                               3V3-----+DISP_EN            |
                                       |                   |
                                       +-------------------+
```
