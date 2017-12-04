#include "UART1or2.h"

static void initialize(int uart_num);
static int UART1or2_send(int uart_num, const char* data, uint32_t len);
static int UART1or2_read(int uart_num, uint8_t* buf, uint32_t bufsize, int timeoutTICK);
static const char TAG[] = "UART1or2_Module";

struct UART1or2_Module UART1or2 = {
	.init = initialize,
	.send = UART1or2_send,
	.read = UART1or2_read,
	.uart1 = {
		.initialzed = 0,
		.uart_num = UART_NUM_1,
		.tx_io_num = NULL, // default 18
		.rx_io_num = NULL, // default 17
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
		.tx_io_num = NULL, // default 28
		.rx_io_num = NULL, // default 27
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

static void initialize(int uart_num) {
	struct UART1or2_config_struct* configp;
	if (uart_num == 1) {
		configp = &(UART1or2.uart1);
	} else if (uart_num == 2) {
		configp = &(UART1or2.uart2);
	} else {
		ESP_LOGE(TAG, "uart_num(%d) invalid, must be 1 or 2", uart_num);
		return;
	}
	uart_param_config(configp->uart_num, &(configp->uart_config));
	uart_set_pin(configp->uart_num, configp->tx_io_num, configp->rx_io_num
		, configp->rts_io_num, configp->cts_io_num);
	// We won't use a buffer for sending data.
	uart_driver_install(configp->uart_num, configp->RX_BUF_SIZE*2, 0, 0, NULL, 0);
	configp->initialzed = 1;
}

static struct UART1or2_config_struct* getConfig(int uart_num) {
	struct UART1or2_config_struct* configp;
	if (uart_num == 1) {
		configp = &(UART1or2.uart1);
	} else if (uart_num == 2) {
		configp = &(UART1or2.uart2);
	} else {
		ESP_LOGE(TAG, "uart_num(%d) invalid, must be 1 or 2", uart_num);
		return NULL;
	}
	if (configp->initialzed == 0) {
		ESP_LOGE(TAG, "uart_num(%d) not initialzed", uart_num);
		return NULL;
	} return configp;
}

static int UART1or2_send(int uart_num, const char* data, uint32_t len) {
	struct UART1or2_config_struct* configp = getConfig(uart_num);
	if (configp == NULL) return -1;
	const int txBytes = uart_write_bytes(configp->uart_num, data, len);
	//ESP_LOGI(TAG, "Wrote %d bytes", txBytes);
	return txBytes;
}

static int UART1or2_read(int uart_num, uint8_t* buf, uint32_t bufsize, int timeoutTICK) {
	struct UART1or2_config_struct* configp = getConfig(uart_num);
	if (configp == NULL) return -1;
	const int rxBytes = uart_read_bytes(configp->uart_num, buf, bufsize, timeoutTICK);
	//ESP_LOGI(TAG, "Read %d bytes: '%s'", bufsize, buf);
    //ESP_LOG_BUFFER_HEXDUMP(TAG, buf, bufsize, ESP_LOG_INFO);
	return rxBytes;
}
