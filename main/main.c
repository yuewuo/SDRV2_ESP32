#include "autoinclude.h"
#include "WS2812B_daemon.h"

EventGroupHandle_t wifi_event_group;

static const char *TAG = "main";

static void app_task(void *pvParameters) {
	app();
	ESP_LOGE(TAG, "app shutdown");
	while (1) {
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

static void httpd_task (void *pvParameters) {
	httpd();
	ESP_LOGE(TAG, "httpd shutdown");
	while (1) {
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

void app_main()
{
	ESP_LOGI(TAG, "main start");
    nvs_init();
    initialise_wifi();
	duHttpInit();
    //xTaskCreate(&https_login_PKU_Gateway_task, "https_login_PKU_Gateway_task", 8192, NULL, 5, NULL);
    //xTaskCreate(&call_baidu, "call_baidu", 8192, NULL, 5, NULL);
    ESP_LOGI(TAG, "main creating tasks");
    xTaskCreate(&httpd_task, "httpd_task", 16384, NULL, 5, NULL);
	xTaskCreate(&app_task, "app_task", 8192, NULL, 5, NULL);
	//xTaskCreate(WS2812B.demo.task, "ws2812host_task", 2048, NULL, 5, NULL);
	xTaskCreate(&WS2812_daemon_task, "WS2812_daemon_task", 8192, NULL, 5, NULL);
	ESP_LOGI(TAG, "main end");
}
