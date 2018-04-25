/*
 * 本文件使用被动的TCP在6050端口监听，如果有链接，则一直发送角度数据
 * （暂时不发送加速度数据，因为角加速度数据采样率很高，达到了8kHz，而实际能传出来的实际采样率需要看）
 * 这里没有测试采样率是否等间隔，不排除因为操作系统调度的原因导致采样不均匀
 * TCP连接收到数据的时间同样不能作为时间基准，因为有buffer延迟
 * 目前这个问题不知道如何解决，需要进一步讨论
 */

#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "i2c.h"
#include "clock.h"
#include "math.h"
#include "autoinclude.h"
#include "esp_task_wdt.h"

#define DEFAULT_MPU_HZ  (100)

#define BUG_DETECT_PRINT(a,has_bug,no_bug) { if(a) \
printf("%s",has_bug); \
else \
printf("%s",no_bug);}

volatile int get_cnt;

void vTimerCallback( TimerHandle_t xTimer )
{
    // xTimerStop( xTimer, 0 );
    if (get_cnt) printf("%d mps\n", get_cnt);
    get_cnt = 0;
}

// high sample rate version
void MPU6050_hsr_demo_task(void *pvParameters) {

    struct int_param_s int_param;
    //unsigned long time1, time2;
    //signed char gyro_orientation[9] = {1, 0, 0, 0,1, 0, 0, 0, 1};
    short gyro_data[3];

    printf("\r\n MPU6050_hsr_demo_task \r\n");

    ANBT_I2C_Configuration();  //IIC初始化

    BUG_DETECT_PRINT(mpu_init(&int_param),"\r\n MPU6050 init fail\r\n","\r\n MPU6050 init success\r\n");
    BUG_DETECT_PRINT(mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL),"\r\n DMP set fail\r\n","\r\n DMP set success\r\n");
    BUG_DETECT_PRINT(mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL),"\r\n set FIFO fail\r\n","\r\n set FIFO success\r\n");
    BUG_DETECT_PRINT(mpu_set_sample_rate(DEFAULT_MPU_HZ),"\r\n set sample rate fail\r\n","\r\n set sample rate success\r\n");
    //BUG_DETECT_PRINT(dmp_load_motion_driver_firmware(),"\r\n load firmware fail\r\n","\r\n load success\r\n");
    //BUG_DETECT_PRINT(dmp_set_orientation(inv_orientation_matrix_to_scalar(gyro_orientation)),"\r\n DMP set init oritation fail\r\n","\r\n DMP set init oritation success\r\n");
    //BUG_DETECT_PRINT(dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT  | DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_CAL_GYRO | DMP_FEATURE_GYRO_CAL),"\r\n DMP init fail\r\n","\r\n DMP init success\r\n");
    //BUG_DETECT_PRINT(dmp_set_fifo_rate(DEFAULT_MPU_HZ),"\r\n set FIFO rate fail\r\n","\r\n set FIFO rate succcess\r\n");

    run_self_test();

    TimerHandle_t xTimerUser;
    xTimerUser = xTimerCreate("print cnt timer", 100, pdTRUE, ( void * ) 0, vTimerCallback);
    printf("xTimerUser = %p\n", xTimerUser);
    if (xTimerUser != NULL) xTimerStart( xTimerUser, 0 );  // 0不阻塞 

    // 监听端口
    struct netconn *conn, *newconn;
    char data[6] = {0,1,2,3,4,5};
    err_t err;
    conn = netconn_new(NETCONN_TCP);
    netconn_bind(conn, NULL, 6050);
    netconn_listen(conn);
    do {
        err = netconn_accept(conn, &newconn);
        if (err == ERR_OK) {
            while (err == ERR_OK) {
                mpu_get_gyro_reg(gyro_data, NULL);
                ++get_cnt;
                //err = netconn_write(newconn, (char*)data , 6, NETCONN_COPY);
                err = netconn_write(newconn, (char*)gyro_data , 3 * sizeof(short), NETCONN_COPY);
            }
            netconn_close(newconn);
            netconn_delete(newconn);
        }
    } while(1);
    ESP_LOGE("MPU6050 server", "err???");
    netconn_close(conn);
    netconn_delete(conn);
    //esp_task_wdt_feed();  // 加上这句话防止WDT超时，经测试，没有用处

}