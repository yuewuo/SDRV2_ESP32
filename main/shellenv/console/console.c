#include "console.h"
#include "console_cmd.h"
#include "shellutil.h"
#include "autoinclude.h"

static const char TAG[] = "Console";

static void register_functions() {
	console_register_basic();
	console_register_nfc();
	console_register_ws2812();
	console_register_gpio();
	// your register functions here
}
static void initialize_filesystem();
static void initialize();
static int execute(char* cmd);
static int execbootscript();

struct Console_Module Console = {
	.initialzed = 0,
	.init = initialize,
	.exec = execute,
	.execbootscript = execbootscript,
};

static void initialize_filesystem() {
	static wl_handle_t wl_handle;
    const esp_vfs_fat_mount_config_t mount_config = {
            .max_files = 4,
            .format_if_mount_failed = true
    };
	esp_err_t err = esp_vfs_fat_spiflash_mount(MOUNT_PATH, "storage", &mount_config, &wl_handle);
	if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (0x%x)", err);
        return;
    } ESP_LOGI(TAG, "OK to mount FATFS (0x%x)", err);
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

static int execbootscript() {
	if (Console.initialzed == 0) {
		Console.init();
	}
	printf("executing bootscript\n");
	/*FILE *fp = fopen(BOOTSCRIPTFILE, "r");
	char buf[MAX_BOOTSCRIPTFILE_SIZE];
	if (fp == NULL) {
		ESP_LOGE(TAG, "bootscript not exist");
		return -1;
	}
	buf[0] = '\0';
	if (fgets(buf, MAX_BOOTSCRIPTFILE_SIZE, fp) == NULL) {
		ESP_LOGE(TAG, "bootscript fgets failed");
	}
	fclose(fp);
	if (buf[0] != '\0') return execute(buf);
	return -1;*/ // TODO: using FATFS to save information failed
	char buf[MAX_BOOTSCRIPTFILE_SIZE];
	if (nvs_get_str_safe(BOOTSCRIPTNVSSTR, "", buf, MAX_BOOTSCRIPTFILE_SIZE) == ESP_OK) {
		return execute(buf);
	} else {
		ESP_LOGE(TAG, "bootscript nvs get failed");
	} return -1;
}

static int execute(char* cmd) {
	if (Console.initialzed == 0) {
		Console.init();
	}
	printf("console received, cmd is %s\n", cmd);
	int ret = 0;
	int quetocnt = 0; // count "
	int mustOKtocontinue = false;
	int nextcontinue = true;
	char* head = cmd;
	esp_err_t err = ESP_OK;
	while (nextcontinue) {
		while (*cmd && *cmd != '\n' && !(*cmd == '&' && *(cmd+1) == '&' && quetocnt%2 == 0)) { if(*cmd == '\"')++quetocnt; ++cmd;}
		if (*cmd == '\0') nextcontinue = false; // hit the end
		int twoand = (*cmd == '&' && *(cmd+1) == '&' && quetocnt%2 == 0);
		if (twoand) *(cmd+1) = ' ';
		*cmd = 0;
		++cmd;
		if (mustOKtocontinue && ret != 0 && err != ESP_OK) continue;
		if (twoand) mustOKtocontinue = true;
		else mustOKtocontinue = false;
		if (cmd != head) {
			printf("run one cmd: %s\n", head);
			err = esp_console_run(head, &ret);
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
		}
		head = cmd;
	}
	return 0;
}
