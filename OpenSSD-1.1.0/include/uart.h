// Copyright 2011 INDILINX Co., Ltd.
//
// This file is part of Jasmine.
//
// Jasmine is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Jasmine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Jasmine. See the file COPYING.
// If not, see <http://www.gnu.org/licenses/>.


#ifndef UART_H
#define UART_H

// Constants
#define UART_WAITTIME			0x100000
#define UART_WAITINFINITE		0
#define UART_COMMBAUDRATE		115200

#define UART_TXFIFO_FULL		0x00
#define UART_TXFIFO_EMPTY		0x20
#define UART_RXFIFO_EMPTY		0x00

#define NO_PARITY				0x0
#define ODD_PARITY				0x4
#define EVEN_PARITY				0x5
#define F1_PARITY				0x6
#define F0_PARITY				0x7

#define ONE_STOPBIT				0x0
#define TWO_STOPBIT				0x1

#define WORDLEN_5BIT			0x0
#define WORDLEN_6BIT			0x1
#define WORDLEN_7BIT			0x2
#define WORDLEN_8BIT			0x3

#define DBG_TX_BUFSIZE			256
#define DBG_RX_BUFSIZE			256

typedef struct
{
	int		word_length		: 2;
	int		stopbit_ctrl	: 1;
	int		parity_mode		: 3;
	int		uart_enable		: 1;
	int		clear_polarity	: 1;
	int		rdy_polarity	: 1;
	int		autofc_enable	: 1;
	int		ir_rx_invmode	: 1;
	int		ir_mode_enable	: 1;
	int		reserved		: 20;
} uartcont_t;

#if OPTION_UART_DEBUG

void uart_init(void);
void uart_print(char* string);
void uart_print_32(UINT32 num);
void uart_print_hex(UINT32 num);
void uart_printf(const char * msg, ...);

#else

#define uart_init(X)
#define uart_print(X)
#define uart_print_32(X)
#define uart_print_hexa(X)
#define uart_printf(fmt,...)

#endif

#endif	// UART_H
