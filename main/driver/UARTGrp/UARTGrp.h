#pragma once

/*
 * The io_num can be configured to anyone!
 * We won't use a buffer for sending data.
 */

#include "autoinclude.h"

struct UARTGrp_config_struct {
	uint8_t initialzed;
	uart_port_t uart_num;

	int tx_io_num;
	int rx_io_num;
	int rts_io_num;
	int cts_io_num;
	uart_config_t uart_config;
		//int baud_rate;
		//uart_word_length_t data_bits;
		//uart_parity_t parity;
		//uart_stop_bits_t stop_bits;
		//uart_hw_flowcontrol_t flow_ctrl;

	int RX_BUF_SIZE;
};
typedef struct UARTGrp_config_struct UARTGrp_config_t;

#define UARTGrpCount 2
struct UARTGrp_Module {
	int (*getIndex)(int uart_num);
	UARTGrp_config_t* (*getConfig)(int uart_num);
	void (*init)(int uart_num); // 1 or 2, also can be UART_NUM_1 / 2
	void (*printInitInfo) (int (*pffunc)(const char*, ...), int uart_num);
	int (*Send)(int uart_num, const char* data, uint32_t len);
	int (*Read)(int uart_num, uint8_t* buf, uint32_t bufsize, int timeoutTICK);

	UARTGrp_config_t uart1;
	UARTGrp_config_t uart2;
};
extern struct UARTGrp_Module UARTGrp;
