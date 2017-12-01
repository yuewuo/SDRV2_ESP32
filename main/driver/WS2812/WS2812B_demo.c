#include "WS2812B.h"

/*
 * This demo is written by Yue Wu from PKU Makerspace
 * It's a host which could handle mode of WS2812
 * doing something like rainbow when there is lots of LED connected
 * for simplicity, put everything in "demo" struct, see "WS2812.h"
 */

static const char TAG[] = "WS2812 Demo Task";

void WS2812_Demo_task(void *pvParameters) {
	ESP_LOGI(TAG, "Task started");

	ESP_LOGI(TAG, "Running Initialization step");
	ws2812_t ws2812;
	ws2812.channel = WS2812B.demo.channel; // rmt channel is 0
	ws2812.gpio = WS2812B.demo.PIN;
	ws2812.size = WS2812B.demo.CNT;
	WS2812B.init(&ws2812);

	ESP_LOGI(TAG, "Running Rainbow for %d(s)", WS2812B.demo.duration);
	TickType_t durationTime = xTaskGetTickCount()
				+ (1000 * WS2812B.demo.duration) / portTICK_PERIOD_MS;
	const uint8_t anim_step = 10;
	const uint8_t anim_max = 250;
	uint8_t pixel_count = WS2812B.demo.CNT; // Number of your "pixels"
	const uint8_t delay = 25; // duration between color changes
	wsRGB_t color;
	color.r = anim_max;
    color.g = 0;
    color.b = 0;
    uint8_t step = 0;
	wsRGB_t color2;
    color2.r = anim_max;
    color2.g = 0;
    color2.b = 0;
    uint8_t step2 = 0;
    wsRGB_t *pixels;
	if (NULL != (pixels = malloc(sizeof(wsRGB_t) * pixel_count))) {
		while (xTaskGetTickCount() < durationTime) {
			color = color2;
			step = step2;
			//printf("rainbow(%d,%d,%d)\n", color.r, color.g, color.b);
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
			WS2812B.setLeds(&ws2812, pixels, pixel_count);
			vTaskDelay(delay / portTICK_PERIOD_MS);
		}
		free(pixels);
	} else {
		ESP_LOGE(TAG, "%s: %d Unable to allocate space!\n", __FILE__, __LINE__);
	}

	ESP_LOGI(TAG, "Running Deinitialization");
	WS2812B.deInit(&ws2812);

	ESP_LOGI(TAG, "Deleting Task Handler");
	vTaskDelete(NULL);
}
