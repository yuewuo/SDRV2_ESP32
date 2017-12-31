#pragma once

#include "autoinclude.h"
#include "driver/spi_master.h"

typedef struct {
    uint8_t initialzed;
	uart_port_t spi_num;

    int miso_io_num;
    int mosi_io_num;
    int rts_io_num;
    int cts_io_num;
} SPIGrp_config_t;
