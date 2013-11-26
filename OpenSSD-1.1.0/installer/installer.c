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

#pragma once
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif
#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif
#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif
#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

#if _MSC_VER <= 1400
#include <stdio.h>
#include <tchar.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#include <afx.h>
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <iostream>
#include "ntddstor.h"
#include <Winioctl.h>
#include <ntddscsi.h>
#include <intrin.h>
#include <conio.h>
#else
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <time.h>
#include <iostream>
#include <Winioctl.h>
#include "ntddstor.h"
#include <ntddscsi.h>
#include <intrin.h>
#include <conio.h>
#endif

#include "jasmine.h"
#include "rom.h"
#include "ata_7.h"

#define PAUSE				{ printf("press any key to continue"); _getch(); printf("\n"); fflush(stdin); fflush(stdout); }

#define FW_BUF_BYTES		(96 * 1024)

#define BLANK_CHK_BYTES		16

#define SCAN_LIST_NOT_LOADED	0
#define SCAN_LIST_OLD			1
#define SCAN_LIST_NEW			2

#define ERR_CORRUPT		(OK + 3)
#define ERR_BLANK		(OK + 4)

#define STOSD(ADDR, VAL, SIZE)		__stosd((unsigned long*) (ADDR), (VAL), (SIZE))

typedef struct
{
	factory_cmd_t fac;

	UINT8		pad[BYTES_PER_SECTOR - sizeof(factory_cmd_t)];
	scan_list_t	scan_list[NUM_BANKS_MAX];
	UINT8		firmware_image_buf[FW_BUF_BYTES];
	UINT32		firmware_image_bytes;
	HANDLE		handle;			// device handle
	BOOL32		up;				// whether initialized
	UINT32		total_bad_vblks;
	BOOL8		scan_list_loaded;
} installer_context_t;

factory_cmd_t* fac;

#ifdef ASSERT
#undef ASSERT
#endif

#define ASSERT(X)											\
{															\
	if (!(X))												\
	{														\
		printf("\ndebug assertion failure\n%s : %d\n",		\
				__FILE__, __LINE__);						\
		_flushall();										\
		_getch();											\
		exit(0);											\
	}														\
}

#define ERR_EXIT										\
{														\
	printf("error at %s %u\n", __FILE__, __LINE__);		\
	_getch();											\
	exit(1);											\
}

void	enable_factory_mode(UINT32);
UINT32	detect_sdram_size();
UINT32	check_flash_id(UINT16* const id);
void	report_system_error();
BOOL32	open_target_drv();
void	reset_banks();
void	reset_fctrl(BOOL32 big_ecc);
BOOL32	init();
void	scan_bad_blks();
void	install();
BOOL32	erase_flash(UINT32 start_pblk_offset, UINT32 end_pblk_offset);
BOOL32	load_scan_list_from_nand();
UINT32	read(UINT32 rbank, UINT32 row, UINT32 num_bytes, UINT8* buf);
UINT8*	get_bsp_intr();
void	print_bank_number(UINT32 rbank);
void	print_scan_list(FILE* file);
void	send_to_dev(void* const buf, UINT32 const sect_cnt);
void	recv_from_dev(void* const buf, UINT32 const lba, UINT32 const sect_cnt);
void	save_scan();
BOOL32	load_scan_list_from_file();
void	ftl_install_mapping_table();	// FTL should provide the function that creates initial mapping table.

UINT8 GETCH(char* msg, char* list);

enum { FLASH_ID_OK, FLASH_ID_VOID, FLASH_ID_ERROR };


const UINT8 c_bank_map[NUM_BANKS] = BANK_MAP;

installer_context_t* hc;

UINT8 GETCH(char* msg, char* list)
{
	UINT32 i;

	if (msg)
		printf(msg);

	while (1)
	{
		char ch = (char) _getch();

		if (ch >= 'A' && ch <= 'Z')
		{
			ch -= 'A' - 'a';
		}

		if (list == NULL)
			return (UINT8) ch;
		else
		{
			for (i = 0; i < strlen(list); i++)
			{
				if (list[i] == ch)
				{
					printf("%c\n", ch);
					fflush(stdin);
					fflush(stdout);
					return (UINT8) ch;
				}
			}
		}
	}
}

void report_system_error()
{
	LPVOID lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(), MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);

	printf("\nSYSTEM ERROR: %s\n", (LPCTSTR) lpMsgBuf);

	LocalFree(lpMsgBuf);
}

void recv_from_dev(void* const buf, UINT32 const lba, UINT32 const sect_cnt)
{
	BOOL	result;
	DWORD	byte_cnt;
	UINT8* buf_ptr = (UINT8*) buf;
	UINT32 remain = sect_cnt;

	if ((((UINT32) buf) % BYTES_PER_SECTOR) != 0)
	{
		printf("ERROR: buffer address is not aligned\n");
		ERR_EXIT;
	}

	SetFilePointer(hc->handle, lba * BYTES_PER_SECTOR, 0, FILE_BEGIN);

	while (remain != 0)
	{
		UINT32 num_sectors = MIN(remain, 256);

		result = ReadFile(hc->handle, buf_ptr, num_sectors * BYTES_PER_SECTOR, &byte_cnt, NULL);

		if (!result)
		{
			printf("\nERROR: no response from drive\t\t\n");
			ERR_EXIT;
		}

		buf_ptr += num_sectors * BYTES_PER_SECTOR;
		remain -= num_sectors;
	}
}

void send_to_dev(void* const buf, UINT32 const sect_cnt)
{
	DWORD byte_cnt;

	if ((((UINT32) buf) % BYTES_PER_SECTOR) != 0)
	{
		printf("ERROR: buffer address is not aligned\n");
		ERR_EXIT;
	}

	if (buf != NULL)
	{
		UINT8* buf_ptr = (UINT8*) buf;
		UINT32 remain = sect_cnt;

		if (sect_cnt == 0)
		{
			ERR_EXIT;
		}

		if (ROM_DRAM_BUF_LBA + sect_cnt >= DRAM_SIZE)
		{
			printf("ERROR: sect_cnt is too large\n");
			ERR_EXIT;
		}

		SetFilePointer(hc->handle, ROM_DRAM_BUF_LBA * BYTES_PER_SECTOR, 0, FILE_BEGIN);

		while (remain != 0)
		{
			UINT32 num_sectors = MIN(remain, 256);

			if (!WriteFile(hc->handle, (void*) buf_ptr, num_sectors * BYTES_PER_SECTOR, &byte_cnt, NULL))
			{
				printf("ERROR: no response from drive\t\t\n");
				ERR_EXIT;
			}

			buf_ptr += num_sectors * BYTES_PER_SECTOR;
			remain -= num_sectors;
		}
	}

	srand((UINT32) time(NULL));

	fac->key_1 = rand();
	fac->key_2 = 0 - fac->key_1;

	SetFilePointer(hc->handle, 0, 0, FILE_BEGIN);

	if (!WriteFile(hc->handle, (void*) &hc->fac, BYTES_PER_SECTOR, &byte_cnt, NULL))
	{
		printf("ERROR: no response from drive\t\t\n");
		ERR_EXIT;
	}
}

void print_bank_number(UINT32 rbank)
{
	static UINT8 c_bank_rmap[NUM_BANKS_MAX] = BANK_RMAP;
	rbank = rbank;
	printf("bank %u=%c%u\t\n", rbank, rbank % NUM_CHNLS_MAX + 'A', rbank / NUM_CHNLS_MAX);
}

UINT32 check_flash_id(UINT16* const id)
{
	UINT32	maker_code = 0;
	UINT32	id_data;
	UINT32	i;
	UINT32	same_count;

	id_data		= 0;
	same_count	= 0;

	for (i = 0; i < 5; i++)
	{
		UINT32 high	= id[i] & 0xFF;
		UINT32 low	= (id[i] >> 8) & 0xFF;

		if (high != low)
		{
			return FLASH_ID_ERROR;	// different ID output from high chip and from low chip
		}

		if (id[i] == id[0])
		{
			same_count++;
		}

		if (i == 0)
		{
			maker_code = low;
		}
		else
		{
			id_data = id_data | (low << (8*(i - 1)));
		}
	}

	if (same_count == 5)
	{
		return FLASH_ID_VOID;	// chip does not exist
	}

	if (maker_code != NAND_SPEC_MF)
	{
		return FLASH_ID_ERROR;
	}
	else if ((id_data & NAND_SPEC_ID_MASK) != NAND_SPEC_ID)
	{
		return FLASH_ID_ERROR;
	}
	else
	{
		return FLASH_ID_OK;
	}
}

BOOL32 init()
{
	UINT32	i;
	UINT8*	buf;
	UINT32	rbank;
	UINT32	bank_bmp;

	if (hc->handle == INVALID_HANDLE_VALUE)
	{
		printf("ERROR: target device not opened\n");
		return FAIL;
	}

	buf = (UINT8*) _aligned_malloc(BYTES_PER_SECTOR, BYTES_PER_SECTOR);

	reset_fctrl(FALSE);
	reset_banks();

	bank_bmp = 0;

	for (rbank = 0; rbank < NUM_BANKS_MAX; rbank++)
	{
		// flash -> SRAM buffer
		fac->cmd		= ROM_CMD_FLASH_CMD;
		fac->subcmd 	= FC_READ_ID;
		fac->rbank		= rbank;
		fac->option 	= 0;
		fac->dma_addr	= INVALID32;
		fac->dma_cnt	= 16;
		fac->col		= 0;
		fac->row_l		= 0;
		fac->row_h		= 0;
		fac->wait		= TRUE;
		fac->clear		= TRUE;
		send_to_dev(NULL, 0);

		// SRAM buffer -> DRAM_BASE
		fac->cmd			= ROM_CMD_MU_CMD;
		fac->subcmd 		= MU_CMD_COPY;
		fac->mu_src_addr	= INVALID32;
		fac->mu_dst_addr	= DRAM_BASE;
		fac->mu_size		= 16;
		send_to_dev(NULL, 0);

		// DRAM_BASE -> host
		recv_from_dev(buf, 0, 1);

		printf("bank %02u(%c%u) low:", rbank, rbank % NUM_CHNLS_MAX + 'A', rbank / NUM_CHNLS_MAX);

		for (i = 0; i < 5; i++)
			printf(" %02X", buf[i*2]);

		printf(" 	high:", rbank);

		for (i = 0; i < 5; i++)
			printf(" %02X", buf[i*2 + 1]);

		printf("\n");

		if (check_flash_id((UINT16*) buf) == FLASH_ID_OK)
		{
			bank_bmp |= (1 << rbank);
		}
	}

	printf("bank_bmp = %08X\n", bank_bmp);

	_aligned_free(buf);

	hc->up = FALSE;

	if (bank_bmp == BANK_BMP)
	{
		hc->up = TRUE;
	}
	else if ((bank_bmp & BANK_BMP) == BANK_BMP)
	{
		printf("WARNING: Flash configuration mismatch. The firmware was built for %08X.\n", BANK_BMP);

		if (GETCH("Proceed anyway?", "yn") == 'y')
		{
			hc->up = TRUE;
		}
	}
	else
	{
		printf("ERROR: Flash configuration mismatch. The firmware was built for %08X.\n", BANK_BMP);
	}

	return hc->up ? OK : FAIL;
}

void enable_factory_mode(UINT32 sdram_size)
{
	UINT32 sum, i;
	factory_cmd_t* fac = &hc->fac;
	UINT32* const buf = (UINT32*) fac;

	for (i = 0; i < BYTES_PER_SECTOR / sizeof(UINT32); i++)
	{
		buf[i] = 0;
	}

	fac->sdram_signature_1 = 0x0c23bb10;
	fac->sdram_signature_2_ptr = (BYTES_PER_SECTOR - 1) / sizeof(UINT32);
	*(buf + fac->sdram_signature_2_ptr) = SDRAM_PARAM_SIGNATURE - fac->sdram_signature_1;

	fac->cmd = ROM_CMD_FACTORY_MODE;

	if(sdram_size == SDRAM_16MB)
		fac->sdram.init = SDRAM_PARAM_75_16_INIT ;
	else if(sdram_size == SDRAM_32MB)
		fac->sdram.init = SDRAM_PARAM_75_32_INIT ;
	else if(sdram_size == SDRAM_64MB)
		fac->sdram.init = SDRAM_PARAM_75_64_INIT ;
	else
		fac->sdram.init = SDRAM_PARAM_ROM_CODE_INIT ;

	fac->sdram.refresh = SDRAM_PARAM_ROM_CODE_REFRESH;
	fac->sdram.timing = SDRAM_PARAM_ROM_CODE_TIMING;
	fac->sdram.mrs = SDRAM_PARAM_ROM_CODE_MRS;
	fac->sdram.bytes = DRAM_SIZE;

	fac->key_1 = 0;
	fac->key_2 = 0;
	fac->last_member = 0;
	sum = 0;

	for (i = 0; i < BYTES_PER_SECTOR / sizeof(UINT32); i++)
	{
		sum += buf[i];
	}

	fac->last_member = FACTORY_CMD_SIGNATURE - sum;

	send_to_dev(NULL, 0);

	fac->cmd = ROM_CMD_WRITE_MEM;
	fac->mem_addr = PHY_DEBUG;
	fac->mem_val = 0x40000139;
	send_to_dev(NULL, 0);

	Sleep(600);

	fac->cmd = ROM_CMD_WRITE_MEM;
	fac->mem_addr = PHY_DEBUG;
	fac->mem_val = 0x400A040E;
	send_to_dev(NULL, 0);

	Sleep(600);
}


#define SDRAM_SIZE_TEST_16_VAL		0x16161616
#define SDRAM_SIZE_TEST_32_VAL		0x32323232
#define SDRAM_SIZE_TEST_BASE		0x40000800
#define SDRAM_SIZE_16_TEST_BASE 	0x40000C00
#define SDRAM_SIZE_32_TEST_BASE 	0x42000800

UINT32 detect_sdram_size(void)
{
	UINT8* buf = (UINT8*) _aligned_malloc(64 * 1024 * 1024, BYTES_PER_SECTOR);
	UINT32 read_value;
	UINT32 sdram_size = INVALID32;

	if (buf == NULL)
	{
		printf("ERROR: memory allocation failure\n");
		ERR_EXIT
	}

	/////////////////////////////////////////
	// Detect 16MB Memory
	fac->cmd = ROM_CMD_WRITE_MEM;
	fac->mem_addr = SDRAM_SIZE_TEST_BASE;
	fac->mem_val = 0x0;
	send_to_dev(NULL, 0);

	fac->cmd = ROM_CMD_WRITE_MEM;
	fac->mem_addr = SDRAM_SIZE_16_TEST_BASE;
	fac->mem_val = SDRAM_SIZE_TEST_16_VAL;
	send_to_dev(NULL, 0);

	// Check if above write operation affects test base address
	fac->cmd		= ROM_CMD_READ_MEM;
	fac->mem_addr	= SDRAM_SIZE_TEST_BASE;
	send_to_dev(NULL, 0);
	recv_from_dev(buf, 0, 1);
	read_value = *(UINT32 *)buf;

	if(SDRAM_SIZE_TEST_16_VAL == read_value)
	{
		sdram_size = SDRAM_16MB;
		goto detect_end;
	}

	/////////////////////////////////////////
	// Detect 32MB Memory
	fac->cmd = ROM_CMD_WRITE_MEM;
	fac->mem_addr = SDRAM_SIZE_TEST_BASE;
	fac->mem_val = 0x0;
	send_to_dev(NULL, 0);

	fac->cmd = ROM_CMD_WRITE_MEM;
	fac->mem_addr = SDRAM_SIZE_32_TEST_BASE;
	fac->mem_val = SDRAM_SIZE_TEST_32_VAL;
	send_to_dev(NULL, 0);

	// At first, read back written data
	fac->cmd		= ROM_CMD_READ_MEM;
	fac->mem_addr	= SDRAM_SIZE_32_TEST_BASE;
	send_to_dev(NULL, 0);
	recv_from_dev(buf, 0, 1);
	read_value = *(UINT32 *)buf;

	if(SDRAM_SIZE_TEST_32_VAL != read_value)
	{
		sdram_size = SDRAM_32MB;
		goto detect_end;
	}

	// Check if above write operation affects test base address
	fac->cmd		= ROM_CMD_READ_MEM;
	fac->mem_addr	= SDRAM_SIZE_TEST_BASE;
	send_to_dev(NULL, 0);
	recv_from_dev(buf, 0, 1);
	read_value = *(UINT32 *)buf;

	if(SDRAM_SIZE_TEST_32_VAL == read_value)
	{
		sdram_size = SDRAM_32MB;
		goto detect_end;
	}

	sdram_size = SDRAM_64MB;

detect_end:

	_aligned_free(buf);

	return sdram_size;
}

static BOOL32 query_device(HANDLE handle)
{
	#define BUF_SIZE 	4096
	#define BUF_ALIGN	4096

	UINT8* buffer = (UINT8*) _aligned_malloc(BUF_SIZE, BUF_ALIGN);
	ULONG dummy;

	STORAGE_PROPERTY_QUERY query;
	query.QueryType = PropertyStandardQuery;
	query.PropertyId = StorageDeviceProperty;

	UINT32 result;

	if (DeviceIoControl(handle, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), (void*) buffer, BUF_SIZE, &dummy, NULL) == 0)
	{
		result = FAIL;
	}
	else
	{
		PSTORAGE_DEVICE_DESCRIPTOR desc = (PSTORAGE_DEVICE_DESCRIPTOR) buffer;

		if (desc->ProductIdOffset != 0 && desc->ProductIdOffset != -1)
		{
			char* product_id = (char*) (buffer + desc->ProductIdOffset);

			if (memcmp(product_id, "YATAPDON", 8) == 0)
			{
				result = OK;
			}
			else
			{
				result = FAIL;
			}
		}
		else
		{
			result = FAIL;
		}

		if (result == FAIL)
		{
			if (desc->VendorIdOffset != 0 && desc->VendorIdOffset != -1)
			{
				char* vendor_id = (char*) (buffer + desc->VendorIdOffset);

				if (memcmp(vendor_id, "YATAPDON", 8) == 0)
				{
					result = OK;
				}
			}
		}
	}

	_aligned_free(buffer);

	return result;
}

BOOL32 open_target_drv()
{
	UINT32	i, drive_id, sdram_size;
	HANDLE	handle;
	char	drive_name[100];

	if (hc->handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hc->handle);
		hc->handle = INVALID_HANDLE_VALUE;
	}

	drive_id = 0xFFFFFFFF;

	for (i = 0; i < 10; i++)
	{
		sprintf(drive_name, "\\\\.\\PHYSICALDRIVE%u", i);

		handle = CreateFile((LPCSTR) drive_name,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_RANDOM_ACCESS | FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH,
			NULL);

		if (handle == INVALID_HANDLE_VALUE)
			continue;

		if (query_device(handle) == OK)
		{
			drive_id = i;
			CloseHandle(handle);
			break;
		}
		else
		{
			CloseHandle(handle);
		}
	}

	if (drive_id == 0xFFFFFFFF)
	{
		printf("ERROR: Jasmine not found\n");
		return FAIL;
	}

	sprintf(drive_name, "\\\\.\\PHYSICALDRIVE%u", drive_id);

	hc->handle = CreateFile(drive_name,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_RANDOM_ACCESS | FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH,
		NULL);

	if (hc->handle == INVALID_HANDLE_VALUE)
	{
		report_system_error();
		ERR_EXIT;
	}

	printf("enabling factory mode...");

	enable_factory_mode(SDRAM_64MB);
	sdram_size = detect_sdram_size();
	printf("\ndetected SDRAM size = %d MB\n", (SDRAM_16MB == sdram_size) ? 16 : ((SDRAM_32MB == sdram_size) ? 32 : 64) );

	if (((DRAM_SIZE == 65075200) && (sdram_size != SDRAM_64MB)) ||
		((DRAM_SIZE == 32537600) && (sdram_size != SDRAM_32MB)) ||
		((DRAM_SIZE == 16268800) && (sdram_size != SDRAM_16MB)))
	{
		printf("ERROR: firmware built for %d MB\n", (DRAM_SIZE == 16268800) ? 16 : ((DRAM_SIZE == 32537600) ? 32 : 64));
		ERR_EXIT;
	}

	enable_factory_mode(sdram_size);

	return OK;
}

void reset_fctrl(BOOL32 strong_ecc)
{
	UINT32 nand_cfg_1, nand_cfg_2, ecc_size;

	if (strong_ecc)
	{
		ecc_size = 2;
	}
	else
	{
		#if SPARE_PER_PHYPAGE / SECTORS_PER_PHYPAGE >= 28
		// RS : 12 symbol correction per sector
		// BCH : 16 bit correction per sector
		ecc_size = 2;

		#elif SPARE_PER_PHYPAGE / SECTORS_PER_PHYPAGE >= 23
		// RS : 12 symbol correction per sector
		// BCH : 12 bit correction per sector
		ecc_size = 1;

		#elif SPARE_PER_PHYPAGE / SECTORS_PER_PHYPAGE >= 16
		// RS : 6 symbol correction per sector
		// BCH : 8 bit correction per sector
		ecc_size = 0;

		#else
		#error("ECC configuration error");
		#endif
	}

	nand_cfg_2 = PG_SIZE(SECTORS_PER_PHYPAGE >> 3)
				| BLK_SIZE(PAGES_PER_VBLK >> 7)
				| CHK_CMD(NAND_SPEC_MF == NAND_SPEC_TOSHIBA && OPTION_2_PLANE)
				| CHK_CODE(0xC0)
				| CHK_MASK(0xC1)
				| BCH((NAND_SPEC_CELL == NAND_SPEC_CELL_MLC) || strong_ecc)
				| ECC_SIZE(ecc_size);

	#if FLASH_TYPE == H27UCG8UDMYR || FLASH_TYPE == H27UBG8T2MYR || FLASH_TYPE == H27UDG8VEMYR ||	\
		FLASH_TYPE == H27UCG8VFATR || FLASH_TYPE == H27UBG8U5ATR || FLASH_TYPE == H27UBG8T2ATR ||	\
		FLASH_TYPE == K9GAG08U0D || FLASH_TYPE == K9LBG08U0D || FLASH_TYPE == K9HCG08U1D || 		\
		FLASH_TYPE == K9GBG08U0M || FLASH_TYPE == K9PFG08U5M
	nand_cfg_1 = 0x18;	// 4Xnm process with LGA TYPE
	#elif NAND_SPEC_MF == NAND_SPEC_SAMSUNG || NAND_SPEC_MF == NAND_SPEC_HYNIX
	{
		#if NAND_SPEC_DIE == NAND_SPEC_DIE_2 && NAND_SPEC_PLANE == NAND_SPEC_PLANE_2 && OPTION_2_PLANE
		{
			#if ROWS_PER_BANK == 262144
			nand_cfg_1 = 0x10 | (1 << 12);		// A30

			#elif ROWS_PER_BANK == 524288
			nand_cfg_1 = 0x10 | (2 << 12);		// A31

			#elif ROWS_PER_BANK == 1048576
			nand_cfg_1 = 0x10 | (3 << 12);		// A32

			#elif ROWS_PER_BANK == 2097152
			nand_cfg_1 = 0x10 | (4 << 12);		// A33

			#elif ROWS_PER_BANK == 4194304
			nand_cfg_1 = 0x10 | (5 << 12);		// A34

			#else
			#error("invalid configuration for 2-plane operation in DDP");
			#endif
		}
		#else
		nand_cfg_1 = 0x10;
		#endif
	}
	#elif NAND_SPEC_MF == NAND_SPEC_MICRON || NAND_SPEC_MF == NAND_SPEC_INTEL
		#if NAND_SPEC_READ == NAND_SPEC_READ_CYCLE_2
		nand_cfg_1 = 0x21;	// Micron 2-plane with ONFI
		#else
		nand_cfg_1 = 0x11;	// Micron 2-plane
		#endif
	#elif NAND_SPEC_MF == NAND_SPEC_TOSHIBA
		#if NAND_SPEC_PLANE == NAND_SPEC_PLANE_2
		nand_cfg_1 = 0x12 | 0x08;	// Toshiba 4Xnm process
		#else
		nand_cfg_1 = 0x12;	// Toshiba copyback
		#endif
	#else
		nand_cfg_1 = 0x10;
	#endif

	fac->cmd			= ROM_CMD_RESET_FLASH;
	fac->nand_cfg_2		= nand_cfg_2;
	fac->time_cycle		= FLASH_PARAM_TIMECYCLE_SAFE;
	fac->nand_cfg_1		= nand_cfg_1;

	send_to_dev(NULL, 0);

	fac->cmd			= ROM_CMD_WRITE_MEM;
	fac->mem_addr		= SATA_BUF_PAGE_SIZE;
	fac->mem_val		= BYTES_PER_PAGE;
	send_to_dev(NULL, 0);
}

void reset_banks()
{
	UINT32 rbank;

	for (rbank = 0; rbank < NUM_BANKS_MAX; rbank++)
	{
		fac->cmd		= ROM_CMD_FLASH_CMD;
		fac->subcmd 	= FC_GENERIC;
		fac->rbank		= rbank;
		fac->col		= 0xFF;
		fac->option 	= 0;
		fac->wait		= TRUE;
		fac->clear		= TRUE;

		send_to_dev(NULL, 0);
	}

	Sleep(100);
}

void print_scan_list(FILE* file)
{
	UINT32 i, num_bad_pblks, num_bad_pblks_h, num_bad_pblks_l, rbank, num_total_entries;

	num_total_entries = 0;

	for (rbank = 0; rbank < NUM_BANKS_MAX; rbank++)
	{
		if ((BANK_BMP & (1 << rbank)) == 0)
			continue;

		num_total_entries += hc->scan_list[rbank].num_entries;
	}

	fprintf(file, "list of initial bad blocks :\n");

	for (rbank = 0; rbank < NUM_BANKS_MAX; rbank++)
	{
		if ((BANK_BMP & (1 << rbank)) == 0)
			continue;

		num_bad_pblks = hc->scan_list[rbank].num_entries;

		num_bad_pblks_l = 0;

		for (i = 0; i < num_bad_pblks; i++)
		{
			if ((hc->scan_list[rbank].list[i] & 0x8000) == 0)
				num_bad_pblks_l++;
		}

		fprintf(file, "\nbank %u(%c%u) L: %u of %u (%.2f%%)\n", rbank, rbank % NUM_CHNLS_MAX + 'A', rbank / NUM_CHNLS_MAX,
			num_bad_pblks_l, PBLKS_PER_BANK, (double) num_bad_pblks_l / PBLKS_PER_BANK * 100);

		for (i = 0; i < num_bad_pblks; i++)
		{
			if ((hc->scan_list[rbank].list[i] & 0x8000) == 0)
				fprintf(file, "%u ", hc->scan_list[rbank].list[i]);
		}

		num_bad_pblks_h = 0;

		for (i = 0; i < num_bad_pblks; i++)
		{
			if (hc->scan_list[rbank].list[i] & 0x8000)
				num_bad_pblks_h++;
		}

		fprintf(file, "\nbank %u(%c%u) H: %u of %u (%.2f%%)\n", rbank, rbank % NUM_CHNLS_MAX + 'A', rbank / NUM_CHNLS_MAX,
			num_bad_pblks_h, PBLKS_PER_BANK, (double) num_bad_pblks_h / PBLKS_PER_BANK * 100);

		for (i = 0; i < num_bad_pblks; i++)
		{
			if (hc->scan_list[rbank].list[i] & 0x8000)
				fprintf(file, "%u ", hc->scan_list[rbank].list[i] & (~0x8000));
		}

		if (num_bad_pblks_l + num_bad_pblks_h != num_bad_pblks)
		{
			GETCH("\nWARNING: scan list seems to be corrupt for this bank\n", NULL);
		}
	}

	fprintf(file, "\n");
}

void scan_bad_blks(void)
{
	UINT32	total_bad_blk_entries;
	UINT32* input;
	UINT32* buf;
	UINT32	total_bad_blks;

	if (hc->up == FALSE)
	{
		printf("ERROR: not initialized\n");
		return;
	}

	buf = (UINT32*) _aligned_malloc(ROM_SCAN_BUF_SIZE, BYTES_PER_SECTOR);
	input = buf;

	fac->start_blk = 1;
	fac->blks_per_bank = PBLKS_PER_BANK;

	_BitScanForward((DWORD*) &fac->shift_rpb, PAGES_PER_VBLK);	// shift_rpb = log_2(PAGES_PER_VBLK)

	fac->cmd = ROM_CMD_SCAN;
	fac->col = BYTES_PER_PHYPAGE;				// the offset of the first byte of the spare area
	fac->dma_cnt = 4;							// number of bytes to inspect
	fac->rows_per_blk = PAGES_PER_VBLK;
	fac->bank_bmp = BANK_BMP;

	send_to_dev(NULL, 0);

	Sleep(1000);

	recv_from_dev(buf, ROM_DRAM_BUF_LBA, ROM_SCAN_BUF_SIZE / BYTES_PER_SECTOR);

	// The output from the ROM code (scan result) is going to be converted into scan list.

	total_bad_blk_entries = *input++;	// bad block #k in high chip and bad block #k in low chip are counted as one
	total_bad_blks = *input++;			// bad block #k in high chip and bad block #k in low chip are counted as two

	printf("total number of bad blocks reported = %u\n", total_bad_blks);

	if (total_bad_blks > (ROM_SCAN_BUF_SIZE / sizeof(UINT32) - 2))
	{
		printf("ERROR: Too many bad blocks. Device-side buffer can hold %u entries.\n",
			ROM_SCAN_BUF_SIZE / sizeof(UINT32) - 2);

		ERR_EXIT;
	}

	while (total_bad_blk_entries-- != 0)
	{
		UINT32	list_entry	= *input++;
		UINT32	rbank		= (list_entry >> 24) & 0x3F;
		UINT32	blk 		= list_entry & 0x00FFFFFF;

		if ((list_entry & 0xC0000000) == 0)
		{
			printf("ERROR: scan result has error\n");
			ERR_EXIT;
		}
		else
		{
			if (list_entry & 0x80000000)
			{
				UINT32 num_entries = hc->scan_list[rbank].num_entries++;
				hc->scan_list[rbank].list[num_entries] = ((UINT16) blk) | 0x8000;
			}

			if (list_entry & 0x40000000)
			{
				UINT32 num_entries = hc->scan_list[rbank].num_entries++;
				hc->scan_list[rbank].list[num_entries] = (UINT16) blk;
			}
		}
	}

	if (*input != 0xFFFFFFFF)
	{
		// This should not be the case, because the ROM code always puts 0xFFFFFFFF at the end of scan result.

		printf("ERROR: termination mark not found from list end\n");
		ERR_EXIT;
	}

	hc->scan_list_loaded = TRUE;

	_aligned_free(buf);
}

BOOL32 erase_flash(UINT32 start_pblk_offset, UINT32 end_pblk_offset)
{
	UINT32	pblk_offset;
	UINT32	rbank;

	if (end_pblk_offset == (UINT32) -1)
		end_pblk_offset = PBLKS_PER_BANK;

	for (pblk_offset = start_pblk_offset; pblk_offset <= end_pblk_offset; pblk_offset++)
	{
		BOOL8 erased[NUM_BANKS_MAX];

		fac->clear = TRUE;

		printf("erasing block %u\r", pblk_offset);

		for (rbank = 0; rbank < NUM_BANKS_MAX; rbank++)
		{
			UINT32 i;
			BOOL32 bad_h, bad_l;

			erased[rbank] = FALSE;

			if ((BANK_BMP & (1 << rbank)) == 0)
				continue;

			fac->cmd	= ROM_CMD_FLASH_CMD;
			fac->subcmd = FC_ERASE;
			fac->rbank	= rbank;
			fac->option = 0;
			fac->col	= 0;
			fac->wait	= FALSE;
			fac->row_l	= pblk_offset * PAGES_PER_VBLK;
			fac->row_h	= pblk_offset * PAGES_PER_VBLK;

			bad_h = bad_l = FALSE;

			for (i = 0; i < hc->scan_list[rbank].num_entries; i++)
			{
				UINT16 bad_pblk_offset = hc->scan_list[rbank].list[i] & (~0x8000);
				UINT16 high_chip_flag = hc->scan_list[rbank].list[i] & 0x8000;

				if (pblk_offset == bad_pblk_offset)
				{
					if (high_chip_flag)
						bad_h = TRUE;
					else
						bad_l = TRUE;
				}
			}

			if (bad_h && bad_l)
			{
				// If both of high chip and low chip have bad blocks at the same block address,
				// we erase nothing at this iteration.

				continue;
			}

			// If either one of high chip or low chip has a bad block, we do not erase it.
			fac->option = (bad_h * 0x20) | (bad_l * 0x10);

			send_to_dev(NULL, 0);

			fac->clear = FALSE;

			erased[rbank] = TRUE;
		}
	}

	fac->cmd = ROM_CMD_FLASH_FINISH;
	send_to_dev(NULL, 0);

	printf("\t\t\t\t\n");

	return OK;
}

void save_scan()
{
	UINT32	rbank;
	FILE*	file;
	char	directory[100];
	char	file_name[150];

	if (hc->scan_list_loaded == FALSE)
	{
		printf("ERROR: have nothing to save\n");
		return;
	}

	printf("new directory = ");
	scanf("%s", directory);

	if (CreateDirectory(directory, NULL) == 0)
	{
		printf("ERROR: directory creation failed\n");
		return;
	}

	for (rbank = 0; rbank < NUM_BANKS_MAX; rbank++)
	{
		UINT32	temp;
		UINT32	size;

		if ((BANK_BMP & (1 << rbank)) == 0)
			continue;

		sprintf(file_name, "%s\\scan_%u.bin", directory, rbank);

		file = fopen(file_name, "w+b");

		if (file == NULL)
		{
			ERR_EXIT;
		}

		size = SCAN_LIST_SIZE;
		temp = fwrite((void*) (hc->scan_list + rbank), 1, size, file);

		if (temp != size)
		{
			ERR_EXIT;
		}

		fclose(file);
	}

	sprintf(file_name, "%s\\scan.txt", directory);

	file = fopen(file_name, "w+t");

	if (file == NULL)
	{
		ERR_EXIT;
	}

	print_scan_list(file);

	fclose(file);
}

BOOL32 load_scan_list_from_file()
{
	UINT32	rbank;
	FILE*	file;
	char	directory[100];
	char	file_name[150];

	printf("directory = ");
	scanf("%s", directory);

	for (rbank = 0; rbank < NUM_BANKS_MAX; rbank++)
	{
		if ((BANK_BMP & (1 << rbank)) == 0)
			continue;

		sprintf(file_name, "%s\\scan_%u.bin", directory, rbank);

		file = fopen(file_name, "rb");

		if (file == NULL)
		{
			printf("ERROR: cannot open file %s\n", file_name);
			return FAIL;
		}

		fread((void*) (hc->scan_list + rbank), 1, SCAN_LIST_SIZE, file);

		fclose(file);

		hc->scan_list_loaded = TRUE;
	}

    print_scan_list(stdout);

	return OK;
}

BOOL32 load_scan_list_from_nand(void)
{
	UINT32	rbank, page_read_result, result;
	UINT32*	buf;

	if (hc->up == FALSE)
	{
		printf("ERROR: not initialized\n");
		return FAIL;
	}

	result = OK;
	buf = (UINT32*) _aligned_malloc(BYTES_PER_SMALL_PAGE, BYTES_PER_SECTOR);

	for (rbank = 0; rbank < NUM_BANKS_MAX; rbank++)
	{
		if ((BANK_BMP & (1 << rbank)) == 0)
			continue;

		page_read_result = read(rbank, SCAN_LIST_PAGE_OFFSET, SCAN_LIST_SIZE, (UINT8*) buf);

		if (page_read_result == OK)
		{
			printf("scan list found on ");
			print_bank_number(rbank);

			memcpy(hc->scan_list + rbank, buf, SCAN_LIST_SIZE);
		}
		else if (page_read_result == ERR_CORRUPT)
		{
			printf("ERROR: corrupted scan list (or unknown type of data) on ");
			print_bank_number(rbank);

			result = FAIL;
		}
		else
		{
			printf("ERROR: cannot find scan list on ");
			print_bank_number(rbank);
			result = FAIL;
		}
	}

	_aligned_free(buf);

	if (result == OK)
	{
		hc->scan_list_loaded = TRUE;
	}

	return result;
}

UINT32 read(UINT32 rbank, UINT32 row, UINT32 num_bytes, UINT8* buf)
{
	if (hc->up == FALSE)
	{
		printf("ERROR: not initialized\n");
		return FAIL;
	}

	if (rbank == INVALID32)
	{
		printf("bank = ");
		scanf("%u", &rbank);

		printf("row (real) = ");
		scanf("%u", &row);

		printf("bytes = ");
		scanf("%u", &num_bytes);
	}

	if ((BANK_BMP & (1 << rbank)) == 0)
	{
		printf("ERROR: specified bank does not exist\n");
		print_bank_number(rbank);
		return FAIL;
	}

	fac->cmd		= ROM_CMD_FLASH_CMD;
	fac->subcmd 	= FC_COL_ROW_READ_OUT;
	fac->rbank		= rbank;
	fac->option 	= FO_E;
	fac->dma_addr	= ROM_DRAM_BUF_ADDR;
	fac->dma_cnt	= num_bytes;
	fac->col		= 0;
	fac->row_l		= row;
	fac->row_h		= row;
	fac->wait		= TRUE;
	fac->clear		= TRUE;
	send_to_dev(NULL, 0);

	UINT8* bsp_intr = get_bsp_intr();

	recv_from_dev(buf, ROM_DRAM_BUF_LBA, num_bytes / BYTES_PER_SECTOR);

	if (bsp_intr[rbank] & FIRQ_DATA_CORRUPT)
	{
		return ERR_CORRUPT;
	}
	else if (bsp_intr[rbank] & FIRQ_ALL_FF)
	{
		return ERR_BLANK;
	}
	else
	{
		return OK;
	}
}

UINT8* get_bsp_intr()
{
	static UINT32 bsp_intr[8];
	UINT32 i;
	UINT32* buf = (UINT32*) _aligned_malloc(BYTES_PER_SECTOR, BYTES_PER_SECTOR);

	for (i = 0; i < NUM_BANKS_MAX; i += 4)
	{
		fac->cmd = ROM_CMD_READ_MEM;
		fac->mem_addr = BSP_INTR_BASE + i;
		send_to_dev(NULL, 0);
		recv_from_dev(buf, 0, 1);
		bsp_intr[i / 4] = *buf;
	}

	_aligned_free(buf);

	return (UINT8*) bsp_intr;
}

static BOOL32 write_to_flash(void* buf, UINT32 rbank, UINT32 page_addr, UINT32 num_bytes)
{
	void* read_buf;
	BOOL32 result = OK;
	UINT32 num_sectors;

	num_sectors = num_bytes / BYTES_PER_SECTOR;

	fac->cmd		= ROM_CMD_FLASH_CMD;
	fac->subcmd 	= FC_COL_ROW_IN_PROG;
	fac->rbank		= rbank;
	fac->option 	= FO_E | FO_B_W_DRDY;
	fac->dma_addr	= ROM_DRAM_BUF_ADDR;
	fac->dma_cnt	= num_bytes;
	fac->col		= 0;
	fac->row_l		= page_addr;
	fac->row_h		= page_addr;
	fac->wait		= TRUE;
	fac->clear		= TRUE;

	send_to_dev(buf, num_sectors);

	fac->cmd		= ROM_CMD_FLASH_CMD;
	fac->subcmd 	= FC_COL_ROW_READ_OUT;
	fac->rbank		= rbank;
	fac->option 	= FO_E;
	fac->dma_addr	= ROM_DRAM_BUF_ADDR;
	fac->dma_cnt	= num_bytes;
	fac->col		= 0;
	fac->row_l		= page_addr;
	fac->row_h		= page_addr;
	fac->wait		= TRUE;
	fac->clear		= TRUE;

	send_to_dev(NULL, 0);

	read_buf = _aligned_malloc(num_bytes, BYTES_PER_SECTOR);
	recv_from_dev(read_buf, ROM_DRAM_BUF_LBA, num_sectors);

	if (memcmp(read_buf, buf, num_bytes) != 0)
	{
		FILE* file;
		char file_name[20];

		printf("\nERROR: data corruption. see good.bin and bad.bin\n");
		print_bank_number(rbank);

		file = fopen("good.bin", "w+b");
		fwrite(buf, 1, num_bytes, file);
		fclose(file);

		sprintf(file_name, "bad%u.bin", rbank);
		file = fopen(file_name, "w+b");
		fwrite(read_buf, 1, num_bytes, file);
		fclose(file);

		result = FAIL;
	}

	_aligned_free(read_buf);

	return result;
}

static BOOL32 install_block_zero(void)
{
	UINT32 num_fw_pages, page_offset = NULL, rbank;
	UINT32* buf;
	UINT32*	read_buf;
	BOOL32 result;
	UINT32 const total_bytes = NUM_BANKS_MAX * BYTES_PER_SMALL_PAGE;

	result = OK;

	// STEP 1 - write the scan list into page_offset #0 of block #0 of each bank

	reset_fctrl(FALSE);		// scan list is written in normal ECC mode

	read_buf = (UINT32*) _aligned_malloc(total_bytes, BYTES_PER_SECTOR);

	fac->cmd = ROM_CMD_DO_NOTHING;
	send_to_dev((void*) hc->scan_list, NUM_BANKS_MAX * SECTORS_PER_SMALL_PAGE);

	for (rbank = 0; rbank < NUM_BANKS_MAX; rbank++)
	{
		if ((BANK_BMP & (1 << rbank)) == 0)
			continue;

		fac->cmd		= ROM_CMD_FLASH_CMD;
		fac->subcmd 	= FC_COL_ROW_IN_PROG;
		fac->rbank		= rbank;
		fac->dma_addr	= ROM_DRAM_BUF_ADDR + rbank * BYTES_PER_SMALL_PAGE;
		fac->option 	= FO_E | FO_B_W_DRDY;
		fac->dma_cnt	= BYTES_PER_SMALL_PAGE;
		fac->col		= 0;
		fac->row_l		= SCAN_LIST_PAGE_OFFSET;
		fac->row_h		= SCAN_LIST_PAGE_OFFSET;
		fac->wait		= FALSE;
		fac->clear		= TRUE;
		send_to_dev(NULL, 0);
	}

	for (rbank = 0; rbank < NUM_BANKS_MAX; rbank++)
	{
		if ((BANK_BMP & (1 << rbank)) == 0)
			continue;

		fac->cmd		= ROM_CMD_FLASH_CMD;
		fac->subcmd 	= FC_COL_ROW_READ_OUT;
		fac->rbank		= rbank;
		fac->dma_addr	= ROM_DRAM_BUF_ADDR + rbank * BYTES_PER_SMALL_PAGE;
		fac->option 	= FO_E;
		fac->dma_cnt	= BYTES_PER_SMALL_PAGE;
		fac->col		= 0;
		fac->row_l		= SCAN_LIST_PAGE_OFFSET;
		fac->row_h		= SCAN_LIST_PAGE_OFFSET;
		fac->wait		= FALSE;
		fac->clear		= TRUE;
		send_to_dev(NULL, 0);
	}

	fac->cmd = ROM_CMD_FLASH_FINISH;
	send_to_dev(NULL, 0);

	recv_from_dev(read_buf, ROM_DRAM_BUF_LBA, NUM_BANKS_MAX * SECTORS_PER_SMALL_PAGE);

	if (memcmp(read_buf, (void*) hc->scan_list, total_bytes))
	{
		FILE* file;

		printf("\nERROR: data corruption. see good.bin and bad.bin\n");

		file = fopen("good.bin", "w+b");
		fwrite((void*) hc->scan_list, 1, total_bytes, file);
		fclose(file);

		file = fopen("bad.bin", "w+b");
		fwrite(read_buf, 1, total_bytes, file);
		fclose(file);

		result = FAIL;
	}

	_aligned_free(read_buf);

	if (result == FAIL)
	{
		return FAIL;
	}

	// STEP 2 - dummy pages filled with 0xFF

	buf = (UINT32*)_aligned_malloc(BYTES_PER_SECTOR, BYTES_PER_SECTOR);
	STOSD(buf, 0xFFFFFFFF, BYTES_PER_SECTOR / sizeof(UINT32));

	for (rbank = 0; rbank < NUM_BANKS_MAX; rbank++)
	{
		if ((BANK_BMP & (1 << rbank)) == 0)
			continue;

		for (page_offset = SCAN_LIST_PAGE_OFFSET + 1; page_offset < STAMP_PAGE_OFFSET; page_offset++)
		{
			if (write_to_flash(buf, rbank, page_offset, BYTES_PER_SECTOR) == FAIL)
			{
				return FAIL;
			}
		}
	}

	// STEP 3 - size information of firmware binary image
	// This information is used by ROM code while loading the image.

	reset_fctrl(TRUE);		// use BCH (16bit/512byte)

	STOSD(buf, 0, BYTES_PER_SECTOR / sizeof(UINT32));

	buf[0] = 0xC0C2E003;				// magic number
	buf[1] = hc->firmware_image_bytes;	// size of the image

	for (rbank = 0; rbank < NUM_BANKS_MAX; rbank++)
	{
		if ((BANK_BMP & (1 << rbank)) == 0)
			continue;

		if (write_to_flash(buf, rbank, STAMP_PAGE_OFFSET, BYTES_PER_SECTOR) == FAIL)
		{
			result = FAIL;
		}
	}

	_aligned_free(buf);

	if (result == FAIL)
	{
		return FAIL;
	}

	// STEP 4 - firmware binary image
	//
	// Due to the way ROM code was implemented, the following three restrictions apply to firmware image:
	// 1. The image should be written with 16bit/512byte BCH.
	// 2. The image should start from page_offset #5 of block #0.
	// 3. Only the first 2KB of each page can be used for storing the image.

	num_fw_pages = (hc->firmware_image_bytes + BYTES_PER_FW_PAGE - 1) / BYTES_PER_FW_PAGE;

	for (rbank = 0; rbank < NUM_BANKS_MAX; rbank++)
	{
		if ((BANK_BMP & (1 << rbank)) == 0)
			continue;

		buf = (UINT32*) hc->firmware_image_buf;

		for (page_offset = FW_PAGE_OFFSET; page_offset < FW_PAGE_OFFSET + num_fw_pages; page_offset++)
		{
			if (write_to_flash(buf, rbank, page_offset, BYTES_PER_FW_PAGE) == FAIL)
			{
				return FAIL;
			}

			buf += BYTES_PER_FW_PAGE / sizeof(UINT32);
		}
	}

	printf("firmware image written to pages %u through %u\n", FW_PAGE_OFFSET, page_offset - 1);

	reset_fctrl(FALSE);

	return OK;
}

static BOOL32 load_firmware_image(void)
{
	FILE* file;
	UINT32 num_bytes;

	file = fopen("firmware.bin", "rb");

	if (file == NULL)
	{
		printf("ERROR: cannot open firmware.bin for reading\n");
		return FAIL;
	}

	num_bytes = fread(hc->firmware_image_buf, 1, FW_BUF_BYTES, file);

	fclose(file);

	if (num_bytes == 0)
	{
		printf("ERROR: cannot read firmware.bin\n");
		return FAIL;
	}

	if (num_bytes == FW_BUF_BYTES)
	{
		printf("ERROR: firmware image too large\n");
		return FAIL;
	}

	hc->firmware_image_bytes = num_bytes;

	return OK;
}

void install(void)
{
	if (hc->up == FALSE)
	{
		printf("ERROR: target device not initialized\n");
		return;
	}

	if (hc->scan_list_loaded == FALSE)
	{
		printf("ERROR: scan list not loaded.\n");
		return;
	}

	if (load_firmware_image() == FAIL)
		return;

	if (erase_flash(0, 0) == FAIL)
		return;

	if (install_block_zero() == FAIL)
		return;

	//ftl_install_mapping_table();

	fac->cmd = ROM_CMD_RESET;
	send_to_dev(NULL, 0);

	GETCH("firmware installation complete", NULL);

	exit(0);
}

enum
{
	MENU_INIT = 1,
	MENU_LOAD_SCAN_DEV,
	MENU_INSTALL,
	MENU_SCAN,
	MENU_ERASE_ALL,
	MENU_SAVE_SCAN,
	MENU_LOAD_SCAN,
	MENU_EXIT,
	NUM_FAC_MENU_ITEMS
};

static UINT32 print_menu()
{
	char* menu_text[NUM_FAC_MENU_ITEMS];

	STOSD(menu_text, NULL, NUM_FAC_MENU_ITEMS);

	menu_text[MENU_INIT] = "initialize";
	menu_text[MENU_SCAN] = "scan init bad blks";
	menu_text[MENU_LOAD_SCAN_DEV] = "read scan list from flash block 0";
	menu_text[MENU_ERASE_ALL] = "erase flash all";
	menu_text[MENU_INSTALL] = "install FW";
	menu_text[MENU_SAVE_SCAN] = "save scan list to file";
	menu_text[MENU_LOAD_SCAN] = "read scan list from file";
	menu_text[MENU_EXIT] = "exit";

	UINT32	i;
	UINT32	menu_sel;

	fflush(stdin);

	printf("\n");

	for (i = 0; i < NUM_FAC_MENU_ITEMS; i++)
	{
		if (menu_text[i] != NULL)
		{
			printf("%2u  %s\n", i, menu_text[i]);
		}
	}

	do
	{
		printf("\nselect :");
		scanf("%u", &menu_sel);
	}
	while (menu_sel >= NUM_FAC_MENU_ITEMS);

	printf("%s\n\n", menu_text[menu_sel]);

	return menu_sel;
}

#if _MSC_VER <= 1400
CWinApp app;
#endif

int main()
{
	#if _MSC_VER <= 1400
	AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0);
	#endif

	hc = (installer_context_t*) VirtualAlloc(NULL, sizeof(installer_context_t), MEM_COMMIT, PAGE_READWRITE);
	fac = &(hc->fac);

	ASSERT(sizeof(factory_cmd_t) <= BYTES_PER_SECTOR);

	hc->handle = INVALID_HANDLE_VALUE;
	hc->up = FALSE;
	hc->scan_list_loaded = FALSE;

	STOSD(hc->scan_list, 0x00000000, sizeof(hc->scan_list) / sizeof(UINT32));

	if (open_target_drv() == FAIL)
	{
		return 1;
	}

	while (1)
	{
		UINT32 menu_sel;

		menu_sel = print_menu();

		if (menu_sel == MENU_EXIT)
			break;

		switch (menu_sel)
		{
			case MENU_INIT:
				init();
				break;

			case MENU_SCAN:
				scan_bad_blks();
				break;

			case MENU_LOAD_SCAN_DEV:
				load_scan_list_from_nand();
				break;

			case MENU_ERASE_ALL:

				if (hc->scan_list_loaded)
				{
					if (GETCH("WARNING: All the blocks will be erased.\nScan list will also be lost.\nContinue?", "yn") == 'y')
					{
						erase_flash(0, (UINT32)-1);
					}
				}
				else
				{
					if (GETCH("WARNING: All the blocks will be erased.\nBecause you have not read a scan list,\ninitial bad blocks will also be erased.\nContinue?", "yn") == 'y')
					{
						erase_flash(0, (UINT32)-1);
					}
				}
				break;

			case MENU_INSTALL:
				install();
				break;

			case MENU_SAVE_SCAN:
				save_scan();
				break;

			case MENU_LOAD_SCAN:
				load_scan_list_from_file();
				break;

		}
	}

	if (hc->handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hc->handle);
	}

	VirtualFree(hc, 0, MEM_RELEASE);

	return 0;
}

