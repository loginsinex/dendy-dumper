/*
 * functor.c
 *
 * Created: 06.03.2022 21:01:51
 *  Author: AGATHA
 */ 

#include "main.h"

bool g_CHRMode = false;
bool g_fShowAddr = false;
uint16_t g_uCounter = 1;
volatile bool g_fEnabledPhi2Strobe = false;

SET_ADDR set_address = set_address_prg;
PREPARE_READ PrepareRead = PrepareRead_prg;
READ_ADDRESS ReadAddress = ReadAddress_prg;
WRITE_ADDRESS WriteAddress = WriteAddress_prg;

bool SetupPRGMode()
{
	PPU_RD_OFF;
	PPU_WR_OFF;
	g_CHRMode = false;
	set_address = set_address_prg;
	if ( g_fQuartzEnabled )
	{
		ReadAddress = QReadAddress_prg;
		WriteAddress = QWriteAddress_prg;		
	}
	else
	{
		ReadAddress = ReadAddress_prg;
		WriteAddress = WriteAddress_prg;		
	}
	PrepareRead = PrepareRead_prg;
	return true;
}

bool SetupCHRMode()
{
	PPU_RD_OFF;
	PPU_WR_OFF;
	g_CHRMode = true;
	set_address = set_address_ppu;
	if ( g_fQuartzEnabled )
	{
		ReadAddress = QReadAddress_ppu;
		WriteAddress = QWriteAddress_ppu;		
	}
	else
	{
		ReadAddress = ReadAddress_ppu;
		WriteAddress = WriteAddress_ppu;		
	}
	PrepareRead = PrepareRead_ppu;
	return true;
}

ISR(TIMER2_COMP_vect)
{
	
}

ISR(TIMER2_OVF_vect)
{
	
}

ISR(TIMER1_CAPT_vect)
{
	
}

ISR(TIMER1_COMPA_vect)
{
	
}

ISR(TIMER1_COMPB_vect)
{
	
}

ISR(TIMER1_OVF_vect)
{
	
}

ISR(TIMER0_COMP_vect)
{
	
}

ISR(TIMER0_OVF_vect)
{
	
}

ISR(SPI_STC_vect)
{
	
}

ISR(USART_RXC_vect)
{
	
}

ISR(USART_UDRE_vect)
{
	
}

ISR(USART_TXC_vect)
{
	
}

ISR(ADC_vect)
{
	
}

ISR(EE_RDY_vect)
{
	
}

ISR(ANA_COMP_vect)
{
	
}

ISR(TWI_vect)
{
	
}

ISR(SPM_RDY_vect)
{
	
}

ISR(BADISR_vect)
{
	uart_putc( '@' );
}