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


#include "jasmine.h"

#if OPTION_UART_DEBUG

#define TXFIFO_FREE_CNT			((GETREG(UART_FIFOCNT) >> 6) & 0x03F)	// number of bytes that are not sent yet
#define RXFIFO_PENDING_CNT		(GETREG(UART_FIFOCNT) & 0x03F)			// number of bytes received but not read yet

static void uart_txbyte(UINT8 data)
{
	while (TXFIFO_FREE_CNT == UART_TXFIFO_FULL);
	SETREG(UART_FIFODATA, (UINT32)data);
}

static UINT8 uart_rxbyte(void)
{
	while (RXFIFO_PENDING_CNT == UART_RXFIFO_EMPTY);
	return GETREG(UART_FIFODATA);
}

void uart_init(void)
{
	UINT32 uart_ctrl;
	UINT32 clkdiv_baudrate;

	SETREG(UART_FIFOCTRL, 0x03);		// reset FIFO

	ASSERT((GETREG(UART_FIFOCNT) == 0x800));

	uart_ctrl = (WORDLEN_8BIT << 0)
				| (ONE_STOPBIT << 2)
				| (NO_PARITY << 3)
				| (0 << 6)				// uart_enable
				| (0 << 7)				// clear_polarity
				| (0 << 8)				// rdy_polarity
				| (0 << 9)				// auto_flow_control_enable
				| (0 << 10)				// ir_rx_invmode
				| (0 << 11);			// ir_mode_enable

	SETREG(UART_CTRL, uart_ctrl);

	clkdiv_baudrate = GETREG(UART_BAUDRATE) & 0xFFFF0000;
	clkdiv_baudrate |= (UINT32) (1 << 21) * (UART_COMMBAUDRATE / 100) / (CLOCK_SPEED/200) + 1;
	SETREG(UART_BAUDRATE, clkdiv_baudrate);
	SETREG(UART_FIFOCTRL, 0x00000000);

	uart_ctrl = uart_ctrl | (1 << 6);	// uart_enable
	SETREG(UART_CTRL, uart_ctrl);
}
void uart_print(char* string)
{
	while (1)
	{
		uart_txbyte(*string);

		string++;

		if (*string == '\0')
			break;
	}

	uart_txbyte('\r');
	uart_txbyte('\n');
}
#include <stdio.h>

typedef char* caddr_t;
extern caddr_t _sbrk ( int incr );

void uart_print_32(UINT32 time)
{
    char buf[21];

    sprintf(buf, "%u", time);
    /* itoa(time, buf); */

    uart_print(buf);
}
void uart_print_hex(UINT32 num)
{
    char str[8];
    UINT8 remain=0;
    UINT8 cnt = 0;

    while (cnt < 8) {
        remain = num%16;
        num = num/16;
        if (remain<10)
            str[cnt]=remain+48;
        else
            str[cnt]=remain+55;
        cnt++;
    }
    cnt--;
    uart_txbyte('0');
    uart_txbyte('x');

    while (1)
	{
    	uart_txbyte(str[cnt]);
        if (cnt == 0) {
            break;
        }
		cnt--;

	}
	uart_txbyte('\r');
	uart_txbyte('\n');
}
#include <stdarg.h>
void uart_printf(const char * msg, ...)
{
  fflush(stdout);

  char out[256];
  va_list ap;
  va_start(ap, msg);
  int len = vsnprintf(out, sizeof(out), msg, ap);
  va_end(ap);
  out[len] = '\0';
  uart_print(out);
}
#endif	// OPTION_UART_DEBUG
