/*
 * GE-Flash USB Programmer USB Driver and I/O Definitions
 *
 * Version 1.01 - 06/16/2013
 *
 * Copyright (C) 2013 KG4LNE. All Rights Reserved.
 */

#ifndef _USB_GE_FLASH_H_
#define _USB_GE_FLASH_H_
                             
// Global Class, SubClass & Protocol constants for Mass Storage
#define  GEFLASH_USB_CLASS              0x08


// GE-Flash Request Packet Command Codes
#define GEFU_CMD_GET_EPROM_TYPE 0x01            // Return the current EPROM type
#define GEFU_CMD_SET_EPROM_TYPE 0x02            // Set the EPROM type X2212 or X2444
#define GEFU_CMD_STORE          0x03            // Store EPROM data from memory buffer to EPROM
#define GEFU_CMD_RECALL         0x04            // Recall EPROM data to in memory buffer
#define GEFU_CMD_READ_BYTE      0x05            // read data at address
#define GEFU_CMD_WRITE_BYTE     0x06            // write data at address
#define GEFU_CMD_RESET          0xFF            // Reset the GE-Flash USB programmer

// EPROM device type for GET/SET EPROM TYPE command
#define GEFU_EPROM_TYPE_X2212   1
#define GEFU_EPROM_TYPE_X2444   2

// GE-Flash USB Programmer Command Packet Structure
typedef struct _GE_FLASH_USB {
    uint8_t     ucCommand;      // command to the programmer
    uint8_t     ucStatus;       // return status
    uint8_t     ucDir;          // direction 1=IN, 0=OUT
    uint8_t     ucData;         // in or out data byte
    uint16_t    wAddress;       // 16-bit address
} GE_FLASH_USB;

// Specific USB setup requests for GE-Flash USB programmer
#define SETUP_GE_FLASH_STORE            0xFB    // Store EPROM data from memory buffer to EPROM
#define SETUP_GE_FLASH_RECALL           0xFC    // Recall EPROM data to in memory buffer
#define SETUP_GE_FLASH_GET_EPROM_TYPE   0xFD    // Return the current EPROM type
#define SETUP_GE_FLASH_SET_EPROM_TYPE   0xFE    // Set the EPROM type X2212 or X2444
#define SETUP_GE_FLASH_RESET            0xFF    // Reset the GE-Flash USB programmer

#endif   // _USB_GE_FLASH_H_

