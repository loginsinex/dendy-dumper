/*
 * dumper.c
 *
 * Created: 04.06.2021 17:35:55
 * Author : AGATHA
 */ 

#include "main.h"

void init_ports()
{
	CFG1_REG_DDR = 
		( PORT_IN	* CIRAM_CE_BIT )		|
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
	QUARTZ_DISABLE;
	M2_CLOCK_UP;	
}


#ifdef PUTTY_MODE
int main(void)
{
    /* Replace with your application code */

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
