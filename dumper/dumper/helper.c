/*
 * helper.c
 *
 * Created: 06.03.2022 20:57:19
 *  Author: AGATHA
 */ 

#include "main.h"

bool g_fQuartzEnabled = false;

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
		BYTE ret = ReadAddress( address );
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
			if ( WriteAddress( address, value ) )
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
	
	if ( g_fShowAddr )
	{
		sprintf( output, "%04X: ", address );
		printstr( output );
	}
	
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
					input[local_len++] = ReadAddress( address );
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

bool SilenceReadInput(char * sinput, unsigned char slen_max)
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
			// uart_putc( chr );
		}
		else
		{
			return false;
		}
	}
	return true;	
}

bool StrToShort( char * input, uint16_t * pValue )
{
	char * p = input;
	*pValue = 0;
	while( *p )
	{
		if ( (*p) >= '0' && (*p) <= '9' )
		{
			*pValue = 0x10 * (*pValue) + ((*p) - '0');
		}
		else if ( (*p) >= 'A' && (*p) <= 'F' )
		{
			*pValue = 0x10 * (*pValue) + ((*p) - 'A' + 10);
		}
		else if ( (*p) >= 'a' && (*p) <= 'f' )
		{
			*pValue = 0x10 * (*pValue) + ((*p) - 'a' + 10);
		}
		else
		{
			return false;
		}
		p++;
	}
	
	return true;
}

bool SilenceReadCounter()
{
	const uint16_t length = 5;
	char input[ length ];
	uint16_t counter = 1;
	if ( SilenceReadInput( input, length ) )
	{
		if ( StrToShort( input, &counter ) )
		{
			if ( counter > 0 && counter < 0x8000 )
			{
				g_uCounter = counter;
				return true;
			}
		}
	}
	
	return false;
}


bool SilenceReadRange()
{
	const uint16_t length = 4;
	const uint16_t range_len = 16;
	char input[ length ];
	uint8_t i = 0, k = 0;
	uint16_t counter = 0;
	
	if ( SilenceReadInput( input, length ) )
	{
		uint16_t ptr;
		if ( StrToShort( input, &ptr ) )
		{
			ptr <<= 4;
			char range[ ( range_len << 1 ) + 1 ];
			range[ range_len << 1 ] = 0;
			
			for( counter = 0; counter < g_uCounter; ++counter )
			{
				PrepareRead( ptr );
				k = 0;
				for( i = 0; i < range_len; ++i )
				{
					BYTE result = ReadAddress( ptr );
					BYTE lo_nybble = ( result & 0x0F ), hi_nybble = ( result >> 4 );
					range[ k++ ] = ( ( hi_nybble < 10 ) ? ( hi_nybble + '0' ) : ( hi_nybble - 10 + 'A' ) );
					range[ k++ ] = ( ( lo_nybble < 10 ) ? ( lo_nybble + '0' ) : ( lo_nybble - 10 + 'A' ) );
					ptr++;
				}
				printstr( range );			
			}
		}
	}
	else
	{
		return false;
	}
	
	return true;
}

bool SilenceWriteByte()
{
	const uint16_t length = 7;
	char input[ length ];
	
	if ( SilenceReadInput( input, length ) )
	{
		uint8_t value;
		uint16_t ptr, long_value;
		if ( StrToShort( input + 4, &long_value ) )
		{
			value = (uint8_t) ( long_value & 0xFF );
			input[ 4 ] = 0;
			if ( StrToShort( input, &ptr ) )
			{
				return WriteAddress( ptr, value );
			}
		}
	}
	
	return false;
}

bool SilenceSetQuartz()
{
	switch( uart_getc() )
	{
		case '?':
		{
			if ( g_fQuartzEnabled )
			{
				uart_putc('1');
			}
			else
			{
				uart_putc('0');
			}
			return true;
		}
		case '0':
		{
			g_fQuartzEnabled = false;
			// QUARTZ_DISABLE;
			QUARTZ_ENABLE;
			M2_CLOCK_UP;
			CFG1_REG_DDR |= M2_BIT;
			return true;
		}
		case '1':
		{
			g_fQuartzEnabled = true;
			CFG1_REG_DDR &= ~M2_BIT;
			// QUARTZ_ENABLE;
			QUARTZ_DISABLE;
			M2_CLOCK_UP;
			return true;
		}
		default:
		{
			return false;
		}
	}
}

bool ResetQuartz()
{
	if ( !g_fQuartzEnabled ) return false;
	
	QUARTZ_ENABLE;
	_delay_ms( 100 );
	QUARTZ_DISABLE;
	return true;
}
