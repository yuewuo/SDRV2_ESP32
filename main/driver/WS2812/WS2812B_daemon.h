#include "WS2812B.h"

extern int WS2812_daemon_SingleColor(wsRGB_t color, TickType_t delay);

extern int WS2812_daemon_Breathing(wsRGB_t color, TickType_t delay);

extern int WS2812_daemon_Rainbow(TickType_t delay);

extern int WS2812_daemon_Print(char* buf, size_t n);

extern void WS2812_daemon_task(void *pvParameters);
