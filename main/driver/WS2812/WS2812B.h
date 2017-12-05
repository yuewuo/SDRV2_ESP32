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

#ifndef MAIN_WS2812B_H_
#define MAIN_WS2812B_H_

#include "autoinclude.h"

typedef struct
{
	uint8_t g;
	uint8_t b;
	uint8_t r;
} wsRGB_t;

struct ws2812_t_struct {
	unsigned char initialized;
	rmt_channel_t channel;
	gpio_num_t gpio;
	rmt_item32_t* items;
	unsigned int size;
};
typedef struct ws2812_t_struct ws2812_t;
void WS2812B_initStruct(ws2812_t* w);

struct WS2812B_Module {
	void (*init)(struct ws2812_t_struct* self);
	void (*setLeds)(struct ws2812_t_struct* self, wsRGB_t* data, unsigned int size);
	void (*deInit)(struct ws2812_t_struct* self);
	struct {
		void (*task)(void* pvParameters);
		rmt_channel_t channel; // = 0
		gpio_num_t PIN; // = 18;
		unsigned int CNT; // = 16;
		int duration; // = 10; (second)
	} demo;
};
extern struct WS2812B_Module WS2812B;

// demo part
void WS2812_Demo_task(void *pvParameters);


/**
 * Init RMT module and allocates space
 * @param channel RMT channel
 * @param gpio GPIO Pin
 * @param size Number of LED's
 */
/*void WS2812B_init(rmt_channel_t channel, gpio_num_t gpio, unsigned int size,
			rmt_item32_t* items);
*/
/**
 * Writes to the LED
 * @param data
 * @param size Number of LED's - Must not exceed initialization size
 */
//void WS2812B_setLeds(wsRGB_t* data, unsigned int size);

/**
 * Deinit driver and free memory space
 */
//void WS2812B_deInit(void);

/*
//rainbow demo
void WS2812B_SampleRun(void);

//host task
void ws2812host_task(void *pvParameters);

#define WS2812CMDTYPE_ALLCHANGE 0x01
struct WS2812Host_State {
	int LEDCount; // can only set once
	struct {
		uint8_t type;
		uint8_t r;
		uint8_t g;
		uint8_t b;
	} cmd;
};

extern struct WS2812Host_State ws2812host_state;*/

#endif /* MAIN_WS2812B_H_ */
