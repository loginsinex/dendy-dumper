/*
 * dumper.c
 *
 * Created: 04.06.2021 17:35:55
 * Author : AGATHA
 */ 

#include "main.h"

// ISR (TIMER0_OVF_vect)
// {
//	//unsigned char ddrd = DDRC, pind = PINC;
//	
//	TCNT0=0x00;										// Clear timer
// }

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

void init_ports()
{
	// CFG1_REG_DDR = 0b11111111;
	
	CFG1_REG_DDR = 
		( PORT_OUT	* CIRAM_CE_BIT )		|
		( PORT_OUT	* M2_BIT )				|
		( PORT_OUT	* PPU_RD_BIT )			|
		( PORT_OUT	* PPU_WR_BIT ) 			|
		( PORT_OUT	* CPU_RW_BIT )			|
		( PORT_IN	* CIRAM_A10_BIT );
		
	CFG2_REG_DDR =
		( PORT_OUT	* PPU_REG_ENABLE_BIT )		|
		( PORT_OUT	* SHIFT_REG_CLOCK_BIT )		|
		( PORT_OUT	* STORAGE_REG_CLOCK_BIT )	|
		( PORT_OUT	* SHIFT_REG_RESET_BIT )		|
		( PORT_OUT	* PPU_REG_BIT )				|
		( PORT_OUT	* PRG_REG_BIT )				|
		( PORT_OUT	* PRG_REG_ENABLE_BIT )		|
		( PORT_OUT	* QUARTZ_ENABLE_BIT );

	// PPU_CART_REG_DDR = 0b00000000;
	// PRG_CART_REG_DDR = 0b00000000;
	// PPU_CART_WRITE = 0b11111111;
	CFG3_REG_DDR = 
		( PORT_OUT	* PPU_CLK_BIT )		|
		( PORT_OUT	* PPU_SHLD_BIT )	|
		( PORT_OUT	* PPU_CLKINH_BIT )	|
		( PORT_IN	* PPU_QH_BIT )		|
		( PORT_OUT	* 0b11110000 );		// reserved
		
	PRG_CART_WRITE = 0b11111111;
	PPU_RD_OFF;
	PPU_WR_OFF;
	PRG_READ;
	M2_CLOCK_UP;
	QUARTZ_DISABLE;
	PPU_ENABLE_ADDR;
	PRG_ENABLE_ADDR;
	
	PPU_CLK_DN;
	PPU_SHLD_UP;
	PPU_CLKINH_UP;
	
	g_fQuartzEnabled = true;
	CFG1_REG_DDR &= ~M2_BIT;
	// QUARTZ_ENABLE;
	QUARTZ_DISABLE;
	M2_CLOCK_UP;	
}


#ifdef PUTTY_MODE
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
			PORTC = 0xFF;
			printstr("Command:\r\n\t0 - PRG read mode\r\n\t1 - CHR read mode\r\n");
			printstr("Enter command: ");
			ReadInput(input, 2);
			PORTC = 0x00;
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
			char str[200];
			sprintf(str, "Command:\r\n\t0 - read byte\r\n\t1 - write byte\r\n\t2 - read range\r\n\t3 - show address [%s]\r\n\t4 - leave\r\n", g_fShowAddr ? "YES" : "NO");
			
			printstr(str);
			printstr("Enter command: ");
			ReadInput(input, 2);
			printstr("\r\n");
			switch(input[0])
			{
				case '0': User_ReadByte(); break;
				case '1': User_WriteByte(); break;
				case '2': User_ReadRange(); break;
				case '3': g_fShowAddr = !g_fShowAddr; break;
				case '4': fSelectMode = true; break;
				default: printstr("ERROR: Invalid command");
			}
		
			printstr("\r\n");			
		}
    }
}
#else
int main(void)
{
	uint8_t err = MCUCSR;
	uint8_t err0 = ( err >> 4 );
	uint8_t err1 = ( err & 0x0f );
	if ( err0 < 0x0a ) err0 += '0';
	else err0 += 'A' - 10;
	if ( err1 < 0x0a ) err1 += '0';
	else err1 += 'A' - 10;
	
	MCUCSR = 0;
	uart_init();
	init_ports();
	
	SEND_STATUS( STATUS_DUMPER_INITIALIZED );
	uart_putc( err0 );
	uart_putc( err1 );
	// if(err & (1<<PORF )) uart_putc('0'); // printstr(("Power-on reset.\r\n"));
	// if(err & (1<<EXTRF)) uart_putc('1'); // printstr(("External reset!\r\n"));
	// if(err & (1<<BORF )) uart_putc('2'); // printstr(("Brownout reset!\r\n"));
	// if(err & (1<<WDRF )) uart_putc('3'); // printstr(("Watchdog reset!\r\n"));
	// if(err & (1<<JTRF )) uart_putc('4'); // printstr(("JTAG reset!\r\n"));
	SEND_STATUS( STATUS_DUMPER_STARTED );
	
	while( 1 )
	{
		BYTE cmd = READ_COMMAND;
		switch( cmd )
		{
			case COMMAND_PRG_MODE:
			{
				SetupPRGMode();
				STATUS_SUCCESS;
				break;
			}
			case COMMAND_PPU_MODE:
			{
				SetupCHRMode();
				STATUS_SUCCESS;
				break;
			}
			case COMMAND_READ_RANGE:
			{
				if ( SilenceReadRange() )
				{
					STATUS_SUCCESS;
				}
				else
				{
					STATUS_FAILED;
				}
				break;
			}
			case COMMAND_WRITE_BYTE:
			{
				if ( SilenceWriteByte() )
				{
					STATUS_SUCCESS;
				}
				else
				{
					STATUS_FAILED;
				}
				break;
			}
			case COMMAND_SET_COUNTER:
			{
				if ( SilenceReadCounter() )
				{					
					STATUS_SUCCESS;
				}
				else
				{
					STATUS_FAILED;
				}
				break;
			}
			case COMMAND_QUARTZ_ENABLE:
			{
				if ( SilenceSetQuartz() )
				{
					STATUS_SUCCESS;
				}
				else
				{
					STATUS_FAILED;
				}
				break;
			}
			case COMMAND_M2_RESET:
			{
				if ( ResetQuartz() )
				{
					STATUS_SUCCESS;
				}
				else
				{
					STATUS_FAILED;
				}
				break;
			}
			case COMMAND_DETECT_MIRRORING:
			{
				if ( DetectMirroring() )
				{
					STATUS_SUCCESS;
				}
				else
				{
					STATUS_FAILED;
				}
				break;
			}
			
			// test M2 wait (undocumented)
			case 'Z':
			{
				switch( uart_getc() )
				{
					case '0':
					{
						uart_putc('>');
						while( PIND & ( 1 << 3 ) );
						uart_putc('!');
						break;
					}
					case '1':
					{
						uart_putc('>');
						while( ! ( PIND & ( 1 << 3 ) ) );
						uart_putc('!');
						break;
					}
					default:
					{
						STATUS_FAILED;
					}
				}
				break;
			}
			default:
			{
				STATUS_FAILED;
				break;
			}
		}
	}
}

#endif