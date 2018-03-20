#include "autoinclude.h"
#include "console_cmd.h"
#include "shellutil.h"

static const char TAG[] = "console gpio";

static int gpio_console(int argc, char** argv);
void console_register_gpio() {
	const static esp_console_cmd_t cmd = {
		.command = "gpio",
		.help = "init get set",
		.hint = NULL,
		.func = gpio_console,
	};
	ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

#define ARGVCMP(x) (0 == strcmp(argv[i], x))
static int gpio_console(int argc, char** argv) {
    if (argc == 1) {
		Shell.Out.printf("gpio console hello~ [by wy 18.3.20]\n");
	} else {
        int i = 1;
        if (ARGVCMP("init")) {
            if (argc != 4) { Shell.Out.printf("usage: gpio init [GPIO_NUM] [GPIO_MODE_DISABLE/GPIO_MODE_INPUT/GPIO_MODE_OUTPUT/GPIO_MODE_OUTPUT_OD/GPIO_MODE_INPUT_OUTPUT_OD/GPIO_MODE_INPUT_OUTPUT]\n"); return -1; }
            int gpio_num = atoi(argv[2]);
            i = 3;
            if (ARGVCMP("GPIO_MODE_DISABLE")) gpio_set_direction(gpio_num, GPIO_MODE_DISABLE);
            else if (ARGVCMP("GPIO_MODE_INPUT")) gpio_set_direction(gpio_num, GPIO_MODE_INPUT);
            else if (ARGVCMP("GPIO_MODE_OUTPUT")) gpio_set_direction(gpio_num, GPIO_MODE_OUTPUT);
            else if (ARGVCMP("GPIO_MODE_OUTPUT_OD")) gpio_set_direction(gpio_num, GPIO_MODE_OUTPUT_OD);
            else if (ARGVCMP("GPIO_MODE_INPUT_OUTPUT_OD")) gpio_set_direction(gpio_num, GPIO_MODE_INPUT_OUTPUT_OD);
            else if (ARGVCMP("GPIO_MODE_INPUT_OUTPUT")) gpio_set_direction(gpio_num, GPIO_MODE_INPUT_OUTPUT);
            else { Shell.Out.printf("gpio unknown mode: %s\n", argv[i]); return -1; }
            Shell.Out.printf("gpio %d init mode %s OK\n", gpio_num, argv[i]);
        } else if (ARGVCMP("set")) {
            if (argc != 4) { Shell.Out.printf("usage: gpio set [GPIO_NUM] [0/1]\n"); return -1; }
            int gpio_num = atoi(argv[2]);
            int gpio_level = atoi(argv[3]);
            gpio_set_level(gpio_num, gpio_level);
            Shell.Out.printf("gpio %d set %s OK\n", gpio_num, gpio_level);
        } else if (ARGVCMP("get")) {
            if (argc != 3) { Shell.Out.printf("usage: gpio get [GPIO_NUM]\n"); return -1; }
            int gpio_num = atoi(argv[2]);
            Shell.Out.printf("gpio %d is %s\n", gpio_num, gpio_get_level(gpio_num));
        }
    }
    return 0;
}