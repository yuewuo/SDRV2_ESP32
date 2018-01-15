#include "shellutil.h"

#define MAX_OUT_BUF_SIZE 4096
static const char TAG[] = "Shell Utilization";

static int out_printf(const char* fmt, ...);
static int out_println(const char* fmt, ...);
static void out_clear();
static const char* out_getbuf();
static int out_getlen();
static char out_buf[MAX_OUT_BUF_SIZE];
static int out_len = 0;

struct ShellUtil_Module Shell = {
	.Out = {
		.printf = out_printf,
		.println = out_println,
		.clear = out_clear,
		.buffer = out_getbuf,
		.length = out_getlen,
	},
};

static int out_printf(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int cnt = vsprintf(out_buf + out_len, fmt, args);
	va_end(args);
	if (cnt + out_len >= MAX_OUT_BUF_SIZE) {
		ESP_LOGE(TAG, "Buffer overflow! Need to reboot and debug!");
		return -1;
	}
	out_len += cnt;
	return cnt;
}

static int out_println(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int cnt = vsprintf(out_buf + out_len, fmt, args);
	va_end(args);
	if (cnt + out_len >= MAX_OUT_BUF_SIZE) {
		ESP_LOGE(TAG, "Buffer overflow! Need to reboot and debug!");
		return -1;
	}
	out_len += cnt;
	sprintf(out_buf + out_len, "\r\n");
	out_len += 2;
	return cnt + 2;
}

static void out_clear() {
	out_len = 0;
}

static const char* out_getbuf() {
	return out_buf;
}

static int out_getlen() {
	return out_len;
}

static const char* consoleCurrentTime_toString() {
	static char timestr[32];
	struct Shell_CurrentTime * c = &shellCurrentTime;
	sprintf(timestr, "%d-%d-%d,%d:%d:%d.%d", c->y, c->M, c->d, c->h, c->m, c->s, c->z);
	return timestr;
}

struct Shell_CurrentTime shellCurrentTime = {
	.toString = consoleCurrentTime_toString,
	.y=0, .M=0, .d=0, .h=0, .m=0, .s=0, .z=0,
};
