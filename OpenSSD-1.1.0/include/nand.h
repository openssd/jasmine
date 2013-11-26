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


#ifndef	NAND_H
#define	NAND_H

// If you want to reduce the SSD capacity for debug/test purpose,
// there are a few ways to accomplish it:
// 1. Define _PBLKS_PER_BANK to be a smaller value.
// 2. Define the BANK_BMP to be a smaller configuration.
// 3. Define SECTORS_PER_PHYPAGE to be a smaller value.

#define MT29F64G08CFABA 		64	// MLC 32Gb/die x DDP = 64Gb/package, 2CE/2RB,  34nm (= MT29F32G08CBABA(mono die, 1CE/1RB), L63B
#define K9LCG08U1M				73	// MLC 32Gb/die x DDP = 64Gb/package, 2CE/2RB (= K9GBG08U0M(mono die, 1CE/1RB) = K9HDG08U5M(QDP, 4CE/4RB))

#define NAND_SPEC_SAMSUNG		0xEC
#define NAND_SPEC_TOSHIBA		0x98
#define NAND_SPEC_MICRON		0x2C
#define NAND_SPEC_HYNIX			0xAD
#define NAND_SPEC_INTEL			0x89
#define NAND_SPEC_SANDISK		0x45

#define NAND_SPEC_PLANE_1		1	// single plane
#define NAND_SPEC_PLANE_2		2	// two planes (even blocks, odd blocks)
#define NAND_SPEC_PLANE_3		3	// two planes (upper blocks, lower blocks)

#define NAND_SPEC_DIE_1			1	// [mono die] or [DDP with 2CE/2RB]
#define NAND_SPEC_DIE_2			2	// [DDP with 1CE/1RB] or [QDP with 2CE/2RB]

#define NAND_SPEC_CELL_SLC		1	// SLC
#define NAND_SPEC_CELL_MLC		2	// MLC

#define NAND_SPEC_MLC_TYPE_1	1	// MLC with lsb = msb - 6
#define NAND_SPEC_MLC_TYPE_2	2	// MLC with lsb = msb - 3
#define NAND_SPEC_MLC_TYPE_3	3	// MLC with lsb = msb - 6 (micron 34nm style)

#define NAND_SPEC_FAST			1	// nRE, nWE toggling is fast (25ns)
#define NAND_SPEC_SLOW			2	// nRE, nWE toggling is slow (30ns)
#define NAND_SPEC_DULL			3	// nRE, nWE toggling is very slow (50ns)
#define NAND_SPEC_VERY_FAST		4	// 20ns

#define NAND_SPEC_READ_CYCLE_1	1
#define NAND_SPEC_READ_CYCLE_2	2
#define NAND_SPEC_READ_CYCLE_3	3

#if FLASH_TYPE == K9LCG08U1M
	#define NAND_SPEC_ID			0x547294D7
	#define NAND_SPEC_ID_MASK		0xFFFFFFFF
	#define NAND_SPEC_MF			NAND_SPEC_SAMSUNG
	#define NAND_SPEC_PLANE 		NAND_SPEC_PLANE_2
	#define NAND_SPEC_DIE			NAND_SPEC_DIE_1
	#define NAND_SPEC_CELL			NAND_SPEC_CELL_MLC
	#define NAND_SPEC_SPEED 		NAND_SPEC_SLOW
	#define NAND_SPEC_MLC			NAND_SPEC_MLC_TYPE_2
	#define NAND_SPEC_READ			NAND_SPEC_READ_CYCLE_2
	#define SECTORS_PER_PHYPAGE 	16
	#define PAGES_PER_VBLK			128
	#define _PBLKS_PER_BANK 		4096
	#define SPARE_PBLKS_PER_BANK	56
	#define SPARE_PER_PHYPAGE		448

#elif FLASH_TYPE == MT29F64G08CFABA
	#define NAND_SPEC_ID			(0x89460468 & NAND_SPEC_ID_MASK)
	#define NAND_SPEC_ID_MASK		0xFFFF0FFF
	#define NAND_SPEC_MF			NAND_SPEC_MICRON
	#define NAND_SPEC_PLANE 		NAND_SPEC_PLANE_2
	#define NAND_SPEC_DIE			NAND_SPEC_DIE_1
	#define NAND_SPEC_CELL			NAND_SPEC_CELL_MLC
	#define NAND_SPEC_SPEED 		NAND_SPEC_FAST
	#define NAND_SPEC_MLC			NAND_SPEC_MLC_TYPE_1
	#define NAND_SPEC_READ			NAND_SPEC_READ_CYCLE_2
	#define SECTORS_PER_PHYPAGE 	8
	#define PAGES_PER_VBLK			256
	#define _PBLKS_PER_BANK 		(8192/2)
	#define SPARE_PBLKS_PER_BANK	0
	#define SPARE_PER_PHYPAGE		224

#else
	#error ("unknown flash type");
#endif

#if NAND_SPEC_PLANE == NAND_SPEC_PLANE_1
#define NUM_PLANES		1			// planes per die (NOT per package)
#else
#define NUM_PLANES		2
#endif

#if SPARE_PER_PHYPAGE / SECTORS_PER_PHYPAGE >= 28
#define OVERHEAD_PER_SECTOR		26
#define READ_REFRESH_THRESHOLD	8
#elif SPARE_PER_PHYPAGE / SECTORS_PER_PHYPAGE >= 27
#define OVERHEAD_PER_SECTOR		26
#define READ_REFRESH_THRESHOLD	8
#elif SPARE_PER_PHYPAGE / SECTORS_PER_PHYPAGE >= 16
#define OVERHEAD_PER_SECTOR		14
#define READ_REFRESH_THRESHOLD	4
#endif

#if OPTION_REDUCED_CAPACITY
// #define PBLKS_PER_BANK	((_PBLKS_PER_BANK + SPARE_PBLKS_PER_BANK) / 32) // 2GB
// #define PBLKS_PER_BANK	((_PBLKS_PER_BANK + SPARE_PBLKS_PER_BANK) / 8) // 8GB
// #define PBLKS_PER_BANK	((_PBLKS_PER_BANK + SPARE_PBLKS_PER_BANK) / 4) // 16GB
#define PBLKS_PER_BANK	((_PBLKS_PER_BANK + SPARE_PBLKS_PER_BANK) / 2) // 32GB
#else
#define PBLKS_PER_BANK	(_PBLKS_PER_BANK + SPARE_PBLKS_PER_BANK)
#endif

#endif	// NAND_H

