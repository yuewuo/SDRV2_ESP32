#ifndef AUTOINCLUDE_H
#define AUTOINCLUDE_H

// The system includes

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/portmacro.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_console.h"
#include "esp_vfs_dev.h" // vfs? virtual file system?
#include "esp_vfs_fat.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "soc/gpio_struct.h"
#include "soc/rmt_struct.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/rmt.h"
#include "driver/uart.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/api.h"
#include "lwip/opt.h"
#include "tcpip_adapter.h"
#include "mbedtls/platform.h"
#include "mbedtls/net.h"
#include "mbedtls/esp_debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "argtable3/argtable3.h"

// user includes

#include "WS2812B.h"
#include "duhttp.h"
#include "UARTGrp.h"

//main.c
static const int CONNECTED_BIT = BIT0;
static const int STA_CONNECTED_BIT = BIT1;
static const int STA_DISCONNECTED_BIT = BIT2;
extern EventGroupHandle_t wifi_event_group;
#define DEFAULT_WIFI_SSID CONFIG_WIFI_SSID
#define DEFAULT_WIFI_PASS CONFIG_WIFI_PASSWORD

//PKU_Login.c
//extern void https_login_PKU_Gateway();
//extern bool PKU_logined;
//extern void set_PKU_ID_Password(const char* ID, const char* password);

//lib.c
extern void initialise_wifi(void);
extern void nvs_init(void);
extern int32_t nvs_get_i32_safe(const char* key, int32_t default_value);
extern bool nvs_set_i32_safe(const char* key, int32_t value);
extern esp_err_t nvs_get_str_safe(const char* key, const char* default_value, char* str, int max_length);
extern bool nvs_set_str_safe(const char* key, const char* value);
extern void delay_ms(int ms);
extern uint8_t htoi2(char *s);

//app.c
extern void app();

//OLED.c
#define u8 char
#define u32 uint32_t
#define OLED_CMD  0
#define OLED_DATA 1

#define OLED_MODE 0
#define SIZE 16
#define XLevelL		0x00
#define XLevelH		0x10
#define Max_Column	128
#define Max_Row		64
#define	Brightness	0xFF
#define X_WIDTH 	128
#define Y_WIDTH 	64

//void OLED_WR_Byte(u8 dat,u8 cmd);
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Init(void);
void OLED_Clear(void);
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t);
void OLED_Fill(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t dot);
void OLED_ShowChar(uint8_t x, uint8_t y, char chr);
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size);
void OLED_ShowString(uint8_t x, uint8_t y, char *p);
void OLED_Set_Pos(uint8_t x, uint8_t y);
void OLED_ShowCHinese(uint8_t x, uint8_t y, uint8_t no);
void OLED_DrawBMP(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, unsigned char BMP[]);

//httpd.c
//void httpd_start(void);
void httpd();
#define BLINK_GPIO 2 //default is 2
void duHttpInit();

//motor.c
//extern int motor_cnt;
//void motor_task(void);
//void motor_init(void);

#endif // AUTOINCLUDE_H
