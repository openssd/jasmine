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


#ifndef	MEM_UTIL_H
#define	MEM_UTIL_H


extern UINT8 g_temp_mem[BYTES_PER_SECTOR];	// scratch pad


//////////////////////////////
// memory utility functions
//////////////////////////////

#define mem_set_sram(ADDR, VAL, BYTES)					_mem_set_sram((UINT32) (ADDR), (UINT32) (VAL), (UINT32) (BYTES))
#define mem_set_dram(ADDR, VAL, BYTES)					_mem_set_dram((UINT32) (ADDR), (UINT32) (VAL), (UINT32) (BYTES))
#define mem_copy(DST, SRC, BYTES)						_mem_copy((void*) (DST), (void*) (SRC), (UINT32) (BYTES))
#define mem_bmp_find_sram(BMP, BYTES, VAL)				_mem_bmp_find_sram((void*) (BMP), (UINT32) (BYTES), (UINT32) (VAL))
#define mem_bmp_find_dram(BMP, BYTES, VAL)				_mem_bmp_find_dram((void*) (BMP), (UINT32) (BYTES), (UINT32) (VAL))
#define mem_search_min_max(ADDR, UNIT, SIZE, CMD)		_mem_search_min_max((void*) (ADDR), (UINT32) (UNIT), (UINT32) (SIZE), (UINT32) (CMD))
#define mem_search_equ(ADDR, UNIT, SIZE, CMD, VAL)		_mem_search_equ((void*) (ADDR), (UINT32) (UNIT), (UINT32) (SIZE), (UINT32) (CMD), (UINT32) (VAL))
#define mem_search_equ_sram(ADDR, UNIT, SIZE, VAL)		mem_search_equ(ADDR, UNIT, SIZE, MU_CMD_SEARCH_EQU_SRAM, VAL)
#define mem_search_equ_dram(ADDR, UNIT, SIZE, VAL)		mem_search_equ(ADDR, UNIT, SIZE, MU_CMD_SEARCH_EQU_DRAM, VAL)

//SKKU
#define mem_cmp_sram(ADDR1, ADDR2, BYTES)				_mem_cmp_sram((void*) (ADDR1), (void*) (ADDR2), (BYTES))
#define mem_cmp_dram(ADDR1, ADDR2, BYTES)               _mem_cmp_dram((void*) (ADDR1), (void*) (ADDR2), (BYTES))

void	_mem_set_sram(UINT32 addr, UINT32 const val, UINT32 bytes);
void	_mem_set_dram(UINT32 addr, UINT32 const val, UINT32 bytes);
void	_mem_copy(void* const dst, const void* const src, UINT32 const bytes);
UINT32	_mem_bmp_find_sram(const void* const bitmap, UINT32 const bytes, UINT32 const val);
UINT32	_mem_bmp_find_dram(const void* const bitmap, UINT32 const bytes, UINT32 const val);
UINT32	_mem_search_min_max(const void* const addr, UINT32 const unit, UINT32 const size, UINT32 const cmd);
UINT32	_mem_search_equ(const void* const addr, UINT32 const unit, UINT32 const size, UINT32 const cmd, UINT32 const val);

UINT32 _mem_cmp_sram(const void* const addr1, const void* const addr2, const UINT32 bytes);
UINT32 _mem_cmp_dram(const void* const addr1, const void* const addr2, const UINT32 bytes);

#define read_dram_8(ADDR)						_read_dram_8((UINT32) (ADDR))
#define read_dram_16(ADDR)						_read_dram_16((UINT32) (ADDR))
#define read_dram_32(ADDR)						_read_dram_32((UINT32) (ADDR))
#define write_dram_8(ADDR, VAL)					_write_dram_8((UINT32) (ADDR), (UINT8) (VAL))
#define write_dram_16(ADDR, VAL)				_write_dram_16((UINT32) (ADDR), (UINT16) (VAL))
#define write_dram_32(ADDR, VAL)				_write_dram_32((UINT32) (ADDR), (UINT32) (VAL))
#define set_bit_dram(BASE_ADDR, BIT_OFFSET)		_set_bit_dram((UINT32) (BASE_ADDR), (UINT32) (BIT_OFFSET))
#define clr_bit_dram(BASE_ADDR, BIT_OFFSET)		_clr_bit_dram((UINT32) (BASE_ADDR), (UINT32) (BIT_OFFSET))
#define tst_bit_dram(BASE_ADDR, BIT_OFFSET)		_tst_bit_dram((UINT32) (BASE_ADDR), (UINT32) (BIT_OFFSET))

UINT8	_read_dram_8(UINT32 const addr);
UINT16	_read_dram_16(UINT32 const addr);
UINT32	_read_dram_32(UINT32 const addr);
void	_write_dram_8(UINT32 const addr, UINT8 const val);
void	_write_dram_16(UINT32 const addr, UINT16 const val);
void	_write_dram_32(UINT32 const addr, UINT32 const val);
void	_set_bit_dram(UINT32 const base_addr, UINT32 const bit_offset);
void	_clr_bit_dram(UINT32 const base_addr, UINT32 const bit_offset);
BOOL32	_tst_bit_dram(UINT32 const base_addr, UINT32 const bit_offset);

#define set_bit_sram(BITMAP, OFFSET)	*(((UINT8*)(BITMAP) + (OFFSET)/8)) |= 1 << ((OFFSET) % 8)
#define clr_bit_sram(BITMAP, OFFSET)	*(((UINT8*)(BITMAP) + (OFFSET)/8)) &= ~(1 << ((OFFSET) % 8))
#define tst_bit_sram(BITMAP, OFFSET)	((*(((UINT8*)(BITMAP)) + (OFFSET)/8) & (1 << ((OFFSET) % 8))) != 0)


//////////////////////////////////
// memory utility register set
//////////////////////////////////

#define	MU_SRC_ADDR 	(MREG_BASE + 0x10)
#define	MU_DST_ADDR 	(MREG_BASE + 0x14)
#define	MU_VALUE   		(MREG_BASE + 0x18)
#define	MU_SIZE    		(MREG_BASE + 0x1C)		// max 32768 bytes for mem_set, mem_find, 32768 items for mem_search
#define	MU_RESULT  		(MREG_BASE + 0x20)
#define	MU_CMD  		(MREG_BASE + 0x24)
#define MU_UNITSTEP		(MREG_BASE + 0x30)		// step <= 32 bytes


//////////////////////////////////
// command codes for MU_CMD
//////////////////////////////////

#define MU_CMD_SET_REPT_SRAM		0x000
#define MU_CMD_SET_INCR_32_SRAM		0x010
#define MU_CMD_SET_INCR_16_SRAM		0x020
#define MU_CMD_SET_INCR_8_SRAM		0x030

#define MU_CMD_SET_REPT_DRAM		0x040
#define MU_CMD_SET_INCR_32_DRAM		0x050
#define MU_CMD_SET_INCR_16_DRAM		0x060
#define MU_CMD_SET_INCR_8_DRAM		0x070

#define MU_CMD_COPY					0x001

#define MU_CMD_FIND_SRAM			0x002
#define MU_CMD_FIND_DRAM			0x042

#define MU_CMD_SEARCH_MAX_SRAM		0x103
#define MU_CMD_SEARCH_MIN_SRAM		0x083
#define MU_CMD_SEARCH_EQU_SRAM		0x003

#define MU_CMD_SEARCH_MAX_DRAM		0x143
#define MU_CMD_SEARCH_MIN_DRAM		0x0C3
#define MU_CMD_SEARCH_EQU_DRAM		0x043

#define MU_MAX_BYTES	32768

#define SDRAM_ECC_UNIT	128

#define MU_UNIT_8	(0 << 8)	// value in MU_UNITSTEP
#define MU_UNIT_16	(1 << 8)
#define MU_UNIT_32	(2 << 8)

#endif	// MEM_UTIL_H
