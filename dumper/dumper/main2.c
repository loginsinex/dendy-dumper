/*
 * dumper.c
 *
 * Created: 04.06.2021 17:35:55
 * Author : AGATHA
 */ 

// fuses: high=0xd9; low=0xc4

/*
	A0 - CPU A0 / PPU A0
	A1 - CPU A1 / PPU A1
	A2 - CPU A2 / PPU A2
	A3 - CPU A3 / PPU A3
	A4 - CPU A4 / PPU A4
	A5 - CPU A5 / PPU A5
	A6 - CPU A6 / PPU A6
	A7 - CPU A7 / PPU A7

	B0 - (XCK)
	B1 - CPU D0 / PPU D0
	B2 - CPU D1 / PPU D1
	B3 - CPU D2 / PPU D2
	B4 - CPU D3 / PPU D3
	B5 - CPU D4 / PPU D4
	B6 - CPU D5 / PPU D5
	B7 - CPU D6 / PPU D6
	
	C0 - CPU A8 / PPU A8
	C1 - CPU A9 / PPU A9
	C2 - CPU A10 / PPU A10
	C3 - CPU A11 / PPU A11
	C4 - CPU A12 / PPU A12
	C5 - CPU A13 / PPU A13
	C6 - CPU A14
	C7 - PPU /A13
	
	D0 - (RXD)
	D1 - (TXD)
	D2 - M2
	D3 - /ROMSEL
	D4 - CPU R/W
	D5 - PPU /WR
	D6 - PPU /RD
	D7 - CPU D7 / PPU D7
*/


#define F_CPU 8000000UL
#define BAUD_RATE 19200UL
#define AVR_ATmega32
#define BAUD_PRESCALE ( F_CPU / ( BAUD_RATE * 16 ) - 1 )

#define MODE_READ		{ DDRB &= ~0b11111110; DDRD &= ~0b10000000; PORTB |= 0b11111110; PORTD |= 0b10000000; }
#define MODE_WRITE		{ DDRB |= 0b11111110; DDRD |= 0b10000000; }
#define M2_LOW			( PORTD &= ~( 0x04 ) )
#define M2_HI			( PORTD |= ( 0x04 ) )
#define ROMSEL_LOW		( PORTD &= ~( 0x08 ) )
#define ROMSEL_HI		( PORTD |= ( 0x08 ) )
#define PRG_READ		( PORTD |= 0x10 )
#define PRG_WRITE		( PORTD &= ~0x10 )
#define CHR_READ_ON		( PORTD &= ~0x20 )
#define CHR_READ_OFF	( PORTD |= 0x20 )
#define CHR_WRITE_ON	( PORTD &= ~0x40 )
#define CHR_WRITE_OFF	( PORTD |= 0x40 )

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#define false				0
#define true				1

typedef char bool;
typedef unsigned short USHORT;
typedef unsigned long ULONG;
typedef unsigned char BYTE;

bool g_CHRMode = false;

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
	UCSRB |= (1 << RXEN) | (1 << TXEN);	/* Turn on transmission and reception */
	UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1);/* Use 8-bit char size */
	UBRRL = BAUD_PRESCALE;			/* Load lower 8-bits of the baud rate */
	UBRRH = (BAUD_PRESCALE >> 8);		/* Load upper 8-bits*/


}

ISR (TIMER0_OVF_vect)
{
	//unsigned char ddrd = DDRC, pind = PINC;
	
	TCNT0=0x00;										// Clear timer
}

void InitializeTimer0()
{
	TIMSK |= ( 1 << TOIE0 ); // set interrupt on overflow of 0 (from 3 timers) timer. Bit TOIE0
	//	TCCR0 = (0<<CS02) | (0<<CS01) | (0<<CS00);	// timer stopped
	//	TCCR0 = (0<<CS02) | (0<<CS01) | (1<<CS00);	// no prescaller
	//  TCCR0 = (0<<CS02) | (1<<CS01) | (0<<CS00);	// clock / 8
	//	TCCR0 = (0<<CS02) | (1<<CS01) | (1<<CS00);	// clock / 64
	TCCR0 = (1<<CS02) | (0<<CS01) | (0<<CS00);	// clock / 256
	//	TCCR0 = (1<<CS02) | (0<<CS01) | (1<<CS00);	// clock / 1024
	// follow values only for external clock
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
		return UDR;
	else
		return -1;
}

unsigned char uart_getc( void )
{
	//ждем приема байта
	while( ( UCSRA & ( 1 << RXC ) ) == 0  );
	
	//считываем принятый байт
	return UDR;
}

void printstr(char * s)
{
	int c = 0;
	while(s[c])
	uart_putc(s[c++]);
}
static void set_address(uint16_t address)
{
	unsigned char l = address & 0xFF;
	unsigned char h = (address >> 8) & 0x7F;
	
	PORTC = h;
	PORTA = l;
}

static inline void set_romsel(uint16_t address)
{
	if (address & 0x8000)
	{
		ROMSEL_LOW;
	} 
	else 
	{
		ROMSEL_HI;
	}
}

void PrepareRead(USHORT address)
{
	MODE_READ;
	PRG_READ;
	set_romsel(address); // set /ROMSEL low if need
}

BYTE ReadPrgAddress(USHORT address)
{
	M2_LOW;
	set_address(address);
	M2_HI;
	_delay_us(5);
	BYTE B = PINB;
	BYTE D = PIND;
	uint8_t result = (( B >> 1 ) | ( D & 0x80 ));
	return result;
}

bool WritePrgAddress(USHORT address, BYTE value)
{
	M2_LOW;
	ROMSEL_HI;
	MODE_WRITE;
	PRG_WRITE;
	PORTD |= ( value & 0x80 );
	PORTB |= ( ( value & 0x7f ) << 1 );
	set_address(address); // PHI2 low, ROMSEL always HIGH
	_delay_us(5);
  
	M2_HI;
	set_romsel(address); // ROMSEL is low if need, PHI2 high
  
	_delay_us(1); // WRITING
  
	// PHI2 low, ROMSEL high
	M2_LOW;
	ROMSEL_HI;
	return true;
}



bool ReadInput(char * sinput, unsigned char slen_max)
{
	unsigned char slen = 0;
	char chr = 0;
	sinput[ slen_max - 1 ] = 0;
	
	while( slen + 1 < slen_max )
	{
		chr = uart_getc();
		if ( ( chr >= '0' && chr <= '9' ) || ( chr >= 'A' && chr <= 'Z' ) || ( chr >= 'a' && chr <= 'z' ) )
		{
			if ( chr >= 'A' && chr <= 'Z' ) chr += 0x20;
			sinput[ slen++ ] = chr;
			uart_putc( chr );
		}
		else if ( chr == '\r' || chr == '\n' )
		{
			sinput[ slen ] = 0;
			printstr("\r\n");
			break;
		}
	}
	return true;
}

bool ReadInputShort( char * prompt, USHORT * pResult )
{
	USHORT result = 0;
	const unsigned char length = 5;
	char input[length];
	printstr(prompt);
	if ( !ReadInput(input, length) ) return false;
	
	char * p = input;
	while(*p)
	{
		if ( (*p) >= '0' && (*p) <= '9' )
		{
			result = 0x10 * result + ((*p) - '0');
		}
		else if ( (*p) >= 'A' && (*p) <= 'F' )
		{
			result = 0x10 * result + ((*p) - 'A' + 10);
		}
		else if ( (*p) >= 'a' && (*p) <= 'f' )
		{
			result = 0x10 * result + ((*p) - 'a' + 10);
		}
		else
		{
			return false;
		}
		p++;
	}
	
	*pResult = result;
	return true;
}

bool ReadInputByte( BYTE * pResult )
{
	USHORT result = 0;
	const unsigned char length = 3;
	char input[length];
	printstr("Enter byte as hex [XX]: ");
	if ( !ReadInput(input, length) ) return false;
	
	char * p = input;
	while(*p)
	{
		if ( (*p) >= '0' && (*p) <= '9' )
		{
			result = 0x10 * result + ((*p) - '0');
		}
		else if ( (*p) >= 'A' && (*p) <= 'F' )
		{
			result = 0x10 * result + ((*p) - 'A' + 10);
		}
		else if ( (*p) >= 'a' && (*p) <= 'f' )
		{
			result = 0x10 * result + ((*p) - 'a' + 10);
		}
		else
		{
			return false;
		}
		p++;
	}
	
	*pResult = result;
	
	return true;	
}

void User_ReadByte()
{
	USHORT address = 0;
	if ( ReadInputShort("Enter address to read as hex [XXXX]: ", &address) )
	{
		PrepareRead( address );
		BYTE ret = ReadPrgAddress( address );
		char output[32];
		sprintf(output, "\r\n* [$%04X] = #$%02X\r\n", address, ret);
		printstr(output);
	}
	else
	{
		printstr("\r\nERROR: Invalid address format");
	}
}

void User_WriteByte()
{
	USHORT address = 0;
	BYTE value = 0;
	if ( ReadInputShort("Enter address to write as hex [XXXX]: ", &address) )
	{
		printstr("\r\n");
		if ( ReadInputByte( &value ) )
		{
			char output[32];
			printstr("\r\n");
			sprintf(output, "* [$%04X] <- #$%02X\r\n: ", address, value );
			printstr(output);
			if ( WritePrgAddress( address, value ) )
			{
				printstr("SUCCESS");
			}
			else
			{
				printstr("FAILED");
			}			
		}
	}
	else
	{
		printstr("\r\nERROR: Invalid address format");
	}	
}

void PrintArr( USHORT address, BYTE * pArray, BYTE length )
{
	char output[70];
	char * p = &output[0];
	output[0] = 0;
	
	sprintf( output, "%04X: ", address );
	printstr( output );
	
	for(BYTE i = 0; i < length; ++i)
	{
		sprintf(p, "%02X", *pArray);
		p += 2;
		pArray++;
		if ( i > 0 && ! ( i % 32 ) )
		{
			printstr(output);
			output[0] = 0;
			p = &output[0];
			printstr("\r\n");
		}
	}

	if ( output[0] ) printstr(output);
	printstr("\r\n");
}

void User_ReadRange()
{
	USHORT ufrom = 0, uto = 0;
	if ( ReadInputShort("Enter begin address to read as hex [XXXX]: ", &ufrom ) )
	{
		printstr("\r\n");
		if ( ReadInputShort("Enter end address as hex [XXXX]: ", &uto ) )
		{
			printstr("\r\n");
			if ( uto <= ufrom )
			{
				printstr("ERROR: Invalid range");
			}
			else
			{
				PrepareRead( ufrom );
				const BYTE read_len=0x20;
				BYTE input[2*read_len], local_len = 0;
				char stroutput[32];
				ULONG lfrom=ufrom,lto=uto;
				sprintf(stroutput, "* Reading range $%04X-$%04X...\r\n", ufrom, uto);
				printstr(stroutput);
				for( ULONG addr = lfrom; addr <= lto; addr++ )
				{
					USHORT address = (USHORT) (addr & 0xFFFF);
					input[local_len++] = ReadPrgAddress( address );
					if ( local_len >= read_len )
					{
						PrintArr( address - local_len + 1, input, read_len );
						local_len = 0;
						_delay_ms(1);
					}
				}
				
				if ( local_len > 0 ) PrintArr(lto - local_len + 1, input, local_len);
			}
		}
		else
		{
			printstr("\r\nERROR: Invalid address");
		}
	}
	else
	{
		printstr("\r\nERROR: Invalid address");
	}

	printstr("\r\nDone.");
}

void init_ports()
{
	DDRA = 0xFF; // Address low
	DDRC = 0xFF; // Address high
	DDRD = 0b01111100;
	PORTD = 0b01111100;
}

bool SetupPRGMode()
{
	CHR_READ_OFF;
	CHR_WRITE_OFF;
	g_CHRMode = false;
	return true;
}

bool SetupCHRMode()
{
	CHR_READ_ON;
	g_CHRMode = true;
	return true;
}

int main(void)
{
    /* Replace with your application code */

	//InitializeTimer0();
	uart_init();
	init_ports();
	const unsigned char length = 16;
	char input[length];
	bool fSelectMode = true;
	printstr("\r\nDumper started!\r\n");
	
    while (1) 
    {
		if ( fSelectMode )
		{
			bool fIsModeSet = false;
			printstr("Command:\r\n\t0 - PRG read mode\r\n\t1 - CHR read mode\r\n");
			printstr("Enter command: ");
			ReadInput(input, 2);
			printstr("\r\n");
			switch(input[0])
			{
				case '0': fIsModeSet = SetupPRGMode(); break;
				case '1': fIsModeSet = SetupCHRMode(); break;
				default: printstr("ERROR: Invalid command");
			}
			
			printstr("\r\n");
			if ( fIsModeSet )
			{
				fSelectMode = false;
			}
		}
		else
		{
			printstr("Command:\r\n\t0 - read byte\r\n\t1 - write byte\r\n\t2 - read range\r\n\t3 - leave\r\n");
			printstr("Enter command: ");
			ReadInput(input, 2);
			printstr("\r\n");
			switch(input[0])
			{
				case '0': User_ReadByte(); break;
				case '1': User_WriteByte(); break;
				case '2': User_ReadRange(); break;
				case '3': fSelectMode = true; break;
				default: printstr("ERROR: Invalid command");
			}
		
			printstr("\r\n");			
		}
    }
}

