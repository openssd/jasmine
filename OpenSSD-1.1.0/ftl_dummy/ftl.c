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

UINT32 g_ftl_read_buf_id;
UINT32 g_ftl_write_buf_id;

// The Dummy FTL does not access NAND flash at all.
// You use Dummy FTL to measure SATA and DRAM speed.
// Since data are not written at all, benchmarks that do read-and-verify cannot be run over this FTL.

void ftl_open(void)
{
}

void ftl_read(UINT32 const lba, UINT32 const total_sectors)
{
	UINT32 num_sectors_to_read;

	UINT32 lpage_addr		= lba / SECTORS_PER_PAGE;	// logical page address
	UINT32 sect_offset 		= lba % SECTORS_PER_PAGE;	// sector offset within the page
	UINT32 sectors_remain	= total_sectors;

	while (sectors_remain != 0)	// one page per iteration
	{
		if (sect_offset + sectors_remain < SECTORS_PER_PAGE)
		{
			num_sectors_to_read = sectors_remain;
		}
		else
		{
			num_sectors_to_read = SECTORS_PER_PAGE - sect_offset;
		}

		UINT32 next_read_buf_id = (g_ftl_read_buf_id + 1) % NUM_RD_BUFFERS;

		while (next_read_buf_id == GETREG(SATA_RBUF_PTR));	// wait if the read buffer is full (slow host)

		SETREG(BM_STACK_RDSET, next_read_buf_id);	// change bm_read_limit
		SETREG(BM_STACK_RESET, 0x02);				// change bm_read_limit

		g_ftl_read_buf_id = next_read_buf_id;

		sect_offset = 0;
		sectors_remain -= num_sectors_to_read;
		lpage_addr++;
	}
}

void ftl_write(UINT32 const lba, UINT32 const total_sectors)
{
	UINT32 num_sectors_to_write;

	UINT32 sect_offset = lba % SECTORS_PER_PAGE;
	UINT32 remain_sectors = total_sectors;

	while (remain_sectors != 0)
	{
		if (sect_offset + remain_sectors >= SECTORS_PER_PAGE)
		{
			num_sectors_to_write = SECTORS_PER_PAGE - sect_offset;
		}
		else
		{
			num_sectors_to_write = remain_sectors;
		}

		while (g_ftl_write_buf_id == GETREG(SATA_WBUF_PTR));	// bm_write_limit should not outpace SATA_WBUF_PTR

		g_ftl_write_buf_id = (g_ftl_write_buf_id + 1) % NUM_WR_BUFFERS;		// Circular buffer

		SETREG(BM_STACK_WRSET, g_ftl_write_buf_id);	// change bm_write_limit
		SETREG(BM_STACK_RESET, 0x01);				// change bm_write_limit

		sect_offset = 0;
		remain_sectors -= num_sectors_to_write;
	}
}

void ftl_flush(void)
{
}

void ftl_isr(void)
{
}

