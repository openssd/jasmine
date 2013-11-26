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

// This file provides wrapper functions for flash.c.
// If you feel it is difficult to use the flash_xxx() functions,
// you can use nand_xxx() functions which are easier to use.

extern UINT32 g_ftl_read_buf_id;
extern UINT32 g_ftl_write_buf_id;
extern UINT32 g_bsp_isr_flag;

// synchronous one full page read
void nand_page_read(UINT32 const bank, UINT32 const vblock, UINT32 const page_num, UINT32 const buf_addr)
{
    UINT32 row;

    ASSERT(bank < NUM_BANKS);
    ASSERT(vblock < VBLKS_PER_BANK);
    ASSERT(page_num < PAGES_PER_BLK);

    // row means ppn
    row = (vblock * PAGES_PER_BLK) + page_num;

    SETREG(FCP_CMD, FC_COL_ROW_READ_OUT);
    SETREG(FCP_OPTION, FO_P | FO_E);
    SETREG(FCP_DMA_ADDR, buf_addr);
    SETREG(FCP_DMA_CNT, BYTES_PER_PAGE);

    SETREG(FCP_COL, 0);
    SETREG(FCP_ROW_L(bank), row);
    SETREG(FCP_ROW_H(bank), row);

    flash_issue_cmd(bank, RETURN_WHEN_DONE);
}
// General purpose page read function
// synchronous page read (for reading metadata)
// asynchronous page read (left/right hole async read user data)
void nand_page_ptread(UINT32 const bank, UINT32 const vblock, UINT32 const page_num, UINT32 const sect_offset, UINT32 const num_sectors, UINT32 const buf_addr, UINT32 const issue_flag)
{
    UINT32 row;

/*     uart_printf("--ptread: bank %d vblock %d page_num %d sect_offset %d, num_sectors %d", bank, vblock, page_num, sect_offset, num_sectors); */
    ASSERT(bank < NUM_BANKS);
    ASSERT(vblock < VBLKS_PER_BANK);
    ASSERT(page_num < PAGES_PER_BLK);

    // row means ppn
    row = (vblock * PAGES_PER_BLK) + page_num;

    SETREG(FCP_CMD, FC_COL_ROW_READ_OUT);
    SETREG(FCP_OPTION, FO_P | FO_E);
    SETREG(FCP_DMA_ADDR, buf_addr);
    SETREG(FCP_DMA_CNT, num_sectors * BYTES_PER_SECTOR);

    SETREG(FCP_COL, sect_offset);
    SETREG(FCP_ROW_L(bank), row);
    SETREG(FCP_ROW_H(bank), row);

    // issue_flag:
    // RETURN_ON_ISSUE, RETURN_WHEN_DONE, RETURN_ON_ACCEPT
    flash_issue_cmd(bank, issue_flag);
}
void nand_page_read_to_host(UINT32 const bank, UINT32 const vblock, UINT32 const page_num)
{
    UINT32 row;

    ASSERT(bank < NUM_BANKS);
    ASSERT(vblock < VBLKS_PER_BANK);
    ASSERT(page_num < PAGES_PER_BLK);

    row = (vblock * PAGES_PER_BLK) + page_num;

    SETREG(FCP_CMD, FC_COL_ROW_READ_OUT);
    SETREG(FCP_DMA_ADDR, RD_BUF_PTR(g_ftl_read_buf_id));
    SETREG(FCP_DMA_CNT, BYTES_PER_PAGE);

    SETREG(FCP_COL, 0);
#if OPTION_FTL_TEST == TRUE
    SETREG(FCP_OPTION, FO_P | FO_E);
#else
    SETREG(FCP_OPTION, FO_P | FO_E | FO_B_SATA_R);
#endif
    SETREG(FCP_ROW_L(bank), row);
    SETREG(FCP_ROW_H(bank), row);

    g_ftl_read_buf_id = (g_ftl_read_buf_id + 1) % NUM_RD_BUFFERS;

    #if OPTION_FTL_TEST == FALSE
    {
        while (1)
        {
            UINT32 sata_id = GETREG(SATA_RBUF_PTR);

            if (g_ftl_read_buf_id != sata_id)
                break;
        }
    }
    #endif
    flash_issue_cmd(bank, RETURN_ON_ISSUE);
}
// partial page read to host
void nand_page_ptread_to_host(UINT32 const bank, UINT32 const vblock, UINT32 const page_num, UINT32 const sect_offset, UINT32 const num_sectors)
{
    UINT32 row;

    ASSERT(bank < NUM_BANKS);
    ASSERT(vblock < VBLKS_PER_BANK);
    ASSERT(page_num < PAGES_PER_BLK);

    row = (vblock * PAGES_PER_BLK) + page_num;

    SETREG(FCP_CMD, FC_COL_ROW_READ_OUT);
    SETREG(FCP_DMA_ADDR, RD_BUF_PTR(g_ftl_read_buf_id));
    SETREG(FCP_DMA_CNT, num_sectors * BYTES_PER_SECTOR);

    SETREG(FCP_COL, sect_offset);
#if OPTION_FTL_TEST == TRUE
    SETREG(FCP_OPTION, FO_P | FO_E);
#else
    SETREG(FCP_OPTION, FO_P | FO_E | FO_B_SATA_R);
#endif
    SETREG(FCP_ROW_L(bank), row);
    SETREG(FCP_ROW_H(bank), row);

    g_ftl_read_buf_id = (g_ftl_read_buf_id + 1) % NUM_RD_BUFFERS;

    #if OPTION_FTL_TEST == FALSE
    {
        while (1)
        {
            UINT32 sata_id = GETREG(SATA_RBUF_PTR);

            if (g_ftl_read_buf_id != sata_id)
                break;
        }
    }
    #endif
    flash_issue_cmd(bank, RETURN_ON_ISSUE);
}
void nand_page_program(UINT32 const bank, UINT32 const vblock, UINT32 const page_num, UINT32 const buf_addr)
{
    UINT32 row;

    ASSERT(bank < NUM_BANKS);
    ASSERT(vblock < VBLKS_PER_BANK);
    ASSERT(page_num < PAGES_PER_BLK);

    row = (vblock * PAGES_PER_BLK) + page_num;

    SETREG(FCP_CMD, FC_COL_ROW_IN_PROG);
    SETREG(FCP_OPTION, FO_P | FO_E | FO_B_W_DRDY);
    SETREG(FCP_DMA_ADDR, buf_addr);
    SETREG(FCP_DMA_CNT, BYTES_PER_PAGE);
    SETREG(FCP_COL, 0);
    SETREG(FCP_ROW_L(bank), row);
    SETREG(FCP_ROW_H(bank), row);

    flash_issue_cmd(bank, RETURN_ON_ISSUE);
}
void nand_page_ptprogram(UINT32 const bank, UINT32 const vblock, UINT32 const page_num, UINT32 const sect_offset, UINT32 const num_sectors, UINT32 const buf_addr)
{
    UINT32 row;

/*     uart_printf("ptprogram: bank %d vblock %d page_num %d sect_offset %d, num_sectors %d", bank, vblock, page_num, sect_offset, num_sectors); */

    ASSERT(bank < NUM_BANKS);
    ASSERT(vblock < VBLKS_PER_BANK);
    ASSERT(page_num < PAGES_PER_BLK);

    row = (vblock * PAGES_PER_BLK) + page_num;

    SETREG(FCP_CMD, FC_COL_ROW_IN_PROG);
    SETREG(FCP_OPTION, FO_P | FO_E | FO_B_W_DRDY);
    SETREG(FCP_DMA_ADDR, buf_addr);
    SETREG(FCP_DMA_CNT, num_sectors * BYTES_PER_SECTOR);
    SETREG(FCP_COL, sect_offset);
    SETREG(FCP_ROW_L(bank), row);
    SETREG(FCP_ROW_H(bank), row);

    flash_issue_cmd(bank, RETURN_ON_ISSUE);
}
void nand_page_program_from_host(UINT32 const bank, UINT32 const vblock, UINT32 const page_num)
{
    UINT32 row;

    ASSERT(bank < NUM_BANKS);
    ASSERT(vblock < VBLKS_PER_BANK);
    ASSERT(page_num < PAGES_PER_BLK);

    row = (vblock * PAGES_PER_BLK) + page_num;

    SETREG(FCP_CMD, FC_COL_ROW_IN_PROG);
#if OPTION_FTL_TEST == TRUE
    SETREG(FCP_OPTION, FO_P | FO_E | FO_B_W_DRDY);
#else
    SETREG(FCP_OPTION, FO_P | FO_E | FO_B_SATA_W);
#endif
    SETREG(FCP_DMA_ADDR, WR_BUF_PTR(g_ftl_write_buf_id));
    SETREG(FCP_DMA_CNT, BYTES_PER_PAGE);

    SETREG(FCP_COL, 0);
    SETREG(FCP_ROW_L(bank), row);
    SETREG(FCP_ROW_H(bank), row);

    flash_issue_cmd(bank, RETURN_ON_ISSUE);

    g_ftl_write_buf_id = (g_ftl_write_buf_id + 1) % NUM_WR_BUFFERS;
}
void nand_page_ptprogram_from_host(UINT32 const bank, UINT32 const vblock, UINT32 const page_num, UINT32 const sect_offset, UINT32 const num_sectors)
{
    UINT32 row;

    ASSERT(bank < NUM_BANKS);
    ASSERT(vblock < VBLKS_PER_BANK);
    ASSERT(page_num < PAGES_PER_BLK);

    row = (vblock * PAGES_PER_BLK) + page_num;

    SETREG(FCP_CMD, FC_COL_ROW_IN_PROG);
#if OPTION_FTL_TEST == TRUE
    SETREG(FCP_OPTION, FO_P | FO_E | FO_B_W_DRDY);
#else
    SETREG(FCP_OPTION, FO_P | FO_E | FO_B_SATA_W);
#endif
    SETREG(FCP_DMA_ADDR, WR_BUF_PTR(g_ftl_write_buf_id));
    SETREG(FCP_DMA_CNT, num_sectors * BYTES_PER_SECTOR);

    SETREG(FCP_COL, sect_offset);
    SETREG(FCP_ROW_L(bank), row);
    SETREG(FCP_ROW_H(bank), row);

    flash_issue_cmd(bank, RETURN_ON_ISSUE);

    g_ftl_write_buf_id = (g_ftl_write_buf_id + 1) % NUM_WR_BUFFERS;
}
void nand_page_copyback(UINT32 const bank, UINT32 const src_vblock, UINT32 const src_page,
                          UINT32 const dst_vblock, UINT32 const dst_page)
{
	BOOL32	do_copyback;

    UINT32 src_row, dst_row;

    ASSERT(bank < NUM_BANKS);
    ASSERT(src_vblock < VBLKS_PER_BANK);
    ASSERT(dst_vblock < VBLKS_PER_BANK);
    ASSERT(src_page < PAGES_PER_BLK);
    ASSERT(dst_page < PAGES_PER_BLK);

    src_row = (src_vblock * PAGES_PER_BLK) + src_page;
    dst_row = (dst_vblock * PAGES_PER_BLK) + dst_page;

	do_copyback = TRUE;

	#if NAND_SPEC_DIE == NAND_SPEC_DIE_2
	{
		#if NAND_SPEC_PLANE == NAND_SPEC_PLANE_3 && OPTION_2_PLANE == FALSE
		{
			if (src_row / (PAGES_PER_BANK / 4) != dst_row / (PAGES_PER_BANK / 4))
				do_copyback = FALSE;
		}
		#else
		{
			if (src_row / (PAGES_PER_BANK / 2) != dst_row / (PAGES_PER_BANK / 2))
				do_copyback = FALSE;
		}
		#endif
	}
	#else
	{
		#if NAND_SPEC_PLANE == NAND_SPEC_PLANE_3 && OPTION_2_PLANE == FALSE
		{
			if (src_row / (PAGES_PER_BANK / 2) != dst_row / (PAGES_PER_BANK / 2))
				do_copyback = FALSE;
		}
		#endif
	}
	#endif

	#if NAND_SPEC_PLANE == NAND_SPEC_PLANE_2 && OPTION_2_PLANE == FALSE
	{
		UINT32 dst_vblk_offset = dst_row / PAGES_PER_BLK;
		UINT32 src_vblk_offset = src_row / PAGES_PER_BLK;

		if (dst_vblk_offset % NUM_PLANES != src_vblk_offset % NUM_PLANES)
		{
			do_copyback = FALSE;
		}
	}
	#endif

	#if FLASH_TYPE == K9WAG08 || FLASH_TYPE == K9K8G08
    {
        if (src_row % 2 != dst_row % 2)
            do_copyback = FALSE;
    }
	#elif FLASH_TYPE == TH58NVG5S0DTG20 && OPTION_2_PLANE == FALSE
	{
		if (src_row / (PAGES_PER_BANK / 4) != dst_row / (PAGES_PER_BANK / 4))
			do_copyback = FALSE;
	}
	#endif

	if (do_copyback)
	{
		SETREG(FCP_CMD, FC_COPYBACK);
		SETREG(FCP_OPTION, FO_P | FO_E | FO_B_W_DRDY);
		SETREG(FCP_COL, 0);
		SETREG(FCP_ROW_L(bank), src_row);
		SETREG(FCP_ROW_H(bank), src_row);
		SETREG(FCP_DST_ROW_H, dst_row);
		SETREG(FCP_DST_ROW_L, dst_row);
		SETREG(FCP_DST_COL, 0);

        flash_issue_cmd(bank, RETURN_ON_ISSUE);
	}
    // no internal copyback
	else
	{
		SETREG(FCP_CMD, FC_COL_ROW_READ_OUT);
		SETREG(FCP_OPTION, FO_P | FO_E);
		SETREG(FCP_DMA_ADDR, COPY_BUF(bank));
		SETREG(FCP_DMA_CNT, BYTES_PER_PAGE);
		SETREG(FCP_ROW_L(bank), src_row);
		SETREG(FCP_ROW_H(bank), src_row);
		SETREG(FCP_COL, 0);
		SETREG(FCP_BANK, REAL_BANK(bank));
		FLASH_POLL_WR;
		FLASH_ISSUE;

		SETREG(FCP_CMD, FC_COL_ROW_IN_PROG);
		SETREG(FCP_OPTION, FO_P | FO_E | FO_B_W_DRDY);
		SETREG(FCP_ROW_L(bank), dst_row);
		SETREG(FCP_ROW_H(bank), dst_row);
		SETREG(FCP_BANK, REAL_BANK(bank));
		FLASH_POLL_WR;
		FLASH_ISSUE;
	}

}
void nand_page_modified_copyback(UINT32 const bank, UINT32 const src_vblock, UINT32 const src_page,
                                 UINT32 const dst_vblock, UINT32 const dst_page,
                                 UINT32 const sect_offset,
                                 UINT32 dma_addr, UINT32 const dma_count)
{
    // TODO
	BOOL32	do_copyback;
	UINT32	src_row, dst_row;

    ASSERT(bank < NUM_BANKS);
    ASSERT(src_vblock < VBLKS_PER_BANK);
    ASSERT(dst_vblock < VBLKS_PER_BANK);
    ASSERT(src_page < PAGES_PER_BLK);
    ASSERT(dst_page < PAGES_PER_BLK);

    src_row = (src_vblock * PAGES_PER_BLK) + src_page;
    dst_row = (dst_vblock * PAGES_PER_BLK) + dst_page;

    do_copyback = TRUE;

	#if NAND_SPEC_DIE == NAND_SPEC_DIE_2
	{
		#if NAND_SPEC_PLANE == NAND_SPEC_PLANE_3 && OPTION_2_PLANE == FALSE
		{
			if (src_row / (PAGES_PER_BANK / 4) != dst_row / (PAGES_PER_BANK / 4))
				do_copyback = FALSE;
		}
		#else
		{
			if (src_row / (PAGES_PER_BANK / 2) != dst_row / (PAGES_PER_BANK / 2))
				do_copyback = FALSE;
		}
		#endif
	}
	#else
	{
		#if NAND_SPEC_PLANE == NAND_SPEC_PLANE_3 && OPTION_2_PLANE == FALSE
		{
			if (src_row / (PAGES_PER_BANK / 2) != dst_row / (PAGES_PER_BANK / 2))
				do_copyback = FALSE;
		}
		#endif
	}
	#endif

	#if NAND_SPEC_PLANE == NAND_SPEC_PLANE_2 && OPTION_2_PLANE == FALSE
	{
		UINT32	dst_vblk_offset = dst_row / PAGES_PER_VBLK;
		UINT32	src_vblk_offset = src_row / PAGES_PER_VBLK;

		if (dst_vblk_offset % NUM_PLANES != src_vblk_offset % NUM_PLANES)
		{
			do_copyback = FALSE;
		}
	}
	#endif

	#if FLASH_TYPE == K9WAG08 || FLASH_TYPE == K9K8G08
	if (src_row % 2 != dst_row % 2)
		do_copyback = FALSE;
	#elif FLASH_TYPE == TH58NVG5S0DTG20 && OPTION_2_PLANE == FALSE
	{
		if (src_row / (PAGES_PER_BANK / 4) != dst_row / (PAGES_PER_BANK / 4))
			do_copyback = FALSE;
	}
	#endif

	#if NAND_SPEC_MODIFY_COPY == FALSE
	do_copyback = FALSE;
	#endif

	if (do_copyback)
	{
		dma_addr -= sect_offset * BYTES_PER_SECTOR;

		SETREG(FCP_CMD, FC_MODIFY_COPYBACK);
		SETREG(FCP_OPTION, FO_P | FO_E | FO_B_W_DRDY);
		SETREG(FCP_DMA_ADDR, dma_addr);
		SETREG(FCP_DMA_CNT, dma_count);
		SETREG(FCP_COL, 0);
		SETREG(FCP_ROW_L(bank), src_row);
		SETREG(FCP_ROW_H(bank), src_row);
		SETREG(FCP_DST_ROW_H, dst_row);
		SETREG(FCP_DST_ROW_L, dst_row);
		SETREG(FCP_DST_COL, sect_offset);
		flash_issue_cmd(bank, RETURN_ON_ISSUE);
	}
	else
	{
		UINT32 buf_addr = COPY_BUF(bank);

		SETREG(FCP_CMD, FC_COL_ROW_READ_OUT);	// read old data
		SETREG(FCP_OPTION, FO_P | FO_E);
		SETREG(FCP_DMA_ADDR, buf_addr);
		SETREG(FCP_DMA_CNT, BYTES_PER_PAGE);
		SETREG(FCP_ROW_L(bank), src_row);
		SETREG(FCP_ROW_H(bank), src_row);
		SETREG(FCP_COL, 0);
		flash_issue_cmd(bank, RETURN_ON_ISSUE);

		SETREG(FCP_CMD, FC_COL_ROW_IN);			// write old data
		SETREG(FCP_OPTION, FO_P | FO_E | FO_B_W_DRDY);
		SETREG(FCP_DMA_CNT, sect_offset * BYTES_PER_SECTOR);
		SETREG(FCP_ROW_L(bank), dst_row);
		SETREG(FCP_ROW_H(bank), dst_row);
		SETREG(FCP_COL, 0);
		flash_issue_cmd(bank, RETURN_ON_ISSUE);

		SETREG(FCP_CMD, FC_IN); 				// write new data (modify)
		SETREG(FCP_DMA_ADDR, dma_addr - sect_offset * BYTES_PER_SECTOR);
		SETREG(FCP_DMA_CNT, dma_count);
		SETREG(FCP_COL, sect_offset);
		flash_issue_cmd(bank, RETURN_ON_ISSUE);

		SETREG(FCP_CMD, FC_IN_PROG);			// write old data
		SETREG(FCP_DMA_ADDR, buf_addr);
		SETREG(FCP_DMA_CNT, BYTES_PER_PAGE - sect_offset * BYTES_PER_SECTOR - dma_count);
		SETREG(FCP_COL, sect_offset + dma_count / BYTES_PER_SECTOR);
		flash_issue_cmd(bank, RETURN_ON_ISSUE);
	}
}
void nand_block_erase(UINT32 const bank, UINT32 const vblock)
{
    ASSERT(bank < NUM_BANKS);
    ASSERT(vblock < VBLKS_PER_BANK);

	SETREG(FCP_CMD, FC_ERASE);
	SETREG(FCP_BANK, REAL_BANK(bank));
	SETREG(FCP_OPTION, FO_P); // if OPTION_2_PLANE == 0, FO_P will be zero.
	SETREG(FCP_ROW_H(bank), vblock * PAGES_PER_VBLK);
	SETREG(FCP_ROW_L(bank), vblock * PAGES_PER_VBLK);

    flash_issue_cmd(bank, RETURN_ON_ISSUE);

/*     uart_printf("erase block #: %d, %d", bank, vblock); */
}
void nand_block_erase_sync(UINT32 const bank, UINT32 const vblock)
{
    ASSERT(bank < NUM_BANKS);
    ASSERT(vblock < VBLKS_PER_BANK);

	SETREG(FCP_CMD, FC_ERASE);
	SETREG(FCP_BANK, REAL_BANK(bank));
	SETREG(FCP_OPTION, FO_P); // if OPTION_2_PLANE == 0, FO_P will be zero.
	SETREG(FCP_ROW_H(bank), vblock * PAGES_PER_VBLK);
	SETREG(FCP_ROW_L(bank), vblock * PAGES_PER_VBLK);

    flash_issue_cmd(bank, RETURN_WHEN_DONE);
}
