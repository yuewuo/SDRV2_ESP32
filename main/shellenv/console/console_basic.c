#include "console_cmd.h"
#include "autoinclude.h"
#include "shellutil.h"

static void register_tasks();
void console_register_basic() {
	register_tasks();
}

static int tasks(int argc, char** argv);
static void register_tasks() {
	const static esp_console_cmd_t cmd = {
		.command = "tasks",
		.help = "print tasks running on freeRTOS",
		.hint = NULL,
		.func = tasks,
	};
	ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

static int tasks(int argc, char** argv) {
	printf("GG I'm tasks!!!\n");
	Shell.Out.printf("Hello~~~\n");
	return 0;
}
