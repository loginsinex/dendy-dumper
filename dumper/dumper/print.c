/*
 * print.c
 *
 * Created: 06.03.2022 20:54:39
 *  Author: AGATHA
 */ 

#include "main.h"

void printstr(char * s)
{
	int c = 0;
	while(s[c])
	uart_putc(s[c++]);
}