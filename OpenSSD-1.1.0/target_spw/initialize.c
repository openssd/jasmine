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

void init_jasmine(void)
{
	UINT32 i, bank;
	extern UINT32 Image$$ER_ZI$$ZI$$Base;
	extern UINT32 Image$$ER_ZI$$ZI$$Length;

	// PLL initialization

	SETREG(CLKSelCon, USE_BYPASS_CLK);

	SETREG(PllCon, PLL_PD); 					// power down
	delay(600);									// at least 500ns
	SETREG(PllCon, PLL_CLK_CONFIG | PLL_PD);	// change settings
	delay(600);									// at least 1us
	SETREG(PllCon, PLL_CLK_CONFIG);				// power up
	while ((GETREG(PllCon) & PLL_LD) == 0); 	// wait lock

	SETREG(CLKSelCon, USE_PLL_CLK);

	// reset hardware modules

	SETREG(PMU_ResetCon, RESET_SDRAM | RESET_BM | RESET_SATA | RESET_FLASH);

	// GPIO bits
	// There are 7 GPIO bits from 0 to 6.
	// 0: This bit is connected to J2 (Factory Mode jumper). The ROM firmware configures it as input mode.
	// While main firmware is running, it can be freely used for arbitrary purpose. Beware that a "1" output while
	// the Factory Mode jumper is set to Normal position (tied to ground) can lead to circuit damage.
	// A "0" output while the jumper is tied to Vcc will also lead to circuit damage.
	// 1: This bit is connected to J3 (Boot ROM). The controller hardware checks its status upon reset.
	// After the reset is done, you can remove the jumper and use the pin as output.
	// 2 through 5: The IO pins for these bits are shared between MAX3232C (UART chip) and J4.
	// In order to use J4, you have to turn on the switches of SW4 and turn off the switches 1 through 4 in SW2.
	// In order to use UART, you have to turn off the switches of SW4 and turn on the switches 1 through 4 in SW2.
	// 6: This bit is connected to D4 (LED) via SW2.

	#if OPTION_UART_DEBUG
	SETREG(GPIO_MOD, 0);
	SETREG(GPIO_DIR, BIT3 | BIT4 | BIT6);	// output pins: 3(UART_TXD), 4(UART_RTS), 6(LED)
	#else
	SETREG(GPIO_MOD, 7);
	SETREG(GPIO_DIR, BIT2 | BIT3 | BIT4 | BIT5 | BIT6);
	#endif

	SETREG(GPIO_REG, 0);					// initial state of LED is "off"

	// ZI region is zero-filled by hardware.
	mem_set_sram((UINT32) &Image$$ER_ZI$$ZI$$Base, 0x00000000, (UINT32) &Image$$ER_ZI$$ZI$$Length);

	SETREG(PHY_DEBUG, 0x40000139);
	while((GETREG(PHY_DEBUG) & BIT30) == 1);

	SETREG(SDRAM_INIT, SDRAM_PARAM_MAIN_FW_INIT);
	SETREG(SDRAM_REFRESH, SDRAM_PARAM_MAIN_FW_REFRESH);
	SETREG(SDRAM_TIMING, SDRAM_PARAM_MAIN_FW_TIMING);
	SETREG(SDRAM_MRS, SDRAM_PARAM_MAIN_FW_MRS);
	SETREG(SDRAM_CTRL, SDRAM_INITIALIZE);		// initialization of SDRAM begins now
	while (GETREG(SDRAM_STATUS) & 0x00000010);	// wait until the initialization completes (200us)

	for (i = 0; i < DRAM_SIZE / MU_MAX_BYTES; i++)
	{
		mem_set_dram(DRAM_BASE + i * MU_MAX_BYTES, 0x00000000, MU_MAX_BYTES);
	}

	#if OPTION_UART_DEBUG
	uart_init();
	uart_print("Welcome to OpenSSD");
	#endif

	SETREG(SDRAM_ECC_MON, 0xFFFFFFFF);

	// configure SDRAM interrupt
	SETREG(SDRAM_INTCTRL, SDRAM_INT_ENABLE);

	// clear interrupt flags in DRAM controller
	SETREG(SDRAM_INTSTATUS, 0xFFFFFFFF);

	// configure ICU
	SETREG(APB_ICU_CON, INTR_SATA);	// SATA = FIQ, other = IRQ
	SETREG(APB_INT_MSK, INTR_SATA | INTR_FLASH | INTR_SDRAM | INTR_TIMER_1 | INTR_TIMER_2 | INTR_TIMER_3);

	// clear interrupt flags in ICU
	SETREG(APB_INT_STS, 0xFFFFFFFF);

	flash_reset();

	SETREG(FCONF_PAUSE, 0);
	SETREG(INTR_MASK, 0);

	for (bank = 0; bank < NUM_BANKS; bank++)
	{
		flash_clear_irq();

		SETREG(FCP_CMD, FC_COL_ROW_READ_OUT);
		SETREG(FCP_OPTION, 0x06);				// FO_E
		SETREG(FCP_DMA_ADDR, g_temp_mem);
		SETREG(FCP_DMA_CNT, BYTES_PER_SECTOR);
		SETREG(FCP_COL, 0);
		SETREG(FCP_ROW_L(bank), STAMP_PAGE_OFFSET);
		SETREG(FCP_ROW_H(bank), STAMP_PAGE_OFFSET);

		flash_issue_cmd(bank, RETURN_WHEN_DONE);

		if ( (BSP_INTR(bank) & 0xFE)== 0 )
			break;
	}

	#if OPTION_FTL_TEST == FALSE
	sata_reset();
	#endif

    ftl_open();

	#if OPTION_FTL_TEST == TRUE
	extern void ftl_test();
	ftl_test();
    led(1);
    while (1);
    #endif
}
