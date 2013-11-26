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



#ifndef FLASH_H
#define FLASH_H

#define FLASH_POLL_WR			while ((GETREG(WR_STAT) & 0x00000001) != 0)
#define FLASH_ISSUE				SETREG(FCP_ISSUE, NULL)

extern UINT8 c_bank_rmap[NUM_BANKS_MAX];

////////////////////////////////
// flash controller registers
////////////////////////////////


#define INTR_BSP				(FREG_BASE + 0x000)
#define INTR_MASK				(FREG_BASE + 0x004)
#define FCONF_NANDCFG_1			(FREG_BASE + 0x008)
#define BANK_RESET				(FREG_BASE + 0x00C)

#define FCONF_CHKCMD			(FREG_BASE + 0x00A)
#define FCONF_PAUSE				(FREG_BASE + 0x010)
#define FCONF_NANDCFG_2			(FREG_BASE + 0x014)
#define FCONF_REBDELAY			(FREG_BASE + 0x018)
#define FCONF_TIMECYCLE			(FREG_BASE + 0x01C)
#define FCONF_CPBDMABASE		(FREG_BASE + 0x020)
#define FCONF_BUFSIZE			(FREG_BASE + 0x024)
#define FCONF_COMMAND			(FREG_BASE + 0x028)
#define WR_STAT					(FREG_BASE + 0x02C)

#define WR_BANK					(FREG_BASE + 0x030)

#define CMD_ID_MAX				0x3FF		// 10 bits

#define FCP_BASE				(FREG_BASE + 0x034)
#define BSP_BASE				(FREG_BASE + 0x160)
#define SIZE_OF_BSP				48	// bytes
#define BSP_INTR_BASE			(FREG_BASE + 0x760)
#define BSP_FSM_BASE			(FREG_BASE + 0x780)
#define SCRAMSEED_BASE			(FREG_BASE + 0x780)	// 8 registers, 15 bits each
#define REMAP_BASE				(FREG_BASE + 0x7C0)

#define FCP_CMD					(FCP_BASE + 0x00)
#define FCP_BANK				(FCP_BASE + 0x04)	// 6 bits
#define FCP_OPTION				(FCP_BASE + 0x08)
#define FCP_DMA_ADDR			(FCP_BASE + 0x0C)
#define FCP_DMA_CNT				(FCP_BASE + 0x10)
#define FCP_COL					(FCP_BASE + 0x14)
#define FCP_ROW0_L				(FCP_BASE + 0x18)
#define FCP_ROW0_H				(FCP_BASE + 0x1C)
#define FCP_DST_COL				(FCP_ROW0_L + NUM_BANKS_MAX*8 + 0x00)
#define FCP_DST_ROW_L			(FCP_ROW0_L + NUM_BANKS_MAX*8 + 0x04)
#define FCP_DST_ROW_H			(FCP_ROW0_L + NUM_BANKS_MAX*8 + 0x08)
#define FCP_CMD_ID				(FCP_ROW0_L + NUM_BANKS_MAX*8 + 0x0C)	// 10 bits
#define FCP_ISSUE				(FCP_ROW0_L + NUM_BANKS_MAX*8 + 0x10)

#define REMAPBANK0				(REMAP_BASE + 0x00)
#define REMAPBANK1				(REMAP_BASE + 0x04)
#define REMAPBANK2				(REMAP_BASE + 0x08)
#define REMAPBANK3				(REMAP_BASE + 0x0C)
#define REMAPBLOCK0 			(REMAP_BASE + 0x10)
#define REMAPBLOCK1 			(REMAP_BASE + 0x14)
#define REMAPBLOCK2 			(REMAP_BASE + 0x18)
#define REMAPBLOCK3 			(REMAP_BASE + 0x1C)

#define NUM_REMAP_REG	4

#define REMAP_BANK_MASK		0x3F
#define REMAP_BANK_INVALID	0x3F
#define REMAP_BANK_HIGH		(1 << 6)
#define REMAP_BANK_LOW		(0 << 6)
#define REMAP_BANK_PLANE	(FO_P << 7)

#define BSP_CHASTAT			(FREG_BASE + 0x7A0)
#define MON_CHABANKIDLE		(FREG_BASE + 0x7A4)
#define MON_TRANCOUNT		(FREG_BASE + 0x7A8)
#define MON_DATACOUNT		(FREG_BASE + 0x7AC)
#define MON_DMACOUNT1		(FREG_BASE + 0x7B0)
#define MON_DMACOUNT2		(FREG_BASE + 0x7B4)
#define MONITOR_CTRL		(FREG_BASE + 0x7B8)
#define MONSELECT       	(FREG_BASE + 0x7BC)
#define BANK_FLAG_SEL		(FREG_BASE + 0x7E0)		// [4:0] BankFlagSel - Bank selection for Secfor flag of ECC/EDC
#define ECC_SEC_FLAG		(FREG_BASE + 0x7E4)		// [31:0] ECCSecFlag - ECC sector flag
#define EDC_SEC_FLAG		(FREG_BASE + 0x7E8)		// [31:0] EDCSecFlag - EDC sector flag
#define TAKEDATAMON			(FREG_BASE + 0x7EC)
#define FTL_READ_PTR		(FREG_BASE + 0x7F0)


/////////////////////
// FCONF_NANDCFG_2
/////////////////////

#define BCH(X)				((X) << 0)			// 0 = RS, 1 = BCH
#define DIS_COM2_SEL(X)		((X) << 1)			// 0 = 0x70, 1 = 0xFF (for FO_H and FO_L)
#define EDC_SEC_INT(X)		((X) << 2)			// 0 = page level, 1 = sector level (FIRQ_CRC_FAIL generation)
#define PG_SIZE(X)			((X) << 8)			// 0 = 2KB, 1 = 4KB, 2 = 8KB
#define BLK_SIZE(X)			((X) << 10)			// 0 = 64, 1 = 128, 2 = 256 (pages per block)
#define ECC_SIZE(X)			((X) << 12)			// 0 = 8, 1 = 12, 2 = 16 (bits correction per sector)
#define CHK_CMD(X)			((X) << 14)			// 0 = 70h, 1 = 71h/78h/F1h (NAND command for status check)
#define CHK_CODE(X)			((X) << 16)			// expected result of status check (8 bits)
#define CHK_MASK(X)			((UINT32)(X) << 24)	// bit mask for status check (8 bits)


/////////////////////
// FCONF_TIMECYCLE
/////////////////////

#define FLASH_SETUP(X)			((X) << 0)
#define FLASH_HOLD(X)			((X) << 4)

#define FLASH_ECC_ENABLE		( 1  << 7 )
#define FLASH_WHR(X)			((X) << 8)
#define FLASH_ADL(X)			((X) << 12)
#define FLASH_RHW(X)			((X) << 17)
#define FLASH_OUT_ENA_DLY(X)	((X) << 22)
#define FLASH_CLE_ALE_DLY(X)	((X) << 23)

#define FLASH_READ_CYCLE_A(X)	((X) << 24)
#define FLASH_READ_CYCLE_B(X)	((X) << 26)
#define FLASH_READ_CYCLE_C(X)	((X) << 28)
#define FLASH_READ_CYCLE_D(X)	((UINT32)(X) << 30)

#define FLASH_PARAM_SETUP_MAX		15
#define FLASH_PARAM_HOLD_MAX		7
#define FLASH_PARAM_READ_CYCLE_MAX	3

#define FLASH_PARAM_75_SETUP			(2 - 1)
#define FLASH_PARAM_75_HOLD				(2 - 1)
#define FLASH_PARAM_75_READ_CYCLE		1

#define FLASH_PARAM_TIMECYCLE_SAFE	(FLASH_SETUP(FLASH_PARAM_SETUP_MAX) | FLASH_HOLD(FLASH_PARAM_HOLD_MAX) | FLASH_WHR(15) | FLASH_ADL(31) | \
									FLASH_RHW(31) | FLASH_OUT_ENA_DLY(1) | FLASH_CLE_ALE_DLY(1) | \
									FLASH_READ_CYCLE_A(1) | FLASH_READ_CYCLE_B(1) | \
									FLASH_READ_CYCLE_C(1) | FLASH_READ_CYCLE_D(1) | FLASH_ECC_ENABLE)

//////////////////
// helper macros
//////////////////

#define _FCP_ROW_L(RBANK)			(FCP_ROW0_L + (RBANK) * 8)
#define _FCP_ROW_H(RBANK)			(FCP_ROW0_H + (RBANK) * 8)
#define _BSP_INTR(RBANK)			(*(volatile UINT8*)(BSP_INTR_BASE + (RBANK)))
#define _BSP_CMD(RBANK)				(BSP_BASE + 0x00 + SIZE_OF_BSP * (RBANK))
#define _BSP_OPTION(RBANK)			(BSP_BASE + 0x04 + SIZE_OF_BSP * (RBANK))
#define _BSP_DMA_ADDR(RBANK)		(BSP_BASE + 0x08 + SIZE_OF_BSP * (RBANK))
#define _BSP_DMA_CNT(RBANK)			(BSP_BASE + 0x0C + SIZE_OF_BSP * (RBANK))
#define _BSP_COL(RBANK)				(BSP_BASE + 0x10 + SIZE_OF_BSP * (RBANK))
#define _BSP_ROW_H(RBANK)			(BSP_BASE + 0x14 + SIZE_OF_BSP * (RBANK))
#define _BSP_ROW_L(RBANK)			(BSP_BASE + 0x18 + SIZE_OF_BSP * (RBANK))
#define _BSP_DST_COL(RBANK)			(BSP_BASE + 0x1C + SIZE_OF_BSP * (RBANK))
#define _BSP_DST_ROW_H(RBANK)		(BSP_BASE + 0x20 + SIZE_OF_BSP * (RBANK))
#define _BSP_DST_ROW_L(RBANK)		(BSP_BASE + 0x24 + SIZE_OF_BSP * (RBANK))
#define _BSP_CMD_ID(RBANK)			(BSP_BASE + 0x28 + SIZE_OF_BSP * (RBANK))
#define _BSP_ECCNUM(RBANK)			(BSP_BASE + 0x2C + SIZE_OF_BSP * (RBANK))
#define _BSP_FSM(RBANK)				(*(volatile UINT8*)(BSP_FSM_BASE + (RBANK)))
#define _CLR_BSP_INTR(RBANK, FLAG)	*(volatile UINT32*)(BSP_INTR_BASE + (RBANK)/4*4) = (FLAG) << (((RBANK)%4)*8)

#define REAL_BANK(BANK)				((UINT32)(c_bank_map[BANK]))
#define FCP_ROW_L(BANK)				_FCP_ROW_L(REAL_BANK(BANK))
#define FCP_ROW_H(BANK)				_FCP_ROW_H(REAL_BANK(BANK))
#define BSP_INTR(BANK)				_BSP_INTR(REAL_BANK(BANK))
#define BSP_CMD(BANK)				_BSP_CMD(REAL_BANK(BANK))
#define BSP_OPTION(BANK)			_BSP_OPTION(REAL_BANK(BANK))
#define BSP_DMA_ADDR(BANK)			_BSP_DMA_ADDR(REAL_BANK(BANK))
#define BSP_DMA_CNT(BANK)			_BSP_DMA_CNT(REAL_BANK(BANK))
#define BSP_COL(BANK)				_BSP_COL(REAL_BANK(BANK))
#define BSP_ROW_H(BANK)				_BSP_ROW_H(REAL_BANK(BANK))
#define BSP_ROW_L(BANK)				_BSP_ROW_L(REAL_BANK(BANK))
#define BSP_DST_COL(BANK)			_BSP_DST_COL(REAL_BANK(BANK))
#define BSP_DST_ROW_H(BANK)			_BSP_DST_ROW_H(REAL_BANK(BANK))
#define BSP_DST_ROW_L(BANK)			_BSP_DST_ROW_L(REAL_BANK(BANK))
#define BSP_CMD_ID(BANK)			_BSP_CMD_ID(REAL_BANK(BANK))
#define BSP_ECCNUM(BANK)			_BSP_ECCNUM(REAL_BANK(BANK))
#define BSP_FSM(BANK)				_BSP_FSM(REAL_BANK(BANK))
#define CLR_BSP_INTR(BANK, FLAG)	_CLR_BSP_INTR(REAL_BANK(BANK), FLAG)


typedef struct tag_fcp
{
	UINT32	cmd;
	UINT32	bank;
	UINT32	option;
	UINT32	dma_addr;
	UINT32	dma_cnt;
	UINT32	col;
	UINT32	row[NUM_BANKS_MAX][CHN_WIDTH];
	UINT32	dst_col;
	UINT32	dst_row[CHN_WIDTH];
	UINT32	cmd_id;
	UINT32	issue;
} fcp_t;


//////////////////////////////
// flash command codes
//////////////////////////////

// write operations
#define FC_COL_ROW_IN_PROG		0x01	// column address - row address - [DRAM or SRAM -> flash] - program command - wait - check result
#define FC_COL_ROW_IN			0x02	// column address - row address - [DRAM or SRAM -> flash]
#define FC_IN					0x03	//                                [DRAM or SRAM -> flash]
#define FC_IN_PROG				0x04	//                                [DRAM or SRAM -> flash] - program command - wait - check result
#define FC_PROG					0x09	//                                                          program command - wait - check result

// read operations
#define FC_COL_ROW_READ_OUT		0x0a	// column address - row address - read command - wait - [flash -> DRAM or SRAM]
#define FC_COL_ROW_READ			0x0b	// column address - row address - read command - wait
#define FC_OUT					0x0c	// [flash -> DRAM or SRAM]
#define FC_COL_OUT				0x0f	// column address change - [flash -> DRAM or SRAM]

// copyback operations
#define FC_COPYBACK				0x12	// see descriptions below
#define FC_MODIFY_COPYBACK		0x17

// others
#define FC_WAIT					0x00	// wait (used after FC_GENERIC when the command involves flash busy signal)
#define FC_ERASE				0x14	// row address - erase command - wait - check result
#define FC_GENERIC				0x15	// generic command (FCP_COL = command code)
#define FC_GENERIC_ADDR			0x16	// generic address (FCP_ROW_L and FCP_ROW_H = address, FCP_DMA_CNT = cycle count)
#define FC_READ_ID				0x10	// read_ID command - [flash -> SRAM] (FCP_OPTION = 0, FCP_DMA_ADDR = SRAM address, FCP_DMA_CNT = 16, FCP_COL = 0)

// FC_COPYBACK
// source column address - source row address - read_for_copyback command - wait
// - [flash -> DRAM copy buffer, the entire page is read for ECC correction, FCP_DMA_ADDR and FCP_DMA_CNT is ignored]
// - destination column address - destination row address
// - [DRAM copy buffer -> flash, only if ECC correction is necessary]
// - program command - wait - check result

// FC_MODIFY_COPYBACK
// In addition to ECC-corrected sectors, you can specify sectors to be modified.
// The new data for modified sectors should be in DRAM, specified by FCP_DMA_ADDR and FCP_DMA_CNT.
// FCP_DST_COL is used for specifying the sectors.


/////////////////////////////
// FCP_OPTION flags
/////////////////////////////

// Buffer management options for write operation
//
// FO_B_SATA_W
//		The hardware does not start [DRAM -> flash] until [SATA -> DRAM] completion.
//		Use this option for handling SATA write requests.
//
// FO_B_W_DRDY
//		The hardware starts [DRAM -> flash] immediately.
//		Use this option for storing a mapping table.
//
// Either FO_B_SATA_W or FO_B_W_DRDY should be used for a write operation.
// Using both FO_B_SATA_W and FO_B_W_DRDY is not allowed.

// Buffer management options for read operation
//
// FO_B_SATA_R
//		Upon completion of [flash -> DRAM], [DRAM -> SATA] will automatically start.
//		Use this option for handling SATA read requests.
//
// If FO_B_SATA_R is not specified for a read operation, [DRAM -> SATA] does not start.
//		Omit FO_B_SATA_R for reading a mapping table.

#define FO_P			(0x01 * OPTION_2_PLANE)		// 1 = use 2-plane mode, 0 = use 1-plane mode
#define FO_E			0x06						// 1 = use ECC/EDC hardware, 0 = do not use ECC/EDC hardware (see the notes below)
#define FO_SCRAMBLE		0x08						// enable data scrambler
#define FO_L			0x10						// disable LOW chip
#define FO_H			0x20						// disable HIGH chip
#define FO_B_W_DRDY		0x40
#if OPTION_FTL_TEST
#define FO_B_SATA_W		FO_B_W_DRDY
#define FO_B_SATA_R		0
#else
#define FO_B_SATA_W		0x80
#define FO_B_SATA_R		0x100
#endif

// Important notes on flash operation:
//
// 1.	Write operation:
//		The first command of a sequence should be FC_COL_ROW_IN_PROG or FC_COL_ROW_IN.
//		The sequence should start with *_COL_ROW_
//		The last command of a sequence should be FC_COL_ROW_IN_PROG, FC_IN_PROG or FC_PROG.
//		For example, the following command sequences are allowed:
//		A.	FC_COL_ROW_IN, FC_IN, FC_IN, FC_IN_PROG
//		B.	FC_COL_ROW_IN, FC_IN, FC_IN, FC_PROG
//		C.	FC_COL_ROW_IN, FC_PROG
//		D.	FC_COL_ROW_IN_PROG
//		The following command sequences are the examples of wrong ones:
//		D.	FC_COL_ROW_IN_PROG, FC_IN (_PROG should always be the last of the sequence)
//		E.	FC_COL_ROW_IN, FC_IN (_PROG should always be the last of the sequence)
//		F.	FC_IN, FC_IN_PROG (COL_ROW should always be the first of the sequence)
// 2.	Read operation:
//		The first command of a sequence should be FC_COL_ROW_READ_OUT or FC_COL_ROW_READ.
//		The sequence should start with *_COL_ROW_
//		For example, the following command sequences are allowed:
//		A.	FC_COL_ROW_READ_OUT
//		B.	FC_COL_ROW_READ_OUT, FC_COL_OUT, FC_COL_OUT
//		C.	FC_COL_ROW_READ, FC_OUT, FC_OUT
//		D.	FC_COL_ROW_READ, FC_COL_OUT
// 3.	The value of FCP_ROW should be the same for all the commands that belong to the same sequence.
//		This rule applies only when FO_P is used.
// 4.	If FO_E is not specified, the hardware does not support [flash -> DRAM] nor [DRAM -> flash].
//		Only [flash -> SRAM] and [SRAM -> flash] are supported when FO_E is not used.
// 5.	If FO_E is not specified, FCP_DMA_CNT should be less than or equal to 512.
//		If you want to read 1024 bytes from flash to SRAM, for example, you can read the first 512 bytes
//		with FC_COL_ROW_READ_OUT and the remaining 512 bytes with FC_OUT.
//		Similarly, FC_COL_ROW_IN and FC_IN_PROG can be used for writing from SRAM to flash.
// 6.	FC_COL_OUT does not allow decreasing column address. You can only increase column address.
// 7.	For commands that involve [DRAM or SRAM -> flash] or [flash -> DRAM or SRAM], FCP_COL is used for calculating
//		the memory address. The rule is
//		memory address = FCP_DMA_ADDR + FCP_COL * BYTES_PER_SECTOR
//		This rule applies to FC_OUT, FC_IN and FC_IN_PROG commands as well, event though they do not explicitly send
//		column address change request to flash.
// 8.	For commands with _IN_ or _OUT_, you should not specify FO_E = 0 and FO_P = 1
// 9.	You should not attempt to do [flash -> SRAM] and/or [SRAM -> flash] concurrently at two or more channels.

#define AUTO_SEL        0x3F


////////////////////////
// Bank Status Port
////////////////////////

typedef struct tag_bsp
{
	UINT32	cmd;
	UINT32	option;
	UINT32	dma_addr;
	UINT32	dma_cnt;
	UINT32	col;
	UINT32	row[CHN_WIDTH];
	UINT32	dst_col;
	UINT32	dst_row[CHN_WIDTH];
	UINT32	cmd_id;
	UINT32	eccnum;
} bsp_t;


//////////////////////////////
// bank FSM codes
//////////////////////////////

#define BANK_IDLE		0x0
#define BANK_GRANT		0x1
#define BANK_START		0x2
#define BANK_CHAGET		0x3
#define BANK_WAIT		0x4
#define BANK_CMDEND		0x5
#define BANK_ECCWAIT	0x6
#define BANK_TAKE		0x7
#define BANK_END		0xf

//////////////////////////////
// channel FSM codes
// BSP_CHASTAT[4:0]
//////////////////////////////

#define CHA_IDLE        0x00
#define CHA_START       0x01
#define CHA_COMMAND1    0x02
#define CHA_ADDR        0x03
#define CHA_WRITE       0x04
#define CHA_READ        0x05
#define CHA_COMMAND2    0x06
#define CHA_WAIT        0x07
#define CHA_WRITE2      0x08
#define CHA_BANKSEL     0x09
#define CHA_COM1STA     0x0a
#define CHA_COM2STA     0x0b
#define CHA_ADDRSTA     0x0c
#define CHA_WRITESTA    0x0d
#define CHA_READSTA     0x0e
#define CHA_ADDREND     0x0f
#define CHA_READNOP     0x10
#define CHA_WRITENOP    0x11
#define CHA_COM1NOP     0x12
#define CHA_ENDNOP      0x13
#define CHA_BUFREADY    0x14
#define CHA_END         0x1f

/////////////////////////
// bank interrupt flags
/////////////////////////

// The hardware does not automatically clear these flags.
// These flags can only be cleared by firmware.

#define FIRQ_CORRECTED		0x01
#define FIRQ_CRC_FAIL		0x02
#define FIRQ_MISMATCH		0x04
#define FIRQ_BADBLK_L		0x08
#define FIRQ_BADBLK_H		0x10
#define FIRQ_ALL_FF			0x20
#define FIRQ_ECC_FAIL		0x80
#define FIRQ_DATA_CORRUPT	0x82


//////////////////////////////
// flash public functions
//////////////////////////////

#define RETURN_ON_ISSUE		0
#define RETURN_ON_ACCEPT	1
#define RETURN_WHEN_DONE	2

void	flash_reset(void);
void	flash_issue_cmd(UINT32 const bank, UINT32 const sync);
void	flash_finish(void);
void	flash_copy(UINT32 const bank, UINT32 const dst_row, UINT32 const src_row);
void	flash_erase(UINT32 const bank, UINT16 const vblk_offset);
void	flash_clear_irq(void);

///////////////////////////////////////
// wrappers of flash public functions
// for beginners
///////////////////////////////////////
void nand_page_read(UINT32 const bank, UINT32 const vblock, UINT32 const page_num, UINT32 const buf_addr);
void nand_page_ptread(UINT32 const bank, UINT32 const vblock, UINT32 const page_num, UINT32 const sect_offset, UINT32 const num_sectors, UINT32 const buf_addr, UINT32 const issue_flag);
void nand_page_read_to_host(UINT32 const bank, UINT32 const vblock, UINT32 const page_num);
void nand_page_ptread_to_host(UINT32 const bank, UINT32 const vblock, UINT32 const page_num, UINT32 const sect_offset, UINT32 const num_sectors);
void nand_page_program(UINT32 const bank, UINT32 const vblock, UINT32 const page_num, UINT32 const buf_addr);
void nand_page_ptprogram(UINT32 const bank, UINT32 const vblock, UINT32 const page_num, UINT32 const sect_offset, UINT32 const num_sectors, UINT32 const buf_addr);
void nand_page_program_from_host(UINT32 const bank, UINT32 const vblock, UINT32 const page_num);
void nand_page_ptprogram_from_host(UINT32 const bank, UINT32 const vblock, UINT32 const page_num, UINT32 const sect_offset, UINT32 const num_sectors);
void nand_page_copyback(UINT32 const bank, UINT32 const src_vblock, UINT32 const src_page,
                          UINT32 const dst_vblock, UINT32 const dst_page);
void nand_page_modified_copyback(UINT32 const bank, UINT32 const src_vblock, UINT32 const src_page,
                                 UINT32 const dst_vblock, UINT32 const dst_page,
                                 UINT32 const sect_offset,
                                 UINT32 dma_addr, UINT32 const dma_count);
void nand_block_erase(UINT32 const bank, UINT32 const vblock);
void nand_block_erase_sync(UINT32 const bank, UINT32 const vblock);

#endif //FLASH_H
