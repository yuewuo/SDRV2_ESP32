#include "autoinclude.h"

#define WS2812_PIN	18

static char nvs_got_str[1024];
esp_err_t nvs_quick_get_str(const char* key, const char* default_value) {
	return nvs_get_str_safe(key, default_value, nvs_got_str, sizeof(nvs_got_str));
}

void rainbow(void* pvParameters);

void app() {

	while(1) {
		vTaskDelay(1);
	}
	//char t;
	//int t;
	/*while (1) {
		printf("get:%d\n", nvs_get_i32_safe("tmp", 4));
		fflush(stdout);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
		nvs_set_i32_safe("tmp", 502);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
		nvs_quick_get_str("WIFI_SSID", "");
		printf("get:%s\n", nvs_got_str);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
		nvs_set_str_safe("WIFI_SSID", "Wireless PKU");
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}*/
	ESP_LOGI("app", "app start");
	OLED_Init();
	OLED_Clear();
	//t=' ';
	while(1)
	{
		/*ESP_LOGI("app", "OLED start");
		OLED_Clear();
	//	LED_ON;
		OLED_ShowCHinese(0,0,0);//ÖÐ
		OLED_ShowCHinese(18,0,1);//Ÿ°
		OLED_ShowCHinese(36,0,2);//Ô°
		OLED_ShowCHinese(54,0,3);//µç
		OLED_ShowCHinese(72,0,4);//×Ó
		OLED_ShowCHinese(90,0,5);//¿Æ
		OLED_ShowCHinese(108,0,6);//ŒŒ
		OLED_ShowString(0,3,"1.3' OLED TEST");
		//OLED_ShowString(8,2,"ZHONGJINGYUAN");
	 //	OLED_ShowString(20,4,"2014/05/01");
		OLED_ShowString(0,6,"ASCII:");
		OLED_ShowString(63,6,"CODE:");
		OLED_ShowChar(48,6,t);//ÏÔÊŸASCII×Ö·û
		t++;
		if(t>'~')t=' ';
		OLED_ShowNum(103,6,t,3,16);//ÏÔÊŸASCII×Ö·ûµÄÂëÖµ


		delay_ms(1000);
		OLED_Clear();
		delay_ms(1000);
	//		LED_OFF;
		OLED_DrawBMP(0,0,128,8,BMP1);  //ÍŒÆ¬ÏÔÊŸ(ÍŒÆ¬ÏÔÊŸÉ÷ÓÃ£¬Éú³ÉµÄ×Ö±íœÏŽó£¬»áÕŒÓÃœÏ¶à¿ÕŒä£¬FLASH¿ÕŒä8KÒÔÏÂÉ÷ÓÃ)
		delay_ms(1000);*/
		/*OLED_DrawBMP(0,0,128,8,BMP2);
		OLED_Clear();
		//delay_ms(1000);
		//ESP_LOGI("app", "OLED off");
		ESP_LOGI("t", "%d", t++);*/
	}
}
