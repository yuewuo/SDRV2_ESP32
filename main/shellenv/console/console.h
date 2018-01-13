#pragma once
#include "autoinclude.h"

#define MOUNT_PATH "/data"
//#define BOOTSCRIPTFILE MOUNT_PATH "/bootscript.sh"
#define BOOTSCRIPTNVSSTR "bootscript"
#define MAX_BOOTSCRIPTFILE_SIZE 1024

struct Console_Module {
	uint8_t initialzed;
	void (*init)();
	int (*exec)(char* cmd);
	int (*execbootscript)();
};
extern struct Console_Module Console;
