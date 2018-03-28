#include "autoinclude.h"
#include "WS2812B_daemon.h"
#include "AD8232_daemon.h"
#include "console/console.h"
#include "shellutil.h"
#include "MPU6050.h"

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
	delay_ms(1000);  // 开机等待，可以亮灯提示用户重启
    nvs_init();
    initialise_wifi();
	duHttpInit();
    //xTaskCreate(&https_login_PKU_Gateway_task, "https_login_PKU_Gateway_task", 8192, NULL, 5, NULL);
    //xTaskCreate(&call_baidu, "call_baidu", 8192, NULL, 5, NULL);
    ESP_LOGI(TAG, "main creating tasks");
	Shell.Out.clear();
	Console.execbootscript();
	ESP_LOGI(TAG, "printing results:");
	printf("%s\n", Shell.Out.buffer());
	ESP_LOGI(TAG, "end printing results:");
    xTaskCreate(&httpd_task, "httpd_task", 16384, NULL, 5, NULL);
	//xTaskCreate(&app_task, "app_task", 8192, NULL, 5, NULL);
	//xTaskCreate(&AD8232_daemon_task, "AD8232_task", 8192, NULL, 5, NULL);
	//xTaskCreate(WS2812B.demo.task, "ws2812host_task", 2048, NULL, 5, NULL);
	xTaskCreate(&WS2812_daemon_task, "WS2812_daemon_task", 8192, NULL, 5, NULL);
	//xTaskCreate(&MPU6050_daemon_task, "MPU6050_daemon_task", 8192, NULL, 5, NULL);
	ESP_LOGI(TAG, "main end");
}
