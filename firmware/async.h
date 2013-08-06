/****************************************************************************
 *
 ***************************************************************************/

/* Useful ASCII Codes */

#define SOH     0x01
#define STX     0x02
#define ENQ     0x05
#define ACK     0x06
#define NAK     0x15
#define SYN     0x16
#define ESC     0x1B

/*
 * USARThronous USART Structure
 */

//#define BAUD_RATE           9600UL

#define USART_TX_BUFSIZE    16
#define USART_RX_BUFSIZE    64

typedef struct _USART {
    struct  _rx {
        uint8_t buf[USART_RX_BUFSIZE];
        size_t  head;       /* rx queue head index */
        size_t  tail;       /* rx queue tail index */
        size_t  count;      /* rx queue size       */
    } rx;
    struct _tx {
        uint8_t buf[USART_TX_BUFSIZE];
        size_t  head;       /* tx queue head index */
        size_t  tail;       /* tx queue tail index */
        size_t  count;      /* tx queue size       */
    } tx;
    struct _stat {
        uint8_t cntl;       /* interrupt cntl bits */
        uint8_t flags;      /* USART status bits   */
        uint8_t line;       /* line status bits    */
        uint8_t modem;      /* modem status bits   */
    } stat;
} USART;

/* interrupt status flags for 'stat.cntl' */
#define C_TXING             0x01        /* tx ISR in progress       */
#define C_RXING             0x02        /* rx ISR in progress       */

/* USART buffer status flags for 'stat.flags */
#define A_RX_UNDERFLOW      0x01        /* receive buffer empty     */
#define A_RX_OVERFLOW       0x02        /* receive buffer overrun   */
#define A_TX_UNDERFLOW      0x04        /* transmit buffer empty    */
#define A_TX_OVERFLOW       0x08        /* transmit buffer overrun  */

/* USART1 line status flags for 'stat.line */
#define L_FRAMING_ERROR     (1<<FE1)    /* framing error occured    */
#if defined(__AVR_ATmega8__)
#define L_PARITY_ERROR      (1<<PE1)    /* parity error occured     */
#else
#define L_PARITY_ERROR      (1<<UPE1)   /* parity error occured     */
#endif
#define L_DATA_OVERRUN      (1<<DOR1)	/* overrun error occured    */

/* modem status bits and macros */
#define M_DELTA_CTS         0x01        /* change in clear to send  */
#define M_DELTA_DSR         0x02        /* change in data set ready */
#define M_RING_TRAIL        0x04        /* trailing edge ring ind.  */
#define M_DELTA_CD          0x08        /* change in carrier detect */
#define M_CTS               0x10        /* modem clear to send      */
#define M_DSR               0x20        /* modem data set ready     */
#define M_RING              0x40        /* modem ring indicator     */
#define M_CD                0x80        /* modem carrier detect     */

/*
 * Function Prototypes
 */

void usart_init(void);

int usart_getc(void);
int usart_putc(char c);
int usart_read(char *buf, size_t buflen);
int usart_write(char *buf, size_t buflen);
int usart_tgetc(int timeout);
int usart_tgets(int timeout, char buf[]);



/* End-Of-File */
