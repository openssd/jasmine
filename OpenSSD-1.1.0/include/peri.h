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


#ifndef	PERI_H
#define	PERI_H

///////////////////
// buffer manager
///////////////////

#define BM_WRITE_LIMIT		(MREG_BASE + 0x00)
#define BM_READ_LIMIT		(MREG_BASE + 0x04)
#define BM_STACK_RESET		(MREG_BASE + 0x08)
#define BM_STACK_WRSET		(MREG_BASE + 0x28)
#define BM_STACK_RDSET		(MREG_BASE + 0x2C)

////////////////////////
// INTERRUPT controller
////////////////////////

#define INTR_SATA			0x00000001
#define INTR_FLASH			0x00000002
#define INTR_SDRAM			0x00000004
#define INTR_UART_TX		0x00000008
#define INTR_UART_RX		0x00000010
#define INTR_TIMER_4		0x00000020
#define INTR_TIMER_3		0x00000040
#define INTR_TIMER_2		0x00000080
#define INTR_TIMER_1		0x00000100
#define INTR_WATCH_DOG		0x00000200
#define INTR_EXT			0x00000400

#define APB_ICU_CON			(0x85000000 + 0x000)
#define APB_INT_STS			(0x85000000 + 0x004)
#define APB_INT_MSK			(0x85000000 + 0x00C)
#define APB_PRI_SET1		(0x85000000 + 0x054)
#define APB_PRI_SET2		(0x85000000 + 0x058)

////////////////////////
// SDRAM controller
////////////////////////

#define SDRAM_INIT			0x48000000
#define SDRAM_CTRL			0x48000004
#define SDRAM_REFRESH		0x48000008
#define SDRAM_TIMING		0x4800000C
#define SDRAM_MRS			0x48000010
#define SDRAM_STATUS		0x48000014
#define SDRAM_INTCTRL		0x48000018
#define SDRAM_INTSTATUS		0x4800001C
#define SDRAM_ECC_MON		0x48000020

// SDRAM_INIT

#define SDRAM_16MB				(0 << 0)
#define SDRAM_32MB				(1 << 0)
#define SDRAM_64MB				(2 << 0)
#define SDRAM_CL_2				(0 << 2)	// CAS latency
#define SDRAM_CL_3				(1 << 2)
#define SDRAM_RAS(X)			((X) << 3)
#define SDRAM_AUTO_WAKE_UP		(1 << 6)
#define SDRAM_ECC_DISABLE		(1 << 8)
#define SDRAM_INIT_PERIOD(X)	(((unsigned int)(X)) << 16)

// SDRAM_CTRL

#define SDRAM_INITIALIZE		(1 << 0)
#define SDRAM_FORCE_REFRESH		(1 << 1)
#define SDRAM_DEEP_POWER_DOWN	(1 << 2)
#define SDRAM_DP_WAKE_UP		(1 << 3)
#define SDRAM_SELF_REFRESH		(1 << 4)
#define SDRAM_SR_WAKE_UP		(1 << 5)

// SDRAM_REFRESH

#define SDRAM_RF_PERIOD(X)		((X) << 0)
#define SDRAM_TRC(X)			((X) << 12)
#define SDRAM_TRP(X)			((X) << 16)

// SDRAM_INTCTRL

#define SDRAM_INT_ECC_FAIL		0x001
#define SDRAM_INT_ECC_CORR		0x002
#define SDRAM_INT_ADDR_OF		0x004
#define SDRAM_INT_ENABLE		0x100

// SDRAM_TIMING

#define SDRAM_RDCLK_DLY(X)		((X) << 0)
#define SDRAM_RD_CAP_DLY_0(X)	((X) << 4)
#define SDRAM_SDCLK_DLY(X)		((X) << 5)
#define SDRAM_RD_CAP_DLY_1(X)	((X) << 9)
#define SDRAM_CAP_CLK_SEL(X)	((X) << 10)

// SDRAM_MRS

#define SDRAM_MRS_KEY(X)		((X) << 0)
#define SDRAM_EMRS_KEY(X)		((X) << 16)


//////////////////////////////////
// 7 segment register set
//////////////////////////////////

#define SEG_SETVAL		(MREG_BASE + 0x34)		// 0x0 ~ 0xF
#define SEG_TIME		(MREG_BASE + 0x38)		// 0.1 sec
#define SEG_CTRL		(MREG_BASE + 0x3C)		// 1 = set, 2 = run

#define MREG_SIZE		(4         + 0x3C)


//////////
// GPIO
//////////

#define	GPIO_REG		(GPIO_BASE + 0x00)
#define	GPIO_MON		(GPIO_BASE + 0x04)
#define	GPIO_DIR		(GPIO_BASE + 0x08)
#define	GPIO_MOD		(GPIO_BASE + 0x0C)
#define	PHY_DEBUG		(GPIO_BASE + 0x40)
#define	PHY_MON			(GPIO_BASE + 0x44)
#define	PHY_TXCTRL		(GPIO_BASE + 0x48)
#define	PHY_PWR			(GPIO_BASE + 0x4C)

#define GPIODIR_B		(GPIO_BASE + 0x80)	// 16bit, 1 = output, 0 = input
#define GPIODIR_C		(GPIO_BASE + 0x84)
#define GPIODIR_D		(GPIO_BASE + 0x88)
#define GPIO_B			(GPIO_BASE + 0x8C)
#define GPIO_C			(GPIO_BASE + 0x90)
#define GPIO_D			(GPIO_BASE + 0x94)
#define GPIODIR_E		(GPIO_BASE + 0x98)
#define GPIODIR_F		(GPIO_BASE + 0x9C)
#define GPIODIR_G		(GPIO_BASE + 0xA0)
#define GPIODIR_H		(GPIO_BASE + 0xA4)
#define GPIO_E			(GPIO_BASE + 0xA8)
#define GPIO_F			(GPIO_BASE + 0xAC)
#define GPIO_G			(GPIO_BASE + 0xB0)
#define GPIO_H			(GPIO_BASE + 0xB4)
#define GPIODIR_I		(GPIO_BASE + 0xB8)
#define GPIODIR_J		(GPIO_BASE + 0xBC)
#define GPIODIR_K		(GPIO_BASE + 0xC0)
#define GPIODIR_L		(GPIO_BASE + 0xC4)
#define GPIO_I			(GPIO_BASE + 0xC8)
#define GPIO_J			(GPIO_BASE + 0xCC)
#define GPIO_K			(GPIO_BASE + 0xD0)
#define GPIO_L			(GPIO_BASE + 0xD4)
#define GPIODIR_M		(GPIO_BASE + 0xD8)
#define GPIODIR_N		(GPIO_BASE + 0xDC)
#define GPIO_M			(GPIO_BASE + 0xE0)
#define GPIO_N			(GPIO_BASE + 0xE4)

#define GPIO_SIZE		(4         + 0xE4)


//////////
// PMU
//////////

#define PMU_ResetCon	0x80000000
#define ClkMaskCon		0x80000004
#define PllCon			0x80000008
#define CLKSelCon		0x8000000C
#define RemapCon		0x80000014
#define ExtINTCon		0x80000018

// PMU_ResetCon
#define RESET_PHYPOR	(1 << 31)
#define RESET_SATAHCLK	(1 << 12)
#define RESET_PMCLK		(1 << 11)
#define RESET_SATADWCLK	(1 << 10)	// reset a part of Transport & Phy Control
#define RESET_PHYDOMAIN	(1 << 8)
#define RESET_TIMER		(1 << 7)
#define RESET_UART		(1 << 6)
#define RESET_GPIO		(1 << 5)
#define RESET_BM		(1 << 4)
#define RESET_SATA		(1 << 3)
#define RESET_FLASH		(1 << 2)
#define RESET_SDRAM		(1 << 1)
#define RESET_ARM		(1 << 0)

// PllCon
#define PLL_F(X)		((X) << 0)		// feedback divider: real_value = PLL_F + 1
#define PLL_R(X)		((X) << 8)		// input divider: real_value = PLL_R + 1
#define PLL_OD(X)		((X) << 13)		// output divider: real_value = 2^PLL_OD
#define PLL_BP			(1 << 15)		// bypass mode
#define PLL_PD			(1 << 16)		// power down mode
#define PLL_LD			(1 << 17)		// lock detector (read only bit)

// CLKSelCon	 [0]
#define	USE_PLL_CLK		(1 << 0)
#define	USE_BYPASS_CLK	(0 << 0)

///////////////////////////////
// uart controller
///////////////////////////////

#define UART_CTRL		0x81000000
#define UART_BAUDRATE	0x81000004
#define UART_FIFOCTRL	0x81000008
#define UART_FIFOCNT	0x8100000C
#define UART_FIFODATA	0x81000010


///////////////////////////////
// watchdog
///////////////////////////////

#define WATCHDOG_BASE	0x84000000

#define WT_LOAD 		(WATCHDOG_BASE + 0x0000)
#define WT_VALUE		(WATCHDOG_BASE + 0x0004)
#define WT_CONTROL		(WATCHDOG_BASE + 0x0008)
#define WT_INT_CLR		(WATCHDOG_BASE + 0x000C)
#define WT_RIS			(WATCHDOG_BASE + 0x0010)
#define WT_MIS			(WATCHDOG_BASE + 0x0014)
#define WT_LOCK 		(WATCHDOG_BASE + 0x0C00)
#define WT_TCR			(WATCHDOG_BASE + 0x0F00)
#define WT_TOP			(WATCHDOG_BASE + 0x0F04)

#define WT_ENABLE_INT	(1 << 0)	// enable the interrupt and the counter
#define WT_ENABLE_RESET (1 << 1)	// enable watchdog module reset output


///////////////////////////////
// timer
///////////////////////////////

#define TIMER_BASE		0x82000000

#define TM_1_LOAD		(TIMER_BASE + 0x00)
#define TM_1_VALUE		(TIMER_BASE + 0x04)
#define TM_1_CONTROL	(TIMER_BASE + 0x08)
#define TM_1_INT_CLR	(TIMER_BASE + 0x0C)
#define TM_1_RIS		(TIMER_BASE + 0x10)
#define TM_1_MIS		(TIMER_BASE + 0x14)
#define TM_1_BG_LOAD	(TIMER_BASE + 0x18)

#define TM_2_LOAD		(TIMER_BASE + 0x20)
#define TM_2_VALUE		(TIMER_BASE + 0x24)
#define TM_2_CONTROL	(TIMER_BASE + 0x28)
#define TM_2_INT_CLR	(TIMER_BASE + 0x2C)
#define TM_2_RIS		(TIMER_BASE + 0x30)
#define TM_2_MIS		(TIMER_BASE + 0x34)
#define TM_2_BG_LOAD	(TIMER_BASE + 0x38)

#define TM_3_LOAD		(TIMER_BASE + 0x40)
#define TM_3_VALUE		(TIMER_BASE + 0x44)
#define TM_3_CONTROL	(TIMER_BASE + 0x48)
#define TM_3_INT_CLR	(TIMER_BASE + 0x4C)
#define TM_3_RIS		(TIMER_BASE + 0x50)
#define TM_3_MIS		(TIMER_BASE + 0x54)
#define TM_3_BG_LOAD	(TIMER_BASE + 0x58)

#define TM_4_LOAD		(TIMER_BASE + 0x60)
#define TM_4_VALUE		(TIMER_BASE + 0x64)
#define TM_4_CONTROL	(TIMER_BASE + 0x68)
#define TM_4_INT_CLR	(TIMER_BASE + 0x6C)
#define TM_4_RIS		(TIMER_BASE + 0x70)
#define TM_4_MIS		(TIMER_BASE + 0x74)
#define TM_4_BG_LOAD	(TIMER_BASE + 0x78)

#define TM_TCR			(TIMER_BASE + 0x0F00)
#define TM_TOP			(TIMER_BASE + 0x0F04)

#define TM_SHOT 		(1 << 0)	// enable one-shot mode ( 0 : wrapping mode )
#define TM_BIT_32		(1 << 1)	// enable 32 bit counter ( 0 : 16 bit counter )
#define TM_PRESCALE_0	(0 << 2)	// 0 stages of prescale, clock is divided by 1
#define TM_PRESCALE_4	(1 << 2)	// 4 stages of prescale, clock is divided by 16
#define TM_PRESCALE_8	(2 << 2)	// 8 stages of prescale, clock is divided by 256
#define TM_INTR 		(1 << 5)	// enable interrupt
#define TM_INTR_DIS		(0 << 5)	// enable interrupt
#define TM_MODE_PRD 	(1 << 6)	// enable periodic mode ( 0 : free-running mode )
#define TM_ENABLE		(1 << 7)	// enable timer module

#define	TIMER_CH1		1
#define	TIMER_CH2		2
#define	TIMER_CH3		3
#define	TIMER_CH4		4	// used for retry timer (see sata_isr.c)

#define	SET_TIMER_LOAD(ch, val)		(SETREG(TM_1_LOAD 	+ (0x20 * (ch - 1)), val))
#define	SET_TIMER_BG_LOAD(ch, val)	(SETREG(TM_1_BG_LOAD	+ (0x20 * (ch - 1)), val))
#define	GET_TIMER_VALUE(ch)			(GETREG(TM_1_VALUE	+ (0x20 * (ch - 1))))
#define	SET_TIMER_CONTROL(ch, val)	(SETREG(TM_1_CONTROL + (0x20 * (ch - 1)), val))
#define	CLEAR_TIMER_INTR(ch)		(SETREG(TM_1_INT_CLR	+ (0x20 * (ch - 1)), 0x01))

#define	PRESCALE_TO_DIV(val)		(val==0?1:val==1?16:256)

#define	TIMER_PRESCALE_0			(0 << 2)	// 1
#define	TIMER_PRESCALE_1			(1 << 2)	// 16
#define	TIMER_PRESCALE_2			(2 << 2)	// 256

#endif	// PERI_H

