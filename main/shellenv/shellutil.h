#include "autoinclude.h"

struct ShellStream {
	int (*printf)(const char* cmd, ...);
	void (*clear)();
	const char* (*buffer)();
	int (*length)();
};
struct ShellUtil_Module {
	//struct ShellStream In;
	struct ShellStream Out;
};
extern struct ShellUtil_Module Shell;
