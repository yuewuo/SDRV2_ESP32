#include "console.h"
#include "console_cmd.h"
#include "shellutil.h"

static const char TAG[] = "Console";

static void register_functions() {
	console_register_basic();
	console_register_nfc();
	console_register_ws2812();
	// your register functions here
}
static void initialize_filesystem();
static void initialize();
static int execute(const char* cmd);

struct Console_Module Console = {
	.initialzed = 0,
	.init = initialize,
	.exec = execute,
};

static void initialize_filesystem() {
	static wl_handle_t wl_handle;
    const esp_vfs_fat_mount_config_t mount_config = {
            .max_files = 10,
            .format_if_mount_failed = true
    };
	esp_err_t err = esp_vfs_fat_spiflash_mount(MOUNT_PATH, "storage", &mount_config, &wl_handle);
	if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (0x%x)", err);
        return;
    }
}

static void initialize() {
	initialize_filesystem();
	esp_console_config_t console_config = {
            .max_cmdline_args = 8,
            .max_cmdline_length = 256,
    };
	ESP_ERROR_CHECK( esp_console_init(&console_config) );

	register_functions();

	Console.initialzed = 1;
}

static int execute(const char* cmd) {
	if (Console.initialzed == 0) {
		Console.init();
	}
	printf("console received, cmd is %s\n", cmd);
	int ret;
    esp_err_t err = esp_console_run(cmd, &ret);
	if (err == ESP_ERR_NOT_FOUND) {
		Shell.Out.printf("ESP_ERR_NOT_FOUND \"%s\"\n", cmd);
		printf("ESP_ERR_NOT_FOUND \"%s\"\n", cmd);
	} else if (err == ESP_ERR_INVALID_ARG) {
		Shell.Out.printf("ESP_ERR_INVALID_ARG \"%s\"\n", cmd);
		printf("ESP_ERR_INVALID_ARG \"%s\"\n", cmd);
	} else if (err == ESP_OK && ret != ESP_OK) {
		Shell.Out.printf("Command returned non-zero error code: 0x%x\n", ret);
		printf("Command returned non-zero error code: 0x%x\n", ret);
	} else if (err != ESP_OK) {
		Shell.Out.printf("Internal error: 0x%x\n", err);
		printf("Internal error: 0x%x\n", err);
	}
	return 0;
}
