#pragma once

/*
 * Create this task to use uart throught a socket!
 */

#include "autoinclude.h"

struct UartSocket_Module {
	void (*task)(void* para);
	const char* taskName;
};
extern struct UartSocket_Module UartSocket;
