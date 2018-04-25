#include "UARTGrp.h"
#include "autoinclude.h"

static const char TAG[] = "UARTGrp_socket";

typedef struct QUEUE
{
    char* head;
    char* tail;
    char* read;
    char* write;
} queue_t;
volatile queue_t queue;
#define QUEUE_SIZE_HERE 45000
#define PACKET_SIZE_HERE 256
void queue_init() {
    char* arr = (char*)malloc(QUEUE_SIZE_HERE);
    queue.head = arr;
    queue.read = arr;
    queue.write = arr;
    queue.tail = arr + QUEUE_SIZE_HERE;
}
void queue_enqueue(char* buf, int buflen) {
    int i;
    for (i=0; i<buflen; ++i) {
        if (((queue.read - queue.write + QUEUE_SIZE_HERE) % QUEUE_SIZE_HERE) == 1) {
            ESP_LOGE(TAG, "queue overflow");
        } else {
            *(queue.write) = buf[i];
            ++(queue.write);
            if (queue.write == queue.tail) queue.write = queue.head;
        }
    }
}
char queue_dequeue() {
    while (queue.read == queue.write) vTaskDelay(1);
    char c = *(queue.read);
    ++(queue.read);
    if (queue.read == queue.tail) queue.read = queue.head;
    return c;
}
void queue_clear() {
    queue.read = queue.write;
}
static volatile char is_socket_on;
static void UARTGrp_socket_hsr_mpu6050_task_socket(void *pvParameters) {
    char buf[PACKET_SIZE_HERE];
    // 监听端口
    struct netconn *conn, *newconn;
    err_t err;
    conn = netconn_new(NETCONN_TCP);
    netconn_bind(conn, NULL, 6050);
    netconn_listen(conn);
    do {
        err = netconn_accept(conn, &newconn);
        if (err == ERR_OK) {
            is_socket_on = 1;
            while (err == ERR_OK) {
                int i;
                for (i=0; i<PACKET_SIZE_HERE; ++i) buf[i] = queue_dequeue();
                err = netconn_write(newconn, (char*)buf , PACKET_SIZE_HERE, NETCONN_NOCOPY);
            }
            is_socket_on = 0;
            netconn_close(newconn);
            netconn_delete(newconn);
        }
    } while(1);
    ESP_LOGE("MPU6050 server", "err???");
    netconn_close(conn);
    netconn_delete(conn);
}



volatile int get_cnt;
void vTimerCallback( TimerHandle_t xTimer )
{
    // xTimerStop( xTimer, 0 );
    if (get_cnt) printf("%d bytes/s\n", get_cnt / 10);
    get_cnt = 0;
}

// 这个任务是和 https://geeklab.pku.edu.cn/gitlab/wuyuepku/stm8-mpu-hsr.git 配套使用的
void UARTGrp_socket_hsr_mpu6050_task(void *pvParameters) {
    is_socket_on = 0;
    queue_init();

    xTaskCreate(&UARTGrp_socket_hsr_mpu6050_task_socket, "UARTGrp_socket_hsr_mpu6050_task_socket", 8192, NULL, 5, NULL);

    TimerHandle_t xTimerUser;
    xTimerUser = xTimerCreate("uart socket cnt timer", 1000, pdTRUE, ( void * ) 0, vTimerCallback);
    printf("xTimerUser = %p\n", xTimerUser);
    if (xTimerUser != NULL) xTimerStart( xTimerUser, 0 );  // 0不阻塞 

    // 初始化串口
    UARTGrp_config_t* configp = UARTGrp.getConfig(1);
    if (configp == NULL) {
        ESP_LOGE(TAG, "uart num %d has been occupied!", 1); return;
    }
    configp->tx_io_num = 18;
    configp->rx_io_num = 19;
    configp->uart_config.baud_rate = 460800;
    UARTGrp.init(1);

    char buf[1024];
    int buflen;

    while (1) {
        buflen = UARTGrp.Read(1, (uint8_t*)buf, 1024, 0);
        if (is_socket_on && buflen) {
            get_cnt += buflen;
            queue_enqueue(buf, buflen);
        }
    }
}
