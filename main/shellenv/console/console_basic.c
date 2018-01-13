#include "console_cmd.h"
#include "autoinclude.h"
#include "shellutil.h"
#include "console.h"

const static char TAG[] = "console_basic";

static void register_tasks();
static void register_print();
static void register_set();
static void register_reboot();
static void register_start();
void console_register_basic() {
	register_tasks();
	register_print();
	register_set();
	register_reboot();
	register_start();
}

static int start(int argc, char** argv) {
	if (argc > 1) {
		if (0 == strcmp(argv[1], "proxy")) {
			if (argc > 2) {
				int ret = -1;
				if (0 == strcmp(argv[2], "UART1")) {
					ret = UARTGrpProxy.init(1);
				} else if (0 == strcmp(argv[2], "UART2")) {
					ret = UARTGrpProxy.init(2);
				} else {
					Shell.Out.println("unknown proxy target: %s", argv[2]);
				}
				if (ret == 0) {
					Shell.Out.println("proxy at %s initialized", argv[2]);
				} else {
					Shell.Out.println("proxy (%s) initialze failed", argv[2]);
				}
			} else {
				ESP_LOGE(TAG, "Need to specify proxy object");
				Shell.Out.println("Need to specify proxy object\n");
				return -1;
			}
		}
	}
	return 0;
}

static int tasks(int argc, char** argv) {
	printf("GG I'm tasks!!!\n");
	Shell.Out.printf("Hello~~~\r\n");
	return 0;
}

static int print(int argc, char** argv) {
	if (argc > 1) {
		if (0 == strcmp(argv[1], "time")) {
			Shell.Out.println("time is %s", shellCurrentTime.toString());
		} else if (0 == strcmp(argv[1], "bootscript")) {
			/*FILE *fp = fopen(BOOTSCRIPTFILE, "r");
			char buf[MAX_BOOTSCRIPTFILE_SIZE]; // TODO: change to global buf
			if (fp == NULL) {
				Shell.Out.println("bootscript not exist");
				return -1;
			}
			buf[0] = '\0';
			if (fgets(buf, MAX_BOOTSCRIPTFILE_SIZE, fp) == NULL) {
				Shell.Out.println("bootscript fgets failed");
			}
			fclose(fp);
			Shell.Out.println("%s", buf);*/ // TODO: using FATFS to save information failed
			char buf[MAX_BOOTSCRIPTFILE_SIZE];
			if (nvs_get_str_safe(BOOTSCRIPTNVSSTR, "", buf, MAX_BOOTSCRIPTFILE_SIZE) == ESP_OK) {
				Shell.Out.println(buf);
			} else {
				Shell.Out.println("bootscript nvs get failed");
				return -1;
			}
		}
	} else {
		Shell.Out.println("usage: set <time><bootscript>");
	}
	return 0;
}

static int set(int argc, char** argv) {
	if (argc > 2) {
		if (0 == strcmp(argv[1], "time")) {
			struct Shell_CurrentTime * c = &shellCurrentTime;
			sscanf(argv[2], "%d-%d-%d,%d:%d:%d.%d", &c->y
					, &c->M, &c->d, &c->h, &c->m, &c->s, &c->z);
			Shell.Out.println("set time OK");
		} else if (0 == strcmp(argv[1], "bootscript")) {
			ESP_LOGI(TAG, "start set");
			/*FILE *fp = fopen(BOOTSCRIPTFILE, "w");
			if (fp == NULL) {
				Shell.Out.println("could not open bootscript for write");
				return -1;
			}
			ESP_LOGI(TAG, "open set");
			fprintf(fp, "%s", argv[2]);
			ESP_LOGI(TAG, "fprintf set");
			fclose(fp);*/ // TODO: using FATFS to save information failed
			int len = strlen(argv[2]);
			ESP_LOGI(TAG, "bootscript is:");
			printf("%s\n", argv[2]);
			if (len < MAX_BOOTSCRIPTFILE_SIZE) {
				if (nvs_set_str_safe(BOOTSCRIPTNVSSTR, argv[2])) {
					Shell.Out.println("set bootscript OK");
				} else {
					Shell.Out.println("set bootscript failed");
					return -1;
				}
			} else {
				Shell.Out.println("bootscript too long (%d out of %d)", len, MAX_BOOTSCRIPTFILE_SIZE);
				return -1;
			}
		}
	} else {
		Shell.Out.println("usage: set <time><bootscript>");
	}
	return 0;
}

static void restart_task(void *pvParameters);
static int reboot(int argc, char** argv) {
	Shell.Out.println("reboot after 500ms\n");
	ESP_LOGE(TAG, "reboot after 500ms\n");
	xTaskCreate(&restart_task, "restart_task", 8192, NULL, 5, NULL);
	return 0;
}

static void restart_task(void *pvParameters) {
	vTaskDelay(500 / portTICK_RATE_MS);
	esp_restart();
}

static void register_tasks() {
	const static esp_console_cmd_t cmd = {
		.command = "tasks",
		.help = "print tasks running on freeRTOS",
		.hint = NULL,
		.func = tasks,
	};
	ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}
static void register_print() {
	const static esp_console_cmd_t cmd = {
		.command = "print",
		.help = "print variables",
		.hint = NULL,
		.func = print,
	};
	ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}
static void register_set() {
	const static esp_console_cmd_t cmd = {
		.command = "set",
		.help = "set variables",
		.hint = NULL,
		.func = set,
	};
	ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}
static void register_reboot() {
	const static esp_console_cmd_t cmd = {
		.command = "reboot",
		.help = "reboot",
		.hint = NULL,
		.func = reboot,
	};
	ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}
static void register_start() {
	const static esp_console_cmd_t cmd = {
		.command = "start",
		.help = "start a deamon or initialze somthing",
		.hint = NULL,
		.func = start,
	};
	ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}
