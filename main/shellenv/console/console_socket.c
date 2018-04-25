#include "console_cmd.h"
#include "autoinclude.h"
#include "shellutil.h"
#include "console.h"

const static char TAG[] = "console_socket";

static int socket_console(int argc, char** argv);
void console_register_socket() {
	const static esp_console_cmd_t cmd = {
		.command = "socket",
		.help = "list init",
		.hint = NULL,
		.func = socket_console,
	};
	ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

static void UART_SOCKET_TASK(void *pvParameters);

#define ARGVCMP(x) (0 == strcmp(argv[i], x))
static int socket_console(int argc, char** argv) {
    for (int i=1; i<argc; ++i) {
        if (ARGVCMP("init")) {
            ++i;
            if (ARGVCMP("uart")) { // 建立uart穿透
                xTaskCreate(UART_SOCKET_TASK, "UART_SOCKET_TASK", 2048, NULL, 5, NULL);
                Shell.Out.printf("build uart1 socket at port 6061\n");
            }
        }
    }
    return 0;
}

static volatile int UART_STATE = 1;
static void UART_SEND2_TCP(void *pvParameters) {
    struct netconn *conn = *((struct netconn **)pvParameters);
    unsigned char buf[1024];
    for(;;) {
        int buflen = UARTGrp.Read(1, buf, 1024, 30);  // 300ms?
        netconn_write(conn, buf, buflen, NETCONN_NOCOPY);
    }
}

static void UART_SOCKET_TASK(void *pvParameters) {
    UARTGrp_config_t* configp = UARTGrp.getConfig(1); // uart1
    if (configp == NULL) ESP_LOGE(TAG, "uart num %d has been occupied!", 1);
    UARTGrp.init(1);

    struct netconn *conn, *newconn;
    err_t err;
    conn = netconn_new(NETCONN_TCP);
    netconn_bind(conn, NULL, 6061);
    netconn_listen(conn);
    ESP_LOGI("UART_SOCKET_TASK", "listened");
    do {
        err = netconn_accept(conn, &newconn);
        if (err == ERR_OK) {
            // 在这里处理所有数据的转发
            xTaskHandle xHandle;
            UART_STATE = 1;
            xTaskCreate(UART_SEND2_TCP, "UART_SEND2_TCP", 2048, &newconn, 5, &xHandle);
            struct netbuf *inbuf;
            char *buf;
            u16_t buflen;
            while (netconn_recv(conn, &inbuf) == ERR_OK) {
                netbuf_data(inbuf, (void**)&buf, &buflen);
                UARTGrp.Send(1, buf, buflen);  // 直接把数据从串口发出去
                netbuf_delete(inbuf);
            }
            if (xHandle != NULL) vTaskDelete(xHandle);
            netconn_delete(newconn);
        }
    } while(err == ERR_OK);
    ESP_LOGE("UART_SOCKET_TASK", "Accept Failed");
    netconn_close(conn);
    netconn_delete(conn);
}