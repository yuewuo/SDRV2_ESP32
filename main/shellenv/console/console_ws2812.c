#include "autoinclude.h"
#include "console_cmd.h"
#include "shellutil.h"
#include "WS2812B_daemon.h"
#include <ctype.h>

static int ws2812_console(int argc, char** argv);
void console_register_ws2812() {
    const static esp_console_cmd_t cmd = {
		.command = "ws2812",
		.help = "on off",
		.hint = NULL,
		.func = ws2812_console,
	};
	ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

#define ARGVCMP(x) (0 == strcmp(argv[i], x))
static int ws2812_console(int argc, char** argv) {
    if (argc == 1) { // return current state
        char str[1024];
        /*int n = */WS2812_daemon_Print(str, sizeof(str));
        Shell.Out.printf(str);
        return 0;
    }
    TickType_t delay = 0;
    wsRGB_t color = {255,255,255};
    int breath = 0;
    int actioni = -1;
    for (int i=1; i<argc; ++i) {
        if (ARGVCMP("-h")) {
            Shell.Out.printf("See more information at http://wuyue98.cn/download/WS2812API.html\n");
            return 0;
        } else if (ARGVCMP("-d")) {
            ++i;
            delay = atol(argv[i]) * 1000 / portTICK_PERIOD_MS;
        } else if (ARGVCMP("-b")) {
            breath = 1;
        } else if (ARGVCMP("-c") || ARGVCMP("--color")) {
            ++i;
            if (ARGVCMP("white")) {
                color.r = 255;
                color.g = 255;
                color.b = 255;
            } else if (ARGVCMP("red")) {
                color.r = 255;
                color.g = 0;
                color.b = 0;
            } else if (ARGVCMP("green")) {
                color.r = 0;
                color.g = 255;
                color.b = 0;
            } else if (ARGVCMP("blue")) {
                color.r = 0;
                color.g = 0;
                color.b = 255;
            } else if (ARGVCMP("purple")) {
                color.r = 255;
                color.g = 0;
                color.b = 255;
            } else if (ARGVCMP("yellow")) {
                color.r = 255;
                color.g = 255;
                color.b = 0;
            } else if (argv[i][0] == '(') {
                //char* s = argv[i] + 1;
                //while (*s && *s != ',') ++s;
                Shell.Out.printf("-c (r,g,b) has not been realized yet\n");
                return -1;
            } else {
                Shell.Out.printf("Color error, using -h to see more\n");
                return -1;
            }
        } else {
            actioni = i;
        }
    }
    int i = actioni;
    if (actioni == -1) {
        Shell.Out.printf("Action error\n"); return -1;
    } else if (ARGVCMP("off")) {
        color.r = 0;
        color.g = 0;
        color.b = 0;
        WS2812_daemon_SingleColor(color, delay);
    } else if (ARGVCMP("on")) {
        if (breath) {
            WS2812_daemon_Breathing(color, delay);
        } else {
            WS2812_daemon_SingleColor(color, delay);
        }
    } else if (ARGVCMP("rainbow")) {
        WS2812_daemon_Rainbow(delay);
    } else {
        Shell.Out.printf("Unknown action: %s\n", argv[i]);
    }
    Shell.Out.printf("WS2812~~~\n");
    return 0;
}
