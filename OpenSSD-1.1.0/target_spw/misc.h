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

#ifndef MISC_H
#define MISC_H

void led(BOOL32 on);
void led_blink(void);
void test_nand_blocks(void);
void start_interval_measurement(UINT32 const timer, UINT32 const prescale);
void start_timer(UINT32 const timer, UINT32 const prescale, UINT32 const init_val);
#if OPTION_UART_DEBUG
void ptimer_start(void);
void ptimer_stop_and_uart_print(void);
#endif

#endif // MISC_H

