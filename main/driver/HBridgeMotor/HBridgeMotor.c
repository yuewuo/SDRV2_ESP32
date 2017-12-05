/*#include "lib.h"

int motor_cnt = 0;

#define GPIO_OUTPUT_IO_zheng    18
#define GPIO_OUTPUT_IO_fan    19

#define GPIO_OUTPUT_PIN_SEL  ((1<<GPIO_OUTPUT_IO_zheng) | (1<<GPIO_OUTPUT_IO_fan))

void motor_init(void) {
	gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

void motor_task(void) {
	while (1) {
		gpio_set_level(GPIO_OUTPUT_IO_zheng, 0);
		gpio_set_level(GPIO_OUTPUT_IO_fan, 0);
		if (motor_cnt != 0) {
			if (motor_cnt > 0) {
				printf("motor zheng move!!!!\n");
				gpio_set_level(GPIO_OUTPUT_IO_zheng, 1);
				--motor_cnt;
			} else {
				printf("motor fan move!!!!\n");
				gpio_set_level(GPIO_OUTPUT_IO_fan, 1);
				++motor_cnt;
			}
		}
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}*/
