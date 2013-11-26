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

void led(BOOL32 on)
{
	UINT32 temp;

	temp = GETREG(GPIO_REG);

	if (on)
	{
		temp = temp | (1 << 6);
	}
	else
	{
		temp = temp & ~(1 << 6);
	}

	SETREG(GPIO_REG, temp);
}

void led_blink(void)
{
	while (1)
	{
		led(1);
		delay(700000);
		led(0);
		delay(700000);
	}
}

#if OPTION_ENABLE_ASSERT
volatile UINT32 g_barrier2;
#endif

#ifdef __GNUC__
void swi_handler(void) __attribute__ ((interrupt ("SWI")));
void swi_handler(void)
#else
__irq void swi_handler(void)
#endif
{
#if OPTION_ENABLE_ASSERT

	g_barrier2 = 0;

	while (g_barrier2 == 0)
	{
		led(1);
		delay(700000);
		led(0);
		delay(700000);
	}

#endif
}

UINT32 g_sdram_ecc_count;
UINT32 g_sdram_ecc_fail_count;
UINT32 g_timer_interrupt_count;

#ifdef __GNUC__
void irq_handler(void) __attribute__ ((interrupt ("IRQ")));
void irq_handler(void)
#else
__irq void irq_handler(void)
#endif
{
	UINT32 intr_stat = GETREG(APB_INT_STS);

	if (intr_stat & (INTR_TIMER_1 | INTR_TIMER_2 | INTR_TIMER_3))
	{
		g_timer_interrupt_count++;

		CLEAR_TIMER_INTR(TIMER_CH1);
		CLEAR_TIMER_INTR(TIMER_CH2);
		CLEAR_TIMER_INTR(TIMER_CH3);
		SETREG(APB_INT_STS, INTR_TIMER_1 | INTR_TIMER_2 | INTR_TIMER_3);
	}
	else if (intr_stat & INTR_FLASH)
	{
		ftl_isr();
	}
	else if (intr_stat & INTR_SDRAM)
	{
		UINT32 sdram_interrupt = GETREG(SDRAM_INTSTATUS);

	    SETREG(SDRAM_INTSTATUS, 0xFFFFFFFF);

		// clear the DRAM interrupt flag at the interrupt controller
	    SETREG(APB_INT_STS, INTR_SDRAM);

		if (sdram_interrupt & SDRAM_INT_ECC_CORR)
		{
			// Bit errors were detected and corrected.
			// Usually this is NOT an indication of poor SDRAM quality.
			// If the firmware has a bug due to which SDRAM is written by CPU without the help of mem util functions,
			// ECC correction or ECC failure can happen.

			g_sdram_ecc_count++;
		}

		if (sdram_interrupt & SDRAM_INT_ECC_FAIL)
		{
			// Bit errors were detected but could not be corrected.
			g_sdram_ecc_fail_count++;
		}

		if (sdram_interrupt & SDRAM_INT_ADDR_OF)
		{
			// There was an attempt to access beyond DRAM address boundary.
            uart_printf("Error: SDRAM interrupt occred: attempt to access beyond DRAM address boundary");
            led_blink();
		}
	}
}

#include <stdlib.h>

typedef struct
{
	UINT32	erase_prog_fail;
	UINT32	read_fail;
}
test_result_t;

static test_result_t g_test_result[NUM_BANKS];

void test_nand_blocks(void)
{
	// This function is a utility that writes random data to flash pages and verifies them.
	// This function takes a long time to complete.

	UINT32 bank, vblk_offset, page_offset, data, bad;

	#define write_buffer_base	DRAM_BASE
	#define read_buffer_base	(DRAM_BASE + BYTES_PER_VBLK)

	disable_irq();
	flash_clear_irq();

	mem_set_sram(g_test_result, 0, sizeof(g_test_result));

	// Configure the flash controller so that any FIRQ_* does not lead to pause state.
	SETREG(FCONF_PAUSE, 0);

	// STEP 1 - prepare random data

	srand(10);

	for (page_offset = 0; page_offset < PAGES_PER_VBLK; page_offset++)
	{
		data = (rand() & 0xFFFF) | (rand() << 16);
		mem_set_dram(write_buffer_base + page_offset * BYTES_PER_PAGE, data, BYTES_PER_PAGE);
	}

	for (vblk_offset = 1; vblk_offset < VBLKS_PER_BANK; vblk_offset++)
	{
		// STEP 2 - erase a block at each bank

		for (bank = 0; bank < NUM_BANKS; bank++)
		{
			UINT32 rbank = REAL_BANK(bank);

			SETREG(FCP_CMD, FC_ERASE);
			SETREG(FCP_BANK, rbank);
			SETREG(FCP_OPTION, FO_P);
			SETREG(FCP_ROW_L(bank), vblk_offset * PAGES_PER_VBLK);
			SETREG(FCP_ROW_H(bank), vblk_offset * PAGES_PER_VBLK);

			while ((GETREG(WR_STAT) & 0x00000001) != 0);
			SETREG(FCP_ISSUE, NULL);
		}

		// STEP 3 - write to every pages of the erased block

		for (page_offset = 0; page_offset < PAGES_PER_VBLK; page_offset++)
		{
			for (bank = 0; bank < NUM_BANKS; bank++)
			{
				UINT32 rbank = REAL_BANK(bank);

				SETREG(FCP_CMD, FC_COL_ROW_IN_PROG);
				SETREG(FCP_BANK, rbank);
				SETREG(FCP_OPTION, FO_P | FO_E | FO_B_W_DRDY);
				SETREG(FCP_DMA_ADDR, write_buffer_base + page_offset * BYTES_PER_PAGE);
				SETREG(FCP_DMA_CNT, BYTES_PER_PAGE);
				SETREG(FCP_COL, 0);
				SETREG(FCP_ROW_L(bank), vblk_offset * PAGES_PER_VBLK + page_offset);
				SETREG(FCP_ROW_H(bank), vblk_offset * PAGES_PER_VBLK + page_offset);

				while ((GETREG(WR_STAT) & 0x00000001) != 0);
				SETREG(FCP_ISSUE, NULL);
			}
		}

		// STEP 4 - check the FC_ERASE and FC_COL_ROW_IN_PROG results.

		bad = 0;

		while (GETREG(MON_CHABANKIDLE) != 0);

		for (bank = 0; bank < NUM_BANKS; bank++)
		{
			if (BSP_INTR(bank) & (FIRQ_BADBLK_H | FIRQ_BADBLK_L))
			{
				bad |= (1 << bank);
				CLR_BSP_INTR(bank, 0xFF);
				g_test_result[bank].erase_prog_fail++;
			}
		}

		// STEP 5 - read and verify
		// We check ECC/CRC results for verification.

		for (page_offset = 0; page_offset < PAGES_PER_VBLK; page_offset++)
		{
			for (bank = 0; bank < NUM_BANKS; bank++)
			{
				UINT32 rbank = REAL_BANK(bank);

				if (bad & (1 << bank))
					continue;

				SETREG(FCP_CMD, FC_COL_ROW_READ_OUT);
				SETREG(FCP_BANK, rbank);
				SETREG(FCP_OPTION, FO_P | FO_E);
				SETREG(FCP_DMA_ADDR, read_buffer_base + bank * BYTES_PER_PAGE);
				SETREG(FCP_DMA_CNT, BYTES_PER_PAGE);
				SETREG(FCP_COL, 0);
				SETREG(FCP_ROW_L(bank), vblk_offset * PAGES_PER_VBLK + page_offset);
				SETREG(FCP_ROW_H(bank), vblk_offset * PAGES_PER_VBLK + page_offset);

				while ((GETREG(WR_STAT) & 0x00000001) != 0);
				SETREG(FCP_ISSUE, NULL);
			}
		}

		// STEP 6 - check the FC_COL_ROW_READ_OUT results

		while (GETREG(MON_CHABANKIDLE) != 0);

		for (bank = 0; bank < NUM_BANKS; bank++)
		{
			if (BSP_INTR(bank) & FIRQ_DATA_CORRUPT)
			{
				bad |= (1 << bank);
				CLR_BSP_INTR(bank, 0xFF);
				g_test_result[bank].read_fail++;
			}
		}

		// STEP 7 - erase the blocks, but not the bad ones

		for (bank = 0; bank < NUM_BANKS; bank++)
		{
			UINT32 rbank = REAL_BANK(bank);

			if (bad & (1 << bank))
				continue;

			SETREG(FCP_CMD, FC_ERASE);
			SETREG(FCP_BANK, rbank);
			SETREG(FCP_OPTION, FO_P);
			SETREG(FCP_ROW_L(bank), vblk_offset * PAGES_PER_VBLK);
			SETREG(FCP_ROW_H(bank), vblk_offset * PAGES_PER_VBLK);

			while ((GETREG(WR_STAT) & 0x00000001) != 0);
			SETREG(FCP_ISSUE, NULL);
		}
	}

	// Now that bad blocks contain non-0xFF data, it is a good time to use install.exe to scan bad blocks.
}

void start_interval_measurement(UINT32 const timer, UINT32 const prescale)
{
	// The timer starts from 0xFFFFFFFF and counts down to 0x00000000 where it will wrap around and start again.
	// The clock speed of timer is CLOCK_SPEED / 2.
	// Assuming that CLOCK_SPEED is 175000000 and prescale is TIMER_PRESCALE_0,
	// the timer counts down every 11.43ns and
	// the time it takes from 0xFFFFFFFF to 0x00000000 is 0xFFFFFFFF / (175000000 / 2) = 49.1 seconds.

	// TIMER_PRESCALE_0: maximum period = 49 seconds, precision = 11.43ns
	// TIMER_PRESCALE_1: maximum period = 785 seconds, precision = 182ns
	// TIMER_PRESCALE_2: maximum period = 12565 seconds, precision = 2.9us

	// Note that the period and the precision is dependent on CLOCK_SPEED.

	ASSERT(timer == TIMER_CH1 || timer == TIMER_CH2 || timer == TIMER_CH3);	// TIMER_CH4 is used as a retry timer
	ASSERT(prescale == TIMER_PRESCALE_0 || prescale == TIMER_PRESCALE_1 || prescale == TIMER_PRESCALE_2);

	SET_TIMER_CONTROL(timer, 0);
	CLEAR_TIMER_INTR(timer);
	SET_TIMER_LOAD(timer, 0xFFFFFFFF);	// initial value of the timer
	SET_TIMER_CONTROL(timer, TM_ENABLE | TM_BIT_32 | TM_MODE_PRD | prescale);

	// After returning from this function,
	// you can use GET_TIMER_VALUE() to see how much time has elapsed since the call to this function.
}

void start_timer(UINT32 const timer, UINT32 const prescale, UINT32 const init_val)
{
	ASSERT(timer == TIMER_CH1 || timer == TIMER_CH2 || timer == TIMER_CH3);
	ASSERT(prescale == TIMER_PRESCALE_0 || prescale == TIMER_PRESCALE_1 || prescale == TIMER_PRESCALE_2);

	SET_TIMER_CONTROL(timer, 0);
	CLEAR_TIMER_INTR(timer);
	SET_TIMER_LOAD(timer, init_val);
	SET_TIMER_CONTROL(timer, TM_ENABLE | TM_BIT_32 | TM_MODE_PRD | TM_INTR | prescale);
}

// @splim
#if OPTION_UART_DEBUG

#include <stdio.h>

typedef char* caddr_t;

/* static void itoa(UINT32 src, char* dst); */
caddr_t _sbrk ( int incr );

void ptimer_start(void)
{
    start_interval_measurement(TIMER_CH1, TIMER_PRESCALE_0);
}
void ptimer_stop_and_uart_print(void)
{
    UINT32 rtime;
    char buf[21];

    rtime = 0xFFFFFFFF - GET_TIMER_VALUE(TIMER_CH1);
    // Tick to us
    rtime = (UINT32)((UINT64)rtime * 2 * 1000000 * PRESCALE_TO_DIV(TIMER_PRESCALE_0) / CLOCK_SPEED);
    sprintf(buf, "%u", rtime);
    uart_print(buf);
}
int  __HEAP_START[BYTES_PER_SECTOR]; // 2KB Byte HEAP

caddr_t _sbrk ( int incr )
{
  static unsigned char *heap = NULL;
  unsigned char *prev_heap;

  if (heap == NULL) {
    heap = (unsigned char *)&__HEAP_START;
  }
  prev_heap = heap;
  /* check removed to show basic approach */

  heap += incr;

  return (caddr_t) prev_heap;
}
#endif	// OPTION_UART_DEBUG


