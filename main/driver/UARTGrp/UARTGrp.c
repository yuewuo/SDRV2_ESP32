#include "UARTGrp.h"

static int getIndex(int uart_num);
static UARTGrp_config_t* getConfig(int uart_num);
static void initialize(int uart_num);
static int UARTGrp_send(int uart_num, const char* data, uint32_t len);
static int UARTGrp_read(int uart_num, uint8_t* buf, uint32_t bufsize, int timeoutTICK);
static void printInitInfo (int (*pffunc)(const char*, ...), int uart_num);
static const char TAG[] = "UARTGrp_Module";

struct UARTGrp_Module UARTGrp = {
	.getIndex = getIndex,
	.getConfig = getConfig,
	.init = initialize,
	.Send = UARTGrp_send,
	.Read = UARTGrp_read,
	.printInitInfo = printInitInfo,
	.uart1 = {
		.initialzed = 0,
		.uart_num = UART_NUM_1,
		.tx_io_num = 18, // default 18
		.rx_io_num = 17, // default 17
		.rts_io_num = NULL, // default 19
		.cts_io_num = NULL, // default 20
		.uart_config = {
			.baud_rate = 115200,
			.data_bits = UART_DATA_8_BITS,
			.parity = UART_PARITY_DISABLE,
	        .stop_bits = UART_STOP_BITS_1,
	        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		},
		.RX_BUF_SIZE = 1024,
	},
	.uart2 = {
		.initialzed = 0,
		.uart_num = UART_NUM_2,
		.tx_io_num = 28, // default 28
		.rx_io_num = 27, // default 27
		.rts_io_num = NULL, // default 21
		.cts_io_num = NULL, // default 22
		.uart_config = {
			.baud_rate = 115200,
			.data_bits = UART_DATA_8_BITS,
			.parity = UART_PARITY_DISABLE,
	        .stop_bits = UART_STOP_BITS_1,
	        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		},
		.RX_BUF_SIZE = 1024,
	},
};

// for storage in vector, get smooth index
static int getIndex(int uart_num) {
	switch (uart_num) {
	case 1: return 0;
	case 2: return 1;
	}
	return -1;
}

#define OPT_MUSTNOTINITIALIZED 0x01
static UARTGrp_config_t* getConfigWithOpts(int uart_num, uint8_t opt) {
	UARTGrp_config_t* configp;
	if (uart_num == 1) {
		configp = &(UARTGrp.uart1);
	} else if (uart_num == 2) {
		configp = &(UARTGrp.uart2);
	} else {
		ESP_LOGE(TAG, "uart_num(%d) invalid, must be 1 or 2", uart_num);
		return NULL;
	}
	if (opt == OPT_MUSTNOTINITIALIZED) {
		if (configp->initialzed == 1) {
			ESP_LOGE(TAG, "uart_num(%d) has benn initialzed", uart_num);
			return NULL;
		}
	} else {
		if (configp->initialzed == 0) {
			ESP_LOGE(TAG, "uart_num(%d) not initialzed", uart_num);
			return NULL;
		}
	}
	return configp;
}

static void printInitInfo (int (*pffunc)(const char*, ...), int uart_num) {
	UARTGrp_config_t* configp = getConfigWithOpts(uart_num, 0);
	if (configp == NULL) return;
	int rx = configp->rx_io_num;
	int tx = configp->tx_io_num;
	pffunc("uart%d init, rx=%d, tx=%d, baud=%d\n", uart_num, rx, tx, configp->uart_config.baud_rate);
}

static UARTGrp_config_t* getConfig(int uart_num) {
	return getConfigWithOpts(uart_num, OPT_MUSTNOTINITIALIZED);
}

static void initialize(int uart_num) {
	UARTGrp_config_t* configp = getConfigWithOpts(uart_num, OPT_MUSTNOTINITIALIZED);
	if (configp == NULL) return;
	uart_param_config(configp->uart_num, &(configp->uart_config));
	uart_set_pin(configp->uart_num, configp->tx_io_num, configp->rx_io_num
		, configp->rts_io_num, configp->cts_io_num);
	// We won't use a buffer for sending data.
	uart_driver_install(configp->uart_num, configp->RX_BUF_SIZE*2, 0, 0, NULL, 0);
	configp->initialzed = 1;
}

static int UARTGrp_send(int uart_num, const char* data, uint32_t len) {
	UARTGrp_config_t* configp = getConfigWithOpts(uart_num, 0);
	if (configp == NULL) return -1;
	return uart_write_bytes(configp->uart_num, data, len);// txBytes;
}

static int UARTGrp_read(int uart_num, uint8_t* buf, uint32_t bufsize, int timeoutTICK) {
	UARTGrp_config_t* configp = getConfigWithOpts(uart_num, 0);
	if (configp == NULL) return -1;
	const int rxBytes = uart_read_bytes(configp->uart_num, buf, bufsize, timeoutTICK);
	return rxBytes;
}
