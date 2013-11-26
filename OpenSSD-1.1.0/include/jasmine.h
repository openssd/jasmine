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

#ifndef	JASMINE_H
#define	JASMINE_H

#define	FLASH_TYPE		K9LCG08U1M
#define	DRAM_SIZE		65075200
#define	BANK_BMP		0x00330033
#define	CLOCK_SPEED		175000000

#define	OPTION_2_PLANE			1	// 1 = 2-plane mode, 0 = 1-plane mode
#define OPTION_ENABLE_ASSERT    0	// 1 = enable ASSERT() for debugging, 0 = disable ASSERT()
#define OPTION_FTL_TEST			0	// 1 = FTL test without SATA communication, 0 = normal
#define OPTION_UART_DEBUG		0   // 1 = enable UART message output, 0 = disable
#define OPTION_SLOW_SATA		0	// 1 = SATA 1.5Gbps, 0 = 3Gbps
#define OPTION_SUPPORT_NCQ		0	// 1 = support SATA NCQ (=FPDMA) for AHCI hosts, 0 = support only DMA mode
#define OPTION_REDUCED_CAPACITY	0	// reduce the number of blocks per bank for testing purpose

#define CHN_WIDTH			2 	// 2 = 16bit IO
#define NUM_CHNLS_MAX		4
#define BANKS_PER_CHN_MAX	8
#define	NUM_BANKS_MAX		32

#include "nand.h"

/////////////////////////////////
// size	constants
/////////////////////////////////

#define	BYTES_PER_SECTOR		512

#define NUM_PSECTORS_8GB		16777216
#define NUM_PSECTORS_16GB		(NUM_PSECTORS_8GB*2)
#define NUM_PSECTORS_32GB		(NUM_PSECTORS_16GB*2)
#define NUM_PSECTORS_40GB		(NUM_PSECTORS_8GB*5)
#define NUM_PSECTORS_48GB		(NUM_PSECTORS_16GB*3)
#define	NUM_PSECTORS_64GB		(NUM_PSECTORS_32GB*2)
#define	NUM_PSECTORS_80GB		(NUM_PSECTORS_16GB*5)
#define	NUM_PSECTORS_96GB		(NUM_PSECTORS_32GB*3)
#define	NUM_PSECTORS_128GB		(NUM_PSECTORS_64GB*2)
#define	NUM_PSECTORS_160GB		(NUM_PSECTORS_32GB*5)
#define	NUM_PSECTORS_192GB		(NUM_PSECTORS_64GB*3)
#define	NUM_PSECTORS_256GB		(NUM_PSECTORS_128GB*2)
#define	NUM_PSECTORS_320GB		(NUM_PSECTORS_64GB*5)
#define	NUM_PSECTORS_384GB		(NUM_PSECTORS_128GB*3)
#define	NUM_PSECTORS_512GB		(NUM_PSECTORS_256GB*2)

#define	BYTES_PER_PAGE			(BYTES_PER_SECTOR *	SECTORS_PER_PAGE)
#define	BYTES_PER_PAGE_EXT		((BYTES_PER_PHYPAGE + SPARE_PER_PHYPAGE) * PHYPAGES_PER_PAGE)
#define BYTES_PER_PHYPAGE		(BYTES_PER_SECTOR *	SECTORS_PER_PHYPAGE)
#define	BYTES_PER_VBLK			(BYTES_PER_SECTOR *	SECTORS_PER_VBLK)
#define BYTES_PER_BANK			((UINT64) BYTES_PER_PAGE * PAGES_PER_BANK)
#define BYTES_PER_SMALL_PAGE	(BYTES_PER_PHYPAGE * CHN_WIDTH)
#if OPTION_2_PLANE
#define PHYPAGES_PER_PAGE		(CHN_WIDTH * NUM_PLANES)
#else
#define PHYPAGES_PER_PAGE		CHN_WIDTH
#endif
#define	SECTORS_PER_PAGE		(SECTORS_PER_PHYPAGE * PHYPAGES_PER_PAGE)
#define SECTORS_PER_SMALL_PAGE	(SECTORS_PER_PHYPAGE * CHN_WIDTH)
#define	SECTORS_PER_VBLK		(SECTORS_PER_PAGE *	PAGES_PER_VBLK)
#define SECTORS_PER_BANK		(SECTORS_PER_PAGE * PAGES_PER_BANK)
#define	PAGES_PER_BANK			(PAGES_PER_VBLK	* VBLKS_PER_BANK)
#define PAGES_PER_BLK           (PAGES_PER_VBLK)
#if OPTION_2_PLANE
#define VBLKS_PER_BANK			(PBLKS_PER_BANK / NUM_PLANES)
#define SPARE_VBLKS_PER_BANK	(SPARE_PBLKS_PER_BANK / NUM_PLANES)
#else
#define VBLKS_PER_BANK			PBLKS_PER_BANK
#define SPARE_VBLKS_PER_BANK	SPARE_PBLKS_PER_BANK
#endif

#define NUM_VBLKS				(VBLKS_PER_BANK * NUM_BANKS)
#define NUM_VPAGES				(PAGES_PER_VBLK * NUM_VBLKS)
#define NUM_PSECTORS			(SECTORS_PER_VBLK * ((VBLKS_PER_BANK - SPARE_VBLKS_PER_BANK) * NUM_BANKS))
#define	NUM_LPAGES				((NUM_LSECTORS + SECTORS_PER_PAGE - 1) / SECTORS_PER_PAGE)

#define ROWS_PER_PBLK			PAGES_PER_VBLK
#define ROWS_PER_BANK			(ROWS_PER_PBLK * PBLKS_PER_BANK)


////////////////////
// block 0
////////////////////

#define BYTES_PER_FW_PAGE			2048
#define SECTORS_PER_FW_PAGE			(BYTES_PER_FW_PAGE / BYTES_PER_SECTOR)
#define SCAN_LIST_PAGE_OFFSET		0
#define STAMP_PAGE_OFFSET			4
#define	FW_PAGE_OFFSET				5


///////////////
// misc
///////////////

#define	FLASH_ID_BYTES			5

// 4 byte ECC parity is appended to the end of every 128 byte data
// The amount of DRAM space that you can use is reduced.
#define	DRAM_ECC_UNIT			128

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#ifdef NULL
#undef NULL
#endif


#define	TRUE		1
#define	FALSE		0
#define	NULL		0
#define	OK			TRUE
#define	FAIL		FALSE
#define INVALID     0xABABABAB // use ftl_faster
#define	INVALID8	((UINT8) -1)
#define	INVALID16	((UINT16) -1)
#define	INVALID32	((UINT32) -1)

typedef	unsigned char		BOOL8;
typedef	unsigned short		BOOL16;
typedef	unsigned int		BOOL32;
typedef	unsigned char		UINT8;
typedef	unsigned short		UINT16;
typedef	unsigned int		UINT32;
typedef unsigned long long	UINT64;

#define MIN(X, Y)				((X) > (Y) ? (Y) : (X))
#define MAX(X, Y)				((X) > (Y) ? (X) : (Y))

void delay(UINT32 const count);

#include "flash.h"
#include "sata.h"
#include "sata_cmd.h"
#include "sata_registers.h"
#include "mem_util.h"
#include "target.h"
#include "bank.h"


//////////////
// scan list
//////////////

#define SCAN_LIST_SIZE				BYTES_PER_SMALL_PAGE
#define SCAN_LIST_ITEMS				((SCAN_LIST_SIZE / sizeof(UINT16)) - 1)

typedef struct
{
	UINT16	num_entries;
	UINT16	list[SCAN_LIST_ITEMS];
}scan_list_t;

// original
// #define NUM_LSECTORS	(21168 + ((NUM_PSECTORS) / 2097152 * 1953504)) // 125045424, 9172304(provisioning ratio: 7.3%)

// #define NUM_LSECTORS	(NUM_PSECTORS  / 100 * 86) // 14% provisioning
#define NUM_LSECTORS	(NUM_PSECTORS  / 100 * 93) // 7% provisioning

#include "ftl.h"
#include "misc.h"

#ifndef PROGRAM_INSTALLER
#include "uart.h"
#endif

#endif	// JASMINE_H

