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

#ifndef _RS232_TASK_H_
#define _RS232_TASK_H_

//_____ I N C L U D E S ____________________________________________________


#include "config.h"

/****************************************************************************
 * VX2212 SERIAL PROTOCOL DEFINITIONS (GE-Flash Specific)
 ***************************************************************************/

/* X2212 Memory Size */

#define MAX_BANKS       2       

#define BANKSIZE        256     /* 256 x 4 bits */


// VX2212 RS-232 Serial Interface Structures and Contants
//                                                    
//                    +------------------------------+   byte
//                +-- |             SOH              |    0
//                |   +------------------------------+
//    Preamble ---+   |       Data Length (msb)      |    1
//      Part      |   +------------------------------+
//                +-- |       Data Length (lsb)      |    2
//                    +---+---+---+---+--------------+
//                +-- | M | 0 | 0 | 0 |   Command    |    3
//      Header ---+   +---+---+---+---+--------------+
//       Part     +-- |         Bank Number          |    4
//                    +------------------------------+
//                +-- |              .               |    n
//                |   |              .               |    .
//        Bank ---+   |             Data             |    .
//        Data    |   |              .               |    .
//                +-- |              .               |    .
//                    +------------------------------+
//                    |          CRC (msb)           |  datalen+5
//                    +------------------------------+
//                    |          CRC (lsb)           |  datalen+6
//                    +------------------------------+
// 
//  Frame Data Description:
// 
//      * SOH:          Start of frame header identifier.
// 
//      * Data length:  Length of 'Data' in bytes (must be 0, 256 or 512)
// 
//      * Flags:        M=MORE, 0, 0, 0
//
//      * Command:      0=Ping, 1=WriteBank, 2=ReadBank, etc...
// 
//      * Bank#:        Bank number (0=Bank-0, 1=Bank-1)
//       
//      * Data:         Banks data bytes (if data-length nonzero)
//       
//      * CRC value:    Xmodem CRC-16 (msb, lsb)
//

// VX2212 Packet Command Codes
#define VXCMD_PING              0x00        // ping cmd, echo back and ACK
#define VXCMD_WRITE_BANK        0x01        // write bank specified by bank# field 
#define VXCMD_READ_BANK         0x02        // read bank specified by bank# field
#define VXCMD_GET_ACTIVE_BANK   0x03        // get radio current active bank
#define VXCMD_SET_ACTIVE_BANK   0x04        // set radio current active bank

#define VXCMD_MASK(c)           (c & 0xFF)  // lower 4 bits are cmd bits

// command byte flag bits (upper four bits)
#define F_VXCMD_RES0            0x10        // reserved flag bit 0
#define F_VXCMD_RES1            0x20        // reserved flag bit 1
#define F_VXCMD_RES2            0x40        // reserver flag bit 2
#define F_VXCMD_MORE            0x80        // more bank data frames will follow

#endif // _RS232_TASK_H_

