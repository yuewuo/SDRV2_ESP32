#include "AD8232_daemon.h"

/*Note: Different ESP32 modules may have different reference voltages varying from
 * 1000mV to 1200mV. Use #define GET_VREF to route v_ref to a GPIO
 */
#define V_REF 1100
#define ADC1_TEST_CHANNEL (ADC1_CHANNEL_6) //GPIO 34

const char TAG[] = "AD8232";

static void socket_server_netconn_serve(struct netconn *conn, esp_adc_cal_characteristics_t* cp) {
    printf("here %d\n", __LINE__);
    /* Read the data from the port, blocking if nothing yet there.
    We assume the request (the part we care about) is in one netbuf */
    static char str[64];
    while (1) {
        uint32_t voltage = adc1_to_voltage(ADC1_TEST_CHANNEL, cp);
        int len = sprintf(str, "%d %d\n", xTaskGetTickCount(), voltage);
        printf("%s", str);
        if (ERR_OK != netconn_write(conn, str, len, NETCONN_NOCOPY)) {
            ESP_LOGE(TAG, "Long Connection Break");
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}

void AD8232_daemon_task(void *pvParameters) {
    // Init ADC and Characteristics
    esp_adc_cal_characteristics_t characteristics;
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_TEST_CHANNEL, ADC_ATTEN_DB_0);
    esp_adc_cal_get_characteristics(V_REF, ADC_ATTEN_DB_0, ADC_WIDTH_BIT_12, &characteristics);
    // Init socket
    struct netconn *conn, *newconn;
    err_t err;
    conn = netconn_new(NETCONN_TCP);
    netconn_bind(conn, NULL, 8232);
    netconn_listen(conn);
    ESP_LOGI("AD8232_server", "listened");
    do {
        //ESP_LOGI("http_server", "doing");
        err = netconn_accept(conn, &newconn);
        ESP_LOGI("AD8232_server", "accepted");
        if (err == ERR_OK) {
            socket_server_netconn_serve(newconn, &characteristics);
            netconn_delete(newconn);
        }
     } while(err == ERR_OK);
     ESP_LOGE("AD8232_server", "err???");
     netconn_close(conn);
     netconn_delete(conn);
}
