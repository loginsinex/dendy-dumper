/*
 * PRG.c
 *
 * Created: 06.03.2022 20:56:33
 *  Author: AGATHA
 */ 

#include "main.h"

void set_address_prg(uint16_t address)
{
	SHIFT_REG_RESET_UP;
	asm( "nop" );
	SHIFT_REG_RESET_DN;
	
	address ^= 0x8000;	// set /ROMSEL
	
	for( unsigned char i = 0; i < 16; ++i )
	{
		SHIFT_REG_CLK_DN;
		PRG_BIT( address & 0x8000 );
		address <<= 1;
		SHIFT_REG_CLK_UP;
	}
	
	STORAGE_REG_CLK_DN;
	asm( "nop" );
	STORAGE_REG_CLK_UP;
	PRG_ENABLE_ADDR;
}

void PrepareRead_prg(uint16_t address)
{
	PRG_CART_REG_DDR = 0b00000000;
	PRG_CART_WRITE = 0b00000000;
	PRG_READ;
}

BYTE ReadAddress_prg(uint16_t address)
{
	BYTE result = 0xff;
	set_address_prg(address);
	M2_CLOCK_DN;
	asm("nop");
	result = PRG_CART_READ;
	M2_CLOCK_UP;
	return result;
}

bool WriteAddress_prg(uint16_t address, BYTE value)
{
	M2_CLOCK_DN;
	PRG_CART_WRITE = 0;
	PRG_CART_REG_DDR = 0b11111111;
	PRG_WRITE;
	//set_romsel( address );
	PRG_CART_WRITE = value;
	set_address_prg( address ); 
	_delay_us(1);
	M2_CLOCK_UP;
	_delay_us(1);
	M2_CLOCK_DN;
	set_address_prg( address & 0x7FFF );
	PRG_READ;
	M2_CLOCK_UP;
	return true;
}

BYTE QReadAddress_prg(uint16_t address)
{
	BYTE result = 0xff;
	M2_SYNC_UP;
	PRG_READ;
	set_address_prg(address);
	M2_SYNC_UP;
	result = PRG_CART_READ;
	return result;
}

bool QWriteAddress_prg(uint16_t address, BYTE value)
{
	set_address_prg( 0 );
	PRG_CART_WRITE = 0;
	PRG_CART_REG_DDR = 0b11111111;
	M2_SYNC_DN;
	set_address_prg( address & 0x7FFF ); // PHI2 low, ROMSEL always HIGH
	PRG_WRITE;
	PRG_CART_WRITE = value;
	set_address_prg( address );
	M2_SYNC_UP;
	set_address_prg( address & 0x7FFF ); // clock /ROMSEL for UNROM
	M2_SYNC_DN;
	PRG_READ;
	PrepareRead_prg( 0 );

	return true;
}
