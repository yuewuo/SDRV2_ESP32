#include "UARTGrp.h"
#include "duhttp.h"
#include "shellutil.h"

static const char TAG[] = "UARTGrp_proxy";

static struct {
    int valid;
    struct DuHttpReceiver* duHttpReceiver;
    struct DuHttp* sendDuHttp;
} proxys[3];

static int initialze(int uart_num) {
    ESP_LOGI(TAG, "initialze proxy at %d", uart_num);
    if (uart_num == 1 || uart_num == 2) {
		UARTGrp_config_t* configp = UARTGrp.getConfig(uart_num);
		if (configp == NULL) {
			ESP_LOGE(TAG, "uart num %d has been occupied!", uart_num);
			return -1;
		}
		UARTGrp.init(uart_num);
        proxys[uart_num].duHttpReceiver = malloc(sizeof(struct DuHttpReceiver));
        proxys[uart_num].sendDuHttp = malloc(sizeof(struct DuHttp));
        if (proxys[uart_num].duHttpReceiver == NULL || proxys[uart_num].sendDuHttp == NULL) {
                ESP_LOGE(TAG, "Malloc failed");
        } else { proxys[uart_num].valid = 1; return 0; }
	} else {
		ESP_LOGE(TAG, "error uart num %d", uart_num);
	}
    return -1;
}

static int doProxy(int uart_num, struct DuHttp* ori, int timeoutTICK, char* sendbuf, int sendbuflen, int* returnSize) {
    if ((uart_num != 1 && uart_num != 2) || proxys[uart_num].valid != 1) {
        ESP_LOGE(TAG, "uart %d not exists or not initialzed", uart_num);
        return -1;
    }
    struct DuHttp* sendDuHttp = proxys[uart_num].sendDuHttp;
    struct DuHttpReceiver* duHttpReceiver = proxys[uart_num].duHttpReceiver;
    DuHttp_Initialize(sendDuHttp);
    DuHttpReceiver_Initialize(duHttpReceiver);
    if (ori->type == DuHttp_Type_GET) {
        DuHttp_Initialize_GET(sendDuHttp, ori->ask.requestedURL);
    } else if (ori->type == DuHttp_Type_POST) {
        DuHttp_Initialize_POST(sendDuHttp, ori->ask.requestedURL);
    }
    int nowait = 0;
    for (int i=0; i<ori->headlineCount; ++i) {
        if (0 == strcmp(ori->headline[i].key, "ESPProxyWait")
                && 0 == strcmp(ori->headline[i].value, "no-wait")) {
            nowait = 1;
            ESP_LOGI(TAG, "proxy no-wait enabled");
        } else if (0 != strcmp(ori->headline[i].key, "Content-Length") && 0 != strcmp(ori->headline[i].key, "ESPProxyTo"))  // 尽可能地节约时间，串口上传输数据非常慢
            DuHttp_PushHeadline(sendDuHttp, ori->headline[i].key, ori->headline[i].value);
    }
    DuHttp_EndHeadline(sendDuHttp);
    DuHttp_PushData(sendDuHttp, ori->content, ori->contentLength);
    static char buf[8192];
    int len = DuHttpSend(sendDuHttp, buf, 8192);
    DuHttp_Release(sendDuHttp);
    int sendlen = UARTGrp.Send(uart_num, buf, len);
    if (sendlen != len) {
        ESP_LOGE(TAG, "UARTGrp Send %d out of %d", sendlen, len);
        return -1;
    }
    if (nowait) {
        DuHttp_Initialize_RESPONSE(sendDuHttp, 200, "OK");
        DuHttp_PushHeadline(sendDuHttp, "Connection", "keep-alive");
        DuHttp_PushHeadline(sendDuHttp,"Content-Type", "text/plain");
        DuHttp_EndHeadline(sendDuHttp);
        DuHttp_PushDataString(sendDuHttp, "proxy no wait\n");
        *returnSize = DuHttpSend(sendDuHttp, sendbuf, sendbuflen);
        return 0;
    }
    int sendlened = 0;
    int gotpack = 0;
    TickType_t endTime = xTaskGetTickCount() + timeoutTICK;
    while ((DuHttpReceiver_TryReadPack(duHttpReceiver, sendDuHttp) == false || (gotpack = 1) == 1) &&
                xTaskGetTickCount() < endTime) {
        int buflen = UARTGrp.Read(uart_num, (uint8_t*)buf, 8192, 30);
        buf[buflen] = '\0';
        ESP_LOGI(TAG, "read(%d): %s", buflen, buf);
        memcpy(sendbuf + sendlened, buf, buflen);
        sendlened += buflen;
        if (sendlened >= sendbuflen) {
            ESP_LOGE(TAG, "buf overflow");
            return -1;
        }
        DuHttpReceiver_InBuf(duHttpReceiver, buf, buflen);
    }
    if (gotpack == 0) {
        ESP_LOGE(TAG, "receving timeout after %d ticks", timeoutTICK);
    }
    sendbuf[sendlened] = '\0';
    ESP_LOGI(TAG, "proxy response is (%d):", sendlened);
    printf("%s\n", sendbuf);
    ESP_LOGI(TAG, "end proxy response");
    *returnSize = sendlened;
    return 0;
}

struct UARTGrp_Proxy_t UARTGrpProxy = {
    .init = initialze,
    .doProxy = doProxy
};
