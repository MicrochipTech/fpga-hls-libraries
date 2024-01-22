/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * @file sample_hw_platform.h
 * @author Microchip FPGA Embedded Systems Solutions
 * @brief Platform definitions
 *
 *
 * SVN $Revision: 13158 $
 * SVN $Date: 2021-01-31 10:57:57 +0530 (Sun, 31 Jan 2021) $
 */
 /*========================================================================*//**
  @mainpage Example file detailing how hw_platform.h should be constructed for
    the Mi-V processors.

    @section intro_sec Introduction
    The hw_platform.h is to be located in the project root directory.
    Currently this file must be hand crafted when using the Mi-V Soft Processor.

    You can use this file as an example.
    Rename this file from sample_hw_platform.h to hw_platform.h and store it in
    the root folder of your project. Then customize it per your HW design.

    @section driver_configuration Project configuration Instructions
    1. Change SYS_CLK_FREQ define to frequency of Mi-V Soft processor clock
    2  Add all other core BASE addresses
    3. Add peripheral Core Interrupt to Mi-V Soft processor interrupt mappings
    4. Define MSCC_STDIO_UART_BASE_ADDR if you want a CoreUARTapb mapped to
       STDIO
*//*=========================================================================*/

#ifndef HW_PLATFORM_H
#define HW_PLATFORM_H

/***************************************************************************//**
 * Soft-processor clock definition
 * This is the only clock brought over from the Mi-V Libero design.
 */
#ifndef SYS_CLK_FREQ
#define SYS_CLK_FREQ                    50000000UL
#endif

/***************************************************************************//**
 * Non-memory Peripheral base addresses
 * Format of define is:
 * <corename>_<instance>_BASE_ADDR
 * The <instance> field is optional if there is only one instance of the core
 * in the design
 */
#define COREUARTAPB0_BASE_ADDR          0x70001000UL
#define COREGPIO_IN_BASE_ADDR           0x70002000UL
#define CORETIMER0_BASE_ADDR            0x70003000UL
#define CORETIMER1_BASE_ADDR            0x70004000UL
#define COREGPIO_OUT_BASE_ADDR          0x70005000UL

// These are not used
#define FLASH_CORE_SPI_BASE             0x70006000UL
#define CORE16550_BASE_ADDR             0x70007000UL

/***************************************************************************//**
 * Peripheral Interrupts are mapped to the corresponding Mi-V Soft processor
 * interrupt from the Libero design.
 *
 * On the legacy RV32 cores, there can be up to 31 external interrupts (IRQ[30:0]
 * pins). The legacy RV32 Soft processor external interrupts are defined in the
   riscv_plic.h
 *
 * These are of the form
 * typedef enum
{
    NoInterrupt_IRQn = 0,
    External_1_IRQn  = 1,
    External_2_IRQn  = 2,
    .
    .
    .
    External_31_IRQn = 31
} IRQn_Type;

 On the legacy RV32 processors, the PLIC identifies the interrupt and passes it
 on to the processor core. The interrupt 0  is not used. The pin IRQ[0] should
 map to External_1_IRQn likewise IRQ[30] should map to External_31_IRQn

e.g

#define TIMER0_IRQn                     External_30_IRQn
#define TIMER1_IRQn                     External_31_IRQn

 On the MIV_RV32 core, there is one external interrupts and it can also have
 up to six optional external system interrupts. On these cores there is no PLIC
 and these interrupts are directly delivered to the processor core, hence unlike
 legacy RV32 core no interrupt number mapping is necessary on MIV_RV32 core.
 */

/****************************************************************************
 * Baud value to achieve a 115200 baud rate with system clock defined by
 * SYS_CLK_FREQ.
 * This value is calculated using the following equation:
 *      BAUD_VALUE = (CLOCK / (16 * BAUD_RATE)) - 1
 *****************************************************************************/
#define BAUD_VALUE_115200               ((SYS_CLK_FREQ / (16 * 115200)) - 1)

/******************************************************************************
 * Baud value to achieve a 57600 baud rate with system clock defined by
 * SYS_CLK_FREQ.
 * This value is calculated using the following equation:
 *      BAUD_VALUE = (CLOCK / (16 * BAUD_RATE)) - 1
 *****************************************************************************/
 #define BAUD_VALUE_57600                ((SYS_CLK_FREQ / (16 * 57600)) - 1)

/***************************************************************************//**
 * Define MSCC_STDIO_THRU_CORE_UART_APB in the project settings if you want the
 * standard IOs to be redirected to a terminal via UART.
 */
#ifdef MSCC_STDIO_THRU_CORE_UART_APB
/*
 * A base address mapping for the STDIO printf/scanf mapping to CortUARTapb
 * must be provided if it is being used
 *
 * e.g. #define MSCC_STDIO_UART_BASE_ADDR COREUARTAPB1_BASE_ADDR
 */
#define MSCC_STDIO_UART_BASE_ADDR COREUARTAPB0_BASE_ADDR

#ifndef MSCC_STDIO_UART_BASE_ADDR
#error MSCC_STDIO_UART_BASE_ADDR not defined- e.g. #define MSCC_STDIO_UART_BASE_ADDR COREUARTAPB1_BASE_ADDR
#endif

#ifndef MSCC_STDIO_BAUD_VALUE
/*
 * The MSCC_STDIO_BAUD_VALUE define should be set in your project's settings to
 * specify the baud value used by the standard output CoreUARTapb instance for
 * generating the UART's baud rate if you want a different baud rate from the
 * default of 115200 baud
 */
#define MSCC_STDIO_BAUD_VALUE           115200
#endif  /*MSCC_STDIO_BAUD_VALUE*/

#endif  /* end of MSCC_STDIO_THRU_CORE_UART_APB */
/*******************************************************************************
 * End of user edit section
 */
#endif /* HW_PLATFORM_H */
