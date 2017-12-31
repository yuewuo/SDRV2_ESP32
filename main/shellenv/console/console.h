#pragma once
#include "autoinclude.h"

#define MOUNT_PATH "/data"

struct Console_Module {
	uint8_t initialzed;
	void (*init)();
	int (*exec)(char* cmd);
};
extern struct Console_Module Console;
