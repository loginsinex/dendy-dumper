/*
 * main.h
 *
 * Created: 06.03.2022 20:52:10
 *  Author: AGATHA
 */ 


#ifndef MAIN_H_
#define MAIN_H_



// #define PUTTY_MODE

#define F_CPU 16000000UL
#define BAUD_RATE 500000L // 19200UL
#define AVR_ATmega32
#define BAUD_PRESCALE ( F_CPU / ( BAUD_RATE * 16 ) - 1 )

#define BIT_ENABLE		1
#define BIT_DISABLE		0

#define PORT_OUT		BIT_ENABLE
#define PORT_IN			BIT_DISABLE

#define CFG1_REG		PORTD
#define CFG1_REG_DDR	DDRD
#define CFG1_REG_READ	PIND
#define CIRAM_CE_BIT	( 1 << PIND2 )
#define M2_BIT			( 1 << PIND3 )
#define PPU_WR_BIT		( 1 << PIND4 )
#define PPU_RD_BIT		( 1 << PIND5 )
#define CPU_RW_BIT		( 1 << PIND6 )
#define CIRAM_A10_BIT	( 1 << PIND7 )

#define PPU_RD_ON		{ CFG1_REG &= ~PPU_RD_BIT; }
#define PPU_RD_OFF		{ CFG1_REG |=  PPU_RD_BIT; }
#define PPU_WR_ON		{ CFG1_REG &= ~PPU_WR_BIT; }
#define PPU_WR_OFF		{ CFG1_REG |=  PPU_WR_BIT; }
#define M2_CLOCK_UP		{ CFG1_REG |=  M2_BIT; }
#define M2_CLOCK_DN		{ CFG1_REG &= ~M2_BIT; }
#define PRG_READ		{ CFG1_REG |=  CPU_RW_BIT; }
#define PRG_WRITE		{ CFG1_REG &= ~CPU_RW_BIT; }
#define IS_CIRAM_A10_BIT_SET	( CFG1_REG_READ & CIRAM_A10_BIT )
#define M2_SYNC_UP		while( ( CFG1_REG_READ & M2_BIT ) )
#define M2_SYNC_DN		while( !( CFG1_REG_READ & M2_BIT ) )


#define CFG2_REG				PORTC
#define CFG2_REG_DDR			DDRC
	
#define PPU_REG_ENABLE_BIT		( 1 << PINC0 )
#define STORAGE_REG_CLOCK_BIT	( 1 << PINC1 )
#define SHIFT_REG_CLOCK_BIT		( 1 << PINC2 )
#define SHIFT_REG_RESET_BIT		( 1 << PINC3 )
#define PPU_REG_BIT				( 1 << PINC4 )
#define PRG_REG_BIT				( 1 << PINC5 )
#define PRG_REG_ENABLE_BIT		( 1 << PINC6 )
#define QUARTZ_ENABLE_BIT		( 1 << PINC7 )

#define PPU_ENABLE_ADDR			{ CFG2_REG &= ~PPU_REG_ENABLE_BIT; }
#define PPU_DISABLE_ADDR		{ CFG2_REG |= PPU_REG_ENABLE_BIT; }
#define SHIFT_REG_CLK_UP		{ CFG2_REG |= SHIFT_REG_CLOCK_BIT; }
#define SHIFT_REG_CLK_DN		{ CFG2_REG &= ~SHIFT_REG_CLOCK_BIT; }
#define STORAGE_REG_CLK_UP		{ CFG2_REG |= STORAGE_REG_CLOCK_BIT; }
#define STORAGE_REG_CLK_DN		{ CFG2_REG &= ~STORAGE_REG_CLOCK_BIT; }
#define SHIFT_REG_RESET_UP		{ CFG2_REG &= ~SHIFT_REG_RESET_BIT; }	
#define SHIFT_REG_RESET_DN		{ CFG2_REG |= SHIFT_REG_RESET_BIT; }	
#define PPU_REG_CLK_UP			{ CFG2_REG |= PPU_REG_BIT; }
#define PPU_REG_CLK_DN			{ CFG2_REG &= ~PPU_REG_BIT; }
#define PRG_REG_CLK_UP			{ CFG2_REG |= PRG_REG_BIT; }
#define PRG_REG_CLK_DN			{ CFG2_REG &= ~PRG_REG_BIT; }
#define PRG_ENABLE_ADDR			{ CFG2_REG &= ~PRG_REG_ENABLE_BIT; }
#define PRG_DISABLE_ADDR		{ CFG2_REG |= PRG_REG_ENABLE_BIT; }
#define QUARTZ_ENABLE			{ CFG2_REG &= ~QUARTZ_ENABLE_BIT; }	
#define QUARTZ_DISABLE			{ CFG2_REG |= QUARTZ_ENABLE_BIT; }

#define PPU_BIT( bit )			( ( bit ) ? ( CFG2_REG |= PPU_REG_BIT ) : ( CFG2_REG &= ~PPU_REG_BIT ) )
#define PRG_BIT( bit )			( ( bit ) ? ( CFG2_REG |= PRG_REG_BIT ) : ( CFG2_REG &= ~PRG_REG_BIT ) )

#define CFG3_REG				PORTA
#define CFG3_REG_DDR			DDRA

#define PPU_QH_BIT			( 1 << PINA0 )
#define PPU_CLK_BIT			( 1 << PINA1 )
#define PPU_CLKINH_BIT		( 1 << PINA2 )
#define PPU_SHLD_BIT		( 1 << PINA3 )

#define PPU_CLK_UP			{ CFG3_REG |= PPU_CLK_BIT; }
#define PPU_CLK_DN			{ CFG3_REG &= ~PPU_CLK_BIT; }
#define PPU_SHLD_UP			{ CFG3_REG |= PPU_SHLD_BIT; }
#define PPU_SHLD_DN			{ CFG3_REG &= ~PPU_SHLD_BIT; }
#define PPU_CLKINH_UP		{ CFG3_REG |= PPU_CLKINH_BIT; }
#define PPU_CLKINH_DN		{ CFG3_REG &= ~PPU_CLKINH_BIT; }
#define PPU_QH_READ			( PINA & PPU_QH_BIT )

#define PRG_CART_REG_DDR		DDRB
#define PRG_CART_READ			PINB
#define PRG_CART_WRITE			PORTB

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
typedef void ( *SET_ADDR )( uint16_t address );
typedef void ( *PREPARE_READ )( uint16_t address );
typedef BYTE ( *READ_ADDRESS )( uint16_t address );
typedef bool ( *WRITE_ADDRESS )( uint16_t address, BYTE value );

extern void uart_init( void );
extern void uart_putc( char c );
extern unsigned char uart_getc( void );
extern void printstr(char * s);
extern bool ReadInput(char * sinput, unsigned char slen_max);
extern bool ReadInputShort( char * prompt, USHORT * pResult );
extern bool ReadInputByte( BYTE * pResult );
extern void User_ReadByte();
extern void User_WriteByte();
extern void PrintArr( USHORT address, BYTE * pArray, BYTE length );
extern void User_ReadRange();

extern bool SilenceReadRange();
extern bool SilenceWriteByte();
extern bool SilenceReadCounter();
extern bool SilenceSetQuartz();

extern bool SetupPRGMode();
extern bool SetupCHRMode();

extern void set_address_prg(uint16_t address);
extern void set_romsel(uint16_t address);
extern void PrepareRead_prg(uint16_t address);
extern BYTE ReadAddress_prg(uint16_t address);
extern bool WriteAddress_prg(uint16_t address, BYTE value);
extern BYTE QReadAddress_prg(uint16_t address);
extern bool QWriteAddress_prg(uint16_t address, BYTE value);

extern void set_address_ppu(uint16_t address);
extern void PrepareRead_ppu(uint16_t address);
extern BYTE ReadAddress_ppu(uint16_t address);
extern bool WriteAddress_ppu(uint16_t address, BYTE value);
extern BYTE QReadAddress_ppu(uint16_t address);
extern bool QWriteAddress_ppu(uint16_t address, BYTE value);
extern bool DetectMirroring();
extern bool ResetQuartz();

extern SET_ADDR set_address;
extern PREPARE_READ PrepareRead;
extern READ_ADDRESS ReadAddress;
extern WRITE_ADDRESS WriteAddress;

extern bool g_CHRMode;
extern bool g_fShowAddr;
extern uint16_t g_uCounter;
extern bool g_fQuartzEnabled;

#include "symbols.h"

#endif /* MAIN_H_ */
