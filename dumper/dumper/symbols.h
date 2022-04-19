/*
 * symbols.h
 *
 * Created: 06.03.2022 23:27:33
 *  Author: AGATHA
 */ 


#ifndef SYMBOLS_H_
#define SYMBOLS_H_

#define SEND_STATUS( sym )	uart_putc( sym )
#define READ_COMMAND		uart_getc()

#define STATUS_DUMPER_INITIALIZED	'#'
#define STATUS_DUMPER_STARTED		'!'
#define STATUS_COMMAND_SUCCESS		'.'
#define STATUS_COMMAND_FAILED		'?'

#define COMMAND_PRG_MODE			'G'
#define COMMAND_PPU_MODE			'P'
#define COMMAND_READ_RANGE			'R'
#define COMMAND_WRITE_BYTE			'W'
#define COMMAND_SET_COUNTER			'C'
#define COMMAND_TIMER_SWITCH		'T'
#define COMMAND_QUARTZ_ENABLE		'Q'
#define COMMAND_M2_RESET			'Y'
#define COMMAND_DETECT_MIRRORING	'M'

#define STATUS_FAILED				SEND_STATUS( STATUS_COMMAND_FAILED )
#define STATUS_SUCCESS				SEND_STATUS( STATUS_COMMAND_SUCCESS )

#endif /* SYMBOLS_H_ */