/*
 * PPU.c
 *
 * Created: 06.03.2022 21:10:03
 *  Author: AGATHA
 */ 

#include "main.h"

void set_address_ppu(uint16_t address)
{
	SHIFT_REG_RESET_UP;
	asm( "nop" );
	SHIFT_REG_RESET_DN;
	
	if ( address & ( 1 << 13 ) )
	{
		address &= ~( 1 << 14 );
	}
	else
	{
		address |= ( 1 << 14 );
	}
	
	for( unsigned char i = 0; i < 16; ++i )
	{
		SHIFT_REG_CLK_DN;
		PPU_BIT( address & 0x8000 );
		address <<= 1;
		SHIFT_REG_CLK_UP;
	}
	
	STORAGE_REG_CLK_DN;
	asm( "nop" );
	STORAGE_REG_CLK_UP;
	PPU_ENABLE_ADDR;
}

void PrepareRead_ppu(uint16_t address)
{
	PPU_CART_REG_DDR = 0b00000000;
	PPU_CART_WRITE = 0b00000000;
}


uint8_t qread_ppu_byte()
{
	uint8_t result = 0;
	
	// hold data
	M2_SYNC_UP;
	PPU_SHLD_DN;
	PPU_SHLD_UP;	

	// begin read	
	M2_SYNC_DN;
	PPU_CLKINH_DN;
	PPU_CLK_DN;

	for( uint8_t i = 0; i < 8; ++i )
	{
		result <<= 1;
		if ( PPU_QH_READ ) result |= 1;
		PPU_CLK_UP;
		asm("nop");
		PPU_CLK_DN;
	}
	
	// free register
	PPU_CLKINH_UP;
	
	return result;	
}

uint8_t read_ppu_byte()
{
	uint8_t result = 0;
	
	// hold data
	PPU_SHLD_DN;
	PPU_SHLD_UP;

	// begin read
	PPU_CLKINH_DN;
	PPU_CLK_DN;

	for( uint8_t i = 0; i < 8; ++i )
	{
		result <<= 1;
		if ( PPU_QH_READ ) result |= 1;
		PPU_CLK_UP;
		asm("nop");
		PPU_CLK_DN;
	}
	
	// free register
	PPU_CLKINH_UP;
	
	return result;
}

BYTE ReadAddress_ppu(uint16_t address)
{
	BYTE result = 0xff;
	set_address_ppu( address );
	M2_CLOCK_DN; PPU_RD_ON;
	asm("nop");
	M2_CLOCK_UP;
	asm("nop");
	result = read_ppu_byte(); // PPU_CART_READ;
	PPU_RD_OFF;
	return result;
}

bool WriteAddress_ppu(uint16_t address, BYTE value)
{
	
	// nothing to do
	return false;
}

BYTE QReadAddress_ppu(uint16_t address)
{
	BYTE result = 0xff;
	set_address_ppu( address );
	M2_SYNC_DN;
	PPU_RD_ON;
	asm("nop");
	result = qread_ppu_byte(); // PPU_CART_READ;
	asm("nop");
	M2_SYNC_DN;
	PPU_RD_OFF;
	return result;
}

bool QWriteAddress_ppu(uint16_t address, BYTE value)
{
	// nothing to do
	return false;
}


bool DetectMirroring()
{
	set_address_ppu( 0 );
	if ( IS_CIRAM_A10_BIT_SET )
	{
		uart_putc( 'Z' );
		return true;
	}
	
	set_address_ppu( 0x400 );
	if ( IS_CIRAM_A10_BIT_SET )
	{
		uart_putc( 'V' );
		return true;
	}
	
	set_address_ppu( 0x800 );
	if ( IS_CIRAM_A10_BIT_SET )
	{
		uart_putc( 'H' );
		return true;
	}
	
	uart_putc( 'F' );
	return true;
}
