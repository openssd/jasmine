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


#ifndef TARGET_H
#define TARGET_H

#if CLOCK_SPEED		==	25000000
	#define	PLL_CLK_CONFIG	PLL_25MHZ
#elif CLOCK_SPEED	==	30000000
	#define	PLL_CLK_CONFIG	PLL_30MHZ
#elif CLOCK_SPEED	==	50000000
	#define	PLL_CLK_CONFIG	PLL_50MHZ
#elif CLOCK_SPEED	==	60000000
	#define	PLL_CLK_CONFIG	PLL_60MHZ
#elif CLOCK_SPEED	==	75000000
	#define	PLL_CLK_CONFIG	PLL_75MHZ
#elif CLOCK_SPEED	==	100000000
	#define	PLL_CLK_CONFIG	PLL_100MHZ
#elif CLOCK_SPEED	==	133000000
	#define	PLL_CLK_CONFIG	PLL_133MHZ
#elif CLOCK_SPEED	==	150000000
	#define	PLL_CLK_CONFIG	PLL_150MHZ
#elif CLOCK_SPEED	==	155000000
	#define	PLL_CLK_CONFIG	PLL_155MHZ
#elif CLOCK_SPEED	==	160000000
	#define	PLL_CLK_CONFIG	PLL_160MHZ
#elif CLOCK_SPEED	==	165000000
	#define	PLL_CLK_CONFIG	PLL_165MHZ
#elif CLOCK_SPEED	==	170000000
	#define PLL_CLK_CONFIG	PLL_170MHZ
#elif CLOCK_SPEED	==	175000000
	#define	PLL_CLK_CONFIG	PLL_175MHZ
#elif CLOCK_SPEED	==  180000000
	#define	PLL_CLK_CONFIG 	PLL_180MHZ
#endif

#define DRAM_BASE		0x40000000
#define FREG_BASE		0x60000000		// flash controller
#define MREG_BASE		0x50000000		// memory utility
#define BS_BASE			0x70000000		// SATA controller
#define GPIO_BASE		0x83000000		// GPIO
#define SRAM_BASE		0x10000000		// before remap
#define ROM_BASE		0x10000000		// after remap

#define SETREG(ADDR, VAL)	*(volatile UINT32*)(ADDR) = (UINT32)(VAL)
#define GETREG(ADDR)		(*(volatile UINT32*)(ADDR))

#define SRAM_SIZE		(96*1024)

#if NAND_SPEC_SPEED == NAND_SPEC_VERY_FAST
#define PS_PER_FLASH_CYCLE 	20000	// pico seconds
#elif NAND_SPEC_SPEED == NAND_SPEC_FAST
#define PS_PER_FLASH_CYCLE 	25000
#elif NAND_SPEC_SPEED == NAND_SPEC_SLOW
#define PS_PER_FLASH_CYCLE	30000
#elif NAND_SPEC_SPEED == NAND_SPEC_DULL
#define PS_PER_FLASH_CYCLE	50000
#endif

#define PS_PER_CLOCK_CYCLE		(1000000 / (CLOCK_SPEED / 1000000))
#define PS_PER_HALF_CYCLE		(PS_PER_CLOCK_CYCLE / 2)

#if NAND_SPEC_SPEED == NAND_SPEC_VERY_FAST
	#if PS_PER_CLOCK_CYCLE >= 10000 	// up to 100MHz
	#define CYCLE_PER_HOLD 1
	#else
	#define CYCLE_PER_HOLD 2
	#endif
#elif NAND_SPEC_SPEED == NAND_SPEC_DULL
#define CYCLE_PER_HOLD 			((15000 + PS_PER_CLOCK_CYCLE - 1) / PS_PER_CLOCK_CYCLE)
#elif PS_PER_CLOCK_CYCLE >= 12000 	// up to 83.3MHz
#define CYCLE_PER_HOLD 1
#else
#define CYCLE_PER_HOLD 2
#endif

#define PS_PER_HOLD 			(PS_PER_CLOCK_CYCLE * CYCLE_PER_HOLD)

#if PS_PER_FLASH_CYCLE < PS_PER_HOLD	// up to 40MHz (NAND_SPEC_FAST) or 33.3MHz (NAND_SPEC_SLOW)
#define PS_PER_SETUP			PS_PER_CLOCK_CYCLE
#else
#define PS_PER_SETUP			(PS_PER_FLASH_CYCLE - PS_PER_HOLD)
#endif

#define _HALF_CYCLE_PER_SETUP	((PS_PER_SETUP + PS_PER_HALF_CYCLE - 1) / PS_PER_HALF_CYCLE)

#if _HALF_CYCLE_PER_SETUP == 1
#define HALF_CYCLE_PER_SETUP	2	// 0.5 -> 1 cycles

#elif _HALF_CYCLE_PER_SETUP == 3
#define HALF_CYCLE_PER_SETUP	4	// 1.5 -> 2 cycles

#else
#define HALF_CYCLE_PER_SETUP	_HALF_CYCLE_PER_SETUP

#endif

#define FLASH_PARAM_SETUP		(HALF_CYCLE_PER_SETUP - 2)

#define PLL_25MHZ		(PLL_F(40-1) | PLL_R(10-1) | PLL_OD(3))		// fref = 5.00MHz	0x6927
#define PLL_30MHZ		(PLL_F(48-1) | PLL_R(10-1) | PLL_OD(3))		// fref = 5.00MHz	0x692F
#define PLL_50MHZ		(PLL_F(40-1) | PLL_R(10-1) | PLL_OD(2))		// fref = 5.00MHz	0x4927
#define PLL_60MHZ		(PLL_F(48-1) | PLL_R(10-1) | PLL_OD(2))		// fref = 5.00MHz
#define PLL_75MHZ		(PLL_F(60-1) | PLL_R(10-1) | PLL_OD(2))		// fref = 5.00MHz
#define PLL_100MHZ		(PLL_F(36-1) | PLL_R(9-1) | PLL_OD(1))		// fref = 5.00MHz
#define PLL_133MHZ		(PLL_F(48-1) | PLL_R(9-1) | PLL_OD(1))		// fref = 5.00MHz
#define PLL_150MHZ		(PLL_F(54-1) | PLL_R(9-1) | PLL_OD(1))		// fref = 5.00MHz
#define PLL_155MHZ		(PLL_F(56-1) | PLL_R(9-1) | PLL_OD(1))		// fref = 5.00MHz
#define PLL_160MHZ		(PLL_F(64-1) | PLL_R(10-1) | PLL_OD(1))		// fref = 5.00MHz
#define PLL_165MHZ		(PLL_F(66-1) | PLL_R(10-1) | PLL_OD(1))		// fref = 5.00MHz
#define PLL_170MHZ		(PLL_F(68-1) | PLL_R(10-1) | PLL_OD(1))		// fref = 5.00MHz
#define PLL_175MHZ		(PLL_F(63-1) | PLL_R(9-1) | PLL_OD(1))		// fref = 5.00MHz
#define PLL_180MHZ		(PLL_F(72-1) | PLL_R(10-1) | PLL_OD(1))		// fref = 5.00MHz

#define	CLOCK_DIV_0001	0
#define	CLOCK_DIV_0002	1
#define	CLOCK_DIV_0032	5
#define	CLOCK_DIV_1024	10
#define	CLOCK_DIV_PLL	0xFF

#define	CLOCK_DIV_0001_SPEED	50000000
#define	CLOCK_DIV_0002_SPEED	(CLOCK_DIV_0001_SPEED >> CLOCK_DIV_0002)
#define CLOCK_DIV_0032_SPEED	(CLOCK_DIV_0001_SPEED >> CLOCK_DIV_0032)
#define	CLOCK_DIV_1024_SPEED	(CLOCK_DIV_0001_SPEED >> CLOCK_DIV_1024)

#include "peri.h"
#include "sdram.h"

#ifndef PROGRAM_INSTALLER

/////////////////
// event queue
/////////////////

#define EQ_SIZE		128
#define EQ_MARGIN	4


///////////////
// interrupt
///////////////

#ifdef __GNUC__

UINT32 disable_irq(void);
void enable_irq(void);
UINT32 disable_fiq(void);
void enable_fiq(void);

#else

#define enable_irq		__enable_irq		// compiler intrinsic functions
#define disable_irq		__disable_irq
#define enable_fiq		__enable_fiq
#define disable_fiq		__disable_fiq

#endif

void enable_interrupt(void);
void disable_interrupt(void);

#if OPTION_ENABLE_ASSERT
	#define ASSERT(X)				       \
	{								       \
		if (!(X))					       \
		{                                  \
            uart_printf("assertion fail: %s, line# %d", __FILE__, __LINE__); \
            led_blink();                   \
			while (1);				       \
		}							       \
	}
#else
	#define ASSERT(X)
#endif

#endif	// PROGRAM_INSTALLER
#endif	// TARGET_H

