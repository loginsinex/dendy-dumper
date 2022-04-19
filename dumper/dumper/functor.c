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

