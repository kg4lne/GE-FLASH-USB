/*
 * GE-Flash USB Programmer
 *
 */

//_____  I N C L U D E S ___________________________________________________

#include "config.h"
#include "conf_usb.h"
#include "spi_drv.h"
#include "async.h"
#include "rs232_task.h"
#include "programmer_task.h"
#include "lib_mcu/usb/usb_drv.h"
#include "usb_descriptors.h"
#include "usb_standard_request.h"
#include "usb_specific_request.h"
#include "usb_ge_flash.h"

#include <util/delay.h>
#include <util/crc16.h>

//_____ D E C L A R A T I O N S ____________________________________________

void usb_eprom_cbw(void);
void usb_eprom_csw(void);
Bool decode_command();

static int x2444_command(uint8_t cmd);
static uint8_t reverse8(uint8_t a);

#define x2444_store()               x2444_command(0x81)
#define x2444_recall()              x2444_command(0x85)
#define x2444_enable_writelatch()   x2444_command(0x84)
#define x2444_disable_writelatch()  x2444_command(0x80)

/* Global Data */

uint8_t     g_buf[BANKSIZE];    /* EPROM data cache buffer */
uint8_t     g_dir;
uint8_t     g_status;
uint8_t     g_data;
uint8_t     g_command;
uint8_t     g_devtype = GEFU_EPROM_TYPE_X2212;
uint16_t    g_address = 0x00;   /* Current read/write address  */

/* External Data */

extern U8 endpoint_status[MAX_EP_NB];

//! This function increments the cpt_sof counter each times
//! the USB Start Of Frame interrupt subroutine is executed (1ms)
//! Usefull to manage time delays
//!

volatile U8 cpt_sof;

void sof_action()
{
   cpt_sof++;
}

/************************************************************************
 * 
 ************************************************************************/

void programmer_task_init(void)
{
    /* Initialize I/O ports */
    io_init();

    Usb_enable_sof_interrupt();
}

/************************************************************************
 * 
 ************************************************************************/

//static U16 g_wEpromBytes = 0;

void programmer_task(void)
{
    static unsigned int cnt;

    if (++cnt >= 60000)
    {
        cnt = 0;
        PORTE ^= _BV(B_LED);
    }

    if (Is_device_enumerated())
    {
        Usb_select_endpoint(EP_MS_OUT);

        if (Is_usb_receive_out())
        {
            usb_eprom_cbw();
            usb_eprom_csw();
        }
    }
}

/************************************************************************
 * USB Command Block Wrapper (CBW) management
 ************************************************************************/

void usb_eprom_cbw(void)
{
    Usb_select_endpoint(EP_MS_OUT);

    /* Read the eprom type ID and command */
    g_command = Usb_read_byte();
    g_status  = Usb_read_byte();
    g_dir     = Usb_read_byte();
    g_data    = Usb_read_byte();

    // Read the 16-bit address
    LSB(g_address) = Usb_read_byte();
    MSB(g_address) = Usb_read_byte();

    /* Set the direction flag */
    if (g_dir != 0x00)
        Usb_set_ms_data_direction_in();
    else
        Usb_set_ms_data_direction_out();

    Usb_ack_receive_out();

    if (Is_usb_ms_data_direction_in())
    {
        Usb_select_endpoint(EP_MS_IN);
    }

    if (decode_command() != TRUE)
    {
        U8 ep;
        Usb_enable_stall_handshake();
        if (Is_usb_ms_data_direction_in())
            ep = (EP_MS_IN & MSK_EP_DIR);
        else
            ep = (EP_MS_OUT & MSK_EP_DIR);
        endpoint_status[ep] = 0x01;
    }
}

/************************************************************************
 * USB Command Status Wrapper (CSW) management
 ************************************************************************/

void usb_eprom_csw(void)
{
    Usb_select_endpoint(EP_MS_IN);
    while (Is_usb_endpoint_stall_requested())
    {
        Usb_select_endpoint(EP_CONTROL);
        if (Is_usb_receive_setup())
        { usb_process_request(); }
        Usb_select_endpoint(EP_MS_IN);
    }

    Usb_select_endpoint(EP_MS_OUT);
    while (Is_usb_endpoint_stall_requested())
    {
        Usb_select_endpoint(EP_CONTROL);
        if (Is_usb_receive_setup())
        { usb_process_request(); }
        Usb_select_endpoint(EP_MS_OUT);
    }

    Usb_select_endpoint(EP_MS_IN);
    while(!Is_usb_write_enabled())
    {
        if(!Is_usb_endpoint_enabled())
            return; // USB Reset
    }

    // Send back the response packet
    Usb_write_byte(g_command);
    Usb_write_byte(g_status);
    Usb_write_byte(g_dir);
    Usb_write_byte(g_data);
    Usb_write_byte(LSB(g_address));
    Usb_write_byte(MSB(g_address));

    Usb_send_in();
}

/************************************************************************
 * Decode the received command and process it
 ************************************************************************/

Bool decode_command()
{
    Bool result = TRUE;
    int i;

    g_status = 0x00;

    switch(g_command)
    {
        // Return the current EPROM type
        case GEFU_CMD_GET_EPROM_TYPE:
            g_data = g_devtype;
            break;

        // Set the EPROM type X2212 or X2444
        case GEFU_CMD_SET_EPROM_TYPE:
            g_devtype = g_data;
            break;

        // Store EPROM data from memory buffer to EPROM
        case GEFU_CMD_STORE:
            PORTE &= ~(_BV(B_LED));
            if (g_devtype == GEFU_EPROM_TYPE_X2444)
                x2444_write(g_buf);
            else
                x2212_write(g_buf);
            break;

        // Recall EPROM data to in memory buffer
        case GEFU_CMD_RECALL:
            PORTE &= ~(_BV(B_LED));
            if (g_devtype == GEFU_EPROM_TYPE_X2444)
                x2444_read(g_buf);
            else
                x2212_read(g_buf);
            break;

        // Read data at address
        case GEFU_CMD_READ_BYTE:
            if (g_address >= BANKSIZE)
            {
                g_status = 0x80;
                break;
            }
            g_data = g_buf[g_address];
            break;

        // Write data at address
        case GEFU_CMD_WRITE_BYTE:
            if (g_address >= BANKSIZE)
            {
                g_status = 0x80;
                break;
            }
            g_buf[g_address] = g_data;
            break;

        // Reset the GE-Flash USB programmer
        case GEFU_CMD_RESET:
            for (i=0; i<BANKSIZE; i++)
                g_buf[i] = 0;
            break;

        default:
            result = FALSE;
            break;
    }

    return result;
}

/************************************************************************
 * X2212 256-Byte Block Read
 ************************************************************************/

int x2212_read(uint8_t buf[256])
{
    uint8_t c;
    uint16_t addr;

    /* Configure PORT-C for CS#, WE#, RECALL#, STORE# usage
     * and set EPWR to enable power to the eprom.
     */

    /* All pins to input mode */
    DDRC  = 0;
    /* Setup output pins that will go high when DDR is set */
    PORTC = _BV(B_CS) | _BV(B_WE) | _BV(B_RECALL) | _BV(B_STORE);

    /* Power up the X2212 chip */
#if (HARDWARE_REV >= 2)
    PORTE |= _BV(B_EPWR);
#else
    PORTE &= ~(_BV(B_EPWR));
#endif
    _NOP();
	
    /* PC4-PC7=output,PC0-PC3 are input */
    DDRC  = _BV(B_CS) | _BV(B_WE) | _BV(B_RECALL) | _BV(B_STORE);
    delay_ms(10);

    /* Configure PORT-A for all outputs as these
     * drive the address lines to the X2212.
     */

    /* All address pins low */
    PORTA = 0x00;
    /* All port A pins are output */
    DDRA  = 0xFF;
    delay_ms(10);

    /* RECALL all data from the eprom into
     * the X2212 static RAM page.
     */

    PORTC &= ~(_BV(B_RECALL));  /* RECALL# low (active) */
    delay_ms(10);
    PORTC |= _BV(B_RECALL);     /* RECALL# high (inactive) */
    delay_ms(10);

    /* Set CS# active prior to read operations */
    PORTC &= ~(_BV(B_CS));
    delay_ms(10);

    /* Loop incrementing the address and reading the data
     * at each sram memory location for all 256 byte nibbles.
     */

    for (addr=0; addr < 256; addr++)
    {
        /* Set the read address */
        PORTA = (uint8_t)addr;
        delay_ms(1);
        /* Read the 4-bit data from the lower nibble */
        c = PINC; 
        buf[addr] = c & 0x0F;
    }

    /* Read complete, set CS#, WE#, RECALL# and STORE# to inactive (high) */
    PORTC = _BV(B_CS) | _BV(B_WE) | _BV(B_RECALL) | _BV(B_STORE);
    delay_ms(10);
    
    /* lower the address lines */
    PORTA = 0;

    /* Power down the X2212 chip */
#if (HARDWARE_REV >= 2)
    PORTE &= ~(_BV(B_EPWR));
#else
    PORTE |= _BV(B_EPWR);
#endif

    /* Set ports A and C to input(tri-state) mode */
    DDRA  = 0x00;
    DDRC  = 0x00;
    return 1;
}

/************************************************************************
 * X2212 256-Byte Block Write
 ************************************************************************/

int x2212_write(uint8_t buf[256])
{
    uint8_t  c;
    uint16_t addr;

    /* Configure PORT-C for CS#, WE#, RECALL#, STORE# usage
     * and set EPWR to enable power to the eprom.
     */

    /* All pins to input mode */
    DDRC  = 0;
    /* Setup output pins that will go high when DDR is set */
    PORTC = _BV(B_CS) | _BV(B_WE) | _BV(B_RECALL) | _BV(B_STORE);
    
    /* Power up the X2212 chip */
#if (HARDWARE_REV >= 2)
    PORTE |= _BV(B_EPWR);
#else
    PORTE &= ~(_BV(B_EPWR));
#endif
    _NOP();
	
    /* Set PC0-PC7 to all outputs */
    DDRC  = 0xFF;
    delay_ms(10);

    /* Configure PORT-A for all outputs as these
     * drive the address lines to the X2212.
     */

    /* All address pins low */
    PORTA = 0x00;
    /* All port A pins are output */
    DDRA  = 0xFF;
    delay_ms(10);

    /* Loop incrementing the address and writing the data
     * at each sram memory location for all 256 byte nibbles.
     */

    /* Set CS# & WE# active prior to write operations */
    PORTC &= ~(_BV(B_CS));
    delay_ms(1);

    for (addr=0; addr < 256; addr++)
    {
        /* Set the write address */
        PORTA = (uint8_t)addr;
        delay_ms(1);
        /* RECALL/STORE=HIGH, CS/WE=LOW, low nibble contains write data */
        c = (_BV(B_RECALL)|_BV(B_STORE)) | (buf[addr] & 0x0F);
        PORTC = c;
        delay_ms(1);
        /* WE back to inactive */
        PORTC |= _BV(B_WE);
        _NOP();
    }

    /* STORE all data to the eprom in the X2212 
     * from the internal static RAM page.
     */
    PORTC &= ~(_BV(B_STORE));   /* STORE# low (active) */
    delay_ms(250);              /* allow store to flash time to complete */

    /* Write complete, set CS#, WE#, RECALL# and STORE# to inactive (high) */
    PORTC |= _BV(B_CS) | _BV(B_WE) | _BV(B_RECALL) | _BV(B_STORE);
    delay_ms(10);
    
    /* Lower the address lines */
    PORTA = 0;
	
    /* Power down the X2212 chip */
    #if (HARDWARE_REV >= 2)
    PORTE &= ~(_BV(B_EPWR));
    #else
    PORTE |= _BV(B_EPWR);
    #endif

    /* Set ports A and C to input(tri-state) mode */
    DDRA  = 0x00;
    DDRC  = 0x00;

    return 1;
}

/************************************************************************
 * X2444 32-Byte Block Read
 ************************************************************************/

int x2444_read(uint8_t buf[32])
{
    uint8_t addr;
    uint8_t cmd;
    uint8_t lsb;
    uint8_t msb;

    /* Power up the X2444 chip */
#if (HARDWARE_REV >= 2)
    PORTE |= _BV(B_EPWR);
#else
    PORTE &= ~(_BV(B_EPWR));
#endif
    delay_ms(25);

    /* Issue a recall(RCL) command */
    x2444_recall();

    /* Flush out the rx buffer */
    Spi_read_data();

    uint16_t* p = (uint16_t*)&buf[0];

    /* Now read each data byte */
    for (addr=0; addr < 32/2; addr++)
    {
        /* Assert the slave select pin */
        Spi_enable_ss();
        /* Build the command and address byte */
        cmd = ((addr << 3) & 0x78) | 0x86;
        Spi_write_data(cmd);
        Spi_write_data(0);
        lsb = reverse8(Spi_read_data());
        Spi_write_data(0);
        msb = reverse8(Spi_read_data());
        /* Store the 16-bit value returned */ 
        *p++ = ((uint16_t)msb << 8) | (uint16_t)lsb;
        /* Deassert the slave select pin */
        Spi_disable_ss();
    }

    delay_ms(25);

    /* Power down the X2444 chip */
#if (HARDWARE_REV >= 2)
    PORTE &= ~(_BV(B_EPWR));
#else
    PORTE |= _BV(B_EPWR);
#endif

    return 1;
}

/************************************************************************
 * X2444 32-Byte Block Write
 ************************************************************************/

int x2444_write(uint8_t buf[32])
{
    uint8_t addr;
    uint8_t cmd;

    /* Power up the X2444 chip */
#if (HARDWARE_REV >= 2)
    PORTE |= _BV(B_EPWR);
#else
    PORTE &= ~(_BV(B_EPWR));
#endif
    delay_ms(25);

    /* A recall must be performed prior to STO command */
    x2444_recall();

    /* Enable WREN write latch enable */
    x2444_enable_writelatch();

    uint16_t* p = (uint16_t*)&buf[0];

    /* Now write all the data bytes */
    for (addr=0; addr < 32/2; addr++)
    {
        /* Assert the slave select pin */
        Spi_enable_ss();
        /* Build the command and address byte */
        cmd = ((addr << 3) & 0x78) | 0x83;
        Spi_write_data(cmd);
        Spi_write_data(reverse8(LSB(*p)));
        Spi_write_data(reverse8(MSB(*p)));
        ++p;
        /* Deassert the slave select pin */
        Spi_disable_ss();
    }

    /* Issue a STO command to store the data */
    x2444_store();

    /* Disable WREN write latch enable */
    x2444_disable_writelatch();
    delay_ms(25);

    /* Power down the X2444 chip */
#if (HARDWARE_REV >= 2)
    PORTE &= ~(_BV(B_EPWR));
#else
    PORTE |= _BV(B_EPWR);
#endif

    return 1;
}

/* send a single command to the X2444 */
int x2444_command(uint8_t cmd)
{
    /* Assert the slave select pin */
    Spi_enable_ss();

    /* Send the command */
    Spi_write_data(cmd);
    delay_ms(25);

    /* Deassert the slave select pin */
    Spi_disable_ss();
    
    return 1;
}

/* reverse the bit order of a byte */
uint8_t reverse8(uint8_t a)
{
    int i;
    uint8_t b = 0;

    for ( i = 0 ; i < 8 ; i ++)
    {
        b <<= 1;
        b |=  ( (a & (1 << i)) >> i);          
    }
    return b; 
}

/************************************************************************
 * 
 ************************************************************************/

void io_init(void)
{
    /* PORT-A
     *
     * PA7  = AD7       (out)
     * PA6  = AD6       (out)
     * PA5  = AD5       (out)
     * PA4  = AD4       (out)
     * PA3  = AD3       (out)
     * PA2  = AD2       (out)
     * PA1  = AD1       (out)
     * PAO  = AD0       (out)
     */

    /* All pins to input */
    DDRA = 0;
    /* all pins low, no pullups */
    PORTA = 0x00;
    /* all pins are output */
    DDRA  = 0xFF;

    /* PORT-B
     *
     * PB7  = CFG2      (in:pullups enabled)
     * PB6  = CFG1      (in:pullups enabled)
     * PB5  = N/C       (in)
     * PB4  = N/C       (in)
     * PB3  = MISO      (in)
     * PB2  = MOSI      (out)
     * PB1  = SCLK      (out)
     * PBO  = SS        (out)
     */

    /* All pins to input */
    DDRB = 0;
    /* all pins low, pullups on PB6 & PB7 */
    PORTB = _BV(PB6) | _BV(PB7);
    _NOP();
    /* SS,SCLK,MOSI=output, all others are input */
    DDRB  = _BV(PB0) | _BV(PB1) | _BV(PB2);
    _NOP();

    /* Configure the SPI port for X2444 communication */
    Spi_select_master();
    Spi_set_msbfirst();
    //Spi_set_lsbfirst();
    Spi_set_mode(SPI_MODE_0);
    //Spi_init_bus();
    Spi_set_rate(SPI_RATE_6);
    Spi_disable_ss();
    Spi_enable();

    Spi_write_data(0x85);
    delay_ms(10);

    /* PORT-C
     *
     * PC7  = CS#       (out)
     * PC6  = WE#       (out)
     * PC5  = RECALL#   (out)
     * PC4  = STORE#    (out)
     * PC3  = DB3       (i/o)
     * PC2  = DB2       (i/o)
     * PC1  = DB1       (i/o)
     * PCO  = DB0       (i/o)
     */

    /* All pins to input */
    DDRC = 0;
    /* all pins low, no pullups */
    PORTC = 0x00;
    /* PC4-PC7=output, all others are input */
    DDRC  = _BV(PC4) | _BV(PC5) | _BV(PC6) | _BV(PC7);

    /* PORT-D
     *
     * PD7  = N/C       (in)
     * PD6  = N/C       (in)
     * PD5  = N/C       (in)
     * PD4  = N/C       (in)
     * PD3  = TXD       (out)
     * PD2  = RXD       (in)
     * PD1  = CTS       (in)
     * PDO  = RTS       (out)
     */

    /* All pins to input */
    DDRD = 0;
    /* all pins low, no pullups */
    PORTD = 0x00;
    /* PD0,PE6,PE7=output, all others are input */
    DDRD  = _BV(PD0) | _BV(PD3);

    /* PORT-E
     *
     * PE7  = UVCON     (out)
     * PE6  = EPWR      (out)
     * PE5  = N/C       (in)
     * PE4  = PBSW      (in)
     * PE3  = UID       (in)
     * PE2  = HWBOOT    (in)
     * PE1  = N/C       (in)
     * PEO  = LED       (out)
     */

    /* All pins to input */
    DDRE = 0;
    /* all pins low, no pullups */
    PORTE = 0x00;
    /* PE0,PE6,PE7=output, all others are input */
    DDRE  = _BV(PE0) | _BV(PE6) | _BV(PE7);
    _NOP();
	/* Make sure the EPROM power is OFF */	
#if (HARDWARE_REV >= 2)
    PORTE = 0;			/* [Rev-B]:Set EPWR low for EPROM power off! */
#else
    PORTE = _BV(PE6);   /* [Rev-A]:Set EPWR high for EPROM power off! */
#endif
    /* Read the configuration jumpers */
    //g_jumpers = PINB & (_BV(B_CFG1) | _BV(B_CFG2));
}


void delay_ms(uint16_t ms)
{
    uint16_t d;

    for (d=0; d < ms; d++) {
        /* 16-bit count - 4 cycles/loop */
        _delay_loop_2((uint16_t)(F_CPU / 4000UL));
    }
}

/* Blink the LED for count and rate duration */

void blink_led(uint16_t count, uint16_t rate)
{
    uint16_t i;

    /* Blink the n times at ms delay rate */

    for (i=0; i < count; i++)
    {
        PORTE |= _BV(B_LED);
        delay_ms(rate);

        PORTE &= ~(_BV(B_LED));
        delay_ms(rate);
    }
}

// End-Of-File
