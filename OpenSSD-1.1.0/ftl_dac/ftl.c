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
// DAC FTL source file
//
//  - DAC(Dynamic dAta Clustering) for hot/cold separtion on the page mapping FTL
//  - Cost-benefit GC Policy
//
// Author; Sang-Phil Lim (SKKU VLDB Lab., Korea)
//
// Features;
//
//  - normal POR support
//
// NOTE;
//
//  - If our implementation is wrong, please report the issue at the `forum' page in OpenSSD project homepage.
//  - In this implementation, we don't use a time threshold parameter for determining 'young' page.
//    (Please refer to the result of Figure 8 in the paper.)
//
// Reference;
//
//  - Mei-Ling Chiang et al, "Using Data Clustering to Improve Cleaning Performance for Flash Memory", Software-Practice and Experience, 1999.
//    (c.f. 'segment' denotes the phyiscal block (erase unit), and 'data block(block)' denotes the page (IO unit) in a large block NAND Flash)
//

#include "jasmine.h"

//----------------------------------
// macro
//----------------------------------
// two tunable parameters
#define NUM_REGION          4 // the number of regions
/* #define WRITE_INTERVAL      500 // use write interval to determine 'young' page. */

// block classification with VC(valid count)
#define VC_FREE             0xAAAA // usable block to write new data
#define VC_MAX              0xABBB // meta block, bad block and temporal GC block
#define VC_ACTIVE           0xACCC // active block for specific region
#define NOT_FOR_VICTIM      0xA000 // these blocks must not be selected as a victim in GC period

#define MISCBLK_VBN         0x1 // vblock #1 <- misc metadata
#define MAPBLKS_PER_BANK    2
#define META_BLKS_PER_BANK  (1 + 1 + MAPBLKS_PER_BANK) // include block #0, misc block

// the number of sectors of misc. metadata info.
#define NUM_MISC_META_SECT  ((sizeof(misc_metadata) + BYTES_PER_SECTOR - 1)/ BYTES_PER_SECTOR)
#define NUM_VCOUNT_SECT     ((VBLKS_PER_BANK * sizeof(UINT16) + BYTES_PER_SECTOR - 1) / BYTES_PER_SECTOR)

//----------------------------------
// metadata structure
//----------------------------------
typedef struct _ftl_statistics
{
    UINT32 gc_cnt;
    UINT32 page_wcount; // page write count
}ftl_statistics;

typedef struct _region_table
{
    UINT32 cur_write_vpn; // current write vpn per region
    UINT32 lpn_list_of_cur_vblock[PAGES_PER_BLK]; // logging lpn list of current write vblock for GC
    UINT32 vcount_of_active_vblock;
}region_table;

typedef struct _misc_metadata
{
/*     UINT32 cur_write_vpn; // physical page for new write */
    UINT32 cur_miscblk_vpn; // current write vpn for logging the misc. metadata
    UINT32 cur_mapblk_vpn[MAPBLKS_PER_BANK]; // current write vpn for logging the age mapping info.
    UINT32 lpn_list_of_vt_vblock[PAGES_PER_BLK]; // using in GC period

    UINT32 free_blk_cnt; // remaining free block count per bank
    UINT32 gc_vblock;
    UINT32 global_age; // monotonically increased when new page write occured in each bank

    region_table region_tbl[NUM_REGION]; // region table
}misc_metadata; // per bank

//----------------------------------
// FTL metadata (maintain in SRAM)
//----------------------------------
static misc_metadata  g_misc_meta[NUM_BANKS];
static ftl_statistics g_ftl_statistics[NUM_BANKS];
static BOOL32         g_bsp_isr_flag[NUM_BANKS];
static UINT32		  g_bad_blk_count[NUM_BANKS];

// SATA read/write buffer pointer id
UINT32 				  g_ftl_read_buf_id;
UINT32 				  g_ftl_write_buf_id;

//----------------------------------
// NAND layout
//----------------------------------
// block #0: scan list, firmware binary image, etc.
// block #1: FTL misc. metadata, vcount
// block #2: page mapping table (map block #0)
// block #3: block region & age info. (map block #1)
// block #4: a free block for gc
// block #36~: user data blocks

//----------------------------------
// macro functions
//----------------------------------
#define get_global_age(bank)    (g_misc_meta[bank].global_age)
#define inc_global_age(bank)    (g_misc_meta[bank].global_age++)

#define is_full_all_blks(bank)  (g_misc_meta[bank].free_blk_cnt <= NUM_REGION)
#define inc_full_blk_cnt(bank)  (g_misc_meta[bank].free_blk_cnt--)
#define dec_full_blk_cnt(bank)  (g_misc_meta[bank].free_blk_cnt++)
#define inc_mapblk_vpn(bank, mapblk_lbn)    (g_misc_meta[bank].cur_mapblk_vpn[mapblk_lbn]++)
#define inc_miscblk_vpn(bank)               (g_misc_meta[bank].cur_miscblk_vpn++)

// page-level striping technique (I/O parallelism)
#define get_num_bank(lpn)                          ((lpn) % NUM_BANKS)
#define get_bad_blk_cnt(bank)                      (g_bad_blk_count[bank])
#define get_cur_write_vpn_of_region(bank, region)  (g_misc_meta[bank].region_tbl[region].cur_write_vpn)
#define inc_cur_write_vpn_of_region(bank, region)  (g_misc_meta[bank].region_tbl[region].cur_write_vpn++)
#define set_new_write_vpn_of_region(bank, region, new_vpn) (g_misc_meta[bank].region_tbl[region].cur_write_vpn = new_vpn)
#define set_lpn_of_region(bank, region, page_num, lpn)  (g_misc_meta[bank].region_tbl[region].lpn_list_of_cur_vblock[page_num] = lpn)
#define get_lpn_of_region(bank, region, page_num)       (g_misc_meta[bank].region_tbl[region].lpn_list_of_cur_vblock[page_num])
#define get_lpn_of_vt_vblock(bank, page_num)            (g_misc_meta[bank].lpn_list_of_vt_vblock[page_num])

#define inc_vcount_of_active_block(bank, region_num) (g_misc_meta[bank].region_tbl[region_num].vcount_of_active_vblock++)
#define dec_vcount_of_active_block(bank, region_num) (g_misc_meta[bank].region_tbl[region_num].vcount_of_active_vblock--)
#define set_vcount_of_active_block(bank, region_num, vcount) (g_misc_meta[bank].region_tbl[region_num].vcount_of_active_vblock = vcount)
#define get_vcount_of_active_block(bank, region_num) (g_misc_meta[bank].region_tbl[region_num].vcount_of_active_vblock)

#define get_miscblk_vpn(bank)                 (g_misc_meta[bank].cur_miscblk_vpn)
#define set_miscblk_vpn(bank, vpn)            (g_misc_meta[bank].cur_miscblk_vpn = vpn)
#define get_mapblk_vpn(bank, mapblk_lbn)      (g_misc_meta[bank].cur_mapblk_vpn[mapblk_lbn])
#define set_mapblk_vpn(bank, mapblk_lbn, vpn) (g_misc_meta[bank].cur_mapblk_vpn[mapblk_lbn] = vpn)

#define get_gc_vblock(bank)           (g_misc_meta[bank].gc_vblock)
#define set_gc_vblock(bank, vblock)   (g_misc_meta[bank].gc_vblock = vblock)

#define CHECK_LPAGE(lpn)              ASSERT((lpn) < NUM_LPAGES)
#define CHECK_VPAGE(vpn)              ASSERT((vpn) < (VBLKS_PER_BANK * PAGES_PER_BLK))

//----------------------------------
// FTL internal function prototype
//----------------------------------
static void   format(void);
static void   set_bad_block(UINT32 const bank, UINT32 const vblk_offset);
static void   write_format_mark(void);
static void   sanity_check(void);
static void   init_metadata_sram(void);
static void   load_metadata(void);
static void   load_misc_metadata(void);
static void   load_dram_metadata_from_mapblk(UINT32 const mapblk_lbn, UINT32 const dram_addr, UINT32 const dram_bytes);
static void   flush_misc_metadata(void);
static void   flush_dram_metadata_into_mapblk(UINT32 const mapblk_lbn, UINT32 const dram_addr, UINT32 const dram_bytes);
static void   write_page(UINT32 const lpn, UINT32 const sect_offset, UINT32 const num_sectors);
static void   set_vpn(UINT32 const lpn, UINT32 const vpn);
static void   garbage_collection(UINT32 const bank);
static void   set_vcount(UINT32 const bank, UINT32 const vblock, UINT32 const vcount);
static void   inc_vcount(UINT32 const bank, UINT32 const vblock);
static void   set_region_num(UINT32 const bank, UINT32 const vblock, UINT32 const region_num);
static void   set_age_of_vblock(UINT32 const bank, UINT32 const vblock, UINT32 const age);
static UINT32 get_vcount(UINT32 const bank, UINT32 const vblock);
static UINT32 get_vpn(UINT32 const lpn);
static UINT32 get_vt_vblock_by_cost_benefit(UINT32 const bank);
static UINT32 get_vt_vblock_by_greedy(UINT32 const bank);
static UINT32 get_region_num(UINT32 const bank, UINT32 const vblock);
static UINT32 get_age_of_vblock(UINT32 const bank, UINT32 const vblock);
static UINT32 assign_new_write_vpn(UINT32 const bank, UINT32 const lpn, UINT32 const old_vpn, UINT32* region_num_for_new_write);
static BOOL32 is_bad_block(UINT32 const bank, UINT32 const vblock);
static BOOL32 check_format_mark(void);
/* static void   set_age_of_lpn(UINT32 const bank, UINT32 const lpn, UINT32 const age); */
/* static UINT32 get_age_of_lpn(UINT32 const bank, UINT32 const lpn); */
/* static BOOL32 is_young_page(UINT32 const bank, UINT32 const lpn); */

static void sanity_check(void)
{
    UINT32 dram_requirement = RD_BUF_BYTES + WR_BUF_BYTES + COPY_BUF_BYTES + FTL_BUF_BYTES
        + HIL_BUF_BYTES + TEMP_BUF_BYTES + BAD_BLK_BMP_BYTES
        + PAGE_MAP_BYTES + VCOUNT_BYTES + VBLK_AGE_BYTES + VBLK_REGION_BYTES;

    if ((dram_requirement > DRAM_SIZE) || // DRAM metadata size check
        (sizeof(misc_metadata) > BYTES_PER_PAGE)) // misc metadata size check
    {
        led_blink();
        while (1);
    }
}
static void build_bad_blk_list(void)
{
	UINT32 bank, num_entries, result, vblk_offset;
	scan_list_t* scan_list = (scan_list_t*) TEMP_BUF_ADDR;

	mem_set_dram(BAD_BLK_BMP_ADDR, NULL, BAD_BLK_BMP_BYTES);

	disable_irq();

	flash_clear_irq();

	for (bank = 0; bank < NUM_BANKS; bank++)
	{
		SETREG(FCP_CMD, FC_COL_ROW_READ_OUT);
		SETREG(FCP_BANK, REAL_BANK(bank));
		SETREG(FCP_OPTION, FO_E);
		SETREG(FCP_DMA_ADDR, (UINT32) scan_list);
		SETREG(FCP_DMA_CNT, SCAN_LIST_SIZE);
		SETREG(FCP_COL, 0);
		SETREG(FCP_ROW_L(bank), SCAN_LIST_PAGE_OFFSET);
		SETREG(FCP_ROW_H(bank), SCAN_LIST_PAGE_OFFSET);

		SETREG(FCP_ISSUE, NULL);
		while ((GETREG(WR_STAT) & 0x00000001) != 0);
		while (BSP_FSM(bank) != BANK_IDLE);

		num_entries = NULL;
		result = OK;

		if (BSP_INTR(bank) & FIRQ_DATA_CORRUPT)
		{
			result = FAIL;
		}
		else
		{
			UINT32 i;

			num_entries = read_dram_16(&(scan_list->num_entries));

			if (num_entries > SCAN_LIST_ITEMS)
			{
				result = FAIL;
			}
			else
			{
				for (i = 0; i < num_entries; i++)
				{
					UINT16 entry = read_dram_16(scan_list->list + i);
					UINT16 pblk_offset = entry & 0x7FFF;

					if (pblk_offset == 0 || pblk_offset >= PBLKS_PER_BANK)
					{
						#if OPTION_REDUCED_CAPACITY == FALSE
						result = FAIL;
						#endif
					}
					else
					{
						write_dram_16(scan_list->list + i, pblk_offset);
					}
				}
			}
		}

		if (result == FAIL)
		{
			num_entries = 0;
		}
		else
		{
			write_dram_16(&(scan_list->num_entries), 0);
        }

		g_bad_blk_count[bank] = 0;

		for (vblk_offset = 1; vblk_offset < VBLKS_PER_BANK; vblk_offset++)
		{
			BOOL32 bad = FALSE;

			#if OPTION_2_PLANE
			{
				UINT32 pblk_offset;

				pblk_offset = vblk_offset * NUM_PLANES;

                // fix bug@v.1.1.0
				if (mem_search_equ_dram(scan_list, sizeof(UINT16), num_entries + 1, pblk_offset) < num_entries + 1)
				{
					bad = TRUE;
				}

				pblk_offset = vblk_offset * NUM_PLANES + 1;

                // fix bug@v.1.1.0
				if (mem_search_equ_dram(scan_list, sizeof(UINT16), num_entries + 1, pblk_offset) < num_entries + 1)
				{
					bad = TRUE;
				}
			}
			#else
			{
                // fix bug@v.1.1.0
				if (mem_search_equ_dram(scan_list, sizeof(UINT16), num_entries + 1, vblk_offset) < num_entries + 1)
				{
					bad = TRUE;
				}
			}
			#endif

			if (bad)
			{
				g_bad_blk_count[bank]++;
				set_bit_dram(BAD_BLK_BMP_ADDR + bank*(VBLKS_PER_BANK/8 + 1), vblk_offset);
			}
		}
	}
}

void ftl_open(void)
{
    /* UINT32 volatile g_break = 0; */
    /* while (g_break == 0); */

	led(0);
    sanity_check();
    //----------------------------------------
    // read scan lists from NAND flash
    // and build bitmap of bad blocks
    //----------------------------------------
	build_bad_blk_list();

    // This example FTL can handle runtime bad block interrupts and read fail (uncorrectable bit errors) interrupts
    flash_clear_irq();

    SETREG(INTR_MASK, FIRQ_DATA_CORRUPT | FIRQ_BADBLK_L | FIRQ_BADBLK_H);
	SETREG(FCONF_PAUSE, FIRQ_DATA_CORRUPT | FIRQ_BADBLK_L | FIRQ_BADBLK_H);

	enable_irq();
    //----------------------------------------
	// If necessary, do low-level format
	// format() should be called after loading scan lists, because format() calls is_bad_block().
    //----------------------------------------
	if (check_format_mark() == FALSE) {
/* 	if (TRUE) { */
        uart_print("do format");
		format();
        uart_print("end format");
	}
    // load FTL metadata
    else {
        load_metadata();
    }
	g_ftl_read_buf_id = 0;
	g_ftl_write_buf_id = 0;

}
static void format(void)
{
    UINT32 bank, vblock, vcount_val;

    ASSERT(NUM_MISC_META_SECT > 0);
    ASSERT(NUM_VCOUNT_SECT > 0);

    uart_printf("Total FTL DRAM metadata size: %d KB", DRAM_BYTES_OTHER / 1024);

    uart_printf("VBLKS_PER_BANK: %d", VBLKS_PER_BANK);
    uart_printf("LBLKS_PER_BANK: %d", NUM_LPAGES / PAGES_PER_BLK / NUM_BANKS);
    uart_printf("META_BLKS_PER_BANK: %d", META_BLKS_PER_BANK);

    //----------------------------------------
    // initialize DRAM metadata
    //----------------------------------------
    mem_set_dram(PAGE_MAP_ADDR, NULL, PAGE_MAP_BYTES);
    mem_set_dram(VCOUNT_ADDR, NULL, VCOUNT_BYTES);
    mem_set_dram(VBLK_REGION_ADDR, NULL, VBLK_REGION_BYTES); // init all region to 0
    mem_set_dram(VBLK_AGE_ADDR, NULL, VBLK_AGE_BYTES); // init vblock's age information
/*     mem_set_dram(PAGE_AGE_ADDR, NULL, PAGE_AGE_BYTES); // init page's age information - NOT USED */

    //----------------------------------------
    // erase all blocks except vblock #0
    //----------------------------------------
	for (vblock = MISCBLK_VBN; vblock < VBLKS_PER_BANK; vblock++) {
		for (bank = 0; bank < NUM_BANKS; bank++) {
            g_bsp_isr_flag[bank] = INVALID;
            vcount_val = VC_MAX;
            if (is_bad_block(bank, vblock) == FALSE) {
				nand_block_erase_sync(bank, vblock);

                if (g_bsp_isr_flag[bank] != INVALID) {
                    set_bad_block(bank, g_bsp_isr_flag[bank]);
                    g_bsp_isr_flag[bank] = INVALID;
                }
                else {
                    vcount_val = VC_FREE; // usable vblocks is set to VC_FREE
                }
            }
            // bad block is set to VC_MAX
            write_dram_16(VCOUNT_ADDR + ((bank * VBLKS_PER_BANK) + vblock) * sizeof(UINT16),
                          vcount_val);
        }
    }
    //----------------------------------------
    // initialize SRAM metadata
    //----------------------------------------
    init_metadata_sram();

    // flush metadata to NAND
    ftl_flush();

    write_format_mark();
	led(1);
    uart_print("format complete");
}
static void init_metadata_sram(void)
{
    UINT32 bank;
    UINT32 vblock;
    UINT32 mapblk_lbn;

    //----------------------------------------
    // initialize misc. metadata
    //----------------------------------------
    for (bank = 0; bank < NUM_BANKS; bank++) {
        g_misc_meta[bank].free_blk_cnt  = VBLKS_PER_BANK - META_BLKS_PER_BANK;
        g_misc_meta[bank].free_blk_cnt -= get_bad_blk_cnt(bank);

        // NOTE: vblock #0,1 don't use for user space
        write_dram_16(VCOUNT_ADDR + ((bank * VBLKS_PER_BANK) + 0) * sizeof(UINT16), VC_MAX);
        write_dram_16(VCOUNT_ADDR + ((bank * VBLKS_PER_BANK) + 1) * sizeof(UINT16), VC_MAX);

        //----------------------------------------
        // assign misc. block (vblock #1 = fixed location)
        //----------------------------------------
        set_miscblk_vpn(bank, MISCBLK_VBN * PAGES_PER_BLK);
        ASSERT(is_bad_block(bank, MISCBLK_VBN) == FALSE);

        vblock = MISCBLK_VBN;

        //----------------------------------------
        // assign map block
        //----------------------------------------
        mapblk_lbn = 0;
        while (mapblk_lbn < MAPBLKS_PER_BANK) {
            vblock++;
            ASSERT(vblock < VBLKS_PER_BANK);
            if (is_bad_block(bank, vblock) == FALSE) {
                set_mapblk_vpn(bank, mapblk_lbn, vblock * PAGES_PER_BLK);
                // set meta blocks as VC_MAX
                write_dram_16(VCOUNT_ADDR + ((bank * VBLKS_PER_BANK) + vblock) * sizeof(UINT16), VC_MAX);
                mapblk_lbn++;
            }
        }
        //----------------------------------------
        // assign a free block for gc
        //----------------------------------------
        do {
            vblock++;
            ASSERT(vblock < VBLKS_PER_BANK);
        }while(is_bad_block(bank, vblock) == TRUE);
        set_gc_vblock(bank, vblock);
        // set the reserved free blocks as VC_MAX
        set_vcount(bank, vblock, VC_MAX);
        set_region_num(bank, vblock, INVALID16);

        //-------------------------------------------------
        // assign free vpn for first new write each region
        //-------------------------------------------------
        for (UINT32 region_num = 0; region_num < NUM_REGION; region_num++) {
            do {
                vblock++;
                ASSERT(vblock < VBLKS_PER_BANK);
            }while(is_bad_block(bank, vblock) == TRUE);
            set_new_write_vpn_of_region(bank, region_num, vblock * PAGES_PER_BLK);
            // vcount of active block is VC_ACTIVE
            set_vcount(bank, vblock, VC_ACTIVE);
            set_vcount_of_active_block(bank, region_num, 0);
            set_region_num(bank, vblock, region_num);
        }
        // remained blocks == VC_FREE
    }
}
// flush FTL metadata(SRAM+DRAM) for normal POR
void ftl_flush(void)
{
    /* ptimer_start(); */
    // flush region & block age information
    flush_dram_metadata_into_mapblk(1, VBLK_AGE_ADDR, VBLK_AGE_BYTES + VBLK_REGION_BYTES);
    // flush page mapping table
    flush_dram_metadata_into_mapblk(0, PAGE_MAP_ADDR, PAGE_MAP_BYTES);
    flush_misc_metadata();
    /* ptimer_stop_and_uart_print(); */
}
// logging misc + vcount metadata
static void flush_misc_metadata(void)
{
    UINT32 misc_meta_bytes = NUM_MISC_META_SECT * BYTES_PER_SECTOR; // per bank
    UINT32 vcount_addr     = VCOUNT_ADDR;
    UINT32 vcount_bytes    = NUM_VCOUNT_SECT * BYTES_PER_SECTOR; // per bank
    UINT32 vcount_boundary = VCOUNT_ADDR + VCOUNT_BYTES; // entire vcount data
    UINT32 bank;

    flash_finish();

    for (bank = 0; bank < NUM_BANKS; bank++) {
        inc_miscblk_vpn(bank);

        // note: if misc. meta block is full, just erase old block & write offset #0
        if ((get_miscblk_vpn(bank) / PAGES_PER_BLK) != MISCBLK_VBN) {
            nand_block_erase(bank, MISCBLK_VBN);
            set_miscblk_vpn(bank, MISCBLK_VBN * PAGES_PER_BLK); // vpn = 128
        }
        // copy misc. metadata to FTL buffer
        mem_copy(FTL_BUF(bank), &g_misc_meta[bank], misc_meta_bytes);

        // copy vcount metadata to FTL buffer
        if (vcount_addr <= vcount_boundary) {
            mem_copy(FTL_BUF(bank) + misc_meta_bytes, vcount_addr, vcount_bytes);
            vcount_addr += vcount_bytes;
        }
    }
    // logging the misc. metadata to nand flash
    for (bank = 0; bank < NUM_BANKS; bank++) {
        nand_page_ptprogram(bank,
                            get_miscblk_vpn(bank) / PAGES_PER_BLK,
                            get_miscblk_vpn(bank) % PAGES_PER_BLK,
                            0,
                            NUM_MISC_META_SECT + NUM_VCOUNT_SECT,
                            FTL_BUF(bank));
    }
    flash_finish();
}
static void flush_dram_metadata_into_mapblk(UINT32 const mapblk_lbn, UINT32 const dram_addr, UINT32 const dram_bytes)
{
    UINT32 bank, mapblk_vpn;
    UINT32 const dram_boundary = dram_addr + dram_bytes;
    UINT32 addr      = dram_addr;
    UINT32 num_bytes = dram_bytes;

    if (num_bytes > BYTES_PER_PAGE) {
        num_bytes = BYTES_PER_PAGE;
    }
    // first of all, check remain pages in the map block each bank
    for (bank = 0; bank < NUM_BANKS; bank++) {
        // if remained pages are not sufficient flush metadata, then erase block to obtain cleaned block
        if ((num_bytes / BYTES_PER_SECTOR / SECTORS_PER_PAGE / NUM_BANKS) >=
            (PAGES_PER_BLK - (get_mapblk_vpn(bank, mapblk_lbn) % PAGES_PER_BLK))) {
            UINT32 mapblk_vbn = get_mapblk_vpn(bank, mapblk_lbn) / PAGES_PER_BLK;
            nand_block_erase(bank, mapblk_vbn);
            set_mapblk_vpn(bank, mapblk_lbn, (mapblk_vbn * PAGES_PER_BLK) - 1);
        }
    }
    // parallel flush
    while (addr < dram_boundary) {
        flash_finish();

        for (bank = 0; bank < NUM_BANKS; bank++) {
            if (addr >= dram_boundary) {
                break;
            }
            if (addr + BYTES_PER_PAGE > dram_boundary) {
                num_bytes = dram_boundary - addr;
            }
            inc_mapblk_vpn(bank, mapblk_lbn);

            mapblk_vpn = get_mapblk_vpn(bank, mapblk_lbn);

            // copy the DRAM metadata to FTL buffer
            mem_copy(FTL_BUF(bank), addr, num_bytes);

            // flush DRAM metadata into map block
            nand_page_ptprogram(bank,
                                mapblk_vpn / PAGES_PER_BLK,
                                mapblk_vpn % PAGES_PER_BLK,
                                0,
                                (num_bytes + BYTES_PER_SECTOR - 1) / BYTES_PER_SECTOR,
                                FTL_BUF(bank));
            addr += num_bytes;
        }
    }
    flash_finish();
}
// load flushed FTL metadta
static void load_metadata(void)
{
    load_misc_metadata();
    // load page mapping table
    load_dram_metadata_from_mapblk(0, PAGE_MAP_ADDR, PAGE_MAP_BYTES);
    // load region & block age information
    load_dram_metadata_from_mapblk(1, VBLK_AGE_ADDR, VBLK_AGE_BYTES + VBLK_REGION_BYTES);
}
// misc + VCOUNT
static void load_misc_metadata(void)
{
    UINT32 misc_meta_bytes = NUM_MISC_META_SECT * BYTES_PER_SECTOR;
    UINT32 vcount_bytes    = NUM_VCOUNT_SECT * BYTES_PER_SECTOR;
    UINT32 vcount_addr     = VCOUNT_ADDR;
    UINT32 vcount_boundary = VCOUNT_ADDR + VCOUNT_BYTES;

    UINT32 load_flag = 0;
    UINT32 bank, page_num;
    UINT32 load_cnt = 0;

    flash_finish();

	disable_irq();
	flash_clear_irq();	// clear any flash interrupt flags that might have been set

    // scan valid metadata in descending order from last page offset
    for (page_num = PAGES_PER_BLK - 1; page_num != ((UINT32) -1); page_num--)
    {
        for (bank = 0; bank < NUM_BANKS; bank++)
        {
            if (load_flag & (0x1 << bank))
            {
                continue;
            }
            // read valid metadata from misc. metadata area
            nand_page_ptread(bank,
                             MISCBLK_VBN,
                             page_num,
                             0,
                             NUM_MISC_META_SECT + NUM_VCOUNT_SECT,
                             FTL_BUF(bank),
                             RETURN_ON_ISSUE);
        }
        flash_finish();

        for (bank = 0; bank < NUM_BANKS; bank++)
        {
            if (!(load_flag & (0x1 << bank)) && !(BSP_INTR(bank) & FIRQ_ALL_FF))
            {
                load_flag = load_flag | (0x1 << bank);
                load_cnt++;
            }
            CLR_BSP_INTR(bank, 0xFF);
        }
    }
    ASSERT(load_cnt == NUM_BANKS);

    for (bank = 0; bank < NUM_BANKS; bank++)
    {
        // misc. metadata
        mem_copy(&g_misc_meta[bank], FTL_BUF(bank), sizeof(misc_metadata));

        // vcount metadata
        if (vcount_addr <= vcount_boundary)
        {
            mem_copy(vcount_addr, FTL_BUF(bank) + misc_meta_bytes, vcount_bytes);
            vcount_addr += vcount_bytes;

        }
    }
	enable_irq();
}
static void load_dram_metadata_from_mapblk(UINT32 const mapblk_lbn, UINT32 const dram_addr, UINT32 const dram_bytes)
{
    UINT32 bank, mapblk_vpn;
    UINT32 const dram_boundary = dram_addr + dram_bytes;
    UINT32 addr       = dram_addr;
    UINT32 num_bytes  = dram_bytes;
    UINT32 load_mapblk_vpn[NUM_BANKS];
    UINT32 temp_page_addr, temp_bytes;

    // get start mapblk vpn for loading metadata
    for (bank = 0; bank < NUM_BANKS; bank++) {
        load_mapblk_vpn[bank] = get_mapblk_vpn(bank, mapblk_lbn) - (((num_bytes + BYTES_PER_PAGE - 1) / BYTES_PER_PAGE) / NUM_BANKS) + 1;

        if (bank < ((num_bytes + BYTES_PER_PAGE - 1) / BYTES_PER_PAGE) % NUM_BANKS) {
            load_mapblk_vpn[bank]--;
        }
    }
    if (num_bytes > BYTES_PER_PAGE) {
        num_bytes = BYTES_PER_PAGE;
    }
    // parallel flush
    while (addr < dram_boundary) {
        temp_page_addr = addr; // temporal backup DRAM addr
        temp_bytes     = num_bytes;

        for (bank = 0; bank < NUM_BANKS; bank++) {
            if (addr >= dram_boundary) {
                break;
            }
            if (addr + BYTES_PER_PAGE > dram_boundary) {
                num_bytes = dram_boundary - addr;
            }
            mapblk_vpn = load_mapblk_vpn[bank];
            load_mapblk_vpn[bank]++;

            // read FTL metadata from map block
            nand_page_ptread(bank,
                             mapblk_vpn / PAGES_PER_BLK,
                             mapblk_vpn % PAGES_PER_BLK,
                             0,
                             (num_bytes + BYTES_PER_SECTOR - 1) / BYTES_PER_SECTOR,
                             FTL_BUF(bank),
                             RETURN_ON_ISSUE);
            addr += num_bytes;
        }
        flash_finish();
        // copy to FTL metadata from FTL buffer
        for (bank = 0; bank < NUM_BANKS; bank++) {

            if (temp_page_addr >= dram_boundary) {
                break;
            }
            if (temp_page_addr + BYTES_PER_PAGE > dram_boundary) {
                temp_bytes = dram_boundary - temp_page_addr;
            }
            mem_copy(temp_page_addr, FTL_BUF(bank), temp_bytes);

            temp_page_addr += temp_bytes;
        }
    }
}
static void write_format_mark(void)
{
	// This function writes a format mark to a page at (bank #0, block #0).

	#ifdef __GNUC__
	extern UINT32 size_of_firmware_image;
	UINT32 firmware_image_pages = (((UINT32) (&size_of_firmware_image)) + BYTES_PER_FW_PAGE - 1) / BYTES_PER_FW_PAGE;
	#else
	extern UINT32 Image$$ER_CODE$$RO$$Length;
	extern UINT32 Image$$ER_RW$$RW$$Length;
	UINT32 firmware_image_bytes = ((UINT32) &Image$$ER_CODE$$RO$$Length) + ((UINT32) &Image$$ER_RW$$RW$$Length);
	UINT32 firmware_image_pages = (firmware_image_bytes + BYTES_PER_FW_PAGE - 1) / BYTES_PER_FW_PAGE;
	#endif

	UINT32 format_mark_page_offset = FW_PAGE_OFFSET + firmware_image_pages;

	mem_set_dram(FTL_BUF_ADDR, 0, BYTES_PER_SECTOR);

	SETREG(FCP_CMD, FC_COL_ROW_IN_PROG);
	SETREG(FCP_BANK, REAL_BANK(0));
	SETREG(FCP_OPTION, FO_E | FO_B_W_DRDY);
	SETREG(FCP_DMA_ADDR, FTL_BUF_ADDR); 	// DRAM -> flash
	SETREG(FCP_DMA_CNT, BYTES_PER_SECTOR);
	SETREG(FCP_COL, 0);
	SETREG(FCP_ROW_L(0), format_mark_page_offset);
	SETREG(FCP_ROW_H(0), format_mark_page_offset);

	// At this point, we do not have to check Waiting Room status before issuing a command,
	// because we have waited for all the banks to become idle before returning from format().
	SETREG(FCP_ISSUE, NULL);

	// wait for the FC_COL_ROW_IN_PROG command to be accepted by bank #0
	while ((GETREG(WR_STAT) & 0x00000001) != 0);

	// wait until bank #0 finishes the write operation
	while (BSP_FSM(0) != BANK_IDLE);
}
static BOOL32 check_format_mark(void)
{
	// This function reads a flash page from (bank #0, block #0) in order to check whether the SSD is formatted or not.

	#ifdef __GNUC__
	extern UINT32 size_of_firmware_image;
	UINT32 firmware_image_pages = (((UINT32) (&size_of_firmware_image)) + BYTES_PER_FW_PAGE - 1) / BYTES_PER_FW_PAGE;
	#else
	extern UINT32 Image$$ER_CODE$$RO$$Length;
	extern UINT32 Image$$ER_RW$$RW$$Length;
	UINT32 firmware_image_bytes = ((UINT32) &Image$$ER_CODE$$RO$$Length) + ((UINT32) &Image$$ER_RW$$RW$$Length);
	UINT32 firmware_image_pages = (firmware_image_bytes + BYTES_PER_FW_PAGE - 1) / BYTES_PER_FW_PAGE;
	#endif

	UINT32 format_mark_page_offset = FW_PAGE_OFFSET + firmware_image_pages;
	UINT32 temp;

	flash_clear_irq();	// clear any flash interrupt flags that might have been set

	SETREG(FCP_CMD, FC_COL_ROW_READ_OUT);
	SETREG(FCP_BANK, REAL_BANK(0));
	SETREG(FCP_OPTION, FO_E);
	SETREG(FCP_DMA_ADDR, FTL_BUF_ADDR); 	// flash -> DRAM
	SETREG(FCP_DMA_CNT, BYTES_PER_SECTOR);
	SETREG(FCP_COL, 0);
	SETREG(FCP_ROW_L(0), format_mark_page_offset);
	SETREG(FCP_ROW_H(0), format_mark_page_offset);

	// At this point, we do not have to check Waiting Room status before issuing a command,
	// because scan list loading has been completed just before this function is called.
	SETREG(FCP_ISSUE, NULL);

	// wait for the FC_COL_ROW_READ_OUT command to be accepted by bank #0
	while ((GETREG(WR_STAT) & 0x00000001) != 0);

	// wait until bank #0 finishes the read operation
	while (BSP_FSM(0) != BANK_IDLE);

	// Now that the read operation is complete, we can check interrupt flags.
	temp = BSP_INTR(0) & FIRQ_ALL_FF;

	// clear interrupt flags
	CLR_BSP_INTR(0, 0xFF);

	if (temp != 0)
	{
		return FALSE;	// the page contains all-0xFF (the format mark does not exist.)
	}
	else
	{
		return TRUE;	// the page contains something other than 0xFF (it must be the format mark)
	}
}
// Testing FTL protocol APIs
void ftl_test_write(UINT32 const lba, UINT32 const num_sectors)
{
    ASSERT(lba + num_sectors <= NUM_LSECTORS);
    ASSERT(num_sectors > 0);

    ftl_write(lba, num_sectors);
}
void ftl_read(UINT32 const lba, UINT32 const num_sectors)
{
    UINT32 remain_sects, num_sectors_to_read;
    UINT32 lpn, sect_offset;
    UINT32 bank, vpn;

    lpn          = lba / SECTORS_PER_PAGE;
    sect_offset  = lba % SECTORS_PER_PAGE;
    remain_sects = num_sectors;

    while (remain_sects != 0)
    {
        if ((sect_offset + remain_sects) < SECTORS_PER_PAGE)
        {
            num_sectors_to_read = remain_sects;
        }
        else
        {
            num_sectors_to_read = SECTORS_PER_PAGE - sect_offset;
        }
        bank = get_num_bank(lpn); // page striping
        vpn  = get_vpn(lpn);
        CHECK_VPAGE(vpn);

        if (vpn != NULL)
        {
            nand_page_ptread_to_host(bank,
                                     vpn / PAGES_PER_BLK,
                                     vpn % PAGES_PER_BLK,
                                     sect_offset,
                                     num_sectors_to_read);
        }
        // The host is requesting to read a logical page that has never been written to.
        else
        {
			UINT32 next_read_buf_id = (g_ftl_read_buf_id + 1) % NUM_RD_BUFFERS;

            #if OPTION_FTL_TEST == 0
			while (next_read_buf_id == GETREG(SATA_RBUF_PTR));	// wait if the read buffer is full (slow host)
            #endif
            // fix bug @ v.1.0.6
			mem_set_dram(RD_BUF_PTR(g_ftl_read_buf_id) + sect_offset*BYTES_PER_SECTOR,
                         0xFFFFFFFF, num_sectors_to_read*BYTES_PER_SECTOR);

            flash_finish();

			SETREG(BM_STACK_RDSET, next_read_buf_id);	// change bm_read_limit
			SETREG(BM_STACK_RESET, 0x02);				// change bm_read_limit

			g_ftl_read_buf_id = next_read_buf_id;
        }
        sect_offset   = 0;
        remain_sects -= num_sectors_to_read;
        lpn++;
    }
}
void ftl_write(UINT32 const lba, UINT32 const num_sectors)
{
    UINT32 remain_sects, num_sectors_to_write;
    UINT32 lpn, sect_offset;

    lpn          = lba / SECTORS_PER_PAGE;
    sect_offset  = lba % SECTORS_PER_PAGE;
    remain_sects = num_sectors;

    while (remain_sects != 0)
    {
        if ((sect_offset + remain_sects) < SECTORS_PER_PAGE)
        {
            num_sectors_to_write = remain_sects;
        }
        else
        {
            num_sectors_to_write = SECTORS_PER_PAGE - sect_offset;
        }
        // single page write individually
        write_page(lpn, sect_offset, num_sectors_to_write);

        sect_offset   = 0;
        remain_sects -= num_sectors_to_write;
        lpn++;
    }
}
static void write_page(UINT32 const lpn, UINT32 const sect_offset, UINT32 const num_sectors)
{
    CHECK_LPAGE(lpn);
    ASSERT(sect_offset < SECTORS_PER_PAGE);
    ASSERT(num_sectors > 0 && num_sectors <= SECTORS_PER_PAGE);

    UINT32 bank, old_vpn, new_vpn;
    UINT32 vblock, page_num, page_offset, column_cnt;

    bank        = get_num_bank(lpn); // page striping
    page_offset = sect_offset;
    column_cnt  = num_sectors;

    old_vpn  = get_vpn(lpn);
    CHECK_VPAGE (old_vpn);

    g_ftl_statistics[bank].page_wcount++;

    // if old data already exist,
    if (old_vpn != NULL)
    {
        vblock   = old_vpn / PAGES_PER_BLK;
        page_num = old_vpn % PAGES_PER_BLK;

        UINT32 region_num_of_old_vpn = get_region_num(bank, vblock);
        ASSERT(region_num_of_old_vpn < NUM_REGION);

        //--------------------------------------------------------------------------------------
        // `Partial programming'
        // we could not determine whether the new data is loaded in the SATA write buffer.
        // Thus, read the left/right hole sectors of a valid page and copy into the write buffer.
        // And then, program whole valid data
        //--------------------------------------------------------------------------------------
        if (num_sectors != SECTORS_PER_PAGE)
        {
            // Performance optimization (but, not proved)
            // To reduce flash memory access, valid hole copy into SATA write buffer after reading whole page
            // Thus, in this case, we need just one full page read + one or two mem_copy
            if ((num_sectors <= 8) && (page_offset != 0))
            {
                // one page async read
                nand_page_read(bank,
                               vblock,
                               page_num,
                               FTL_BUF(bank));
                // copy `left hole sectors' into SATA write buffer
                if (page_offset != 0)
                {
                    mem_copy(WR_BUF_PTR(g_ftl_write_buf_id),
                             FTL_BUF(bank),
                             page_offset * BYTES_PER_SECTOR);
                }
                // copy `right hole sectors' into SATA write buffer
                if ((page_offset + column_cnt) < SECTORS_PER_PAGE)
                {
                    UINT32 const rhole_base = (page_offset + column_cnt) * BYTES_PER_SECTOR;

                    mem_copy(WR_BUF_PTR(g_ftl_write_buf_id) + rhole_base,
                             FTL_BUF(bank) + rhole_base,
                             BYTES_PER_PAGE - rhole_base);
                }
            }
            // left/right hole async read operation (two partial page read)
            else
            {
                // read `left hole sectors'
                if (page_offset != 0)
                {
                    nand_page_ptread(bank,
                                     vblock,
                                     page_num,
                                     0,
                                     page_offset,
                                     WR_BUF_PTR(g_ftl_write_buf_id),
                                     RETURN_ON_ISSUE);
                }
                // read `right hole sectors'
                if ((page_offset + column_cnt) < SECTORS_PER_PAGE)
                {
                    nand_page_ptread(bank,
                                     vblock,
                                     page_num,
                                     page_offset + column_cnt,
                                     SECTORS_PER_PAGE - (page_offset + column_cnt),
                                     WR_BUF_PTR(g_ftl_write_buf_id),
                                     RETURN_ON_ISSUE);
                }
            }
        }
        // full page write
        page_offset = 0;
        column_cnt  = SECTORS_PER_PAGE;
        // invalid old page (decrease vcount)
        // if the block is in 'active' state,
        if (vblock == (get_cur_write_vpn_of_region(bank, region_num_of_old_vpn) / PAGES_PER_BLK)) {
            dec_vcount_of_active_block(bank, region_num_of_old_vpn);
            ASSERT(get_vcount_of_active_block(bank, region_num_of_old_vpn) < PAGES_PER_BLK);
        }
        // else if this block is not active,
        else {
            set_vcount(bank, vblock, get_vcount(bank, vblock) - 1);
            ASSERT(get_vcount(bank,vblock) < (PAGES_PER_BLK - 1));
        }
    } // end if

    // set new write vpn & get region number to write new data
    UINT32 region_num_for_new_write;

    new_vpn = assign_new_write_vpn(bank, lpn, old_vpn, &region_num_for_new_write);
    CHECK_VPAGE(new_vpn);

    ASSERT(get_vcount_of_active_block(bank, region_num_for_new_write) < (PAGES_PER_BLK - 1));
    ASSERT(get_region_num(bank, new_vpn / PAGES_PER_BLK) < NUM_REGION);

    vblock   = new_vpn / PAGES_PER_BLK;
    page_num = new_vpn % PAGES_PER_BLK;

    // write new data (make sure that the new data is ready in the write buffer frame)
    // (c.f FO_B_SATA_W flag in flash.h)
    nand_page_ptprogram_from_host(bank,
                                  vblock, page_num,
                                  page_offset, column_cnt);
    // update metadata
    set_vpn(lpn, new_vpn);
    // logging lpn info. for determining valid page in GC period
    set_lpn_of_region(bank, region_num_for_new_write, page_num, lpn);
/*     set_age_of_lpn(bank, lpn, get_global_age(bank)); */

    // increase valid count of active block
    // NOTE;
    //   - vcount of active block is managed in SRAM.
    //     After the block is full, it will store to DRAM(i.e., VCOUNT_ADDR)
    inc_vcount_of_active_block(bank, region_num_for_new_write);

    // NOTE: increase global age when every single new page write
    inc_global_age(bank);
}
// get vpn from PAGE_MAP
static UINT32 get_vpn(UINT32 const lpn)
{
    CHECK_LPAGE(lpn);
    return read_dram_32(PAGE_MAP_ADDR + lpn * sizeof(UINT32));
}
// set vpn to PAGE_MAP
static void set_vpn(UINT32 const lpn, UINT32 const vpn)
{
    CHECK_LPAGE(lpn);
    ASSERT(vpn >= (META_BLKS_PER_BANK * PAGES_PER_BLK) && vpn < (VBLKS_PER_BANK * PAGES_PER_BLK));

    write_dram_32(PAGE_MAP_ADDR + lpn * sizeof(UINT32), vpn);
}
// get valid page count of vblock
static UINT32 get_vcount(UINT32 const bank, UINT32 const vblock)
{
    UINT32 vcount;

    ASSERT(bank < NUM_BANKS);
    ASSERT((vblock >= META_BLKS_PER_BANK) && (vblock < VBLKS_PER_BANK));

    vcount = read_dram_16(VCOUNT_ADDR + (((bank * VBLKS_PER_BANK) + vblock) * sizeof(UINT16)));
    ASSERT((vcount < PAGES_PER_BLK) || (vcount & NOT_FOR_VICTIM) != FALSE);

    return vcount;
}
// set valid page count of vblock
static void set_vcount(UINT32 const bank, UINT32 const vblock, UINT32 const vcount)
{
    ASSERT(bank < NUM_BANKS);
    ASSERT((vblock >= META_BLKS_PER_BANK) && (vblock < VBLKS_PER_BANK));
    ASSERT((vcount < PAGES_PER_BLK) || (vcount & NOT_FOR_VICTIM) != FALSE);

    write_dram_16(VCOUNT_ADDR + (((bank * VBLKS_PER_BANK) + vblock) * sizeof(UINT16)), vcount);
}
static void inc_vcount(UINT32 const bank, UINT32 const vblock)
{
    ASSERT(bank < NUM_BANKS);
    ASSERT((vblock >= META_BLKS_PER_BANK) && (vblock < VBLKS_PER_BANK));
    UINT32 vcount = read_dram_16(VCOUNT_ADDR + (((bank * VBLKS_PER_BANK) + vblock) * sizeof(UINT16)));;
    ASSERT(vcount < (PAGES_PER_BLK - 1));
    write_dram_16(VCOUNT_ADDR + (((bank * VBLKS_PER_BANK) + vblock) * sizeof(UINT16)), vcount + 1);
}
static UINT32 assign_new_write_vpn(UINT32 const bank, UINT32 const lpn, UINT32 const old_vpn, UINT32* region_num_for_new_write)
{
    ASSERT(bank < NUM_BANKS);

    if (old_vpn == NULL) {
        *region_num_for_new_write = 0;
    }
    else {
        *region_num_for_new_write = get_region_num(bank, old_vpn / PAGES_PER_BLK);
        ASSERT((*region_num_for_new_write) < NUM_REGION);

/*         // check whether the old vpn is 'young' or not - NOT USED  */
/*         if (*region_num_for_new_write < (NUM_REGION - 1) && is_young_page(bank, lpn)) { */
/*             // if the page is young, then write new data into upper region */
/*             (*region_num_for_new_write)++; */
/*         } */
        // FORCE PROMOTING TO UPPER REGION
        if ((*region_num_for_new_write) < (NUM_REGION - 1)) {
            (*region_num_for_new_write)++; 
        }
        ASSERT((*region_num_for_new_write) < NUM_REGION);
    }
    UINT32 write_vpn, vblock;
    UINT32 const new_region_num = *region_num_for_new_write;

    // previous written vpn
    write_vpn = get_cur_write_vpn_of_region(bank, new_region_num);
    vblock    = write_vpn / PAGES_PER_BLK;

    // NOTE: if next new write page's offset is
    // the last page offset of vblock (i.e. PAGES_PER_BLK - 1),
    if ((write_vpn % PAGES_PER_BLK) == (PAGES_PER_BLK - 2)) {
        // then, because of the flash controller limitation
        // (prohibit accessing a spare area (i.e. OOB)),
        // thus, we persistenly write a lpn list into last page of vblock.
        ASSERT(get_vcount(bank, vblock) == VC_ACTIVE);

        mem_copy(FTL_BUF(bank), g_misc_meta[bank].region_tbl[new_region_num].lpn_list_of_cur_vblock, sizeof(UINT32) * PAGES_PER_BLK);
        nand_page_ptprogram(bank, vblock, PAGES_PER_BLK - 1,
                            0, (sizeof(UINT32) * PAGES_PER_BLK + BYTES_PER_SECTOR - 1) / BYTES_PER_SECTOR,
                            FTL_BUF(bank));

        mem_set_sram(g_misc_meta[bank].region_tbl[new_region_num].lpn_list_of_cur_vblock, 0x00000000, sizeof(UINT32) * PAGES_PER_BLK);

        // set vcount of current active block (this block is not in 'active' state anymore)
        set_vcount(bank, vblock, get_vcount_of_active_block(bank, new_region_num));
        set_vcount_of_active_block(bank, new_region_num, 0);
        set_age_of_vblock(bank, vblock, get_global_age(bank));

        inc_full_blk_cnt(bank);

        while (is_full_all_blks(bank)) {
            garbage_collection(bank);
        }

        if ((get_cur_write_vpn_of_region(bank, new_region_num) % PAGES_PER_BLK) != (PAGES_PER_BLK - 2)) {
            inc_cur_write_vpn_of_region(bank, new_region_num);
            return get_cur_write_vpn_of_region(bank, new_region_num);
        }
/*         uart_printf("bank %d, fullcnt %d, current active block offset: region %d, offset %d",bank, g_misc_meta[bank].free_blk_cnt, new_region_num, get_cur_write_vpn_of_region(bank, new_region_num) % PAGES_PER_BLK); */
        // search for usable blocks
        do {
            vblock++;

            if (vblock == VBLKS_PER_BANK) {
                vblock = META_BLKS_PER_BANK;
            }
        }while (get_vcount(bank, vblock) != VC_FREE);
    }
    // write page -> next block
    if (vblock != (write_vpn / PAGES_PER_BLK)) {
        write_vpn = vblock * PAGES_PER_BLK;
        ASSERT(get_vcount(bank, vblock) != VC_MAX || get_vcount(bank, vblock) != VC_ACTIVE);
        set_region_num(bank, vblock, new_region_num);
        // now the vblock is currently in 'active' state
        set_vcount(bank, vblock, VC_ACTIVE);
        set_vcount_of_active_block(bank, new_region_num, 0);
    }
    else {
        write_vpn++;
    }
    ASSERT(get_vcount(bank, write_vpn / PAGES_PER_BLK) == VC_ACTIVE);
    set_new_write_vpn_of_region(bank, new_region_num, write_vpn);

    return get_cur_write_vpn_of_region(bank, new_region_num);
}
static UINT32 get_region_num(UINT32 const bank, UINT32 const vblock)
{
    ASSERT(vblock < VBLKS_PER_BANK);
    return read_dram_16(VBLK_REGION_ADDR + (bank * VBLKS_PER_BANK + vblock) * sizeof(UINT16));
}
static UINT32 get_age_of_vblock(UINT32 const bank, UINT32 const vblock)
{
    ASSERT(vblock < VBLKS_PER_BANK);
    return read_dram_32(VBLK_AGE_ADDR + (bank * VBLKS_PER_BANK + vblock) * sizeof(UINT32));
}
static void set_age_of_vblock(UINT32 const bank, UINT32 const vblock, UINT32 const age)
{
    ASSERT(vblock < VBLKS_PER_BANK);
    write_dram_32(VBLK_AGE_ADDR + (bank * VBLKS_PER_BANK + vblock) * sizeof(UINT32), age);
}
static void set_region_num(UINT32 const bank, UINT32 const vblock, UINT32 const region_num)
{
    ASSERT(vblock < VBLKS_PER_BANK);
    ASSERT(region_num < NUM_REGION || region_num == INVALID16);

    write_dram_16(VBLK_REGION_ADDR + (bank * VBLKS_PER_BANK + vblock) * sizeof(UINT16), region_num);
}
static BOOL32 is_bad_block(UINT32 const bank, UINT32 const vblk_offset)
{
    if (tst_bit_dram(BAD_BLK_BMP_ADDR + bank*(VBLKS_PER_BANK/8 + 1), vblk_offset) == FALSE) {
        return FALSE;
    }
    return TRUE;
}
static void set_bad_block(UINT32 const bank, UINT32 const vblk_offset)
{
    set_bit_dram(BAD_BLK_BMP_ADDR + bank*(VBLKS_PER_BANK/8 + 1), vblk_offset);
    g_bad_blk_count[bank]++;
    uart_printf("found additional bad block: bank %d vblock %d", bank, vblk_offset);
}
static void garbage_collection(UINT32 const bank)
{
    ASSERT(bank < NUM_BANKS);
    g_ftl_statistics[bank].gc_cnt++;

    UINT32 src_lpn, src_vpn;
    UINT32 vt_vblock;
    UINT32 free_vpn;
    UINT32 vcount; // valid page count in victim block
    UINT32 region_num_of_vt_vblock, region_num_to_cpback = 0;
    UINT32 cur_write_vpn;

    g_ftl_statistics[bank].gc_cnt++;

/*     vt_vblock = get_vt_vblock_by_greedy(bank); */
    vt_vblock = get_vt_vblock_by_cost_benefit(bank);
    vcount    = get_vcount(bank, vt_vblock);
    ASSERT(vt_vblock >= META_BLKS_PER_BANK && vt_vblock < VBLKS_PER_BANK);
    ASSERT(vcount <= (PAGES_PER_BLK - 1));

/*     uart_printf("garbage_collection bank %d, vt_vblock %d, vcount %d, full cnt", bank, vt_vblock, vcount, g_misc_meta[bank].free_blk_cnt); */

    // if no valid pages in the victim block
    if (vcount == 0) {
        // then, erase victim block
        nand_block_erase(bank, vt_vblock);
        set_region_num(bank, vt_vblock, INVALID16);
        set_vcount(bank, vt_vblock, VC_FREE);

        dec_full_blk_cnt(bank);

        return;
    }
    // get base vpn of victim vblock
    src_vpn   = vt_vblock * PAGES_PER_BLK;
    region_num_of_vt_vblock = get_region_num(bank, vt_vblock);
    ASSERT(region_num_of_vt_vblock < NUM_REGION);

    // 1. load physical to logical page number list from last page offset of victim block (4B x PAGES_PER_BLK)
    nand_page_ptread(bank, vt_vblock, PAGES_PER_BLK - 1, 0,
                     (sizeof(UINT32) * PAGES_PER_BLK + BYTES_PER_SECTOR - 1) / BYTES_PER_SECTOR, FTL_BUF(bank), RETURN_WHEN_DONE);
    mem_copy(g_misc_meta[bank].lpn_list_of_vt_vblock, FTL_BUF(bank), sizeof(UINT32) * PAGES_PER_BLK);

    // FORCE DEMOTING TO LOWER REGION
    if (region_num_of_vt_vblock > 0) {
        region_num_to_cpback = region_num_of_vt_vblock - 1;
    }
    // 2. copy-back all valid pages to free space
    for (UINT32 src_page = 0; src_page < (PAGES_PER_BLK - 1); src_page++) {
        // get lpn of victim block from a read lpn list
        src_lpn = get_lpn_of_vt_vblock(bank, src_page);
        CHECK_VPAGE(get_vpn(src_lpn));
        CHECK_LPAGE(src_lpn);

        // check whether the page is valid or not
        if (get_vpn(src_lpn) != src_vpn) {
            // invalid page
            src_vpn++;
            continue;
        }
        CHECK_LPAGE(src_lpn);

/*         // if the valid page is 'old', then copy the page into lower region - NOT USED */
/*         if (region_num_of_vt_vblock > 0 && (!is_young_page(bank, src_lpn))) { */
/*             region_num_to_cpback--; */
/*         } */
        // previously written offset
        cur_write_vpn = get_cur_write_vpn_of_region(bank, region_num_to_cpback);

        if ((cur_write_vpn % PAGES_PER_BLK) == (PAGES_PER_BLK - 2)) {
            // NOTE: vcount of active region is always VC_ACTIVE
            if (get_vcount(bank, (cur_write_vpn / PAGES_PER_BLK)) == VC_ACTIVE) {
                UINT32 vblock = cur_write_vpn / PAGES_PER_BLK;
                // then write lpn list in the last offset, and allocate new free block
                mem_copy(FTL_BUF(bank), g_misc_meta[bank].region_tbl[region_num_to_cpback].lpn_list_of_cur_vblock, sizeof(UINT32) * PAGES_PER_BLK);
                nand_page_ptprogram(bank, vblock, PAGES_PER_BLK - 1,
                                    0, (sizeof(UINT32) * PAGES_PER_BLK + BYTES_PER_SECTOR - 1) / BYTES_PER_SECTOR,
                                    FTL_BUF(bank));
                mem_set_sram(g_misc_meta[bank].region_tbl[region_num_to_cpback].lpn_list_of_cur_vblock, 0x00000000, sizeof(UINT32) * PAGES_PER_BLK);

                // set vcount of current active block (this block is not in active state anymore)
                set_vcount(bank, vblock, get_vcount_of_active_block(bank, region_num_to_cpback));
                set_vcount_of_active_block(bank, region_num_to_cpback, 0);
                set_age_of_vblock(bank, vblock, get_global_age(bank));
                inc_full_blk_cnt(bank);
            }
            // set region information of new free block
            free_vpn = get_gc_vblock(bank); // temporal
            ASSERT(get_vcount(bank, free_vpn) != VC_ACTIVE);
            set_vcount(bank, free_vpn, VC_ACTIVE); // current active block
/*             set_vcount(bank, free_vpn, NULL); */
            set_region_num(bank, free_vpn, region_num_to_cpback);
            // assign new write page
            set_new_write_vpn_of_region(bank, region_num_to_cpback, free_vpn * PAGES_PER_BLK);
        }
        else {
            // assign new write page
            inc_cur_write_vpn_of_region(bank, region_num_to_cpback);
        }
        free_vpn = get_cur_write_vpn_of_region(bank, region_num_to_cpback);
        ASSERT(vt_vblock != (free_vpn / PAGES_PER_BLK));

        nand_page_copyback(bank,
                           vt_vblock, src_page,
                           free_vpn / PAGES_PER_BLK, free_vpn % PAGES_PER_BLK);
        // update metadata
        set_vpn(src_lpn, free_vpn);
        set_lpn_of_region(bank, region_num_to_cpback, (free_vpn % PAGES_PER_BLK), src_lpn);
        inc_vcount_of_active_block(bank, region_num_to_cpback);
        ASSERT(get_vcount_of_active_block(bank, region_num_to_cpback) < PAGES_PER_BLK);

        src_vpn++;
    } // end of GC

    // erase victim block
    nand_block_erase(bank, vt_vblock);
    set_region_num(bank, vt_vblock, INVALID16);

    // update metadata for victim vblock
    // if GC block is in 'active' state,
    if (get_vcount(bank, get_gc_vblock(bank)) == VC_ACTIVE) {
        set_gc_vblock(bank, vt_vblock);
        set_vcount(bank, vt_vblock, VC_MAX);
    }
    else {
        // set vt_vblock as a FREE block
        set_vcount(bank, vt_vblock, VC_FREE);
    }
    dec_full_blk_cnt(bank);
    /* uart_print("garbage_collection end"); */
}
//--------------------------------------------------------------------
// Victim selection policy: Greedy GC (Alternate)
//--------------------------------------------------------------------
static UINT32 get_vt_vblock_by_greedy(UINT32 const bank)
{
    ASSERT(bank < NUM_BANKS);

    UINT32 valid_cnt;
    UINT32 vt_vblock, max_valid_cnt;
    UINT32 vblock;

    max_valid_cnt = PAGES_PER_BLK - 1;

    for (vblock = META_BLKS_PER_BANK; vblock < VBLKS_PER_BANK; vblock++) {
        valid_cnt  = get_vcount(bank, vblock);

        // skip bad blocks, reserved free blocks, full valid blocks and current active blocks
        if ((valid_cnt & NOT_FOR_VICTIM) != FALSE) {
            continue;
        }
        //--------------
        // normal case
        //--------------
        if (valid_cnt == 0) {
            vt_vblock = vblock;
            break;
        }
        // select the victim block which has a maximum cost
        if (max_valid_cnt > valid_cnt) {
            vt_vblock = vblock;
        }
    }
    ASSERT(is_bad_block(bank, vt_vblock) == FALSE);
    ASSERT(get_vcount(bank, vt_vblock) <= (PAGES_PER_BLK - 1));

    return vt_vblock;
}
//--------------------------------------------------------------------
// Victim selection policy: Cost-benefit GC (valid count, age of block)
// Question) Can we optimize this policy in terms of processing overhead?
//--------------------------------------------------------------------
static UINT32 get_vt_vblock_by_cost_benefit(UINT32 const bank)
{
    ASSERT(bank < NUM_BANKS);

    UINT32 vblock_cost, valid_cnt;
    UINT32 vt_vblock = META_BLKS_PER_BANK;
    UINT32 max_cost = 0;
    UINT32 vblock;

    for (vblock = META_BLKS_PER_BANK; vblock < VBLKS_PER_BANK; vblock++) {
        valid_cnt  = get_vcount(bank, vblock);

        // skip bad blocks, reserved free blocks, full valid blocks and current active blocks
        if ((valid_cnt & NOT_FOR_VICTIM) != FALSE) {
            continue;
        }
        //--------------
        // normal case below
        //--------------
        if (valid_cnt == 0) {
            vt_vblock = vblock;
            break;
        }
        // get cost by Cost-benefit policy
        vblock_cost = (get_global_age(bank) - get_age_of_vblock(bank, vblock)) * (PAGES_PER_BLK - valid_cnt) / (valid_cnt << 1);

        // select the victim block which has a maximum cost
        if (vblock_cost > max_cost) {
            vt_vblock = vblock;
            max_cost  = vblock_cost;
        }
    }
    ASSERT(is_bad_block(bank, vt_vblock) == FALSE);
    ASSERT(get_vcount(bank, vt_vblock) <= (PAGES_PER_BLK - 1));

    return vt_vblock;
}
/* static UINT32 get_age_of_lpn(UINT32 const bank, UINT32 const lpn) */
/* { */
/*     ASSERT(lpn < (NUM_LPAGES)); */
/*     return read_dram_32(PAGE_AGE_ADDR + lpn * sizeof(UINT32)); */
/* } */
/* // NOTE: age(UINT32) should not overflow (TODO) */
/* static void set_age_of_lpn(UINT32 const bank, UINT32 const lpn, UINT32 const age) */
/* { */
/*     ASSERT(lpn < (NUM_LPAGES)); */
/*     write_dram_32(PAGE_AGE_ADDR + lpn * sizeof(UINT32), age); */
/* } */
/* static BOOL32 is_young_page(UINT32 const bank, UINT32 const lpn) */
/* { */
/*     UINT32 age = get_age_of_lpn(bank, lpn); */
/*     // tuning factor? (write interval as a resident time threshold) */
/*     return ((get_global_age(bank) - age) < WRITE_INTERVAL) ? TRUE : FALSE; */
/* } */
// BSP interrupt service routine
void ftl_isr(void)
{
    UINT32 bank;
    UINT32 bsp_intr_flag;

    uart_print("BSP interrupt occured...");
    // interrupt pending clear (ICU)
    SETREG(APB_INT_STS, INTR_FLASH);

    for (bank = 0; bank < NUM_BANKS; bank++) {
        while (BSP_FSM(bank) != BANK_IDLE);
        // get interrupt flag from BSP
        bsp_intr_flag = BSP_INTR(bank);

        if (bsp_intr_flag == 0) {
            continue;
        }
        UINT32 fc = GETREG(BSP_CMD(bank));
        // BSP clear
        CLR_BSP_INTR(bank, bsp_intr_flag);

        // interrupt handling
		if (bsp_intr_flag & FIRQ_DATA_CORRUPT) {
            uart_printf("BSP interrupt at bank: 0x%x, vblock #: %d", bank, GETREG(BSP_ROW_H(bank)) / PAGES_PER_BLK);
            uart_print("FIRQ_DATA_CORRUPT occured...");
            uart_printf("data corrupted...vblock #: %d, offset # %d", GETREG(BSP_ROW_H(bank)) / PAGES_PER_BLK, GETREG(BSP_ROW_H(bank)) % PAGES_PER_BLK);
		}
		if (bsp_intr_flag & (FIRQ_BADBLK_H | FIRQ_BADBLK_L)) {
            uart_printf("BSP interrupt at bank: 0x%x", bank);
			if (fc == FC_COL_ROW_IN_PROG || fc == FC_IN_PROG || fc == FC_PROG) {
                uart_print("find runtime bad block when block program...");
			}
			else {
                uart_printf("find runtime bad block when block erase...vblock #: %d", GETREG(BSP_ROW_H(bank)) / PAGES_PER_BLK);
                g_bsp_isr_flag[bank] = GETREG(BSP_ROW_H(bank)) / PAGES_PER_BLK;

/* 				ASSERT(fc == FC_ERASE); */
			}
		}
    }
}
