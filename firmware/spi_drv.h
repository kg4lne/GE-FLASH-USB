/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//! SPI Low level drivers access.
//! \brief
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

#ifndef _SPI_DRV_H_
#define _SPI_DRV_H_

//_____ I N C L U D E S  ___________________________

#include "config.h"


//_____ M A C R O S  ___________________________

//----- Modes -----
#define  MSK_SPI_MODE            ((1<<CPOL)|(1<<CPHA))
#define  MSK_SPI_CPHA_LEADING    (0<<CPHA)
#define  MSK_SPI_CPHA_TRAILING   (1<<CPHA)
#define  MSK_SPI_CPOL_LOW        (0<<CPOL)
#define  MSK_SPI_CPOL_HIGH       (1<<CPOL)
#define  SPI_MODE_0              (MSK_SPI_CPOL_LOW|MSK_SPI_CPHA_LEADING)
#define  SPI_MODE_1              (MSK_SPI_CPOL_LOW|MSK_SPI_CPHA_TRAILING)
#define  SPI_MODE_2              (MSK_SPI_CPOL_HIGH|MSK_SPI_CPHA_LEADING)
#define  SPI_MODE_3              (MSK_SPI_CPOL_HIGH|MSK_SPI_CPHA_TRAILING)

//----- Bit rates -----
#define  MSK_SPI_MULT2           0x80
#define  MSK_SPR                 ((1<<SPR1)|(1<<SPR0))
#define  MSK_SPI_DIV4            ((0<<SPR1)|(0<<SPR0))
#define  MSK_SPI_DIV16           ((0<<SPR1)|(1<<SPR0))
#define  MSK_SPI_DIV64           ((1<<SPR1)|(0<<SPR0))
#define  MSK_SPI_DIV128          ((1<<SPR1)|(1<<SPR0))
#define  SPI_RATE_0              (MSK_SPI_MULT2|MSK_SPI_DIV4)     // Fper / 2 
#define  SPI_RATE_1              (MSK_SPI_DIV4)                   // Fper / 4 
#define  SPI_RATE_2              (MSK_SPI_MULT2|MSK_SPI_DIV16)    // Fper / 8 
#define  SPI_RATE_3              (MSK_SPI_DIV16)                  // Fper / 16 
#define  SPI_RATE_4              (MSK_SPI_MULT2|MSK_SPI_DIV64)    // Fper / 32 
#define  SPI_RATE_5              (MSK_SPI_DIV64)                  // Fper / 64 
#define  SPI_RATE_6              (MSK_SPI_DIV128)                 // Fper / 128 

//______  D E F I N I T I O N  ___________________________

// Global enable
#define  Spi_enable()            (SPCR |=  (1<<SPE))
#define  Spi_disable()           (SPCR &= ~(1<<SPE))

// Interrupt configuration
#define  Spi_enable_it()         (SPCR |=  (1<<SPIE))
#define  Spi_disable_it()        (SPCR &= ~(1<<SPIE))

// SPI Configuration : slave/master, modes (Clock Phase, Clock Polarity), Data Order, speed
#define  Spi_select_slave()      (SPCR &= ~(1<<MSTR))
#define  Spi_select_master()     (SPCR |=  (1<<MSTR))
#define  Spi_set_lsbfirst()      (SPCR |=  (1<<DORD))
#define  Spi_set_msbfirst()      (SPCR &= ~(1<<DORD))
#define  Spi_set_mode(mode)      {SPCR &= ~MSK_SPI_MODE; SPCR |= mode;}
#define  Spi_set_rate(rate)      {SPCR &= ~MSK_SPR; SPCR |= rate&MSK_SPR; (rate & MSK_SPI_MULT2)?  Spi_set_doublespeed() : Spi_clear_doublespeed();}
#define  Spi_set_doublespeed()   (SPSR |=  (1<<SPI2X))
#define  Spi_clear_doublespeed() (SPSR &= ~(1<<SPI2X))
#define  Spi_init_bus()          ((DDRB |= (1<<DDB2)|(1<<DDB1)|(1<<DDB0)))
#define  Spi_disable_ss()        (PORTB &= ~(0x01))                 // CS=low to disable
#define  Spi_enable_ss()         (PORTB |= 0x01)                    // CS=high to enable

// SPI Control
#define  Spi_wait_spif()         while ((SPSR & (1<<SPIF)) == 0)    // for any SPI_RATE_x 
#define  Spi_wait_eor()          while ((SPSR & (1<<SPIF)) == 0)    // wait end of reception 
#define  Spi_wait_eot()          while ((SPSR & (1<<SPIF)) == 0)    // wait end of transmission 
#define  Spi_eor()               ((SPSR & (1<<SPIF)) == (1<<SPIF))  // check end of reception 
#define  Spi_eot()               ((SPSR & (1<<SPIF)) == (1<<SPIF))  // check end of transmission 
#define  Spi_is_colision()       (SPSR&(1<<WCOL))
#define  Spi_get_byte()          (SPDR)
#define  Spi_tx_ready()          (SPSR & (1<<SPIF))
#define  Spi_rx_ready()          Spi_tx_ready()
#define  Spi_ack_read()          (SPSR)
#define  Spi_ack_write()         (SPDR)
#define  Spi_ack_cmd()           (SPSR)

// SPI transfer
#define  Spi_read_data()         (SPDR)
#define  Spi_write_data(byte)    {(SPDR=byte);Spi_wait_spif();}

#endif  // _SPI_DRV_H_ 

