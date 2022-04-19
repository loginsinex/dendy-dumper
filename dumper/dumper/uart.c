/*
 * uart.c
 *
 * Created: 06.03.2022 20:53:19
 *  Author: AGATHA
 */ 

#include "main.h"

void uart_init( void )
{
	UCSRB = (1 << RXEN) | (1 << TXEN);	/* Turn on transmission and reception */
	UCSRC = (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1);/* Use 8-bit char size */
	UBRRL = BAUD_PRESCALE;			/* Load lower 8-bits of the baud rate */
	UBRRH = (BAUD_PRESCALE >> 8);		/* Load upper 8-bits*/
}

void uart_putc( char c )
{
	while( ( UCSRA & ( 1 << UDRE ) ) == 0  );
	UDR = c;
}

unsigned char uart_getc( void )
{
	while( ( UCSRA & ( 1 << RXC ) ) == 0  );	
	return UDR;
}
