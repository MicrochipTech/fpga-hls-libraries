#pragma once

// Drivers
#include "core_i2c.h"
#include "core_timer.h"
#include "core_gpio.h"
#include "miv_rv32_hal/miv_rv32_hal.h"
#include "fpga_design_config/fpga_design_config.h"
#include "imx334_corei2c/imx334_corei2c.h"

#define MIPI_TRNG_RST   GPIO_2

#define TIMER_LOAD_VALUE 25

// Print only if UART is enabled
#ifdef USE_UART
    #include "core_uart_apb.h"
    #include <stdio.h>
    #define UART_BUF_SZ 512
    #define PRINTF(fmt, args...) {sprintf((char *)gTxBuf, fmt, ## args); UART_polled_tx_string(&gUart, gTxBuf);}
#else
    #define PRINTF(fmt, args...)   
#endif

// delay function
#ifdef __cplusplus
extern "C" {
#endif

void usdelay(uint32_t us);

#ifdef __cplusplus
}
#endif
