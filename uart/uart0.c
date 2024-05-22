#include "uart0.h"
#include "../kernel/mbox.h"
// #include "uart1.h"

/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void uart0_init(int baudrate, int databits, int stopbits, int parity, int handshaking)
{
	// uart1_puts("UART0 init\n");
	// int baudrate = 115200;
	// int databits = 8;
	// int stopbits = 1;
	// int parity = 0;
	// int handshaking = 0;
	
	unsigned int r;

	/* Turn off UART0 */
	UART0_CR = 0x0;
	/* NEW: set up UART clock for consistent divisor values
	--> may not work with QEMU, but will work with real board */
	mBuf[0] = 9 * 4;
	mBuf[1] = MBOX_REQUEST;
	mBuf[2] = MBOX_TAG_SETCLKRATE; // set clock rate
	mBuf[3] = 12;				   // Value buffer size in bytes
	mBuf[4] = 0;				   // REQUEST CODE = 0
	mBuf[5] = 2;				   // clock id: UART clock
	mBuf[6] = UART0_CLK;		   // rate: 4Mhz
	mBuf[7] = 0;				   // clear turbo
	mBuf[8] = MBOX_TAG_LAST;
	mbox_call(ADDR(mBuf), MBOX_CH_PROP);

	/* Setup GPIO pins 14 and 15 */

	/* Set GPIO14 and GPIO15 to be pl011 TX/RX which is ALT0	*/
	r = GPFSEL1;
	r &= ~((7 << 12) | (7 << 15));		// clear bits 17-12 (FSEL15, FSEL14)
	r |= (0b100 << 12) | (0b100 << 15); // Set value 0b100 (select ALT0: TXD0/RXD0)
	GPFSEL1 = r;

	/* enable GPIO 14, 15 */
#ifdef RPI3	   // RBP3
	GPPUD = 0; // No pull up/down control
	// Toogle clock to flush GPIO setup
	r = 150;
	while (r--)
	{
		asm volatile("nop");
	} // waiting 150 cycles
	GPPUDCLK0 = (1 << 14) | (1 << 15); // enable clock for GPIO 14, 15
	r = 150;
	while (r--)
	{
		asm volatile("nop");
	} // waiting 150 cycles
	GPPUDCLK0 = 0; // flush GPIO setup

#else // RPI4
	r = GPIO_PUP_PDN_CNTRL_REG0;
	r &= ~((3 << 28) | (3 << 30)); // No resistor is selected for GPIO 14, 15
	GPIO_PUP_PDN_CNTRL_REG0 = r;
#endif

	/* Mask all interrupts. */
	UART0_IMSC = 0;
	/* Clear pending interrupts. */
	UART0_ICR = 0x7FF;

	/* Set integer & fractional part of Baud rate
	Divider = UART_CLOCK/(16 * Baud)
	Default UART_CLOCK = 48MHz (old firmware it was 3MHz);
	Integer part register UART0_IBRD  = integer part of Divider
	Fraction part register UART0_FBRD = (Fractional part * 64) + 0.5 */

	float div = (float)(UART0_CLK) / (16 * baudrate);

	// NEW: with UART_CLOCK = 4MHz as set by mailbox:
	UART0_IBRD = (int)div;
	UART0_FBRD = (int)((div - (float)UART0_IBRD) * 64 + 0.5);

	/* Set up the Line Control Register */
	/* Enable FIFO */
	UART0_LCRH = 0;
	UART0_LCRH |= UART0_LCRH_FEN;

	switch (databits)
	{
	case 5:
		UART0_LCRH |= UART0_LCRH_WLEN_5BIT;
		break;
	case 6:
		UART0_LCRH |= UART0_LCRH_WLEN_6BIT;
		break;
	case 7:
		UART0_LCRH |= UART0_LCRH_WLEN_7BIT;
		break;
	case 8:
		UART0_LCRH |= UART0_LCRH_WLEN_8BIT;
		break;
	default:
		UART0_LCRH |= UART0_LCRH_WLEN_8BIT;
		break;
	}

	switch (parity)
	{
	case 0:
		UART0_LCRH &= ~UART0_LCRH_PEN; // no parity
		break;
	case 1:
		UART0_LCRH |= UART0_LCRH_PEN;  // enable parity
		UART0_LCRH &= ~UART0_LCRH_EPS; // odd parity
		break;
	case 2:
		UART0_LCRH |= UART0_LCRH_PEN; // enable parity
		UART0_LCRH |= UART0_LCRH_EPS; // even parity
		break;
	default:
		UART0_LCRH &= ~UART0_LCRH_PEN; // no parity
		break;
	}

	if (stopbits == 2)
	{
		UART0_LCRH |= UART0_LCRH_STP2;
	}
	else
	{
		UART0_LCRH &= ~UART0_LCRH_STP2;
	}

	if(handshaking){
		/* Set GPIO16 and GPIO17 to be pl011 CTS/RTS which is ALT3	*/
		r = GPFSEL1;
		r &= ~((7 << 18) | (7 << 21));		// clear bits 23-18 (FSEL16, FSEL17)
		r |= (0b111 << 12) | (0b111 << 15); // Set value 0b111 (select ALT3: CTS0/RTS0)
		GPFSEL1 = r;

				/* enable GPIO 16, 17 */
	#ifdef RPI3	   // RBP3
		GPPUD = 0; // No pull up/down control
		// Toogle clock to flush GPIO setup
		r = 150;
		while (r--)
		{
			asm volatile("nop");
		} // waiting 150 cycles
		GPPUDCLK0 = (1 << 16) | (1 << 17); // enable clock for GPIO 16, 17
		r = 150;
		while (r--)
		{
			asm volatile("nop");
		} // waiting 150 cycles
		GPPUDCLK0 = 0; // flush GPIO setup

	#else // RPI4
		r = GPIO_PUP_PDN_CNTRL_REG1;
		r &= ~((3 << 00) | (3 << 02)); // No resistor is selected for GPIO 16, 17
		GPIO_PUP_PDN_CNTRL_REG1 = r;
	#endif

		UART0_CR |= UART0_CR_CTSEN;
		UART0_CR |= UART0_CR_RTSEN;
	
	}

	/* Enable UART0, receive, and transmit */
	UART0_CR = 0x301; // enable Tx, Rx, FIFO
}

/**
 * Send a character
 */
void uart0_sendc(char c)
{

	/* Check Flags Register */
	/* And wait until transmitter is not full */
	do
	{
		asm volatile("nop");
	} while (UART0_FR & UART0_FR_TXFF);

	/* Write our data byte out to the data register */
	UART0_DR = c;

	/* Wait until the busy flag is cleared (IMPORTANT TO AVOID BUG)*/
	do
	{
		asm volatile("nop");
	} while (UART0_FR & UART0_FR_BUSY);
}

int is_uart0_byte_ready()
{
	return !(UART0_FR & UART0_FR_RXFE);
}

/**
 * Receive a character
 */
char uart0_getc()
{
	char c = 0;

	/* Check Flags Register */
	/* Wait until Receiver is not empty
	 * (at least one byte data in receive fifo)*/
	do
	{
		asm volatile("nop");
	} while (UART0_FR & UART0_FR_RXFE);

	/* read it and return */
	c = (unsigned char)(UART0_DR);
	/* convert carriage return to newline */
	return (c == '\r' ? '\n' : c);
}

/**
 * Display a string
 */
void uart0_puts(char *s)
{
	while (*s)
	{
		/* convert newline to carriage return + newline */
		if (*s == '\n')
			uart0_sendc('\r');
		uart0_sendc(*s++);
	}
}

/**
 * Display a value in hexadecimal format
 */
void uart0_hex(unsigned int num)
{
	uart0_puts("0x");
	for (int pos = 28; pos >= 0; pos = pos - 4)
	{

		// Get highest 4-bit nibble
		char digit = (num >> pos) & 0xF;

		/* Convert to ASCII code */
		// 0-9 => '0'-'9', 10-15 => 'A'-'F'
		digit += (digit > 9) ? (-10 + 'A') : '0';
		uart0_sendc(digit);
	}
}

/*
**
* Display a value in decimal format
*/
void uart0_dec(int num)
{
	// A string to store the digit characters
	char str[32] = "";

	// Calculate the number of digits
	int len = 1;
	int temp = num;
	while (temp >= 10)
	{
		len++;
		temp = temp / 10;
	}

	// Store into the string and print out
	for (int i = 0; i < len; i++)
	{
		int digit = num % 10; // get last digit
		num = num / 10;		  // remove last digit from the number
		str[len - (i + 1)] = digit + '0';
	}
	str[len] = '\0';

	uart0_puts(str);
}