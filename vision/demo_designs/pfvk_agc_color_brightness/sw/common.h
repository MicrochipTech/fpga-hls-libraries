#pragma once 

#ifdef USE_UART
#include "core_uart_apb.h"
#endif
#include "core_i2c.h"
#include "core_timer.h"
#include "core_gpio.h"
#include "imx334_corei2c.h"
#include <stdio.h>
#include "fpga_design_config/fpga_design_config.h"
#include "miv_rv32_hal/miv_rv32_hal.h"

#define LED1            GPIO_0
#define LED2            GPIO_1
#define LED3            GPIO_2
#define LED4            GPIO_3
#define MIPI_TRNG_RST   GPIO_4
#define AXI_RST         GPIO_5

#define TIMER_LOAD_VALUE 25000000

#define UART_BUF_SZ 512

#ifdef USE_UART
#define PRINTF(fmt, args...) {sprintf((char *)gTxBuf, fmt, ## args); UART_polled_tx_string(&gUart, gTxBuf);}
#else
#define PRINTF(fmt, args...)
#endif
