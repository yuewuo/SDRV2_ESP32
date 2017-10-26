#include "lib/lib.h"

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

extern const char str[] asm("_binary_index_html_start");
extern const char str1[] asm("_binary_action_html_start");

void app_main()
{
printf("%s\n", str);
printf("%s\n", str1);
	ESP_LOGI(TAG, "main start");
    nvs_init();
    initialise_wifi();
		duHttpInit();
    //xTaskCreate(&https_login_PKU_Gateway_task, "https_login_PKU_Gateway_task", 8192, NULL, 5, NULL);
    //xTaskCreate(&call_baidu, "call_baidu", 8192, NULL, 5, NULL);
    ESP_LOGI(TAG, "main creating tasks");
    xTaskCreate(&httpd_task, "httpd_task", 2048, NULL, 5, NULL);
    xTaskCreate(&app_task, "app_task", 8192, NULL, 5, NULL);
    ESP_LOGI(TAG, "main end");
}
