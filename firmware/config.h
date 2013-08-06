/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file contains the system configuration definition.
//!
//! - Compiler:           IAR EWAVR and GNU GCC for AVR
//! - Supported devices:  AT90USB1287, AT90USB1286, AT90USB647, AT90USB646
//!
//! \author               Atmel Corporation: http://www.atmel.com \n
//!                       Support and FAQ: http://support.atmel.no/
//!
//! ***************************************************************************

/* Copyright (c) 2009 Atmel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an Atmel
 * AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE EXPRESSLY AND
 * SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

// Compiler switch (do not change these settings)
#include "compiler.h"   // Compiler definitions
#include <avr/io.h>     // Use AVR-GCC library


//! @defgroup global_config Application configuration
//! @{

#include "conf_scheduler.h"          // Scheduler tasks declaration

//! Enable (define) or not (undefine) the ADC usage
#undef USE_ADC

//! To include proper target hardware definitions, select target board (USBKEY)
#define  STK525   1					// do not change these settings
#define  TARGET_BOARD STK525
//#include "stk_525.h"

#define HARDWARE_REV	2			// Rev-A=1, Rev-B=2

//! CPU core frequency in kHz
#define FOSC 8000

/* GE-Flash Programmer Board I/O Definitions */

/* PORT-B I/O Bit Values */
#define B_CFG2		PB7
#define B_CFG1    	PB6

/* PORT-C I/O Bit Values */
#define B_CS      	PC7
#define B_WE      	PC6
#define B_RECALL  	PC5
#define B_STORE   	PC4

/* PORT-D I/O Bit Values */
#define B_TXD     	PD3
#define B_RXD     	PD2
#define B_CTS     	PD1
#define B_RTS     	PD0

/* PORT-E I/O Bit Values */
#define B_UVCON   	PE7
#define B_EPWR    	PE6
#define B_PBSW    	PE4
#define B_LED    	PE0

// -------- END Generic Configuration -------------------------------------

/* Default serial port baudrate */
#define BAUDRATE    9600UL

/****************************************************************************
 * GENERIC HELPER MACROS
 ***************************************************************************/

//#define _CLI()  __asm__ __volatile__ ("cli")
//#define _SEI()  __asm__ __volatile__ ("sei")
#define _NOP()  __asm__ __volatile__ ("nop")

#define ENTER_CRITICAL()	Disable_interrupt()
#define EXIT_CRITICAL()		Enable_interrupt()


// ADC Sample configuration, if we have one ... ___________________________

//! ADC Prescaler value
//#define ADC_PRESCALER 64
//! Right adjust
//#define ADC_RIGHT_ADJUST_RESULT 1
//! AVCC As reference voltage (See adc_drv.h)
//#define ADC_INTERNAL_VREF  2

//! @}

#endif // _CONFIG_H_

