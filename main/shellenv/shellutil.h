#include "autoinclude.h"

struct ShellStream {
	int (*printf)(const char* cmd, ...);
	int (*println)(const char* cmd, ...); // adding \r\n at end
	void (*clear)();
	const char* (*buffer)();
	int (*length)();
};
struct ShellUtil_Module {
	//struct ShellStream In;
	struct ShellStream Out;
};
extern struct ShellUtil_Module Shell;

struct Shell_CurrentTime {
	int y; int M; int d; int h; int m; int s; int z;
	const char* (*toString)();
};
extern struct Shell_CurrentTime shellCurrentTime;
