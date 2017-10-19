#include "lib.h"

static const char *TAG = "basiclib";

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

void initialise_wifi(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t sta_config = {
    	.sta = {
    		.ssid = DEFAULT_WIFI_SSID,
    		.password = DEFAULT_WIFI_PASS
    	}
    };
    ESP_LOGI(TAG, "Working in STA_MODE");
    ESP_LOGI(TAG, "Setting WiFi STA SSID \"%s\"...", sta_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

nvs_handle my_nvs_handle;

void nvs_init(void) {
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");
    err = nvs_open("storage", NVS_READWRITE, &my_nvs_handle);
    if (err != ESP_OK) {
    	ESP_LOGE(TAG, "Error (%d) opening NVS handle!\n", err);
    }
}

int32_t nvs_get_i32_safe(const char* key, int32_t default_value) {
	esp_err_t err;
	int32_t tmp;
	err = nvs_get_i32(my_nvs_handle, key, &tmp);
	if (err == ESP_OK) return tmp;
	return default_value;
}

bool nvs_set_i32_safe(const char* key, int32_t value) {
	esp_err_t err = nvs_set_i32(my_nvs_handle, key, value);
	if (err != ESP_OK) return false;
	err = nvs_commit(my_nvs_handle);
	return err == ESP_OK;
}

esp_err_t nvs_get_str_safe(const char* key, const char* default_value, char* str, int max_length) {
	size_t required_size;
	int i;
	esp_err_t err = nvs_get_str(my_nvs_handle, key, NULL, &required_size);
	if (err == ESP_OK) {
		if (required_size < max_length) {
			return nvs_get_str(my_nvs_handle, key, str, &required_size);
		}
	}
	for (i=0; i<max_length && default_value[i]; ++i) {
		str[i] = default_value[i];
	} if (i == max_length) str[i - 1] = 0; else str[i] = 0;
	return err;
}

bool nvs_set_str_safe(const char* key, const char* value) {
	esp_err_t err = nvs_set_str(my_nvs_handle, key, value);
	if (err != ESP_OK) return false;
	err = nvs_commit(my_nvs_handle);
	return err == ESP_OK;
}





