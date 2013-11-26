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

#ifndef ROM_H
#define ROM_H

#define ROM_SRAM_BUF_SIZE		16384
#define ROM_DRAM_BUF_LBA		2048
#define ROM_DRAM_BUF_ADDR		(DRAM_BASE + ROM_DRAM_BUF_LBA * BYTES_PER_SECTOR)
#define	ROM_SCAN_BUF_SIZE		(100*1024)	// bad block entry size (bytes)

#define FACTORY_CMD_SIGNATURE	0x3FB82E0A

#define ROM_BYTES_PER_PAGE		2048

#define ROM_CMD_FLASH_CMD		0x02
#define ROM_CMD_FLASH_FINISH	0x01
#define ROM_CMD_MU_CMD			0x03
#define ROM_CMD_RUN_CODE		0x04
#define ROM_CMD_RESET_FLASH		0x07	// reset the flash controller - not the flash memory
#define ROM_CMD_SCAN			0x08
#define ROM_CMD_FACTORY_MODE	0x0B
#define ROM_CMD_READ_MEM		0x10
#define ROM_CMD_WRITE_MEM		0x11
#define ROM_CMD_RESET			0x12
#define ROM_CMD_DO_NOTHING		0xF0

// factory command frame

#include "sdram.h"

typedef struct
{
	UINT32	cmd;
	UINT32	subcmd;
	UINT32	key_1;

	// ROM_CMD_FLASH_CMD
	UINT32	rbank;
	UINT32	option;
	UINT32	dma_addr;
	UINT32	dma_cnt;
	UINT32	col;
	UINT32	row_l;
	UINT32	row_h;
	UINT32	dst_col;
	UINT32	dst_row_l;
	UINT32	dst_row_h;
	BOOL32	wait;

	// flash controller parameters
	UINT32	nand_cfg_2;
	UINT32	time_cycle;
	UINT32	nand_cfg_1;
	UINT32	rbbqdpselect;

	// ROM_CMD_SCAN
	UINT32	bank_bmp;
	UINT32	rows_per_blk;
	UINT32	shift_rpb;
	UINT32	blks_per_bank;
	UINT32	start_blk;

	// ROM_CMD_MU_CMD
	UINT32	mu_src_addr;
	UINT32	mu_dst_addr;
	UINT32	mu_value;
	UINT32	mu_size;

	// memory access
	UINT32	mem_addr;
	UINT32	mem_val;

	UINT32	key_2;

	// SDRAM parameters
	UINT32	sdram_signature_1;
	UINT32	sdram_signature_2_ptr;
	sdram_t	sdram;

	BOOL32	clear;

	UINT32	last_member;
} factory_cmd_t;

#endif	// ROM_H

