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
//
// FASTer FTL header file
//
// Author; Sang-Phil Lim (SKKU VLDB Lab.)
//
// Reference;
//   - Sang-Phil Lim, Sang-Won Lee and Bongki Moon, "FASTer FTL for Enterprise-Class Flash Memory SSDs"
//     IEEE SNAPI 2010: 6th IEEE International Workshop on Storage Network Architecture and Parallel I/Os, May 2010
//

#ifndef FTL_H
#define FTL_H

/////////////////
// DRAM buffers
/////////////////

#define NUM_RW_BUFFERS		((DRAM_SIZE - DRAM_BYTES_OTHER) / BYTES_PER_PAGE - 1)
#define NUM_RD_BUFFERS		(((NUM_RW_BUFFERS / 8) + NUM_BANKS - 1) / NUM_BANKS * NUM_BANKS)
#define NUM_WR_BUFFERS		(NUM_RW_BUFFERS - NUM_RD_BUFFERS)
#define NUM_COPY_BUFFERS	NUM_BANKS_MAX
#define NUM_FTL_BUFFERS		NUM_BANKS
#define NUM_HIL_BUFFERS		1
#define NUM_TEMP_BUFFERS	1

#define DRAM_BYTES_OTHER	((NUM_COPY_BUFFERS + NUM_FTL_BUFFERS + NUM_HIL_BUFFERS + NUM_TEMP_BUFFERS) * BYTES_PER_PAGE + BAD_BLK_BMP_BYTES \
                             + FTL_BMT_BYTES + HASH_BUCKET_BYTES + HASH_NODE_BYTES + VC_BITMAP_BYTES + SC_BITMAP_BYTES + BLK_ERASE_CNT_BYTES)

#define WR_BUF_PTR(BUF_ID)	(WR_BUF_ADDR + ((UINT32)(BUF_ID)) * BYTES_PER_PAGE)
#define WR_BUF_ID(BUF_PTR)	((((UINT32)BUF_PTR) - WR_BUF_ADDR) / BYTES_PER_PAGE)
#define RD_BUF_PTR(BUF_ID)	(RD_BUF_ADDR + ((UINT32)(BUF_ID)) * BYTES_PER_PAGE)
#define RD_BUF_ID(BUF_PTR)	((((UINT32)BUF_PTR) - RD_BUF_ADDR) / BYTES_PER_PAGE)

#define _COPY_BUF(RBANK)	(COPY_BUF_ADDR + (RBANK) * BYTES_PER_PAGE)
#define COPY_BUF(BANK)		_COPY_BUF(REAL_BANK(BANK))
#define FTL_BUF(BANK)       (FTL_BUF_ADDR + ((BANK) * BYTES_PER_PAGE))

///////////////////////////////
// DRAM segmentation
///////////////////////////////

#define RD_BUF_ADDR			DRAM_BASE										// base address of SATA read buffers
#define RD_BUF_BYTES		(NUM_RD_BUFFERS * BYTES_PER_PAGE)

#define WR_BUF_ADDR			(RD_BUF_ADDR + RD_BUF_BYTES)					// base address of SATA write buffers
#define WR_BUF_BYTES		(NUM_WR_BUFFERS * BYTES_PER_PAGE)

#define COPY_BUF_ADDR		(WR_BUF_ADDR + WR_BUF_BYTES)					// base address of flash copy buffers
#define COPY_BUF_BYTES		(NUM_COPY_BUFFERS * BYTES_PER_PAGE)

#define FTL_BUF_ADDR		(COPY_BUF_ADDR + COPY_BUF_BYTES)				// a buffer dedicated to FTL internal purpose
#define FTL_BUF_BYTES		(NUM_FTL_BUFFERS * BYTES_PER_PAGE)

#define HIL_BUF_ADDR		(FTL_BUF_ADDR + FTL_BUF_BYTES)					// a buffer dedicated to HIL internal purpose
#define HIL_BUF_BYTES		(NUM_HIL_BUFFERS * BYTES_PER_PAGE)

#define TEMP_BUF_ADDR		(HIL_BUF_ADDR + HIL_BUF_BYTES)					// general purpose buffer
#define TEMP_BUF_BYTES		(NUM_TEMP_BUFFERS * BYTES_PER_PAGE)

#define BAD_BLK_BMP_ADDR	(TEMP_BUF_ADDR + TEMP_BUF_BYTES)				// bitmap of initial bad blocks
#define BAD_BLK_BMP_BYTES	(((NUM_VBLKS / 8) + DRAM_ECC_UNIT - 1) / DRAM_ECC_UNIT * DRAM_ECC_UNIT)

/******** FTL metadata ********/

// static hash library for FASTer FTL
#include "shashtbl.h"

#define HASH_BUCKET_SIZE    ((LOG_BLK_PER_BANK * PAGES_PER_BLK) >> 2)
#define HASH_NODE_BYTES_PER_BANK ((LOG_BLK_PER_BANK + ISOL_BLK_PER_BANK) * PAGES_PER_BLK * sizeof(hashnode))
#define HASH_BUCKET_BYTES_PER_BANK (HASH_BUCKET_SIZE * sizeof(hashnode_ptr))

//------------------------------
// 1. address mapping information
//------------------------------
// map block mapping table
// NOTE:
//   vbn #0 : super block
// misc blk
//   vbn #1: maintain misc. DRAM metadata
// map blk
//   vbn #2: maintain data/log/isol/free BMT
//   vbn #3: maintain log page mapping hash table
//   vbn #4: bitmap info, block age
#define MAP_BLK_PER_BANK    3
#define NUM_MAP_BLK         (MAP_BLK_PER_BANK * NUM_BANKS)

// total BMT bytes
#define FTL_BMT_BYTES       ((DATA_BMT_BYTES + LOG_BMT_BYTES + ISOL_BMT_BYTES + FREE_BMT_BYTES + BYTES_PER_SECTOR - 1) / BYTES_PER_SECTOR * BYTES_PER_SECTOR)

// data block mapping table
#define NUM_DATA_BLK        ((NUM_LPAGES + PAGES_PER_BLK - 1) / PAGES_PER_BLK)
#define DATA_BLK_PER_BANK   ((NUM_DATA_BLK + NUM_BANKS - 1) / NUM_BANKS)
#define DATA_BMT_ADDR       (BAD_BLK_BMP_ADDR + BAD_BLK_BMP_BYTES)
#define DATA_BMT_BYTES      ((NUM_BANKS * DATA_BLK_PER_BANK * sizeof(UINT16) + DRAM_ECC_UNIT - 1) / DRAM_ECC_UNIT * DRAM_ECC_UNIT)

// log block mapping table
#define NUM_LOG_BLK         (LOG_BLK_PER_BANK * NUM_BANKS)
#define LOG_BLK_PER_BANK    (VBLKS_PER_BANK - 1 - 1 - MAP_BLK_PER_BANK - DATA_BLK_PER_BANK - ISOL_BLK_PER_BANK - FREE_BLK_PER_BANK)
#define LOG_BMT_ADDR        (DATA_BMT_ADDR + DATA_BMT_BYTES)
#define LOG_BMT_BYTES       ((NUM_BANKS * LOG_BLK_PER_BANK * sizeof(UINT16) + DRAM_ECC_UNIT - 1) / DRAM_ECC_UNIT * DRAM_ECC_UNIT)

// isolation block mapping table
#define RESERV_ISOL_BLK     2
#define ISOL_BLK_PER_BANK   ((VBLKS_PER_BANK - DATA_BLK_PER_BANK) / 5) // 20% of log space
#if (ISOL_BLK_PER_BANK < 8)
#undef ISOL_BLK_PER_BANK
#define ISOL_BLK_PER_BANK   8
#endif
#define ISOL_BMT_ADDR       (LOG_BMT_ADDR + LOG_BMT_BYTES)
#define ISOL_BMT_BYTES      ((NUM_BANKS * ISOL_BLK_PER_BANK * sizeof(UINT16) + DRAM_ECC_UNIT - 1) / DRAM_ECC_UNIT * DRAM_ECC_UNIT)

// free block mapping table
#define FREE_BLK_PER_BANK   2
#define NUM_FREE_BLK        (FREE_BLK_PER_BANK * NUM_BANKS)
#define FREE_BMT_ADDR       (ISOL_BMT_ADDR + ISOL_BMT_BYTES)
#define FREE_BMT_BYTES      ((NUM_FREE_BLK * sizeof(UINT16) + DRAM_ECC_UNIT - 1) / DRAM_ECC_UNIT * DRAM_ECC_UNIT)

// log page mapping table (linear array structure) - NOT USED
//
// NOTE;
//  - Basically, whenever we need to find the position(i.e., log LPN) of the target LPN(i.e., data LPN) in log blocks,
//    we linearly search the log page mapping table. (e.g., new data write op., read op., merge op., etc.)
//    However, if the storage's capacity is getting larger, it might take a long time even we use Memory utility H/W engine (max. 180us to search 32KB range).
//    Thus, Alternatively we adopt the static hash structure for log page mapping table although the memory requirement is quite increased (almost 3 times).
//
// #define LOG_PMT_ADDR		(FREE_BMT_ADDR + FREE_BMT_BYTES)
// #define LOG_PMT_BYTES	((((LOG_BLK_PER_BANK + ISOL_BLK_PER_BANK) * PAGES_PER_BLK * NUM_BANKS * sizeof(UINT32)) + BYTES_PER_SECTOR - 1) / BYTES_PER_SECTOR * BYTES_PER_SECTOR)

// log page mapping table (static hash structure)
#define HASH_BUCKET_ADDR (FREE_BMT_ADDR + FREE_BMT_BYTES)
#define HASH_BUCKET_BYTES ((NUM_BANKS * HASH_BUCKET_BYTES_PER_BANK + DRAM_ECC_UNIT - 1) / DRAM_ECC_UNIT * DRAM_ECC_UNIT)

#define HASH_NODE_ADDR (HASH_BUCKET_ADDR + HASH_BUCKET_BYTES)
#define HASH_NODE_BYTES ((NUM_BANKS * HASH_NODE_BYTES_PER_BANK + DRAM_ECC_UNIT - 1) / DRAM_ECC_UNIT * DRAM_ECC_UNIT)

//--------------------------------------
// 2. additional FTL metadata
//--------------------------------------
// validation check bitmap table
//
// NOTE;
//   - To figure out that the valid page of target LPN is in log blocks or not , we just check this bit information.
//     If the bit of target LPN is set, we can obviously know the up-to-date data is existed in log blocks despite not acessing log page mapping table.
//
#define VC_BITMAP_ADDR		(HASH_NODE_ADDR + HASH_NODE_BYTES)
#define VC_BITMAP_BYTES		(((NUM_BANKS * DATA_BLK_PER_BANK * PAGES_PER_BLK / 8) + BYTES_PER_SECTOR - 1) / BYTES_PER_SECTOR * BYTES_PER_SECTOR)

// second chance bitmap table
#define SC_BITMAP_ADDR      (VC_BITMAP_ADDR + VC_BITMAP_BYTES)
#define SC_BITMAP_BYTES     (((NUM_BANKS * LOG_BLK_PER_BANK * PAGES_PER_BLK / 8) + DRAM_ECC_UNIT - 1) / DRAM_ECC_UNIT * DRAM_ECC_UNIT)

// block wear-out count for explicit wear-leveling
#define BLK_ERASE_CNT_ADDR  (SC_BITMAP_ADDR + SC_BITMAP_BYTES)
#define BLK_ERASE_CNT_BYTES ((NUM_BANKS * VBLKS_PER_BANK * sizeof(UINT32) + DRAM_ECC_UNIT - 1) / DRAM_ECC_UNIT * DRAM_ECC_UNIT)

// non-volatile metadata structure (SRAM)
typedef struct _ftl_statistics
{
    UINT32 gc_cnt;
    UINT32 total_hostwrite_cnt; // page-level
    UINT32 total_erase_cnt;
    UINT32 total_write_cnt;
    UINT32 total_cpback_cnt;
    UINT32 total_read_cnt;
}ftl_statistics;

typedef struct _misc_metadata
{
    UINT32 cur_miscblk_vpn; // vblock #1 (fixed block)
    UINT32 cur_mapblk_vpn[MAP_BLK_PER_BANK];
    UINT32 cur_write_swlog_offset;
    UINT32 swlog_data_lbn; // assigned data lbn for sw log block
    UINT32 corr_ref[2]; // correlated reference (for sequential write detection)

    UINT32 actual_rwlogblk_num;
    UINT32 cur_write_rwlog_lpn;
    UINT32 cur_write_isol_lpn;
    UINT32 cur_vt_isol_lpn;
    UINT32 rwlog_free_blk_cnt;
    UINT32 isol_free_blk_cnt;
    UINT32 free_blk_cnt;
    UINT32 free_list_head;
    UINT32 free_list_tail;

    // for progressive merge to provide uniform reponse time and performance flucatuation
    UINT32 write_before_pmerge;
    UINT32 pmerge_trigger;
    UINT32 pmerge_interval;
    UINT32 base_pmerge_interval;

    UINT32 lpn_list_of_rwlog_blk[PAGES_PER_BLK];    // read lpn list from last page of block to merge/gc
    UINT32 lpn_list_of_cur_isol_blk[PAGES_PER_BLK]; // logging lpn list of current isol blk
    UINT32 lpn_list_of_vt_isol_blk[PAGES_PER_BLK];  // read lpn list of victim isol blk
}misc_metadata; // per bank

///////////////////////////////
// FTL public functions
///////////////////////////////

void ftl_open(void);
void ftl_read(UINT32 const lba, UINT32 const num_sectors);
void ftl_write(UINT32 const lba, UINT32 const num_sectors);
void ftl_test_write(UINT32 const lba, UINT32 const num_sectors);
void ftl_flush(void);
void ftl_isr(void);

#endif //FTL_H
