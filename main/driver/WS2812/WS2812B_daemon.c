#include "WS2812B_daemon.h"

/*
 * This demo is written by Yue Wu at 2017.12
 * It's a daemon which could run as a task
 * because that LED is an output device, no necessary callback is needed,
 *     we could use this daemon to control the LED using higher level API
 */

#define pixel_count 13 // you can change this value before compile

static const char TAG[] = "WS2812 Daemon";

struct WS2812_state {
	TickType_t startTime;
	wsRGB_t pixels[pixel_count]; // using at
	int vec; // using as a bit set
	wsRGB_t para1;
	int para2;
	int para3;
	wsRGB_t para4;
	int type;
}; typedef struct WS2812_state WS2812_state_t;
static void initStat(WS2812_state_t* stat);

static WS2812_state_t* nowStat = NULL;
static WS2812_state_t* nextStat = NULL;

#define Type_SingleColor 0x01
#define Type_Breathing 0x02
#define Type_Rainbow 0x03

static int WS2812_daemon_isStartedReg = 0;
int WS2812_daemon_isStarted() {
	return WS2812_daemon_isStartedReg;
}

int WS2812_daemon_SingleColor(wsRGB_t color, TickType_t delay) {
	if (nextStat == NULL) return -1;
	initStat(nextStat); // avoid resetting
	for (int i=0; i<pixel_count; ++i) {
		nextStat->pixels[i] = color;
	}
	nextStat->type = Type_SingleColor;
	nextStat->startTime = delay + xTaskGetTickCount();
	return 0;
}

int WS2812_daemon_Breathing(wsRGB_t color, TickType_t delay) {
	if (nextStat == NULL) return -1;
	initStat(nextStat); // avoid resetting
	nextStat->para1 = color;
	nextStat->type = Type_Breathing;
	nextStat->startTime = delay + xTaskGetTickCount();
	return 0;
}

int WS2812_daemon_Rainbow(TickType_t delay) {
	if (nextStat == NULL) return -1;
	initStat(nextStat); // avoid resetting
	nextStat->type = Type_Rainbow;
	nextStat->startTime = delay + xTaskGetTickCount();
	return 0;
}

#define getSeted(vec) (((vec) >> 0) & 0x01)
#define setSeted(vec) ((vec) |= (0x01 << 0))

static void initStat(WS2812_state_t* stat) {
	stat->startTime = 0;
	stat->vec = 0;
	stat->para1.g = stat->para1.r = stat->para1.b = 0;
	stat->para2 = 0;
	stat->para3 = 0;
	stat->para4.g = stat->para4.r = stat->para4.b = 0;
}

static void callSingleColor(ws2812_t* ws2812, WS2812_state_t* stat) {
	//ESP_LOGI(TAG, "call callSingleColor");
	if (! getSeted(stat->vec)) { // seted, not set again
		//ESP_LOGI(TAG, "change color");
		setSeted(stat->vec);
		WS2812B.setLeds(ws2812, stat->pixels, pixel_count);
	}
}

static void callBreathing(ws2812_t* ws2812, WS2812_state_t* stat) {
	// color in para1
	static const char up = 0;
	static const char down = 1;
	static const signed char delta = 5;
	int step = stat->para2;
	int orient = stat->para3;
	if (! getSeted(stat->vec)) { // seted, not set again
		setSeted(stat->vec);
		orient = up;
		step = 0;
	}
	// do update
	if (orient == up) {
		step += delta;
		if (step > 255) {
			step = 255;
			orient = down;
		}
	} else {
		step -= delta;
		if (step < 0) {
			step = 0;
			orient = up;
		}
	}
	for (int i=0; i<pixel_count; ++i) {
		stat->pixels[i].r = (int)((double)(stat->para1.r) / 255 * step);
		stat->pixels[i].g = (int)((double)(stat->para1.g) / 255 * step);
		stat->pixels[i].b = (int)((double)(stat->para1.b) / 255 * step);
	}
	WS2812B.setLeds(ws2812, stat->pixels, pixel_count);
	// save
	stat->para2 = step;
	stat->para3 = orient;
}

static void callRainbow(ws2812_t* ws2812, WS2812_state_t* stat) {
	static const uint8_t anim_step = 10;
	static const uint8_t anim_max = 250;
	wsRGB_t color = stat->para1;
	wsRGB_t color2 = stat->para4;
	uint8_t step = stat->para2;
	uint8_t step2 = stat->para3;
	wsRGB_t* pixels = stat->pixels;
	if (! getSeted(stat->vec)) { // initialize
		setSeted(stat->vec);
		color.r = anim_max;
	    color.g = 0;
	    color.b = 0;
		step = 0;
		color2.r = anim_max;
	    color2.g = 0;
	    color2.b = 0;
		step2 = 0;
	}
	// do loop
	color = color2;
	step = step2;
	for (uint8_t i = 0; i < pixel_count; i++) {
		pixels[i] = color;
		if (i == 1) {
			color2 = color;
			step2 = step;
		}
		switch (step) {
		case 0:
			color.g += anim_step;
			if (color.g >= anim_max) step++;
			break;
		case 1:
			color.r -= anim_step;
			if (color.r == 0) step++;
			break;
		case 2:
			color.b += anim_step;
			if (color.b >= anim_max) step++;
			break;
		case 3:
			color.g -= anim_step;
			if (color.g == 0) step++;
			break;
		case 4:
			color.r += anim_step;
			if (color.r >= anim_max) step++;
			break;
		case 5:
			color.b -= anim_step;
			if (color.b == 0) step = 0;
			break;
		}
	}
	WS2812B.setLeds(ws2812, pixels, pixel_count);

	// save
	stat->para1 = color;
	stat->para2 = step;
	stat->para3 = step2;
	stat->para4 = color2;
}

static int callUpdate(ws2812_t* ws2812, WS2812_state_t* stat, WS2812_state_t* nxtstat) {
	//ESP_LOGI(TAG, "call update");
	if (nxtstat->startTime == 0 || xTaskGetTickCount() < nxtstat->startTime) {
		// keep in this state
		if (stat->type == Type_SingleColor) {
			callSingleColor(ws2812, stat);
		} else if (stat->type == Type_Breathing) {
			callBreathing(ws2812, stat);
		} else if (stat->type == Type_Rainbow) {
			callRainbow(ws2812, stat);
		}
	} else { // change to next state
		initStat(stat); // avoid loop
		return 1;
	}
	return 0;
}

void WS2812_daemon_task(void *pvParameters) {
	if (WS2812_daemon_isStarted()) {
		ESP_LOGI(TAG, "Daemon is started. Only one instance should exist.");
		return;
	}
	WS2812_daemon_isStartedReg = 1;
	ESP_LOGI(TAG, "Task started");

	ESP_LOGI(TAG, "Running Initialization step");
	ws2812_t ws2812;
	ws2812.channel = WS2812B.demo.channel; // rmt channel is 0
	ws2812.gpio = WS2812B.demo.PIN;
	ws2812.size = WS2812B.demo.CNT;
	WS2812B.init(&ws2812);
	const TickType_t delay = 30 / portTICK_PERIOD_MS; // 30ms
	nowStat = malloc(sizeof(WS2812_state_t));
	nextStat = malloc(sizeof(WS2812_state_t));
	if (nowStat == NULL || nextStat == NULL) {
		ESP_LOGE(TAG, "Malloc failed");
		goto errorDeinit;
	}
	nowStat->startTime = 0;
	nextStat->startTime = 0;

	ESP_LOGI(TAG, "daemon Initialization finished");

	while (1) {
		if (callUpdate(&ws2812, nowStat, nextStat)) { // swap
			WS2812_state_t* tmp = nowStat;
			nowStat = nextStat;
			nextStat = tmp;
		}
		vTaskDelay(delay);
	}

errorDeinit:

	ESP_LOGI(TAG, "Running Deinitialization");
	WS2812B.deInit(&ws2812);

	ESP_LOGI(TAG, "Deleting Task Handler");
	vTaskDelete(NULL);
}

int WS2812_daemon_Print(char* buf, size_t n) {
	strcpy(buf, "WS2812_daemon_Print not realized yet");
	return 0;
}
