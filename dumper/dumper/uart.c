/*
 * uart.c
 *
 * Created: 06.03.2022 20:53:19
 *  Author: AGATHA
 */ 

#include "main.h"

void uart_init( void )
{
/*	//настройка скорости обмена
	UBRRH = 0;
	UBRRL = F_CPU / ( BAUD_RATE * 16 ) - 1;		// F=8 MHz controller, B=9600 port speed [ == F / ( B * 16 ) - 1 ]
	//8 бит данных, 1 стоп бит, без контроля четности
	UCSRC = ( 1 << URSEL ) | ( 1 << UCSZ1 ) | ( 1 << UCSZ0 );
	//разрешить прием и передачу данных
	UCSRB = ( 1 << TXEN ) | ( 1 <<RXEN );
*/
	UCSRB = (1 << RXEN) | (1 << TXEN);	/* Turn on transmission and reception */
	UCSRC = (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1);/* Use 8-bit char size */
	UBRRL = BAUD_PRESCALE;			/* Load lower 8-bits of the baud rate */
	UBRRH = (BAUD_PRESCALE >> 8);		/* Load upper 8-bits*/
}

void uart_putc( char c )
{
	//ждем окончания передачи предыдущего байта
	while( ( UCSRA & ( 1 << UDRE ) ) == 0  );
	//передача данных
	UDR = c;
}

unsigned int isuart_getc( void )
{
	if ( ( UCSRA & ( 1 << RXC ) ) != 0  )
	{
		return UDR;
	}
	else
	{
		return -1;
	}
}

unsigned char uart_getc( void )
{
	//ждем приема байта
	while( ( UCSRA & ( 1 << RXC ) ) == 0  );
	
	//считываем принятый байт
	uint8_t result = UDR; /*, tosend = result;
	
	CFG2_REG &= ~QUARTZ_ENABLE_BIT;
	asm( "nop"); ( tosend & 0b00000001 ) ? ( CFG2_REG |= QUARTZ_ENABLE_BIT ) : ( CFG2_REG &= ~QUARTZ_ENABLE_BIT );
	asm( "nop"); ( tosend & 0b00000010 ) ? ( CFG2_REG |= QUARTZ_ENABLE_BIT ) : ( CFG2_REG &= ~QUARTZ_ENABLE_BIT );
	asm( "nop"); ( tosend & 0b00000100 ) ? ( CFG2_REG |= QUARTZ_ENABLE_BIT ) : ( CFG2_REG &= ~QUARTZ_ENABLE_BIT );
	asm( "nop"); ( tosend & 0b00001000 ) ? ( CFG2_REG |= QUARTZ_ENABLE_BIT ) : ( CFG2_REG &= ~QUARTZ_ENABLE_BIT );
	asm( "nop"); ( tosend & 0b00010000 ) ? ( CFG2_REG |= QUARTZ_ENABLE_BIT ) : ( CFG2_REG &= ~QUARTZ_ENABLE_BIT );
	asm( "nop"); ( tosend & 0b00100000 ) ? ( CFG2_REG |= QUARTZ_ENABLE_BIT ) : ( CFG2_REG &= ~QUARTZ_ENABLE_BIT );
	asm( "nop"); ( tosend & 0b01000000 ) ? ( CFG2_REG |= QUARTZ_ENABLE_BIT ) : ( CFG2_REG &= ~QUARTZ_ENABLE_BIT );
	asm( "nop"); ( tosend & 0b10000000 ) ? ( CFG2_REG |= QUARTZ_ENABLE_BIT ) : ( CFG2_REG &= ~QUARTZ_ENABLE_BIT );
	asm( "nop"); CFG2_REG |= QUARTZ_ENABLE_BIT;
	*/
	
	return result;
}