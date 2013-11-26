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
// Synthetic test cases for verifying FTL logic
//

#include "jasmine.h"

#if OPTION_FTL_TEST == TRUE

#include <stdlib.h>

#define IO_LIMIT           (NUM_LSECTORS)
#define RANDOM_SEED        (IO_LIMIT)
#define NUM_PSECTORS_4KB   ((4 * 1024) / 512)
#define NUM_PSECTORS_8KB   (NUM_PSECTORS_4KB << 1)
#define NUM_PSECTORS_16KB  (NUM_PSECTORS_8KB << 1)
#define NUM_PSECTORS_32KB  (NUM_PSECTORS_16KB << 1)
#define NUM_PSECTORS_64KB  (NUM_PSECTORS_32KB << 1)
#define NUM_PSECTORS_128KB ((128 * 1024) / 512)
#define NUM_PSECTORS_256KB ((256 * 1024) / 512)

extern UINT32 g_ftl_read_buf_id;
extern UINT32 g_ftl_write_buf_id;

static void tc_write_seq(const UINT32 start_lsn, const UINT32 io_num, const UINT32 sector_size);
static void tc_write_rand(const UINT32 start_lsn, const UINT32 io_num, const UINT32 sector_size);
static void fillup_dataspace(void);
static void aging_with_rw(UINT32 io_cnt);

/* void ftl_test(void) */
/* { */
/*     UINT32 i, j, wr_buf_addr, rd_buf_addr, data; */
/*     UINT32 lba, num_sectors = 128; */
/*     /\* UINT32 io_cnt = 1000; *\/ */
/*     /\* UINT32 const start_lba = 328064; *\/ */
/*     UINT32 io_cnt = 100000; */
/*     UINT32 r_data; */
/*     UINT32 const start_lba = 0; */

/* 	/\* g_barrier = 0; *\/ */
/* 	/\* while (g_barrier == 0); *\/ */
/*     /\* led(0); *\/ */

/*     // STEP 1 - write */
/*     for (UINT32 loop = 0; loop < 1; loop++) { */
/*         wr_buf_addr = WR_BUF_ADDR; */
/*         data = 0; */
/*         r_data = 0; */

/*         lba  = start_lba; */

/*         rd_buf_addr = RD_BUF_ADDR; */

/*         uart_print_32(loop); uart_print(""); */

/*         for (i = 0; i < io_cnt; i++) { */
/*             wr_buf_addr = WR_BUF_PTR(g_ftl_write_buf_id) + ((lba % SECTORS_PER_PAGE) * BYTES_PER_SECTOR); */
/*             r_data = data; */

/*             for (j = 0; j < num_sectors; j++) { */
/*                 mem_set_dram(wr_buf_addr, data, BYTES_PER_SECTOR); */

/*                 wr_buf_addr += BYTES_PER_SECTOR; */

/*                 if (wr_buf_addr >= WR_BUF_ADDR + WR_BUF_BYTES) { */
/*                     wr_buf_addr = WR_BUF_ADDR; */
/*                 } */
/*                 data++; */
/*             } */
/*             ftl_write(lba, num_sectors); */

/*             rd_buf_addr = RD_BUF_PTR(g_ftl_read_buf_id) + ((lba % SECTORS_PER_PAGE) * BYTES_PER_SECTOR); */
/*             ftl_read(lba, num_sectors); */

/*             flash_finish(); */

/*             for (j = 0; j < num_sectors; j++) { */
/*                 UINT32 sample = read_dram_32(rd_buf_addr); */

/*                 if (sample != r_data) { */
/*                     uart_printf("ftl test fail...io#: %d, %d", lba, num_sectors); */
/*                     uart_printf("sample data %d should be %d", sample, r_data); */
/*                     led_blink(); */
/*                 } */
/*                 rd_buf_addr += BYTES_PER_SECTOR; */

/*                 if (rd_buf_addr >= RD_BUF_ADDR + RD_BUF_BYTES) { */
/*                     rd_buf_addr = RD_BUF_ADDR; */
/*                 } */
/*                 r_data++; */
/*             } */
/*             lba += (num_sectors * 9); */

/*             if (lba >= (UINT32)NUM_LSECTORS) { */
/*                 lba = 0; */
/*             } */
/*         } */
/*     } */
/*     ftl_flush(); */
/* } */
void ftl_test(void)
{
    uart_print("start ftl test...");
/*     fillup_dataspace(); */
/*     tc_write_seq(0, 5000, NUM_PSECTORS_64KB); */
    tc_write_rand(0, 200000, NUM_PSECTORS_4KB);
/*     tc_write_rand(0, 2000000, NUM_PSECTORS_4KB); */
    uart_print("ftl test passed!");
}
static void aging_with_rw(UINT32 io_cnt)
{
    UINT32 lba;
    UINT32 const num_sectors = SECTORS_PER_PAGE * NUM_BANKS;
    srand(RANDOM_SEED);

    uart_printf("start aging with random writes");
    while (io_cnt > 0) {
        do {
            lba = rand() % IO_LIMIT;
        }while (lba >= (NUM_LSECTORS - num_sectors));
        // page alignment
        lba = lba / SECTORS_PER_PAGE * SECTORS_PER_PAGE;
        ftl_write(lba, num_sectors);

        io_cnt--;
    }
    uart_printf("complete!");
}
// fill entire dataspace
static void fillup_dataspace(void)
{
    UINT32 lba = 0;
    UINT32 const num_sectors = SECTORS_PER_PAGE * NUM_BANKS;

    uart_printf("start fill entire data space");
    while (lba < (NUM_LSECTORS - num_sectors))
    {
        ftl_write(lba, num_sectors);
        lba += num_sectors;
    }
    uart_printf("complete!");
}
static void tc_write_seq(const UINT32 start_lsn, const UINT32 io_num, const UINT32 sector_size)
{
    UINT32 i, j, wr_buf_addr, rd_buf_addr, data;
    UINT32 lba, num_sectors = sector_size;
    UINT32 io_cnt = io_num;
    UINT32 const start_lba = start_lsn;

    /* UINT32 volatile g_barrier = 0; while (g_barrier == 0); */
    led(0);

    // STEP 1 - write
    for (UINT32 loop = 0; loop < 5; loop++)
    {
        wr_buf_addr = WR_BUF_ADDR;
        data = 0;
        lba  = start_lba;

        uart_print_32(loop); uart_print("");

        for (i = 0; i < io_cnt; i++)
        {
            wr_buf_addr = WR_BUF_PTR(g_ftl_write_buf_id) + ((lba % SECTORS_PER_PAGE) * BYTES_PER_SECTOR);
            for (j = 0; j < num_sectors; j++)
            {
                mem_set_dram(wr_buf_addr, data, BYTES_PER_SECTOR);

                wr_buf_addr += BYTES_PER_SECTOR;

                if (wr_buf_addr >= WR_BUF_ADDR + WR_BUF_BYTES)
                {
                    wr_buf_addr = WR_BUF_ADDR;
                }
                data++;
            }
            ptimer_start();
            ftl_write(lba, num_sectors);
            ptimer_stop_and_uart_print();

            lba += num_sectors;

            if (lba >= (UINT32)NUM_LSECTORS)
            {
                uart_print("adjust lba because of out of lba");
                lba = 0;
            }
        }

        // STEP 2 - read and verify
        rd_buf_addr = RD_BUF_ADDR;
        data = 0;
        lba  = start_lba;
        num_sectors = MIN(num_sectors, NUM_RD_BUFFERS * SECTORS_PER_PAGE);

        for (i = 0; i < io_cnt; i++)
        {
            rd_buf_addr = RD_BUF_PTR(g_ftl_read_buf_id) + ((lba % SECTORS_PER_PAGE) * BYTES_PER_SECTOR);
            /* ptimer_start(); */
            ftl_read(lba, num_sectors);

            flash_finish();
            /* ptimer_stop_and_uart_print(); */

            for (j = 0; j < num_sectors; j++)
            {
                UINT32 sample = read_dram_32(rd_buf_addr);

                if (sample != data)
                {
                    uart_printf("ftl test fail...io#: %d, %d", lba, num_sectors);
                    uart_printf("sample data %d should be %d", sample, data);
                    led_blink();
                }

                rd_buf_addr += BYTES_PER_SECTOR;

                if (rd_buf_addr >= RD_BUF_ADDR + RD_BUF_BYTES)
                {
                    rd_buf_addr = RD_BUF_ADDR;
                }
                data++;
            }

            lba += num_sectors;

            if (lba >= IO_LIMIT + num_sectors)
            {
                lba = 0;
            }
        }
    }
    ftl_flush();
}
static void tc_write_rand(const UINT32 start_lsn, const UINT32 io_num, const UINT32 sector_size)
{
    UINT32 i, j, wr_buf_addr, rd_buf_addr, data, r_data;
    UINT32 lba, num_sectors = sector_size;
    UINT32 io_cnt = io_num;

    /* UINT32 volatile g_barrier = 0; while (g_barrier == 0); */
    led(0);
    srand(RANDOM_SEED);

    for (UINT32 loop = 0; loop < 1; loop++) {
        wr_buf_addr = WR_BUF_ADDR;
        data = 0;
        uart_printf("test loop cnt: %d", loop);

        for (i = 0; i < io_cnt; i++) {
            do {
                lba = rand() % IO_LIMIT;
            }while(lba + num_sectors >= IO_LIMIT);

            wr_buf_addr = WR_BUF_PTR(g_ftl_write_buf_id) + ((lba % SECTORS_PER_PAGE) * BYTES_PER_SECTOR);
            r_data = data;

            for (j = 0; j < num_sectors; j++) {
                mem_set_dram(wr_buf_addr, data, BYTES_PER_SECTOR);

                wr_buf_addr += BYTES_PER_SECTOR;

                if (wr_buf_addr >= WR_BUF_ADDR + WR_BUF_BYTES) {
                    wr_buf_addr = WR_BUF_ADDR;
                }
                data++;
            }
/*             ptimer_start(); */
            ftl_write(lba, num_sectors);
/*             ptimer_stop_and_uart_print(); */
            rd_buf_addr = RD_BUF_PTR(g_ftl_read_buf_id) + ((lba % SECTORS_PER_PAGE) * BYTES_PER_SECTOR);
/*             ptimer_start(); */
            ftl_read(lba, num_sectors);
/*             ptimer_stop_and_uart_print(); */

            flash_finish();

            for (j = 0; j < num_sectors; j++) {
                UINT32 sample = read_dram_32(rd_buf_addr);

                if (sample != r_data) {
                    uart_printf("ftl test fail...io#: %d, %d", lba, num_sectors);
                    uart_printf("sample data %d should be %d", sample, r_data);
                    led_blink();
                }
                rd_buf_addr += BYTES_PER_SECTOR;

                if (rd_buf_addr >= RD_BUF_ADDR + RD_BUF_BYTES) {
                    rd_buf_addr = RD_BUF_ADDR;
                }
                r_data++;
            }
        } // end for
    }
    ftl_flush();
}
#endif // OPTION_FTL_TEST
