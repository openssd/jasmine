// 2011 INDILINX Co., Ltd.
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
// FASTer FTL source file
//
// Author; Sang-Phil Lim (Sungkyunkwan Univ. VLDB Lab., Korea)
//
// NOTE;
//
//   - This scheme is one of the 'hybrid mapping FTL' which shows relatively good performance on write-skewed workloads than FAST FTL
//     (e.g., 70/30, 80/20, OLTP workload)
//   - In my observation, this scheme shows poor performance than DAC FTL, but the memory requirement is quite low and provide stable performance.
//   - And also, in terms of SPOR, I think it can get more chances than page mapping FTLs to adopting a light-weight recovery algorithm.
//     (such as, Sungup Moon et al, "Crash recovery in FAST FTL", SEUS 2010
//
// Features;
//
//   - normal POR support
//
// Reference;
//
//   - Sang-Phil Lim et al, "FASTer FTL for Enterprise-Class Flash Memory SSDs", IEEE SNAPI 2010.

#include "jasmine.h"

//----------------------------------
// macro
//----------------------------------
#define SW_LOG_LBN          0
#define MISCBLK_VBN         0x1 // vblock #1 <- misc metadata
#define META_BLKS_PER_BANK  (1 + 1 + MAP_BLK_PER_BANK) // include block #0, misc, map block

//----------------------------------
// FTL metadata (maintain in SRAM)
//----------------------------------
extern SHASHTBL       g_shashtbl[NUM_BANKS]; // ../target_spw/shashtbl.c
static misc_metadata  g_misc_meta[NUM_BANKS];
static ftl_statistics g_ftl_statistics[NUM_BANKS];
// volatile metadata
static UINT32		  g_bad_blk_count[NUM_BANKS];
static BOOL32         g_bsp_isr_flag[NUM_BANKS];

static BOOL32 g_gc_flag[NUM_BANKS];
static UINT8  g_mem_to_clr[PAGES_PER_BLK / 8];
static UINT8  g_mem_to_set[PAGES_PER_BLK / 8];

// SATA read/write buffer pointer id
UINT32 		  g_ftl_read_buf_id;
UINT32 		  g_ftl_write_buf_id;

//----------------------------------
// logical NAND flash memory layout
//----------------------------------
// block #0: scan list, firmware binary image, etc.
// block #1: FTL misc. metadata (including map block vpn information) (fixed location)
// block #2: maintain data/log/isolation/free block mapping table (map block lbn #0)
// block #3: maintain log page mapping hash table (map block lbn #1)
// block #4: FTL bitmap tables(validation check, second chance), vblock age info (map block lbn #2)
// block #5~x-1: data blocks
// block #x~y-1: log blocks
// block #y~z-1: isolation blocks
// block #z~k-1: free blocks

//----------------------------------
// general macro functions
//----------------------------------
#define get_num_bank(lpn) ((lpn) % NUM_BANKS) // page-level striping technique (I/O parallelism)

#define CHECK_LPAGE(lpn)                     ASSERT((lpn) < (NUM_BANKS * DATA_BLK_PER_BANK *PAGES_PER_BLK))
#define CHECK_VPAGE(vpn)                     ASSERT((vpn) < (NUM_BANKS * VBLKS_PER_BANK * PAGES_PER_BLK))
#define check_block_offset_boundary(offset)  ASSERT((offset) < PAGES_PER_BLK)

#define get_miscblk_vpn(bank)                (g_misc_meta[bank].cur_miscblk_vpn)
#define inc_miscblk_vpn(bank)                (g_misc_meta[bank].cur_miscblk_vpn++)
#define set_miscblk_vpn(bank, vpn)           (g_misc_meta[bank].cur_miscblk_vpn = vpn)
#define get_mapblk_vpn(bank, mapblk_lbn)     (g_misc_meta[bank].cur_mapblk_vpn[mapblk_lbn])
#define inc_mapblk_vpn(bank, mapblk_lbn)     (g_misc_meta[bank].cur_mapblk_vpn[mapblk_lbn]++)
#define set_mapblk_vpn(bank, mapblk_lbn, vpn) (g_misc_meta[bank].cur_mapblk_vpn[mapblk_lbn] = vpn)
#define get_data_lbn(data_lpn)               ((data_lpn) / NUM_BANKS / PAGES_PER_BLK)
#define get_data_offset(data_lpn)            (((data_lpn) / NUM_BANKS) % PAGES_PER_BLK)
#define get_bank_of_data_lpn(data_lpn)       ((data_lpn) % NUM_BANKS)
#define get_data_lpn(bank, data_lbn, offset) (((data_lbn) * (PAGES_PER_BLK * NUM_BANKS)) + ((offset) * NUM_BANKS) + bank)
#define get_base_data_lpn(data_lpn)          ((get_data_lbn(data_lpn) * PAGES_PER_BLK * NUM_BANKS) + get_bank_of_data_lpn(data_lpn))
#define get_base_lpn_of_lbn(bank, data_lbn)  (((data_lbn) * PAGES_PER_BLK * NUM_BANKS) + bank)
#define get_next_data_lpn(data_lpn)          ((data_lpn) + NUM_BANKS)
#define get_log_lbn(log_lpn)                 ((log_lpn) / PAGES_PER_BLK)
#define get_log_offset(log_lpn)              ((log_lpn) % PAGES_PER_BLK)

#define is_full_swlog_blk(bank)             (g_misc_meta[bank].cur_write_swlog_offset == (PAGES_PER_BLK - 1))
#define is_full_rwlog_blks(bank)            (g_misc_meta[bank].rwlog_free_blk_cnt == 0)
#define is_full_isol_blks(bank)             (g_misc_meta[bank].isol_free_blk_cnt <= RESERV_ISOL_BLK)
#define inc_full_rwlog_blk_cnt(bank)        (g_misc_meta[bank].rwlog_free_blk_cnt--)
#define dec_full_rwlog_blk_cnt(bank)        (g_misc_meta[bank].rwlog_free_blk_cnt++)
#define inc_full_isol_blk_cnt(bank)         (g_misc_meta[bank].isol_free_blk_cnt--)
#define dec_full_isol_blk_cnt(bank)         (g_misc_meta[bank].isol_free_blk_cnt++)

#define get_bad_blk_cnt(bank)                  (g_bad_blk_count[bank])
#define inc_mapblk_vpn(bank, mapblk_lbn)       (g_misc_meta[bank].cur_mapblk_vpn[mapblk_lbn]++)
#define get_cur_write_rwlog_lpn(bank)          (g_misc_meta[bank].cur_write_rwlog_lpn)
#define get_rwlog_offset(bank)                 (g_misc_meta[bank].cur_write_rwlog_lpn % PAGES_PER_BLK)
#define set_cur_write_rwlog_lpn(bank, log_lpn) (g_misc_meta[bank].cur_write_rwlog_lpn = log_lpn)
#define get_cur_write_isol_lpn(bank)           (g_misc_meta[bank].cur_write_isol_lpn)
#define set_cur_write_isol_lpn(bank, isol_lpn) (g_misc_meta[bank].cur_write_isol_lpn = isol_lpn)
#define inc_cur_write_rwlog_lpn(bank)          (g_misc_meta[bank].cur_write_rwlog_lpn = (g_misc_meta[bank].cur_write_rwlog_lpn + 1) % (LOG_BLK_PER_BANK * PAGES_PER_BLK))
#define inc_cur_write_isol_lpn(bank)           (g_misc_meta[bank].cur_write_isol_lpn = (g_misc_meta[bank].cur_write_isol_lpn + 1) % (ISOL_BLK_PER_BANK * PAGES_PER_BLK))

#define get_vt_isol_lpn(bank)               (g_misc_meta[bank].cur_vt_isol_lpn)
#define inc_vt_isol_lpn(bank)               (g_misc_meta[bank].cur_vt_isol_lpn = (g_misc_meta[bank].cur_vt_isol_lpn + 1) % (ISOL_BLK_PER_BANK * PAGES_PER_BLK))
#define get_data_lbn_of_swlog_blk(bank)     (g_misc_meta[bank].swlog_data_lbn)
#define set_data_lbn_of_swlog_blk(bank, data_lbn) (g_misc_meta[bank].swlog_data_lbn = data_lbn)
#define get_swlog_offset(bank)              (g_misc_meta[bank].cur_write_swlog_offset)
#define set_swlog_offset(bank, offset)      (g_misc_meta[bank].cur_write_swlog_offset = offset)
#define inc_swlog_offset(bank)              (g_misc_meta[bank].cur_write_swlog_offset++)

//----------------------------------
// FTL internal function prototype
//----------------------------------
static void   sanity_check(void);
static void   build_bad_blk_list(void);
static void   set_data_vbn(UINT32 const bank, UINT32 const data_lbn, UINT32 const vblock);
static void   set_log_vbn(UINT32 const bank, UINT32 const log_lbn, UINT32 const vblock);
static void   set_isol_vbn(UINT32 const bank, UINT32 const isol_lbn, UINT32 const vblock);
static void   set_data_lpn_in_log_blk(UINT32 const bank, UINT32 const log_lpn, UINT32 const data_lpn);
static void   adj_data_lpn_in_log_blk(UINT32 const bank, UINT32 const new_log_lpn, UINT32 const data_lpn);
static void   set_scbit(UINT32 const bank, UINT32 const log_lpn);
static void   clr_scbit(UINT32 const bank, UINT32 const log_lpn);
static void   mark_invalid_in_data_blk(UINT32 const bank, UINT32 const data_lpn);
static void   mark_valid_in_data_blk(UINT32 const bank, UINT32 const data_lpn);
static void   read_lpn_list_of_vblock(UINT32 const bank, UINT32 const vblock);
static void   ret_free_vbn(UINT32 const bank, UINT32 const vblock);
static void   switch_merge(UINT32 const bank);
static void   update_block_wearout_info(UINT32 const bank, UINT32 const vblock);
static void   inc_block_erase_cnt(UINT32 const bank, UINT32 const vblock);
static void   partial_merge(UINT32 const bank);
static void   full_merge(UINT32 const bank);
static void   set_all_scbit_in_log_blk(UINT32 const bank, UINT32 const log_lbn);
static void   clr_all_scbit_in_log_blk(UINT32 const bank, UINT32 const log_lbn);
static void   explicit_wear_leveling(void);
static void   format(void);
static void   write_to_log_block(UINT32 const lpn, UINT32 const sect_offset, UINT32 const num_sectors);
static void   init_metadata_sram(void);
static void   flush_misc_metadata(void);
static void   flush_dram_metadata_into_mapblk(UINT32 const mapblk_lbn, UINT32 const dram_addr, UINT32 const dram_bytes);
static void   load_metadata(void);
static void   load_misc_metadata(void);
static void   load_dram_metadata_from_mapblk(UINT32 const mapblk_lbn, UINT32 const dram_addr, UINT32 const dram_bytes);
static void   write_format_mark(void);
static void   garbage_collection(UINT32 const bank);
static void   set_bad_block(UINT32 const bank, UINT32 const vblk_offset);
static UINT32 assign_cur_write_rwlog_lpn(UINT32 const bank);
static UINT32 search_valid_in_log_area(UINT32 const bank, UINT32 const data_lpn);
static UINT32 prepare_to_new_write(UINT32 const bank, UINT32 const data_lpn);
static UINT32 get_vpn(UINT32 const bank, UINT32 const lpn);
static UINT32 get_data_vbn(UINT32 const bank, UINT32 const data_lbn);
static UINT32 get_log_vbn(UINT32 const bank, UINT32 const log_lbn);
static UINT32 get_isol_vbn(UINT32 const bank, UINT32 const isol_lbn);
static UINT32 get_vt_log_lbn_for_gc(UINT32 const bank);
static UINT32 is_ok_write_to_sw_logblk(UINT32 const bank, UINT32 const data_lpn);
static UINT32 get_free_vbn(UINT32 const bank);
static UINT32 get_update_cnt(UINT32 const bank, UINT32 const data_lbn);
static BOOL32 get_scbit(UINT32 const bank, UINT32 const log_lpn);
static BOOL32 is_bad_block(UINT32 const bank, UINT32 const vblk_offset);
static BOOL32 is_valid_in_log_area(UINT32 const bank, UINT32 const data_lpn);
static BOOL32 check_format_mark(void);
/* static void   set_data_lpn_in_log_pmap(UINT32 const bank, UINT32 const log_lpn, UINT32 const data_lpn); */
/* static void   mark_invalid_in_log_pmap(UINT32 const bank, UINT32 const log_lpn); */
/* static UINT32 get_data_lpn_in_log_pmap(UINT32 const bank, UINT32 const log_lpn); */

static void sanity_check(void)
{
    UINT32 dram_requirement = RD_BUF_BYTES + WR_BUF_BYTES + DRAM_BYTES_OTHER;

    if ((dram_requirement > DRAM_SIZE) ||
        (sizeof(misc_metadata) > BYTES_PER_PAGE)) {
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

	for (bank = 0; bank < NUM_BANKS; bank++) {
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

		if (BSP_INTR(bank) & FIRQ_DATA_CORRUPT) {
			result = FAIL;
		}
		else {
			UINT32 i;

			num_entries = read_dram_16(&(scan_list->num_entries));

			if (num_entries > SCAN_LIST_ITEMS) {
				result = FAIL;
			}
			else {
				for (i = 0; i < num_entries; i++) {
					UINT16 entry = read_dram_16(scan_list->list + i);
					UINT16 pblk_offset = entry & 0x7FFF;

					if (pblk_offset == 0 || pblk_offset >= PBLKS_PER_BANK) {
                        #if OPTION_REDUCED_CAPACITY == FALSE
						result = FAIL;
                        #endif
					}
					else {
						write_dram_16(scan_list->list + i, pblk_offset);
					}
				}
			}
		}
		if (result == FAIL) {
			num_entries = 0;
		}
		else {
			write_dram_16(&(scan_list->num_entries), 0);
		}
		g_bad_blk_count[bank] = 0;

		for (vblk_offset = 1; vblk_offset < VBLKS_PER_BANK; vblk_offset++) {
			BOOL32 bad = FALSE;

            #if OPTION_2_PLANE
            {
				UINT32 pblk_offset;

				pblk_offset = vblk_offset * NUM_PLANES;

				if (mem_search_equ_dram(scan_list, sizeof(UINT16), num_entries + 1, pblk_offset) < num_entries + 1) {
					bad = TRUE;
				}
				pblk_offset = vblk_offset * NUM_PLANES + 1;

				if (mem_search_equ_dram(scan_list, sizeof(UINT16), num_entries + 1, pblk_offset) < num_entries + 1) {
					bad = TRUE;
				}
			}
            #else
			{
				if (mem_search_equ_dram(scan_list, sizeof(UINT16), num_entries + 1, vblk_offset) < num_entries + 1)	{
					bad = TRUE;
				}
			}
            #endif

			if (bad) {
				g_bad_blk_count[bank]++;
				set_bit_dram(BAD_BLK_BMP_ADDR + bank*(VBLKS_PER_BANK/8 + 1), vblk_offset);
			}
		}
	}
}
void ftl_open(void)
{
/*     UINT32 volatile g_break = 0; */
/*     while (g_break == 0); */

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
/* 	if (check_format_mark() == FALSE) { */
	if (TRUE) {
		format();
	}
    // load FTL metadata
    else {
        load_metadata();
    }
	g_ftl_read_buf_id = 0;
	g_ftl_write_buf_id = 0;

    mem_set_sram(g_mem_to_set, 0xFFFFFFFF, PAGES_PER_BLK / 8);
    mem_set_sram(g_mem_to_clr, NULL, PAGES_PER_BLK / 8);

}
// TODO
static void format(void)
{
    uart_printf("Total FTL DRAM metadata size: %d KB", DRAM_BYTES_OTHER / 1024);

    uart_print("do format");
    uart_print("NUM_PSECTORS");
    uart_print_32(NUM_PSECTORS);
    uart_print("NUM_LSECTORS");
    uart_print_32(NUM_LSECTORS);
    uart_print("VBLKS_PER_BANK");
    uart_print_32(VBLKS_PER_BANK);
    uart_print("DATA_BLK_PER_BANK");
    uart_print_32(DATA_BLK_PER_BANK);
    uart_print("LOG_BLK_PER_BANK");
    uart_print_32(LOG_BLK_PER_BANK);
    uart_print("ISOL_BLK_PER_BANK");
    uart_print_32(ISOL_BLK_PER_BANK);
    uart_print("FREE_BLK_PER_BANK");
    uart_print_32(FREE_BLK_PER_BANK);

    //----------------------------------------
    // initialize DRAM metadata
    //----------------------------------------
    // data/log/isolation/free block mapping table
    mem_set_dram(DATA_BMT_ADDR, NULL, DATA_BMT_BYTES);
    mem_set_dram(LOG_BMT_ADDR, NULL, LOG_BMT_BYTES);
    mem_set_dram(ISOL_BMT_ADDR, NULL, ISOL_BMT_BYTES);
    mem_set_dram(FREE_BMT_ADDR, NULL, FREE_BMT_BYTES);
    // log page mapping table (linear) - NOT USED
/*     mem_set_dram(LOG_PMT_ADDR, INVALID, LOG_PMT_BYTES); */
    // validation check bitmap table
    mem_set_dram(VC_BITMAP_ADDR, NULL, VC_BITMAP_BYTES);
    // second chance bitmap table
    mem_set_dram(SC_BITMAP_ADDR, NULL, SC_BITMAP_BYTES);
    // block wearout table
    mem_set_dram(BLK_ERASE_CNT_ADDR, NULL, BLK_ERASE_CNT_BYTES);

    // setting map/data/log/isolation/free block mapping table
    // NOTE: exclude bad blocks
    UINT32 lbn, vblock;
    for (UINT32 bank = 0; bank < NUM_BANKS; bank++) {
        vblock = MISCBLK_VBN;

        // misc. block (fixed location)
        nand_block_erase(bank, vblock);

        g_bsp_isr_flag[bank] = INVALID;
        // map block
        for (lbn = 0; lbn < MAP_BLK_PER_BANK;) {
            vblock++;
            if (is_bad_block(bank, vblock) == TRUE) {
                continue;
            }
            nand_block_erase_sync(bank, vblock);
            if (g_bsp_isr_flag[bank] != INVALID) {
                set_bad_block(bank, g_bsp_isr_flag[bank]);
                g_bsp_isr_flag[bank] = INVALID;
                continue;
            }
            set_mapblk_vpn(bank, lbn, vblock * PAGES_PER_BLK - 1);
            lbn++;
        }
        // data block mapping table
        for (lbn = 0; lbn < DATA_BLK_PER_BANK;) {
            vblock++;
            if (is_bad_block(bank, vblock) == TRUE) {
                continue;
            }
            nand_block_erase_sync(bank, vblock);
            if (g_bsp_isr_flag[bank] != INVALID) {
                set_bad_block(bank, g_bsp_isr_flag[bank]);
                g_bsp_isr_flag[bank] = INVALID;
                continue;
            }
            set_data_vbn(bank, lbn, vblock);
            lbn++;
        }
        // free block mapping table
        for (lbn = 0; lbn < FREE_BLK_PER_BANK;) {
            vblock++;
            if (is_bad_block(bank, vblock) == TRUE) {
                continue;
            }
            nand_block_erase_sync(bank, vblock);
            if (g_bsp_isr_flag[bank] != INVALID) {
                set_bad_block(bank, g_bsp_isr_flag[bank]);
                g_bsp_isr_flag[bank] = INVALID;
                continue;
            }
            ret_free_vbn(bank, vblock);
            lbn++;
        }
        // isolation block mapping table
        for (lbn = 0; lbn < ISOL_BLK_PER_BANK;) {
            vblock++;
            if (is_bad_block(bank, vblock) == TRUE) {
                continue;
            }
            nand_block_erase_sync(bank, vblock);
            if (g_bsp_isr_flag[bank] != INVALID) {
                set_bad_block(bank, g_bsp_isr_flag[bank]);
                g_bsp_isr_flag[bank] = INVALID;
                continue;
            }
            set_isol_vbn(bank, lbn, vblock);
            lbn++;
        }
        // log block mapping table
        // NOTE: log block count = LOG_BLK_PER_BANK - bad_block_count
        for (lbn = 0; vblock < VBLKS_PER_BANK;) {
            vblock++;
            if (vblock >= VBLKS_PER_BANK) {
                break;
            }
            if (is_bad_block(bank, vblock) == TRUE) {
                continue;
            }
            nand_block_erase_sync(bank, vblock);
            if (g_bsp_isr_flag[bank] != INVALID) {
                set_bad_block(bank, g_bsp_isr_flag[bank]);
                g_bsp_isr_flag[bank] = INVALID;
                continue;
            }
            set_log_vbn(bank, lbn, vblock);
            lbn++;
        }
        uart_printf("above log blocks are invalid..bank %d lbn %d", bank, lbn);
        // set remained log blocks as `invalid'
        while (lbn < LOG_BLK_PER_BANK) {
            write_dram_16(LOG_BMT_ADDR + ((bank * LOG_BLK_PER_BANK + lbn) * sizeof(UINT16)),
                          (UINT16)-1);
            lbn++;
        }
    }
    //----------------------------------------
    // initialize SRAM metadata
    //----------------------------------------
    init_metadata_sram();

    // flush FTL metadata into NAND flash
    ftl_flush();

    write_format_mark();
	led(1);
    uart_print("format complete");
}
// TODO
static void init_metadata_sram(void)
{
    for (UINT32 bank = 0; bank < NUM_BANKS; bank++) {
        set_miscblk_vpn(bank, (MISCBLK_VBN * PAGES_PER_BLK) - 1);

        mem_set_sram(&g_ftl_statistics[bank], NULL, sizeof(ftl_statistics));
        set_swlog_offset(bank, ((UINT32)-1));
        set_data_lbn_of_swlog_blk(bank, INVALID);
        g_misc_meta[bank].corr_ref[0] = g_misc_meta[bank].corr_ref[1] = INVALID;
        set_cur_write_rwlog_lpn(bank, PAGES_PER_BLK - 1);
        set_cur_write_isol_lpn(bank, ((UINT32)-1));
        g_misc_meta[bank].cur_vt_isol_lpn = (UINT32)-1;

        g_misc_meta[bank].rwlog_free_blk_cnt = LOG_BLK_PER_BANK - 1 - get_bad_blk_cnt(bank); // except SW log block(1) and bad blocks
        g_misc_meta[bank].actual_rwlogblk_num = g_misc_meta[bank].rwlog_free_blk_cnt;
        uart_printf("actual rw log block number: %d", g_misc_meta[bank].actual_rwlogblk_num);

        g_misc_meta[bank].isol_free_blk_cnt  = ISOL_BLK_PER_BANK;
        // init. free block info. (already done)
        g_misc_meta[bank].write_before_pmerge = 0;
        g_misc_meta[bank].pmerge_trigger = 0;
        g_misc_meta[bank].pmerge_interval = PAGES_PER_BLK >> 2;
        g_misc_meta[bank].base_pmerge_interval = PAGES_PER_BLK >> 2;
    }
    shashtbl_init(); // log page mapping hash table
}
// flush FTL metadata(SRAM+DRAM) for normal POR
void ftl_flush(void)
{
    /* ptimer_start(); */
    // flush bit map table, block erase cnt
    flush_dram_metadata_into_mapblk(2, VC_BITMAP_ADDR, VC_BITMAP_BYTES + SC_BITMAP_BYTES + BLK_ERASE_CNT_BYTES);
    // flush log page mapping table
    flush_dram_metadata_into_mapblk(1, HASH_BUCKET_ADDR, HASH_BUCKET_BYTES + HASH_NODE_BYTES);
    // flush data/log/isolation/free block mapping table
    flush_dram_metadata_into_mapblk(0, DATA_BMT_ADDR, VC_BITMAP_BYTES + SC_BITMAP_BYTES + BLK_ERASE_CNT_BYTES);
    flush_misc_metadata(); // SRAM metadata

    /* ptimer_stop_and_uart_print(); */
}
// flush misc metadata into misc. block (vblock #1)
// Assumption: the size of misc metadata is less than BYTES_PER_PAGE
#define NUM_MISC_META_SECT   ((sizeof(misc_metadata) + sizeof(ftl_statistics) + sizeof(SHASHTBL) + BYTES_PER_SECTOR - 1) / BYTES_PER_SECTOR)
static void flush_misc_metadata(void)
{
    UINT32 const misc_meta_bytes      = sizeof(misc_metadata); // flush per bank
    UINT32 const ftl_statistics_bytes = sizeof(ftl_statistics);
    UINT32 const hashtbl_bytes        = sizeof(SHASHTBL);
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
        mem_copy(FTL_BUF(bank) + misc_meta_bytes, &g_ftl_statistics[bank], ftl_statistics_bytes);
        mem_copy(FTL_BUF(bank) + misc_meta_bytes + hashtbl_bytes, &g_shashtbl[bank], hashtbl_bytes);

        nand_page_ptprogram(bank,
                            get_miscblk_vpn(bank) / PAGES_PER_BLK,
                            get_miscblk_vpn(bank) % PAGES_PER_BLK,
                            0,
                            NUM_MISC_META_SECT,
                            FTL_BUF(bank));
    }
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
            mapblk_vpn = get_mapblk_vpn(bank, mapblk_lbn);
            nand_block_erase(bank, mapblk_vpn / PAGES_PER_BLK);
            set_mapblk_vpn(bank, mapblk_lbn, (get_free_vbn(bank) * PAGES_PER_BLK) - 1);
            ret_free_vbn(bank, mapblk_vpn / PAGES_PER_BLK);
        }
    }
    // parallel flushing FTL metata
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

/*             uart_printf("flush mapblk_lbn %d: bank %d, addr %x, bytes %d, vblock %d, page %d", mapblk_lbn, bank, pmap_addr, pmap_bytes, mapblk_vpn / PAGES_PER_BLK, mapblk_vpn % PAGES_PER_BLK); */

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
    // load data/log/isolation/free block mapping table from map block lbn #0
    load_dram_metadata_from_mapblk(0, DATA_BMT_ADDR, VC_BITMAP_BYTES + SC_BITMAP_BYTES + BLK_ERASE_CNT_BYTES);
    // load log page mapping table from map block lbn #1
    load_dram_metadata_from_mapblk(1, HASH_BUCKET_ADDR, HASH_BUCKET_BYTES + HASH_NODE_BYTES);
    // load bit map table, block erase cnt from map block lbn #2
    load_dram_metadata_from_mapblk(2, VC_BITMAP_ADDR, VC_BITMAP_BYTES + SC_BITMAP_BYTES + BLK_ERASE_CNT_BYTES);
}
// load misc + ftl_statistics metadata
// Assumption: the size of misc metadata is less than BYTES_PER_PAGE
static void load_misc_metadata(void)
{
    UINT32 const misc_meta_bytes      = sizeof(misc_metadata); // flush per bank
    UINT32 const ftl_statistics_bytes = sizeof(ftl_statistics);
    UINT32 const hashtbl_bytes        = sizeof(SHASHTBL);

    UINT32 load_flag = 0;
    UINT32 bank;
    UINT32 load_cnt = 0;
    UINT32 miscblk_vbn = MISCBLK_VBN;

    flash_finish();

	disable_irq();
	flash_clear_irq();	// clear any flash interrupt flags that might have been set

    // scan valid metadata in descending order from last page offset
    for (UINT32 page_num = PAGES_PER_BLK - 1; page_num != ((UINT32) -1); page_num--) {
        for (bank = 0; bank < NUM_BANKS; bank++) {
            if (load_flag & (0x1 << bank)) {
                continue;
            }
            // read valid metadata from misc. metadata area
            nand_page_ptread(bank,
                             miscblk_vbn,
                             page_num,
                             0,
                             NUM_MISC_META_SECT,
                             FTL_BUF(bank),
                             RETURN_ON_ISSUE);
        }
        flash_finish();
        for (bank = 0; bank < NUM_BANKS; bank++) {

            // checking read ALL 0xFF?
            if (!(load_flag & (0x1 << bank)) && !(BSP_INTR(bank) & FIRQ_ALL_FF)) {
                 // now we read misc metadata from NAND successfully
                load_flag = load_flag | (0x1 << bank);
                load_cnt++;
            }
            CLR_BSP_INTR(bank, 0xFF);
        }
    }
    ASSERT(load_cnt == NUM_BANKS);

    // copy to SRAM misc. metadata from FTL buffer
    for (bank = 0; bank < NUM_BANKS; bank++) {
        // 1. misc. metadata
        mem_copy(&g_misc_meta[bank], FTL_BUF(bank), misc_meta_bytes);
        // 2. ftl statstics
        mem_copy(&g_ftl_statistics[bank], FTL_BUF(bank) + misc_meta_bytes, ftl_statistics_bytes);
        // 3. hash table pointer
        mem_copy(&g_shashtbl[bank], FTL_BUF(bank) + misc_meta_bytes + hashtbl_bytes, hashtbl_bytes);
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

	if (temp != 0) {
		return FALSE;	// the page contains all-0xFF (the format mark does not exist.)
	}
	else {
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

    while (remain_sects != 0) {

        if ((sect_offset + remain_sects) < SECTORS_PER_PAGE) {
            num_sectors_to_read = remain_sects;
        }
        else {
            num_sectors_to_read = SECTORS_PER_PAGE - sect_offset;
        }
        bank = get_num_bank(lpn); // page striping
        vpn  = get_vpn(bank, lpn);
        CHECK_VPAGE(vpn);
        g_ftl_statistics[bank].total_read_cnt++;

        if (vpn != NULL) {
            nand_page_ptread_to_host(bank,
                                     vpn / PAGES_PER_BLK,
                                     vpn % PAGES_PER_BLK,
                                     sect_offset,
                                     num_sectors_to_read);
        }
        // The host is requesting to read a logical page that has never been written to.
        else {
			UINT32 next_read_buf_id = (g_ftl_read_buf_id + 1) % NUM_RD_BUFFERS;

            #if OPTION_FTL_TEST == FALSE
            // wait if the read buffer is full (slow host)
			while (next_read_buf_id == GETREG(SATA_RBUF_PTR));
            #endif
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

    while (remain_sects != 0) {
        if ((sect_offset + remain_sects) < SECTORS_PER_PAGE) {
            num_sectors_to_write = remain_sects;
        }
        else {
            num_sectors_to_write = SECTORS_PER_PAGE - sect_offset;
        }
        // single page write for the host data individually
        write_to_log_block(lpn, sect_offset, num_sectors_to_write);
        sect_offset   = 0;
        remain_sects -= num_sectors_to_write;
        lpn++;
    }
}
/* #define ADJ_PMERGE_PERIOD   (DATA_BLK_PER_BANK >> 2) */
#define ADJ_PMERGE_PERIOD   (LOG_BLK_PER_BANK * PAGES_PER_BLK >> 3)
BOOL32 g_inc_pmerge_interval_flag[NUM_BANKS];
static void write_to_log_block(UINT32 const lpn, UINT32 const sect_offset, UINT32 const num_sectors)
{
    CHECK_LPAGE(lpn);
    ASSERT(sect_offset < SECTORS_PER_PAGE);
    ASSERT(num_sectors > 0 && num_sectors <= SECTORS_PER_PAGE);

    UINT32 bank;
    UINT32 vblock, page_num, page_offset, column_cnt;

    bank        = get_num_bank(lpn); // page striping
    page_offset = sect_offset;
    column_cnt  = num_sectors;

    g_gc_flag[bank] = FALSE;
    g_ftl_statistics[bank].total_write_cnt++;
    g_misc_meta[bank].write_before_pmerge++;

    //------------------------------------
    //pre-processing before writing new data
    //------------------------------------
    UINT32 new_log_lpn = prepare_to_new_write(bank, lpn);
    #if OPTION_ENABLE_ASSERT
    ASSERT(new_log_lpn != INVALID);
    ASSERT(new_log_lpn % PAGES_PER_BLK != (PAGES_PER_BLK));
    ASSERT(new_log_lpn < (LOG_BLK_PER_BANK * PAGES_PER_BLK));
    if (new_log_lpn / PAGES_PER_BLK == SW_LOG_LBN) {
        ASSERT(get_data_lbn_of_swlog_blk(bank) != INVALID);
        ASSERT(get_log_offset(new_log_lpn) == get_data_offset(lpn));
    }
    #endif
    UINT32 src_vbn, src_offset;

    // NOTE: partial write req.
    if (column_cnt != SECTORS_PER_PAGE) {
        if (is_valid_in_log_area(bank, lpn)) {
            UINT32 old_log_lpn = search_valid_in_log_area(bank, lpn);
            if (get_log_lbn(old_log_lpn) < LOG_BLK_PER_BANK) {
                src_vbn = get_log_vbn(bank, get_log_lbn(old_log_lpn));
            }
            else {
                src_vbn = get_isol_vbn(bank, get_log_lbn(old_log_lpn) - LOG_BLK_PER_BANK);
            }
            src_offset = get_log_offset(old_log_lpn);

            shashtbl_remove(bank, lpn);
            ASSERT(shashtbl_get(bank, lpn) == INVALID);
/*             // data invalid in the log area */
/*             mark_invalid_in_log_pmap(bank, old_log_lpn); */
        }
        else {
            src_vbn = get_data_vbn(bank, get_data_lbn(lpn));
            src_offset = get_data_offset(lpn);

            // data invalidation in the data block
            mark_invalid_in_data_blk(bank, lpn);
        }
        ASSERT(src_vbn < VBLKS_PER_BANK);
        ASSERT(src_offset < PAGES_PER_BLK);
        //--------------------------------------------------------------------------------------
        // `Partial programming'
        // we could not determine whether the new data is loaded in the SATA write buffer.
        // Thus, read the left/right hole sectors of a valid page and copy into the write buffer.
        // And then, program whole valid data
        //--------------------------------------------------------------------------------------
        // read partial valid data from data/log block
        // left hole sector
        if (page_offset != 0) {
            nand_page_ptread(bank,
                             src_vbn, src_offset,
                             0, page_offset,
                             WR_BUF_PTR(g_ftl_write_buf_id),
                             RETURN_ON_ISSUE);
        }
        // right hole sector
        if ((page_offset + column_cnt) < SECTORS_PER_PAGE) {
            nand_page_ptread(bank,
                             src_vbn, src_offset,
                             page_offset + column_cnt, SECTORS_PER_PAGE - (page_offset + column_cnt),
                             WR_BUF_PTR(g_ftl_write_buf_id),
                             RETURN_ON_ISSUE);
        }
        page_offset = 0;
        column_cnt  = SECTORS_PER_PAGE;
    }
    else {
        if (is_valid_in_log_area(bank, lpn)) {
            UINT32 old_log_lpn = search_valid_in_log_area(bank, lpn);
            shashtbl_remove(bank, lpn);
            ASSERT(shashtbl_get(bank, lpn) == INVALID);

/*             // data invalid in the log area */
/*             mark_invalid_in_log_pmap(bank, old_log_lpn); */
        }
        else {
            // data invalidation in the data block
            mark_invalid_in_data_blk(bank, lpn);
        }
    }
    vblock   = get_log_vbn(bank, get_log_lbn(new_log_lpn));
    page_num = get_log_offset(new_log_lpn);
    ASSERT(vblock < VBLKS_PER_BANK);
    ASSERT(page_num < PAGES_PER_BLK);

    //----
    // write new data (make sure that the new data is ready in the write buffer frame)
    // (c.f FO_B_SATA_W flag in flash.h)
    //----
    nand_page_ptprogram_from_host(bank,
                                  vblock,
                                  page_num,
                                  page_offset,
                                  column_cnt);
    //----
    // update log page mapping table
    //----
    set_data_lpn_in_log_blk(bank, new_log_lpn, lpn);
/*     set_data_lpn_in_log_pmap(bank, new_log_lpn, lpn); */

    // if sw log block is full
    if (is_full_swlog_blk(bank)) {
        switch_merge(bank);
        g_gc_flag[bank] = TRUE;
    }
    //------------------------
    // do progressive merge
    //------------------------
    if (g_gc_flag[bank] == FALSE) {
        if (is_full_isol_blks(bank)) {
            full_merge(bank);
            g_inc_pmerge_interval_flag[bank] = TRUE;
        }
        else if ((ISOL_BLK_PER_BANK - 1) > g_misc_meta[bank].isol_free_blk_cnt &&
                 new_log_lpn / PAGES_PER_BLK != SW_LOG_LBN) {
            g_misc_meta[bank].pmerge_trigger++;

            if (g_misc_meta[bank].pmerge_trigger >= (g_misc_meta[bank].pmerge_interval + bank << 1)) {
                full_merge(bank);
                g_misc_meta[bank].pmerge_trigger = 0;
            }
        }
    }
    // self-tuning progressive merge interval
    if (g_misc_meta[bank].write_before_pmerge >= ADJ_PMERGE_PERIOD &&
        g_misc_meta[bank].isol_free_blk_cnt < ISOL_BLK_PER_BANK) {

        if (g_misc_meta[bank].base_pmerge_interval < 2) {
            g_misc_meta[bank].base_pmerge_interval = PAGES_PER_BLK >> 2;
        }
        if (g_misc_meta[bank].isol_free_blk_cnt > (ISOL_BLK_PER_BANK - 2)) {
            g_misc_meta[bank].pmerge_interval++;
        }
        else if (g_inc_pmerge_interval_flag[bank]) {
            if (g_misc_meta[bank].base_pmerge_interval > 1) {
                g_misc_meta[bank].base_pmerge_interval--;
            }
            g_misc_meta[bank].pmerge_interval = g_misc_meta[bank].base_pmerge_interval;
            g_inc_pmerge_interval_flag[bank]  = FALSE;
        }
        g_misc_meta[bank].write_before_pmerge = 0;
    }
    // TODO
    // explicit wear leveling
    if (!g_gc_flag[bank]) {
        explicit_wear_leveling();
    }
}
// get_vpn to find up-to-date data
static UINT32 get_vpn(UINT32 const bank, UINT32 const lpn)
{
    CHECK_LPAGE(lpn);

    UINT32 vpn;

    // if up-to-date data for the target lpn exists in the log area, then
    // read from log block
    if (is_valid_in_log_area(bank, lpn)) {
        // search valid data in the log space
        UINT32 log_lpn = search_valid_in_log_area(bank, lpn);

        UINT32 src_vbn;
        if (get_log_lbn(log_lpn) < LOG_BLK_PER_BANK) {
            src_vbn = get_log_vbn(bank, get_log_lbn(log_lpn));
        }
        else {
            src_vbn = get_isol_vbn(bank, get_log_lbn(log_lpn) - LOG_BLK_PER_BANK);
        }
        vpn  = (src_vbn * PAGES_PER_BLK) + (log_lpn % PAGES_PER_BLK);
    }
    // else, read from data block
    else {
        UINT32 data_vbn = get_data_vbn(bank, get_data_lbn(lpn));
        vpn = (data_vbn * PAGES_PER_BLK) + get_data_offset(lpn);
    }
    return vpn;
}
// search lpn of target valid page from log page mapping table
// using static hash table
// return: log_lpn
static UINT32 search_valid_in_log_area(UINT32 const bank, UINT32 const data_lpn)
{
    UINT32 log_lpn = shashtbl_get(bank, data_lpn);
    ASSERT(log_lpn != INVALID);

    return log_lpn;
}
// search lpn of target valid page from log page mapping table using H/W search engine
/* static UINT32 search_valid_in_log_area(UINT32 const bank, UINT32 const data_lpn) */
/* { */
/* } */
static UINT32 assign_cur_write_rwlog_lpn(UINT32 const bank)
{
    // get previous written rw log lpn;
    UINT32 log_lpn = get_cur_write_rwlog_lpn(bank);
    UINT32 log_lbn = get_log_lbn(log_lpn);

    // if current rw log block is full
    if ((log_lpn % PAGES_PER_BLK) == (PAGES_PER_BLK - 2)) {
        //write lpn list in the last page of current rw log block
        mem_copy(FTL_BUF(bank), g_misc_meta[bank].lpn_list_of_rwlog_blk, sizeof(UINT32) * PAGES_PER_BLK);
        nand_page_ptprogram(bank,
                            get_log_vbn(bank, log_lpn / PAGES_PER_BLK), PAGES_PER_BLK - 1,
                            0, (sizeof(UINT32) * PAGES_PER_BLK + BYTES_PER_SECTOR - 1) / BYTES_PER_SECTOR,
                            FTL_BUF(bank));
        // inc rw log block full cnt;
        inc_full_rwlog_blk_cnt(bank);

        // check 'full state of rw log area'
        while (is_full_rwlog_blks(bank)) {
            garbage_collection(bank);
        }
        if (get_cur_write_rwlog_lpn(bank) % PAGES_PER_BLK != (PAGES_PER_BLK - 2)) {
            inc_cur_write_rwlog_lpn(bank);
            return get_cur_write_rwlog_lpn(bank);
        }
        log_lbn = get_log_lbn(get_cur_write_rwlog_lpn(bank));

        do {
            log_lbn = (log_lbn + 1) % LOG_BLK_PER_BANK;
        } while (get_log_vbn(bank, log_lbn) == (UINT16)-1);
    }
    if (get_log_lbn(log_lpn) != log_lbn) {
        if (log_lbn == 0) {
            set_cur_write_rwlog_lpn(bank, PAGES_PER_BLK);
        }
        else {
            set_cur_write_rwlog_lpn(bank, log_lbn * PAGES_PER_BLK);
        }
    }
    else {
        inc_cur_write_rwlog_lpn(bank);
    }
    return get_cur_write_rwlog_lpn(bank);
}

#define is_empty_swlogblk(bank) (g_misc_meta[bank].swlog_data_lbn == INVALID)
// prepare to write new data in the log area
// return: log_lpn to write new data
#define SW_APPEND 0
#define SW_DETECT 1
#define SW_NO     2
static UINT32 prepare_to_new_write(UINT32 const bank, UINT32 const data_lpn)
{
    // check whether new write is possible to write into sw log block or not,
    switch (is_ok_write_to_sw_logblk(bank, data_lpn)) {
        case SW_NO:
            // get next write rw log lpn
            return assign_cur_write_rwlog_lpn(bank);
        case SW_APPEND:
            // append data into sw log block
            inc_swlog_offset(bank);
            ASSERT(get_swlog_offset(bank) < PAGES_PER_BLK);
            ASSERT(get_data_offset(data_lpn) != 0);
            return get_swlog_offset(bank);
        case SW_DETECT:
            // if sw log block already has data, merge first.
            if (!is_empty_swlogblk(bank)) {
                partial_merge(bank);
            }
            // and then, copyback valid pages
            // which lpns are smaller than current data_lpn into the sw log block
            UINT32 base_data_lpn;
            UINT32 src_lpn;
            UINT32 src_offset;
            UINT32 src_vbn;

            base_data_lpn = get_base_data_lpn(data_lpn);
            ASSERT(get_data_offset(base_data_lpn) == 0);
            ASSERT(get_swlog_offset(bank) == ((UINT32)-1));

            while(base_data_lpn < data_lpn) {
                inc_swlog_offset(bank);
                check_block_offset_boundary(get_swlog_offset(bank));
                ASSERT(get_swlog_offset(bank) == get_data_offset(base_data_lpn));

                set_data_lbn_of_swlog_blk(bank, get_data_lbn(data_lpn));
                // check whether valid data is in log block or not using valid_check bitmap table
                if (is_valid_in_log_area(bank, base_data_lpn)) {
                    // src_lpn <= log_lpn
                    src_lpn    = search_valid_in_log_area(bank, base_data_lpn);
                    src_offset = get_log_offset(src_lpn);

                    shashtbl_update(bank, base_data_lpn, get_swlog_offset(bank));

                    if (get_log_lbn(src_lpn) < LOG_BLK_PER_BANK) {
                        src_vbn = get_log_vbn(bank, get_log_lbn(src_lpn));
                    }
                    else {
                        src_vbn = get_isol_vbn(bank, get_log_lbn(src_lpn) - LOG_BLK_PER_BANK);
                    }
                    ASSERT(src_vbn < VBLKS_PER_BANK);
                }
                // valid data is in data block
                else {
                    // src_lpn <= data_lpn
                    src_lpn = base_data_lpn;
                    src_offset = get_data_offset(src_lpn);

                    // invalid data in data block
                    mark_invalid_in_data_blk(bank, src_lpn);
                    src_vbn = get_data_vbn(bank, get_data_lbn(src_lpn));

                    ASSERT(shashtbl_get(bank, src_lpn) == INVALID);
                    shashtbl_insert(bank, base_data_lpn, get_swlog_offset(bank));
                }
                // now, copy back valid page to sw log block
                nand_page_copyback(bank,
                                   src_vbn, src_offset,
                                   get_log_vbn(bank, SW_LOG_LBN), get_swlog_offset(bank));
                base_data_lpn = get_next_data_lpn(base_data_lpn);
            } // end while
            // consecutive sequential write detected...
            if (get_data_lbn_of_swlog_blk(bank) == INVALID) {
                set_data_lbn_of_swlog_blk(bank, get_data_lbn(data_lpn));
                ASSERT(get_swlog_offset(bank) == ((UINT32)-1));
                ASSERT(get_data_offset(data_lpn) == 0);
            }
            inc_swlog_offset(bank);
            return get_swlog_offset(bank);
        default:
            return INVALID;
    }
}
// select a victim rw log block in round-robin manner
static UINT32 get_vt_log_lbn_for_gc(UINT32 const bank)
{
    UINT32 vt_log_lbn = get_log_lbn(get_cur_write_rwlog_lpn(bank));

    do{
        vt_log_lbn = (vt_log_lbn + 1) % LOG_BLK_PER_BANK;
    }while (get_log_vbn(bank, vt_log_lbn) == (UINT16)-1);

    // except sw log block
    if (vt_log_lbn == SW_LOG_LBN) {
        vt_log_lbn = SW_LOG_LBN + 1;
        return SW_LOG_LBN + 1;
    }
    #if OPTION_ENABLE_ASSERT
    if (g_ftl_statistics[bank].gc_cnt == 0) {
        ASSERT(vt_log_lbn == (SW_LOG_LBN + 1));
    }
    #endif
    return vt_log_lbn;
}

#define COR_REF_THRESHOLD 4
#define is_same_data_lbn_with_swlog_blk(bank, data_lpn)  (get_data_lbn_of_swlog_blk(bank) == get_data_lbn(data_lpn))
static UINT32 is_ok_write_to_sw_logblk(UINT32 const bank, UINT32 const data_lpn)
{
    if (is_same_data_lbn_with_swlog_blk(bank, data_lpn)) {
        if ((get_swlog_offset(bank) + 1) == get_data_offset(data_lpn)) {
            g_misc_meta[bank].corr_ref[0] = data_lpn;
            return SW_APPEND;
        }
        return SW_NO;
    }
    // detect sequential write stream by correlated reference
    if (get_next_data_lpn(g_misc_meta[bank].corr_ref[0]) == data_lpn) {
        g_misc_meta[bank].corr_ref[0] = data_lpn;
        if (g_misc_meta[bank].corr_ref[1] > COR_REF_THRESHOLD) {
            return SW_DETECT;
        }
        g_misc_meta[bank].corr_ref[1]++;
    }
    else {
        g_misc_meta[bank].corr_ref[0] = data_lpn;
        g_misc_meta[bank].corr_ref[1] = 1;
    }
    return SW_NO;
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
// TODO
static void set_runtime_bad_blk(UINT32 const bank, UINT32 const vblk_offset)
{
    // Q) How can we handle the bad block occured in runtime?
}
// invalidation of data in data block
static void mark_invalid_in_data_blk(UINT32 const bank, UINT32 const data_lpn)
{
    UINT32 data_lbn = get_data_lbn(data_lpn);
    UINT32 offset   = get_data_offset(data_lpn);
    UINT32 adj_data_lpn = data_lbn * PAGES_PER_BLK + offset; // correct statement!

    ASSERT(tst_bit_dram(VC_BITMAP_ADDR + (bank * ((DATA_BLK_PER_BANK * PAGES_PER_BLK / 8))), adj_data_lpn) == FALSE);

    set_bit_dram(VC_BITMAP_ADDR + (bank * ((DATA_BLK_PER_BANK * PAGES_PER_BLK / 8))), adj_data_lpn);
}
// mark as valid in data block
static void mark_valid_in_data_blk(UINT32 const bank, UINT32 const data_lpn)
{
    UINT32 data_lbn = get_data_lbn(data_lpn);
    UINT32 offset   = get_data_offset(data_lpn);
    UINT32 adj_data_lpn = data_lbn * PAGES_PER_BLK + offset; // correct statement!

    ASSERT(tst_bit_dram(VC_BITMAP_ADDR + (bank * ((DATA_BLK_PER_BANK * PAGES_PER_BLK / 8))), adj_data_lpn) != FALSE);
    clr_bit_dram(VC_BITMAP_ADDR + (bank * ((DATA_BLK_PER_BANK * PAGES_PER_BLK / 8))), adj_data_lpn);
    ASSERT(tst_bit_dram(VC_BITMAP_ADDR + (bank * ((DATA_BLK_PER_BANK * PAGES_PER_BLK / 8))), adj_data_lpn) == FALSE);
}
static BOOL32 is_valid_in_log_area(UINT32 const bank, UINT32 const data_lpn)
{
    UINT32 data_lbn = get_data_lbn(data_lpn);
    UINT32 offset   = get_data_offset(data_lpn);
    UINT32 adj_data_lpn = data_lbn * PAGES_PER_BLK + offset; // correct statement!

    if (tst_bit_dram(VC_BITMAP_ADDR + (bank * ((DATA_BLK_PER_BANK * PAGES_PER_BLK / 8))), adj_data_lpn) == FALSE) {
        return FALSE;
    }
    return TRUE;
}
static UINT32 get_update_cnt(UINT32 const bank, UINT32 const data_lbn)
{
    UINT32 update_cnt = 0;
    UINT32 adj_data_lpn = data_lbn * PAGES_PER_BLK; // correct statement!
    UINT32 const base_addr = bank * (DATA_BLK_PER_BANK * PAGES_PER_BLK / 8);

    // get page update count using validation check bitmap table
    for (UINT32 page_num = 0; page_num < PAGES_PER_BLK; page_num += 4) {
        update_cnt += (tst_bit_dram(VC_BITMAP_ADDR + base_addr, adj_data_lpn++) == FALSE ? FALSE : TRUE);
        update_cnt += (tst_bit_dram(VC_BITMAP_ADDR + base_addr, adj_data_lpn++) == FALSE ? FALSE : TRUE);
        update_cnt += (tst_bit_dram(VC_BITMAP_ADDR + base_addr, adj_data_lpn++) == FALSE ? FALSE : TRUE);
        update_cnt += (tst_bit_dram(VC_BITMAP_ADDR + base_addr, adj_data_lpn++) == FALSE ? FALSE : TRUE);
    }
    ASSERT(update_cnt <= PAGES_PER_BLK);

    return update_cnt;
}
// switch the log block to an up-to-date data block
// all pages written in sw log block are valid,
static void switch_merge(UINT32 const bank)
{
    UINT32 old_data_lbn = get_data_lbn_of_swlog_blk(bank);
    UINT32 old_data_vbn = get_data_vbn(bank, old_data_lbn);
    UINT32 old_log_vbn  = get_log_vbn(bank, SW_LOG_LBN);
    UINT32 data_lpn;
    UINT32 log_lpn;

/*     uart_printf("do switch merge, bank: %d lbn: %d", bank, old_data_lbn); */

    // invalid data in log pmap/vc_bitmap
    #if OPTION_ENABLE_ASSERT
    UINT32 log_offset = get_swlog_offset(bank);
    ASSERT(log_offset == (PAGES_PER_BLK - 1));
    data_lpn = get_base_lpn_of_lbn(bank, old_data_lbn);

    for (UINT32 offset = 0; offset < PAGES_PER_BLK; offset++) {
/*         data_lpn = get_data_lpn_in_log_pmap(bank, offset); */
        log_lpn = shashtbl_get(bank, data_lpn);
        ASSERT(log_lpn != INVALID);
        data_lpn = get_next_data_lpn(data_lpn);
    }
    #endif
    // update metadata
    // invalid old versions in the log area
    data_lpn = get_base_lpn_of_lbn(bank, old_data_lbn);

    for (UINT32 offset = 0; offset < PAGES_PER_BLK; offset++) {
        log_lpn = search_valid_in_log_area(bank, data_lpn);
        // some up-to-date pages locate in rw log area
        if ((log_lpn / PAGES_PER_BLK) != SW_LOG_LBN) {
            data_lpn = get_next_data_lpn(data_lpn);
            continue;
        }
        shashtbl_remove(bank, data_lpn);
        ASSERT(shashtbl_get(bank, data_lpn) == INVALID);
        mark_valid_in_data_blk(bank, data_lpn);

        data_lpn = get_next_data_lpn(data_lpn);
    }
    // update block mapping table
    set_data_vbn(bank, old_data_lbn, old_log_vbn);
    set_data_lbn_of_swlog_blk(bank, INVALID);
    // update metadata for sw log block
    set_swlog_offset(bank, ((UINT32)-1));
    set_log_vbn(bank, SW_LOG_LBN, get_free_vbn(bank));

    // erase old data block
    nand_block_erase(bank, old_data_vbn);
    ret_free_vbn(bank, old_data_vbn);

    update_block_wearout_info(bank, old_data_vbn);
}
// partial merge for the sw log block
// NOTE: all pages written in sw log block are valid,
//       but, valid pages for the other data pages could be written in rw log area
static void partial_merge(UINT32 const bank)
{
    ASSERT(get_swlog_offset(bank) != ((UINT32)-1));
    ASSERT(get_data_lbn_of_swlog_blk(bank) != INVALID);

    inc_swlog_offset(bank);

/*     uart_printf("partial_merge occured: bank %d", bank); */

    UINT32 const swlog_offset = get_swlog_offset(bank);
    UINT32 data_lbn = get_data_lbn_of_swlog_blk(bank);
    UINT32 data_vbn = get_data_vbn(bank, data_lbn);
    UINT32 log_vbn  = get_log_vbn(bank, SW_LOG_LBN);
    UINT32 log_lpn;

    ASSERT(swlog_offset < PAGES_PER_BLK);
    ASSERT(data_lbn != INVALID);
    ASSERT(data_lbn < DATA_BLK_PER_BANK);

    UINT32 data_lpn = get_base_lpn_of_lbn(bank, data_lbn);

    for (UINT32 offset = 0; offset < PAGES_PER_BLK; offset++) {
        if (is_valid_in_log_area(bank, data_lpn)) {
            log_lpn = search_valid_in_log_area(bank, data_lpn);

            if (offset < swlog_offset) {
                if (log_lpn / PAGES_PER_BLK == SW_LOG_LBN) {
                    shashtbl_remove(bank, data_lpn);
                    ASSERT(shashtbl_get(bank, data_lpn) == INVALID);
                    mark_valid_in_data_blk(bank, data_lpn);
                }
            }
            // copy-back into sw log block
            else {
                // need to copy-back
                shashtbl_remove(bank, data_lpn);
                ASSERT(shashtbl_get(bank, data_lpn) == INVALID);
                mark_valid_in_data_blk(bank, data_lpn);

                UINT32 log_lbn = get_log_lbn(log_lpn);
                UINT32 src_vbn;
                if (log_lbn < LOG_BLK_PER_BANK) {
                    src_vbn = get_log_vbn(bank, log_lbn);
                }
                else {
                    src_vbn = get_isol_vbn(bank, log_lbn - LOG_BLK_PER_BANK);
                }
                nand_page_copyback(bank,
                                   src_vbn, get_log_offset(log_lpn),
                                   log_vbn, offset);
            }
        }
        // valid data in the data block
        else if (offset >= swlog_offset) {
            ASSERT(shashtbl_get(bank, data_lpn) == INVALID);
            nand_page_copyback(bank,
                               data_vbn, offset,
                               log_vbn, offset);
        } // end if
        data_lpn = get_next_data_lpn(data_lpn);
    } // end for

    // update block mapping table
    set_data_vbn(bank, data_lbn, log_vbn);
    set_data_lbn_of_swlog_blk(bank, INVALID);

    // update metadata for sw log block
    set_swlog_offset(bank, ((UINT32) -1));
    set_log_vbn(bank, SW_LOG_LBN, get_free_vbn(bank));

    // erase old data block
    nand_block_erase(bank, data_vbn);
    ret_free_vbn(bank, data_vbn);

    update_block_wearout_info(bank, data_vbn);
}
// full merge for the target data LBN
// NOTE: full merge operation is proceeded for cold page in the isolation area
static void full_merge(UINT32 const bank)
{
    UINT32 vt_log_lpn;
    UINT32 vt_data_lpn;

    do {
        // set next victim isol lpn to do full merge op.
        inc_vt_isol_lpn(bank);

        ASSERT((get_cur_write_isol_lpn(bank) / PAGES_PER_BLK) != (get_vt_isol_lpn(bank) / PAGES_PER_BLK));
        vt_log_lpn = (LOG_BLK_PER_BANK * PAGES_PER_BLK) + get_vt_isol_lpn(bank);

        if (vt_log_lpn % PAGES_PER_BLK == 0) {
            // read lpn list of target isol vbn
            nand_page_ptread(bank,
                             get_isol_vbn(bank, get_vt_isol_lpn(bank) / PAGES_PER_BLK), PAGES_PER_BLK - 1,
                             0, sizeof(UINT32) * PAGES_PER_BLK / BYTES_PER_SECTOR,
                             FTL_BUF(bank),
                             RETURN_WHEN_DONE);
            mem_copy(g_misc_meta[bank].lpn_list_of_vt_isol_blk, FTL_BUF(bank), sizeof(UINT32) * PAGES_PER_BLK);
        }
        ASSERT(vt_log_lpn % PAGES_PER_BLK != (PAGES_PER_BLK - 1));
        vt_data_lpn = g_misc_meta[bank].lpn_list_of_vt_isol_blk[vt_log_lpn % PAGES_PER_BLK];

        if (shashtbl_get(bank, vt_data_lpn) != vt_log_lpn) {
            vt_data_lpn = INVALID;
        }
        // if it is the last page of the victim isol block and the page is invalid,
        // then just erase isol block and return
        if ((vt_log_lpn % PAGES_PER_BLK) == (PAGES_PER_BLK - 2) && (vt_data_lpn == INVALID)) {
            dec_full_isol_blk_cnt(bank);
            // then, erase the isol block and allocate new isol block
            UINT32 isol_lbn = get_vt_isol_lpn(bank) / PAGES_PER_BLK;
            UINT32 isol_vbn = get_isol_vbn(bank, isol_lbn);
            UINT32 free_vbn = get_free_vbn(bank);

            ASSERT(isol_lbn < ISOL_BLK_PER_BANK);
            inc_vt_isol_lpn(bank); // skip last page
            set_isol_vbn(bank, isol_lbn, free_vbn);

            nand_block_erase(bank, isol_vbn);
            ret_free_vbn(bank, isol_vbn);
            update_block_wearout_info(bank, isol_vbn);
            return;
        }
    }while(vt_data_lpn == INVALID);

    CHECK_LPAGE(vt_data_lpn);

    UINT32 vt_data_lbn = get_data_lbn(vt_data_lpn);

    if (vt_data_lbn == get_data_lbn_of_swlog_blk(bank)) {
/*         uart_printf("partial merge occured in full merge period"); */
        partial_merge(bank);
        UINT32 data_lpn = g_misc_meta[bank].lpn_list_of_vt_isol_blk[vt_log_lpn % PAGES_PER_BLK];

        if (shashtbl_get(bank, data_lpn) != vt_log_lpn) {
            return;
        }
    }
    UINT32 vt_data_vbn = get_data_vbn(bank, vt_data_lbn);
    UINT32 data_lpn    = get_base_data_lpn(vt_data_lpn);
    UINT32 free_vbn    = get_free_vbn(bank);

    ASSERT(vt_data_vbn < VBLKS_PER_BANK && free_vbn < VBLKS_PER_BANK);

    UINT32 log_lpn;
    for (UINT32 offset = 0; offset < PAGES_PER_BLK; offset++) {
        if (is_valid_in_log_area(bank, data_lpn)) {
            log_lpn = search_valid_in_log_area(bank, data_lpn);

            shashtbl_remove(bank, data_lpn);
            ASSERT(shashtbl_get(bank, data_lpn) == INVALID);

            mark_valid_in_data_blk(bank, data_lpn);

            UINT32 log_lbn = get_log_lbn(log_lpn);
            UINT32 src_vbn;
            if (log_lbn < LOG_BLK_PER_BANK) {
                src_vbn = get_log_vbn(bank, log_lbn);
            }
            else {
                src_vbn = get_isol_vbn(bank, log_lbn - LOG_BLK_PER_BANK);
            }
            nand_page_copyback(bank,
                               src_vbn, get_log_offset(log_lpn),
                               free_vbn, offset);
        }
        else {
            ASSERT(shashtbl_get(bank, data_lpn) == INVALID);
            nand_page_copyback(bank,
                               vt_data_vbn, offset,
                               free_vbn, offset);
        }
        data_lpn = get_next_data_lpn(data_lpn);
    } // end for
    // update block mapping table
    set_data_vbn(bank, vt_data_lbn, free_vbn);

    // erase old data block
    nand_block_erase(bank, vt_data_vbn);
    ret_free_vbn(bank, vt_data_vbn);
    update_block_wearout_info(bank, vt_data_vbn);

    // if it is the last page of the victim isol block,
    if ((vt_log_lpn % PAGES_PER_BLK) == (PAGES_PER_BLK - 2)) {
        dec_full_isol_blk_cnt(bank);
        // then, erase the isol block and allocate new isol block
        UINT32 isol_lbn = get_vt_isol_lpn(bank) / PAGES_PER_BLK;
        UINT32 isol_vbn = get_isol_vbn(bank, isol_lbn);
        UINT32 free_vbn = get_free_vbn(bank);

        ASSERT(isol_lbn < ISOL_BLK_PER_BANK);
        set_isol_vbn(bank, isol_lbn, free_vbn);
        inc_vt_isol_lpn(bank); // skip last page

        nand_block_erase(bank, isol_vbn);
        ret_free_vbn(bank, isol_vbn);
        update_block_wearout_info(bank, isol_vbn);
    }
}
// GC for valid pages to giving a second chance
/* #define GC_THRESHOLD (PAGES_PER_BLK * 6 / 10) // 60% of pages per block */
#define GC_THRESHOLD (PAGES_PER_BLK >> 1) // 50% of pages per block (shows optimal performance)
static void garbage_collection(UINT32 const bank)
{
    g_ftl_statistics[bank].gc_cnt++;

    UINT32 vt_log_lbn   = get_vt_log_lbn_for_gc(bank);
    ASSERT(vt_log_lbn < LOG_BLK_PER_BANK);
/*     uart_printf("do garbage_collection: bank %d, log_lbn %d", bank, vt_log_lbn); */

    UINT32 log_lpn   = vt_log_lbn * PAGES_PER_BLK;
    UINT32 log_vbn   = get_log_vbn(bank, vt_log_lbn);
    UINT32 valid_cnt = 0;
    UINT32 sc_cnt    = 0; // count valid pages which are not received a second chance

    // read lpn list from nand (to verify validity of page in victim log block)
    read_lpn_list_of_vblock(bank, log_vbn);

    // calcuate 'number of valid pages' in victim log block
    for (UINT32 offset = 0; offset < (PAGES_PER_BLK - 1); offset++) {
        if (shashtbl_get(bank, g_misc_meta[bank].lpn_list_of_rwlog_blk[offset]) != log_lpn) {
            // this log page is not valid.
            g_misc_meta[bank].lpn_list_of_rwlog_blk[offset] = INVALID;
            log_lpn++;
            continue;
        }
        sc_cnt += get_scbit(bank, log_lpn);

        valid_cnt++;
        log_lpn++;
    }
    // NOTE: garbage collection optimization
    // all pages are valid & no pages have received a second chance
    if (valid_cnt == (PAGES_PER_BLK - 1) && (sc_cnt == 0)) {
        UINT32 log_lbn = vt_log_lbn;
        set_all_scbit_in_log_blk(bank, log_lbn);
        do {
            log_lbn = (log_lbn + 1) % LOG_BLK_PER_BANK;
            if (log_lbn == SW_LOG_LBN) {
                log_lbn = SW_LOG_LBN + 1;
            }
        } while(get_log_vbn(bank, log_lbn) == (UINT16)-1);
        // set previous written page
        set_cur_write_rwlog_lpn(bank, (log_lbn * PAGES_PER_BLK) - 1);
        ASSERT(log_lbn < LOG_BLK_PER_BANK);
        return;
    }
    UINT32 free_vbn        = get_free_vbn(bank);
    UINT32 free_log_offset = 0;

    ASSERT(log_vbn < VBLKS_PER_BANK && free_vbn < VBLKS_PER_BANK);

    //-----------------------------
    // normal case in GC
    //-----------------------------
    // if all pages are invalid
    if (valid_cnt == 0 && sc_cnt != 0) {
        // unset a second chace bit for all of pages
        clr_all_scbit_in_log_blk(bank, vt_log_lbn);
    }
    // all pages will be received a second chance (even already received) again
    // i.e. all pages copy back into new log block
    else if (valid_cnt != 0 && valid_cnt < GC_THRESHOLD) {
        UINT32 data_lpn;
        UINT32 target_log_lpn = vt_log_lbn * PAGES_PER_BLK; // to write copy-back page
        // unset a second chace bit for all of pages
        clr_all_scbit_in_log_blk(bank, vt_log_lbn);

        for (UINT32 offset = 0; offset < (PAGES_PER_BLK - 1); offset++) {
            data_lpn = g_misc_meta[bank].lpn_list_of_rwlog_blk[offset];
            if (data_lpn == INVALID) {
                continue;
            }
            adj_data_lpn_in_log_blk(bank, target_log_lpn, data_lpn);
            set_scbit(bank, target_log_lpn);

            nand_page_copyback(bank,
                               log_vbn, offset,
                               free_vbn, free_log_offset);
            target_log_lpn++;
            free_log_offset++;
        }
    }
    // some pages migrate to the isolation area
    else if (valid_cnt != 0){
        UINT32 scbit;
        UINT32 target_log_lpn; // to write copy-back page
        UINT32 data_lpn;
        UINT32 isol_vbn;
/*         BOOL32 sc_flag; */
/*         sc_flag = (valid_cnt < (PAGES_PER_BLK - 3)) ? TRUE : FALSE; */

        log_lpn = vt_log_lbn * PAGES_PER_BLK;
        // migrate cold page or give second chance for valid pages which are not received that
        for (UINT32 offset = 0; offset < (PAGES_PER_BLK - 1); offset++) {
            data_lpn = g_misc_meta[bank].lpn_list_of_rwlog_blk[offset];

            if (data_lpn == INVALID) {
                clr_scbit(bank, log_lpn);
                log_lpn++;
                continue;
            }
            scbit = get_scbit(bank, log_lpn);
            clr_scbit(bank, log_lpn);

/*             UINT32 update_cnt = get_update_cnt(bank, get_data_lbn(data_lpn)); */
/*             // give one more chance again */
/*             if (((scbit & sc_flag) == TRUE) && (update_cnt < 2)) { */
/*                 scbit = FALSE; */
/*             } */
            //---------------------------------------
            switch(scbit) {
                // This page is definitely cold. So, we migrate it to the isolation area
                case TRUE:
                    ASSERT(g_misc_meta[bank].isol_free_blk_cnt > 0);

                    // get next write isol lpn
                    inc_cur_write_isol_lpn(bank);
                    ASSERT((get_cur_write_isol_lpn(bank) / PAGES_PER_BLK) != (get_vt_isol_lpn(bank) / PAGES_PER_BLK));

                    // if current isolation block is full,
                    if (get_cur_write_isol_lpn(bank) % PAGES_PER_BLK == (PAGES_PER_BLK - 1)) {
                        // write lpn list in the last page of isol block
                        mem_copy(FTL_BUF(bank),
                                 g_misc_meta[bank].lpn_list_of_cur_isol_blk,
                                 sizeof(UINT32) * PAGES_PER_BLK);
                        nand_page_ptprogram(bank,
                                            get_isol_vbn(bank, get_cur_write_isol_lpn(bank) / PAGES_PER_BLK),
                                            PAGES_PER_BLK - 1,
                                            0, (sizeof(UINT32) * PAGES_PER_BLK + BYTES_PER_SECTOR - 1) / BYTES_PER_SECTOR,
                                            FTL_BUF(bank));
                        // skip last page
                        inc_cur_write_isol_lpn(bank);
                        inc_full_isol_blk_cnt(bank);
                    }
                    target_log_lpn = (LOG_BLK_PER_BANK * PAGES_PER_BLK) + get_cur_write_isol_lpn(bank);
                    ASSERT((get_cur_write_isol_lpn(bank) / PAGES_PER_BLK) != (get_vt_isol_lpn(bank) / PAGES_PER_BLK));
                    // set data lpn of current isol blk
                    g_misc_meta[bank].lpn_list_of_cur_isol_blk[target_log_lpn % PAGES_PER_BLK] = data_lpn;

                    shashtbl_update(bank, data_lpn, target_log_lpn);
                    isol_vbn = get_isol_vbn(bank, get_cur_write_isol_lpn(bank) / PAGES_PER_BLK);
                    nand_page_copyback(bank,
                                       log_vbn, offset,
                                       isol_vbn, target_log_lpn % PAGES_PER_BLK);
                    break;
                    // This page is still warm. Thus, copy-back to the log area again
                case FALSE:
                    target_log_lpn = vt_log_lbn * PAGES_PER_BLK + free_log_offset;
                    set_scbit(bank, target_log_lpn);
                    adj_data_lpn_in_log_blk(bank, target_log_lpn, data_lpn);
                    nand_page_copyback(bank,
                                       log_vbn, offset,
                                       free_vbn, free_log_offset);
                    free_log_offset++;
                    break;
            }
            log_lpn++;
        } // end for
    } // end if-else
    ASSERT(free_log_offset < (PAGES_PER_BLK - 1));

    // 3. update metadata
    set_log_vbn(bank, vt_log_lbn, free_vbn);
    nand_block_erase(bank, log_vbn);
    ret_free_vbn(bank, log_vbn);
    dec_full_rwlog_blk_cnt(bank);

    // set previous written page
    set_cur_write_rwlog_lpn(bank, ((vt_log_lbn * PAGES_PER_BLK) + free_log_offset - 1));

    update_block_wearout_info(bank, log_vbn);

    g_gc_flag[bank] = TRUE;
    /* uart_print("garbage_collection end"); */
}
static UINT32 get_free_vbn(UINT32 const bank)
{
    ASSERT(g_misc_meta[bank].free_blk_cnt > 0);

    UINT32 free_blk_offset = g_misc_meta[bank].free_list_tail;
    g_misc_meta[bank].free_list_tail = (free_blk_offset + 1) % FREE_BLK_PER_BANK;
    g_misc_meta[bank].free_blk_cnt--;
    return read_dram_16(FREE_BMT_ADDR + ((bank * FREE_BLK_PER_BANK)+ free_blk_offset) * sizeof(UINT16));
}
static void ret_free_vbn(UINT32 const bank, UINT32 const vblock)
{
    ASSERT(g_misc_meta[bank].free_blk_cnt <= FREE_BLK_PER_BANK);
    ASSERT(vblock < VBLKS_PER_BANK);
    ASSERT(is_bad_block(bank, vblock) == FALSE);

    UINT32 free_blk_offset = g_misc_meta[bank].free_list_head;
    write_dram_16(FREE_BMT_ADDR + ((bank * FREE_BLK_PER_BANK)+ free_blk_offset) * sizeof(UINT16), vblock);
    g_misc_meta[bank].free_list_head = (free_blk_offset + 1) % FREE_BLK_PER_BANK;
    g_misc_meta[bank].free_blk_cnt++;
}
// get data vbn from data block mapping table
static UINT32 get_data_vbn(UINT32 const bank, UINT32 const data_lbn)
{
    ASSERT(data_lbn < DATA_BLK_PER_BANK);

    return read_dram_16(DATA_BMT_ADDR + ((bank * DATA_BLK_PER_BANK + data_lbn) * sizeof(UINT16)));
}
// set data vbn to data block mapping table
static void set_data_vbn(UINT32 const bank, UINT32 const data_lbn, UINT32 const vblock)
{
    ASSERT(data_lbn < DATA_BLK_PER_BANK);
    ASSERT(vblock < VBLKS_PER_BANK);

    write_dram_16(DATA_BMT_ADDR + ((bank * DATA_BLK_PER_BANK + data_lbn) * sizeof(UINT16)), vblock);
}
// get log vbn from log block mapping table
static UINT32 get_log_vbn(UINT32 const bank, UINT32 const log_lbn)
{
    ASSERT(log_lbn < LOG_BLK_PER_BANK);

    return read_dram_16(LOG_BMT_ADDR + ((bank * LOG_BLK_PER_BANK + log_lbn) * sizeof(UINT16)));
}
// set log vbn to log block mapping table
static void set_log_vbn(UINT32 const bank, UINT32 const log_lbn, UINT32 const vblock)
{
    ASSERT(log_lbn < LOG_BLK_PER_BANK);
    ASSERT(vblock < VBLKS_PER_BANK);

    write_dram_16(LOG_BMT_ADDR + ((bank * LOG_BLK_PER_BANK + log_lbn) * sizeof(UINT16)), vblock);
}
static UINT32 get_isol_vbn(UINT32 const bank, UINT32 const isol_lbn)
{
    ASSERT(isol_lbn < ISOL_BLK_PER_BANK);

    return read_dram_16(ISOL_BMT_ADDR + ((bank * ISOL_BLK_PER_BANK + isol_lbn) * sizeof(UINT16)));
}
static void set_isol_vbn(UINT32 const bank, UINT32 const isol_lbn, UINT32 const vblock)
{
    ASSERT(isol_lbn < ISOL_BLK_PER_BANK);
    ASSERT(vblock < VBLKS_PER_BANK);

    write_dram_16(ISOL_BMT_ADDR + ((bank * ISOL_BLK_PER_BANK + isol_lbn) * sizeof(UINT16)), vblock);
}
/* static UINT32 get_data_lpn_in_log_pmap(UINT32 const bank, UINT32 const log_lpn) */
/* { */
/*     ASSERT(log_lpn < ((LOG_BLK_PER_BANK + ISOL_BLK_PER_BANK) * PAGES_PER_BLK)); */
/*     return read_dram_32(LOG_PMT_ADDR + (((bank * (LOG_BLK_PER_BANK + ISOL_BLK_PER_BANK) * PAGES_PER_BLK) + log_lpn) * sizeof(UINT32))); */
/* } */
/* static void set_data_lpn_in_log_pmap(UINT32 const bank, UINT32 const log_lpn, UINT32 const data_lpn) */
/* { */
/*     ASSERT(log_lpn < ((LOG_BLK_PER_BANK + ISOL_BLK_PER_BANK) * PAGES_PER_BLK)); */
/*     write_dram_32(LOG_PMT_ADDR + (((bank * (LOG_BLK_PER_BANK + ISOL_BLK_PER_BANK) * PAGES_PER_BLK) + log_lpn) * sizeof(UINT32)), data_lpn); */
/* } */
static void set_data_lpn_in_log_blk(UINT32 const bank, UINT32 const log_lpn, UINT32 const data_lpn)
{
    ASSERT(log_lpn < ((LOG_BLK_PER_BANK + ISOL_BLK_PER_BANK) * PAGES_PER_BLK));

    if (log_lpn >= PAGES_PER_BLK) {
        g_misc_meta[bank].lpn_list_of_rwlog_blk[log_lpn % PAGES_PER_BLK] = data_lpn;
    }
    ASSERT(shashtbl_get(bank, data_lpn) == INVALID);

    shashtbl_insert(bank, data_lpn, log_lpn);
}
static void adj_data_lpn_in_log_blk(UINT32 const bank, UINT32 const new_log_lpn, UINT32 const data_lpn)
{
    ASSERT(new_log_lpn < ((LOG_BLK_PER_BANK + ISOL_BLK_PER_BANK) * PAGES_PER_BLK));
    ASSERT(shashtbl_get(bank, data_lpn) != INVALID);

    g_misc_meta[bank].lpn_list_of_rwlog_blk[new_log_lpn % PAGES_PER_BLK] = data_lpn;
    shashtbl_update(bank, data_lpn, new_log_lpn);
}
static void read_lpn_list_of_vblock(UINT32 const bank, UINT32 const vblock)
{
    ASSERT(vblock < VBLKS_PER_BANK);

    // read lpn list of target vblock
    nand_page_ptread(bank,
                     vblock, PAGES_PER_BLK - 1,
                     0, (sizeof(UINT32) * PAGES_PER_BLK + BYTES_PER_SECTOR - 1) / BYTES_PER_SECTOR,
                     FTL_BUF(bank),
                     RETURN_WHEN_DONE);

    mem_copy(g_misc_meta[bank].lpn_list_of_rwlog_blk, FTL_BUF(bank), sizeof(UINT32) * PAGES_PER_BLK);
}
/* static void mark_invalid_in_log_pmap(UINT32 const bank, UINT32 const log_lpn) */
/* { */
/*     ASSERT(log_lpn < ((LOG_BLK_PER_BANK + ISOL_BLK_PER_BANK) * PAGES_PER_BLK)); */
/*     ASSERT(read_dram_32(LOG_PMT_ADDR + (((bank * (LOG_BLK_PER_BANK + ISOL_BLK_PER_BANK) * PAGES_PER_BLK) + log_lpn) * sizeof(UINT32))) != INVALID); */

/*     write_dram_32(LOG_PMT_ADDR + (((bank * (LOG_BLK_PER_BANK + ISOL_BLK_PER_BANK) * PAGES_PER_BLK) + log_lpn) * sizeof(UINT32)), INVALID); */
/* } */
static BOOL32 get_scbit(UINT32 const bank, UINT32 const log_lpn)
{
    ASSERT(log_lpn < (LOG_BLK_PER_BANK * PAGES_PER_BLK));

    UINT32 scbit = tst_bit_dram(SC_BITMAP_ADDR + (bank * ((LOG_BLK_PER_BANK * PAGES_PER_BLK / 8))), log_lpn);

    return (scbit == 0) ? FALSE:TRUE;
}
static void set_scbit(UINT32 const bank, UINT32 const log_lpn)
{
    ASSERT(log_lpn < (LOG_BLK_PER_BANK * PAGES_PER_BLK));

    set_bit_dram(SC_BITMAP_ADDR + (bank * ((LOG_BLK_PER_BANK * PAGES_PER_BLK / 8))), log_lpn);
}
static void clr_scbit(UINT32 const bank, UINT32 const log_lpn)
{
    ASSERT(log_lpn < (LOG_BLK_PER_BANK * PAGES_PER_BLK));
    ASSERT(SC_BITMAP_ADDR + (bank * ((LOG_BLK_PER_BANK * PAGES_PER_BLK / 8))) < (SC_BITMAP_ADDR + SC_BITMAP_BYTES));

    clr_bit_dram(SC_BITMAP_ADDR + (bank * ((LOG_BLK_PER_BANK * PAGES_PER_BLK / 8))), log_lpn);
}
static void set_all_scbit_in_log_blk(UINT32 const bank, UINT32 const log_lbn)
{
    ASSERT(log_lbn < LOG_BLK_PER_BANK);
    ASSERT(SC_BITMAP_ADDR + ((bank * LOG_BLK_PER_BANK + log_lbn) * PAGES_PER_BLK / 8) < (SC_BITMAP_ADDR + SC_BITMAP_BYTES));

    mem_copy(SC_BITMAP_ADDR + ((bank * LOG_BLK_PER_BANK + log_lbn) * PAGES_PER_BLK / 8),
             g_mem_to_set,
             PAGES_PER_BLK / 8);
}
static void clr_all_scbit_in_log_blk(UINT32 const bank, UINT32 const log_lbn)
{
    ASSERT(log_lbn < LOG_BLK_PER_BANK);
    ASSERT(SC_BITMAP_ADDR + ((bank * LOG_BLK_PER_BANK + log_lbn) * PAGES_PER_BLK / 8) < (SC_BITMAP_ADDR + SC_BITMAP_BYTES));

    mem_copy(SC_BITMAP_ADDR + ((bank * LOG_BLK_PER_BANK + log_lbn) * PAGES_PER_BLK / 8),
             g_mem_to_clr,
             PAGES_PER_BLK / 8);
}
static void update_block_wearout_info(UINT32 const bank, UINT32 const vblock)
{
    g_ftl_statistics[bank].total_erase_cnt++;
    // increase block wear-out count
    inc_block_erase_cnt(bank, vblock);
}
static void inc_block_erase_cnt(UINT32 const bank, UINT32 const vblock)
{
    ASSERT(vblock < VBLKS_PER_BANK);

    UINT32 erase_cnt = read_dram_32(BLK_ERASE_CNT_ADDR + ((bank * VBLKS_PER_BANK + vblock) * sizeof(UINT32)));
    write_dram_32(BLK_ERASE_CNT_ADDR + ((bank * VBLKS_PER_BANK + vblock) * sizeof(UINT32)), erase_cnt + 1);
}
static void explicit_wear_leveling(void)
{
    // TODO
    // switch log block and victim data block for explicit wear-leveling
    // get cost for all data blocks
}
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
            uart_printf("BSP interrupt at bank: 0x%x", bank);
            uart_printf("FIRQ_DATA_CORRUPT occured...vblock %d, page %d", GETREG(BSP_ROW_H(bank)) / PAGES_PER_BLK, GETREG(BSP_ROW_H(bank)) % PAGES_PER_BLK);
		}
		if (bsp_intr_flag & (FIRQ_BADBLK_H | FIRQ_BADBLK_L)) {
            uart_printf("BSP interrupt at bank: 0x%x", bank);
			if (fc == FC_COL_ROW_IN_PROG || fc == FC_IN_PROG || fc == FC_PROG) {
                uart_print("find runtime bad block when block program...");
			}
			else {
                uart_printf("find runtime bad block when block erase...vblock #: %d", GETREG(BSP_ROW_H(bank)) / PAGES_PER_BLK);
                g_bsp_isr_flag[bank] = GETREG(BSP_ROW_H(bank)) / PAGES_PER_BLK;

				ASSERT(fc == FC_ERASE);
			}
		}
    }
}
