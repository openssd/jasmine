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

UINT8 g_temp_mem[BYTES_PER_SECTOR];	// scratch pad

#define DEBUG_MEM_UTIL	0

#if DEBUG_MEM_UTIL
volatile UINT32 busy;
#endif

void _mem_copy(void* const dst, const void* const src, UINT32 const num_bytes)
{
	UINT32 d = (UINT32) dst;
	UINT32 s = (UINT32) src;
	BOOL32 was_disabled;

	ASSERT(d % sizeof(UINT32) == 0);
	ASSERT(s % sizeof(UINT32) == 0);
	ASSERT(num_bytes % sizeof(UINT32) == 0);
	ASSERT(num_bytes <= MU_MAX_BYTES);

    // if the memory data copy DRAM-to-DRAM,
    // then `num_bytes' should be aligned DRAM_ECC_UNIT size
    if (d > DRAM_BASE && s > DRAM_BASE && ((num_bytes % DRAM_ECC_UNIT) != 0))
    {
        ASSERT(0);
    }
	was_disabled = disable_irq();

	#if DEBUG_MEM_UTIL
	ASSERT(busy == 0);
	busy = 1;
	#endif

	SETREG(MU_SRC_ADDR, s);
	SETREG(MU_DST_ADDR, d);
	SETREG(MU_SIZE, num_bytes);
	SETREG(MU_CMD, MU_CMD_COPY);

	while (GETREG(MU_RESULT) == 0xFFFFFFFF);

	#if DEBUG_MEM_UTIL
	busy = 0;
	#endif

	if (!was_disabled)
		enable_irq();
}


UINT32 _mem_bmp_find_sram(const void* const bitmap, UINT32 const num_bytes, UINT32 const val)
{
	UINT32 retval;
	BOOL32 was_disabled;

	ASSERT(num_bytes <= MU_MAX_BYTES && num_bytes != 0);
	ASSERT(num_bytes % sizeof(UINT32) == 0);
	ASSERT(val == 0 || val == 1);
	ASSERT((UINT32) bitmap < DRAM_BASE);

	// can be called in IRQ handling
	was_disabled = disable_irq();

	#if DEBUG_MEM_UTIL
	ASSERT(busy == 0);
	busy = 1;
	#endif

	SETREG(MU_SRC_ADDR, bitmap);
	SETREG(MU_VALUE, val);
	SETREG(MU_SIZE, num_bytes);
	SETREG(MU_CMD, MU_CMD_FIND_SRAM);

	while (1)
	{
		retval = GETREG(MU_RESULT);

		if (retval != 0xFFFFFFFF)
			break;
	}

	#if DEBUG_MEM_UTIL
	busy = 0;
	#endif

	if (!was_disabled)
		enable_irq();

	return retval;
}

UINT32 _mem_bmp_find_dram(const void* const bitmap, UINT32 const num_bytes, UINT32 const val)
{
	UINT32 retval;
	BOOL32 was_disabled;

	ASSERT(num_bytes <= MU_MAX_BYTES && num_bytes != 0);
	ASSERT(num_bytes % sizeof(UINT32) == 0);
	ASSERT(val == 0 || val == 1);
	ASSERT((UINT32) bitmap >= DRAM_BASE);

	was_disabled = disable_irq();

	#if DEBUG_MEM_UTIL
	ASSERT(busy == 0);
	busy = 1;
	#endif

	SETREG(MU_SRC_ADDR, bitmap);
	SETREG(MU_VALUE, val);
	SETREG(MU_SIZE, num_bytes);
	SETREG(MU_CMD, MU_CMD_FIND_DRAM);

	while (1)
	{
		retval = GETREG(MU_RESULT);

		if (retval != 0xFFFFFFFF)
			break;
	}

	#if DEBUG_MEM_UTIL
	busy = 0;
	#endif

	if (!was_disabled)
		enable_irq();

	return retval;
}

void _mem_set_sram(UINT32 addr, UINT32 const val, UINT32 num_bytes)
{
	UINT32 size;

	ASSERT((UINT32)addr % sizeof(UINT32) == 0);
	ASSERT(num_bytes % sizeof(UINT32) == 0);
	ASSERT((UINT32) addr + num_bytes <= SRAM_SIZE);

	#if DEBUG_MEM_UTIL
	ASSERT(busy == 0);
	busy = 1;
	#endif

	SETREG(MU_VALUE, val);

	do
	{
		size = MIN(num_bytes, MU_MAX_BYTES);

		SETREG(MU_DST_ADDR, addr);
		SETREG(MU_SIZE, size);
		SETREG(MU_CMD, MU_CMD_SET_REPT_SRAM);

		addr += size;
		num_bytes -= size;

		while (GETREG(MU_RESULT) == 0xFFFFFFFF);
	}
	while (num_bytes != 0);

	#if DEBUG_MEM_UTIL
	busy = 0;
	#endif
}


void _mem_set_dram(UINT32 addr, UINT32 const val, UINT32 num_bytes)
{
	UINT32 size;

	ASSERT((UINT32)addr % SDRAM_ECC_UNIT == 0);
	ASSERT(num_bytes % SDRAM_ECC_UNIT == 0);
	ASSERT((UINT32) addr >= DRAM_BASE);

	#if DEBUG_MEM_UTIL
	ASSERT(busy == 0);
	busy = 1;
	#endif

	SETREG(MU_VALUE, val);

	do
	{
		size = MIN(num_bytes, MU_MAX_BYTES);

		SETREG(MU_DST_ADDR, addr);
		SETREG(MU_SIZE, size);
		SETREG(MU_CMD, MU_CMD_SET_REPT_DRAM);

		addr += size;
		num_bytes -= size;

		while (GETREG(MU_RESULT) == 0xFFFFFFFF);
	}
	while (num_bytes != 0);

	#if DEBUG_MEM_UTIL
	busy = 0;
	#endif
}

UINT32 _mem_search_min_max(const void* const addr, UINT32 const num_bytes_per_item, UINT32 const num_items, UINT32 const cmd)
{
	UINT32 retval;

	ASSERT((UINT32)addr % sizeof(UINT32) == 0);
	ASSERT(num_items != 0 && num_bytes_per_item * num_items <= MU_MAX_BYTES);

	#if DEBUG_MEM_UTIL
	ASSERT(busy == 0);
	busy = 1;
	#endif

	if (num_bytes_per_item == sizeof(UINT8))
	{
		SETREG(MU_UNITSTEP, MU_UNIT_8 | 1);
	}
	else if (num_bytes_per_item == sizeof(UINT16))
	{
		SETREG(MU_UNITSTEP, MU_UNIT_16 | 2);
	}
	else
	{
		SETREG(MU_UNITSTEP, MU_UNIT_32 | 4);
	}
	SETREG(MU_SRC_ADDR, addr);
	SETREG(MU_SIZE, num_items);
	SETREG(MU_CMD, cmd);

	while (1)
	{
		retval = GETREG(MU_RESULT);

		if (retval != 0xFFFFFFFF)
			break;
	}

	#if DEBUG_MEM_UTIL
	busy = 0;
	#endif

	return retval;
}

UINT32 _mem_search_equ(const void* const addr, UINT32 const num_bytes_per_item, UINT32 const num_items, UINT32 const cmd, UINT32 const val)
{
	UINT32 retval;

	ASSERT((UINT32)addr % sizeof(UINT32) == 0);
	ASSERT(num_bytes_per_item * num_items <= MU_MAX_BYTES);

	if (num_items == 0)
	{
		return 1;
	}

	#if DEBUG_MEM_UTIL
	ASSERT(busy == 0);
	busy = 1;
	#endif

	if (num_bytes_per_item == sizeof(UINT8))
	{
		SETREG(MU_UNITSTEP, MU_UNIT_8 | 1);
	}
	else if (num_bytes_per_item == sizeof(UINT16))
	{
		SETREG(MU_UNITSTEP, MU_UNIT_16 | 2);
	}
	else
	{
		SETREG(MU_UNITSTEP, MU_UNIT_32 | 4);
	}

	SETREG(MU_SRC_ADDR, addr);
	SETREG(MU_VALUE, val);
	SETREG(MU_SIZE, num_items);
	SETREG(MU_CMD, cmd);

	while (1)
	{
		retval = GETREG(MU_RESULT);

		if (retval != 0xFFFFFFFF)
			break;
	}

	#if DEBUG_MEM_UTIL
	busy = 0;
	#endif

	return retval;
}

void _write_dram_32(UINT32 const addr, UINT32 const val)
{
	ASSERT(addr >= DRAM_BASE && addr < (DRAM_BASE + DRAM_SIZE));

	if (addr % sizeof(UINT32) != 0)
		while (1);

	mem_copy(addr, &val, sizeof(UINT32));
}

void _write_dram_16(UINT32 const addr, UINT16 const val)
{
	ASSERT(addr >= DRAM_BASE && addr < (DRAM_BASE + DRAM_SIZE));

	UINT32 offset = addr % 4;
	UINT32 aligned_addr = addr & 0xFFFFFFFC;
	UINT32 real_addr = DRAM_BASE + (aligned_addr - DRAM_BASE)/128*132 + (aligned_addr - DRAM_BASE)%128;
	UINT32 old_val = *(volatile UINT32*)real_addr;
	UINT32 new_val = (old_val & ~(0xFFFF << (offset * 8))) | (val << (offset * 8));
	mem_copy(aligned_addr, &new_val, sizeof(UINT32));
}

void _write_dram_8(UINT32 const addr, UINT8 const val)
{
	ASSERT(addr >= DRAM_BASE && addr < (DRAM_BASE + DRAM_SIZE));

	UINT32 offset = addr % 4;
	UINT32 aligned_addr = addr & 0xFFFFFFFC;
	UINT32 real_addr = DRAM_BASE + (aligned_addr - DRAM_BASE)/128*132 + (aligned_addr - DRAM_BASE)%128;
	UINT32 old_val = *(volatile UINT32*)real_addr;
	UINT32 new_val = (old_val & ~(0xFF << (offset * 8))) | (val << (offset * 8));
	mem_copy(aligned_addr, &new_val, sizeof(UINT32));
}

void _set_bit_dram(UINT32 const base_addr, UINT32 const bit_offset)
{
	ASSERT(base_addr >= DRAM_BASE && base_addr < (DRAM_BASE + DRAM_SIZE));

	UINT32 val;
	UINT32 addr = base_addr + bit_offset / 8;
	UINT32 aligned_addr = addr & 0xFFFFFFFC;
	UINT32 new_bit_offset = (base_addr*8 + bit_offset) % 32;
	UINT32 real_addr = DRAM_BASE + (aligned_addr - DRAM_BASE)/128*132 + (aligned_addr - DRAM_BASE)%128;

	val = *(volatile UINT32*)real_addr;
	val |= (1 << new_bit_offset);
	mem_copy(aligned_addr, &val, sizeof(UINT32));
}

void _clr_bit_dram(UINT32 const base_addr, UINT32 const bit_offset)
{
	ASSERT(base_addr >= DRAM_BASE && base_addr < (DRAM_BASE + DRAM_SIZE));

	UINT32 val;
	UINT32 addr = base_addr + bit_offset / 8;
	UINT32 aligned_addr = addr & 0xFFFFFFFC;
	UINT32 new_bit_offset = (base_addr*8 + bit_offset) % 32;
	UINT32 real_addr = DRAM_BASE + (aligned_addr - DRAM_BASE)/128*132 + (aligned_addr - DRAM_BASE)%128;

	val = *(volatile UINT32*)real_addr;
	val &= ~(1 << new_bit_offset);
	mem_copy(aligned_addr, &val, sizeof(UINT32));
}

BOOL32 _tst_bit_dram(UINT32 const base_addr, UINT32 const bit_offset)
{
	ASSERT(base_addr >= DRAM_BASE && base_addr < (DRAM_BASE + DRAM_SIZE));

	UINT32 val;
	UINT32 addr = base_addr + bit_offset / 8;
	UINT32 aligned_addr = addr & 0xFFFFFFFC;
	UINT32 new_bit_offset = (base_addr*8 + bit_offset) % 32;
	UINT32 real_addr = DRAM_BASE + (aligned_addr - DRAM_BASE)/128*132 + (aligned_addr - DRAM_BASE)%128;
	val = *(volatile UINT32*)real_addr;

	return val & (1 << new_bit_offset);
}

UINT8 _read_dram_8(UINT32 const addr)
{
	ASSERT(addr >= DRAM_BASE && addr < (DRAM_BASE + DRAM_SIZE));

    UINT32 aligned_addr = addr & 0xFFFFFFFC;
	UINT32 byte_offset = addr % 4;
	UINT32 val;
	UINT32 real_addr = DRAM_BASE + (aligned_addr - DRAM_BASE)/128*132 + (aligned_addr - DRAM_BASE)%128;
	val = *(volatile UINT32*)real_addr;

	return (UINT8) (val >> (byte_offset * 8));
}

UINT16 _read_dram_16(UINT32 const addr)
{
	ASSERT(addr >= DRAM_BASE && addr < (DRAM_BASE + DRAM_SIZE));

    UINT32 aligned_addr = addr & 0xFFFFFFFC;
	UINT32 byte_offset = addr % 4;
	UINT32 val;
	UINT32 real_addr = DRAM_BASE + (aligned_addr - DRAM_BASE)/128*132 + (aligned_addr - DRAM_BASE)%128;
	val = *(volatile UINT32*)real_addr;

	return (UINT16) (val >> (byte_offset * 8));
}

UINT32 _read_dram_32(UINT32 const addr)
{
	ASSERT(addr >= DRAM_BASE && addr < (DRAM_BASE + DRAM_SIZE));
	ASSERT(addr % sizeof(UINT32) == 0);

	UINT32 val;
	UINT32 real_addr = DRAM_BASE + (addr - DRAM_BASE)/128*132 + (addr - DRAM_BASE)%128;
	val = *(volatile UINT32*)real_addr;

	return val;
}

UINT32 _mem_cmp_sram(const void* const addr1, const void* const addr2, const UINT32 num_bytes)
{
    UINT8 *ptr1, *ptr2;
    UINT8 val1, val2;
    UINT32 i;

	ASSERT((UINT32) addr1 < DRAM_BASE);
	ASSERT((UINT32) addr2 < DRAM_BASE);
	ASSERT(num_bytes != 0);

    ptr1 = (UINT8 *)addr1;
    ptr2 = (UINT8 *)addr2;

    for (i = 0; i < num_bytes; i++)
    {
        val1 = *ptr1++;
        val2 = *ptr2++;

        if (val1 != val2)
        {
            return (val1 > val2 ) ? 1 : -1;
        }
    }
    return 0;
}

UINT32 _mem_cmp_dram(const void* const addr1, const void* const addr2, const UINT32 num_bytes)
{
    UINT8 *ptr1, *ptr2;
    UINT8 val1, val2;
    UINT32 i;

	ASSERT((UINT32)addr1 >= DRAM_BASE && (UINT32)addr1 < (DRAM_BASE + DRAM_SIZE));
	ASSERT((UINT32)addr2 >= DRAM_BASE && (UINT32)addr2 < (DRAM_BASE + DRAM_SIZE));

    ASSERT(num_bytes != 0);

    ptr1 = (UINT8 *)addr1;
    ptr2 = (UINT8 *)addr2;

    for (i = 0; i < num_bytes; i++)
    {
        val1 = read_dram_8(ptr1);
        val2 = read_dram_8(ptr2);

        if (val1 != val2)
        {
            return (val1 > val2 ) ? 1 : -1;
        }
        ptr1++;
        ptr2++;
    }
    return 0;
}


