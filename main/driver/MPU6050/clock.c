#include "clock.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

/*static unsigned long g_timecount;

void SysTick_Handler(void)
{
	g_timecount++;
}*/

void clock_conf(void)
{
	//SystemInit();
	//SysTick_Config(72000);
	//g_timecount = 0;
}

void get_clock_ms(unsigned long *count)
{
	//*count = g_timecount;
	*count = xTaskGetTickCount() * portTICK_PERIOD_MS;
}
#if false
void delay_ms(unsigned long n)
{
	/*unsigned long m;
	
	m = g_timecount;
	while(g_timecount <= m+n )
		;	*/
	vTaskDelay(n / portTICK_PERIOD_MS);
}
#endif