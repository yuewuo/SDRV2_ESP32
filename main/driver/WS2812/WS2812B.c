#include "WS2812B.h"

/*struct WS2812Host_State ws2812host_state = {
	0, //LEDCount
	{0,0,0,0} // cmd
};

#define WS2812_PIN	18

#define WS2812DropLoop(str, para...) do {\
	ESP_LOGE("WS2812Host", str, ##para);\
	while (1) {\
		vTaskDelay(1);\
	}\
} while(0)

void ws2812host_task(void *pvParameters) {
	//WS2812B_SampleRun();
	int i;
	int LEDCount;
	uint8_t cmdtype;
	wsRGB_t color;
	wsRGB_t *pixels;
	while (ws2812host_state.LEDCount == 0) {
		vTaskDelay(1);
	}
    LEDCount = ws2812host_state.LEDCount;
	ESP_LOGI("WS2812Host", "Set LED Count = %d", LEDCount);
	if (LEDCount > 0 && LEDCount <= 64) {
		WS2812B_init(0, WS2812_PIN, LEDCount);
	} else WS2812DropLoop("LEDCount errer(LEDCount = %d)", LEDCount);


    color.r = 0;
    color.g = 0;
    color.b = 0;

    pixels = malloc(sizeof(wsRGB_t) * LEDCount);
	//WS2812B_setLeds(pixels, 1);

	while (1) {
		cmdtype = ws2812host_state.cmd.type;
		if (cmdtype != 0) {
			ESP_LOGI("WS2812", "commandtype = %d", (int)cmdtype);
			if (cmdtype == WS2812CMDTYPE_ALLCHANGE) {
				color.r = ws2812host_state.cmd.r;
				color.g = ws2812host_state.cmd.g;
				color.b = ws2812host_state.cmd.b;
				for (i=0; i<LEDCount; ++i) {
					pixels[i] = color;
				}
				WS2812B_setLeds(pixels, LEDCount);
			}
		}
		ws2812host_state.cmd.type = 0;
		vTaskDelay(0);
	}
}*/






















/*  Copyright (C) 2017  Florian Menne
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef WS2812B_INVERTED
#define WS2812B_INVERTED 0
#endif

#ifndef WS2812B_USE_PL9823
#define WS2812B_USE_PL9823 0
#endif

#if WS2812B_INVERTED == 0
#if WS2812B_USE_PL9823 == 0
static const rmt_item32_t wsLogicZero = {.level0 = 1, .duration0 = 32, .level1 = 0, .duration1 = 68};
static const rmt_item32_t wsLogicOne = {.level0 = 1, .duration0 = 64, .level1 = 0, .duration1 = 36};
#else
static const rmt_item32_t wsLogicZero = {.level0 = 1, .duration0 = 28, .level1 = 0, .duration1 = 109};
static const rmt_item32_t wsLogicOne = {.level0 = 1, .duration0 = 109, .level1 = 0, .duration1 = 28};
#endif

#else
#if WS2812B_USE_PL9823 == 0
static const rmt_item32_t wsLogicZero = {.level0 = 0, .duration0 = 32, .level1 = 1, .duration1 = 68};
static const rmt_item32_t wsLogicOne = {.level0 = 0, .duration0 = 64, .level1 = 1, .duration1 = 36};
#else
static const rmt_item32_t wsLogicZero = {.level0 = 0, .duration0 = 28, .level1 = 1, .duration1 = 109};
static const rmt_item32_t wsLogicOne = {.level0 = 0, .duration0 = 109, .level1 = 1, .duration1 = 28};
#endif

#endif

//static rmt_channel_t channel;
//static unsigned int size;
//static rmt_item32_t* items;

static void WS2812B_init(rmt_channel_t channel, gpio_num_t gpio, unsigned int size, rmt_item32_t** itemsp);
static void WS2812B_setLeds(wsRGB_t* data, unsigned int size, rmt_item32_t* items, rmt_channel_t channel);
static void WS2812B_deInit(rmt_channel_t chan, rmt_item32_t* items);

static void WS2812B_init_moduleversion (ws2812_t* w) {
	WS2812B_init(w->channel, w->gpio, w->size, &(w->items));
	w->initialized = 1;
}

static void WS2812B_setLeds_moduleversion (ws2812_t* w, wsRGB_t* data, unsigned int size) {
	WS2812B_setLeds(data, size, w->items, w->channel);
}

static void WS2812_deInit_moduleversion(ws2812_t* w) {
	WS2812B_deInit(w->channel, w->items);
	w->initialized = 0;
}

struct WS2812B_Module WS2812B = {
	.init = WS2812B_init_moduleversion,
	.setLeds = WS2812B_setLeds_moduleversion,
	.deInit = WS2812_deInit_moduleversion,
	.demo = {
		.task = WS2812_Demo_task,
		.channel = 0,
		.PIN = 18,
		.CNT = 16,
		.duration = 10,
	},
};

void WS2812B_initStruct(ws2812_t* w) {
	w->initialized = 0;
	w->items = NULL;
}

void WS2812B_init(rmt_channel_t channel, gpio_num_t gpio, unsigned int size, rmt_item32_t** itemsp)
{
	*itemsp = NULL;

	if(!size)
	{
		printf("%s: %d Invalid size 0!\n", __FILE__, __LINE__);
		return;
	}

	if(NULL == (*itemsp = malloc(sizeof(rmt_item32_t) * size * 24)))
	{
		printf("%s: %d Unable to allocate space!\n", __FILE__, __LINE__);
		return;
	}

	rmt_config_t rmt_tx;
	memset(&rmt_tx, 0, sizeof(rmt_config_t));

	rmt_tx.channel = channel;
	rmt_tx.gpio_num = gpio;
	rmt_tx.mem_block_num = 1;
	rmt_tx.clk_div = 1;
	rmt_tx.tx_config.idle_output_en = 1;
#if WS2812B_INVERTED == 1
	rmt_tx.tx_config.idle_level = RMT_IDLE_LEVEL_HIGH;
#endif

	rmt_config(&rmt_tx);
	rmt_driver_install(rmt_tx.channel, 0, 0);
}

static void WS2812B_setLeds(wsRGB_t* data, unsigned int size, rmt_item32_t* items, rmt_channel_t channel)
{
	unsigned int itemCnt = 0;

	for(int i = 0; i < size; i++)
		for(int j = 0; j < 24; j++)
		{
			if(j < 8)
			{
#if WS2812B_USE_PL9823 == 0
				if(data[i].g & (1<<(7-j))) items[itemCnt++] = wsLogicOne;
#else
				if(data[i].r & (1<<(7-j))) items[itemCnt++] = wsLogicOne;
#endif
				else items[itemCnt++] = wsLogicZero;
			}

			else if (j < 16)
			{
#if WS2812B_USE_PL9823 == 0
				if(data[i].r & (1<<(7 - (j%8) ))) items[itemCnt++] = wsLogicOne;
#else
				if(data[i].g & (1<<(7 - (j%8) ))) items[itemCnt++] = wsLogicOne;
#endif
				else items[itemCnt++] = wsLogicZero;
			}
			else
			{
				if(data[i].b & (1<<( 7 - (j%8) ))) items[itemCnt++] = wsLogicOne;
				else items[itemCnt++] = wsLogicZero;
			}

		}

	rmt_write_items(channel, items, size * 24, false);
}

static void WS2812B_deInit(rmt_channel_t channel, rmt_item32_t* items)
{
	rmt_driver_uninstall(channel);
	free(items);
}
