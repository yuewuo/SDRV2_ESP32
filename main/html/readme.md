# ESP32-easy-develop套件说明

**github页面在此，你可以在此下载[源代码](https://github.com/wuyuepku/SDRV2_ESP32)**

[TOC]



## 系统简介

本套件提供统一的web接口用来控制ESP32单片机，通过支持GET和POST命令，以及解析命令行字符串的能力，用户可以在不烧录单片机的情况下配置硬件，编写开机启动脚本，和控制其他外设。ESP32单片机可从[Ai-Thinker](https://www.ai-thinker.com/product/esp32)公司购买（不打广告，只是因为他们的文档比较详细）。

## 硬件驱动

### 1. WS2812 全彩LED

本模块作为第一个被开发出来的driver，充分利用了ESP32的功能，其中涉及DMA的使用，才能在操作系统存在的情况下通过管脚输出特定高速的波形，在此不赘述。这个driver的web接口如下：

`http://<ESP32的IP>/exec/<你要执行的命令>`

这个命令字段有以下几个选项：

1. `ws2812 -h` 返回helper信息
2. `ws2812 on -c white` 打开白色的灯，-c可以换成--color，white可以是red,green,blue,purple,yellow，也可以是三个rgb的值(255,255,255)括号括起来，中间没有空格
3. `ws2812 off` 关闭灯
4. `ws2812 rainbow` 开启彩虹灯模式，非常好看咯
5. `ws2812 on -b -c white` 白色呼吸灯，-b可以换成--breath
6. `ws2812 on -d 5` 5秒后开启灯，这个功能可以用作延迟功能，多个延迟操作只取最后一个，`-d`意思是delay

### 2. 串口穿透

这个功能可以让串口输出双向穿透，还没有完工

### 3. PN532模块驱动，可以NFC扫卡读写卡

文档稍后再写

### 4. AD8232 心率模块

心率模块

### 5. OLED 128$\times$64模块

用来显示图案和文字