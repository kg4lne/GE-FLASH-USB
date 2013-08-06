/*
 *
 *
 */

//_____  I N C L U D E S ___________________________________________________

#include "config.h"
#include "conf_usb.h"
#include "async.h"
#include "rs232_task.h"
#include "programmer_task.h"
#include "lib_mcu/usb/usb_drv.h"
#include "usb_descriptors.h"
#include "usb_standard_request.h"
#include "usb_specific_request.h"

#include <util/delay.h>
#include <util/crc16.h>


//_____ D E C L A R A T I O N S ____________________________________________

static int tx_vx2212_packet(uint8_t flgcmd, uint8_t bank, uint8_t* pbuf, uint16_t datasize);
static int rx_vx2212_packet(void);

/* Global Data */

extern uint8_t g_buf[BANKSIZE];     /* serial rx frame data buffer */


/************************************************************************
 * 
 ************************************************************************/

void rs232_task_init(void)
{
	/* Initialize serial port */
    usart_init();

    //sei();
}


/************************************************************************
 * 
 ************************************************************************/

void rs232_task(void)
{
    int c;

    /* Any character '.' character received? */
    if ((c = usart_getc()) == '.')
    {
        /* Got a period, echo back STX to begin packet rx */
        usart_putc(STX);

        /* LED on to indicating begin programming mode */
        PORTE |= _BV(B_LED);

        /* Attempt to read a data/command packet */
        rx_vx2212_packet(); 

        /* LED off, programming cycle complete */
        PORTE &= ~(_BV(B_LED));
    }
}

/************************************************************************
 * 
 ************************************************************************/

/* Attempt to receive a serial command packet from the host (GE-Flash).
 * If we get a valid packet then decode the command and process it.
 */

int rx_vx2212_packet(void)
{
    int         c;
    int         stat = -1;
    uint8_t     flgcmd;
    //uint8_t     banknum;
    uint16_t    lsb;
    uint16_t    msb;
    uint16_t    len;
    uint16_t    i;
    uint16_t    crc;
    uint16_t    crc_packet;

    crc = 0;

    /* Read the SOH first (wait up to 5 seconds) */
    if ((c = usart_tgetc(500)) == -1)
        return 1;
    if (c != SOH)
        return 2;

    // Read 'len_msb' byte
    if ((c = usart_tgetc(100)) == -1)
        return 3;
    crc = _crc_xmodem_update(crc, c);
    msb = (uint16_t)c;

    // Read 'len_lsb' byte
    if ((c = usart_tgetc(100)) == -1)
        return 4;
    crc = _crc_xmodem_update(crc, c);
    lsb = (uint16_t)c;
    len = (msb << 8) | lsb;

    // Read 'flgs/cmd' byte
    if ((c = usart_tgetc(100)) == -1)
        return 5;
    crc = _crc_xmodem_update(crc, c);
    flgcmd = (uint16_t)c;

    // Read 'bank#' byte state
    if ((c = usart_tgetc(100)) == -1)
        return 6;
    crc = _crc_xmodem_update(crc, c);
    //banknum = (uint16_t)c & 0x1;

    // frame length validation
    if (len > 256)
        return 7;

    // Read any data for length specified
    for (i=0; i < len; i++)
    {
        // Read a byte of data from the stream
        if ((c = usart_tgetc(100)) == -1)
            return 8;
        // Update CRC with the data byte
        crc = _crc_xmodem_update(crc, c);
        // Store it in the rx frame buffer
        g_buf[i] = c;
    }

    /* final CRC calculation */
    crc = _crc_xmodem_update(_crc_xmodem_update(crc,0),0);

    // Read CRC MSB byte
    if ((msb = usart_tgetc(100)) == -1)
        return 9;

    if ((lsb = usart_tgetc(100)) == -1)
        return 10;

    // Get the packet CRC value
    crc_packet = (msb << 8) | lsb;

    // Check if the CRC's match
    if (crc != crc_packet)
    {
        usart_putc(NAK);
        usart_putc(msb);
        usart_putc(lsb);
        // We return 0 to indicate continue reading
        // another packet after a NAK. 
        return 0;
    }

    /* VALID FRAME RECEIVED - DECODE THE COMMAND */

    stat = 0;

    switch(flgcmd & 0x0F)
    {
        case VXCMD_PING:
            break;

        case VXCMD_WRITE_BANK:
            /* Copy from temp rx buffer to active emulation buffer */
            //memcpy(&s_bank[banknum * BANKSIZE], g_buf, BANKSIZE);

            /* Test to see if this is the last packet or MORE are to follow */
            if (flgcmd & F_VXCMD_MORE)
            {
                /* More packets will follow this one.. */
                /* Tell host we stored the bank */
                usart_putc(ACK);
                usart_putc(msb);
                usart_putc(lsb);
            }
            else
            {
                /* Save the new data to our EPROM */
                //eeprom_write_block(s_bank, 0, BANKSIZE * MAX_BANKS);
                x2212_write(g_buf);

                /* Tell host we stored the bank */
                usart_putc(ACK);
                usart_putc(msb);
                usart_putc(lsb);
                /* Blink the LED 2 times upon completion */
                blink_led(3, 75);
                /* exit read packet loop */
                stat = -1;
            }
            break;

        case VXCMD_READ_BANK:
            /* Read X2212 data into our memory buffer */
            x2212_read(g_buf);
            /* Send the packet to GE-Flash */
            tx_vx2212_packet(VXCMD_READ_BANK, 0, g_buf, BANKSIZE);
            break;

        case VXCMD_GET_ACTIVE_BANK:
            break;

        case VXCMD_SET_ACTIVE_BANK:
            break;

        default:
            break;
    }

    return stat;
}


int tx_vx2212_packet(uint8_t flgcmd, uint8_t bank, uint8_t* pbuf, uint16_t datasize)
{
    int      i;
    uint16_t lsb;
    uint16_t msb;
    uint16_t crc = 0;

    /* Send the SOH first (0) */
    usart_putc(SOH);

    /* Send the length MSB/LSB bytes */
    msb = (uint16_t)datasize >> 4;;
    lsb = (uint16_t)datasize & 0xff;

    /* Send CRC MSB byte (1) */
    crc = _crc_xmodem_update(crc, msb);
    usart_putc(msb);

    /* Send CRC LSB byte (2) */
    crc = _crc_xmodem_update(crc, lsb);
    usart_putc(lsb);

    /* Send the FLG/CMD byte (3) */
    crc = _crc_xmodem_update(flgcmd, lsb);
    usart_putc(flgcmd);

    /* Send the BANK# byte (4) */
    crc = _crc_xmodem_update(flgcmd, bank);
    usart_putc(bank);

    /* Send any DATA payload */
    if (pbuf && datasize)
    {
        for (i=0; i < datasize; i++)
        {
            crc = _crc_xmodem_update(crc, pbuf[i]);
            usart_putc(pbuf[i]);
        }
    }

    /* Send the CRC MSB */
    usart_putc((int)(crc >> 4) & 0xff);

    /* Send the CRC LSB */
    usart_putc((int)crc & 0xff);

    return 0;
}

// End-Of-File
