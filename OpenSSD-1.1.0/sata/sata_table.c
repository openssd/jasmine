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
#include "sata.h"
#include "sata_cmd.h"

//	Command Class, LBA type in Register FIS
const UINT8 ata_cmd_class_table[256] = {
			ATR_LOCK_FREE |	CCL_OTHER,			// 0x00	NOP
							CCL_UNDEFINED,		// 0x01
							CCL_UNDEFINED,		// 0x02
							CCL_UNDEFINED,		// 0x03
							CCL_UNDEFINED,		// 0x04
							CCL_UNDEFINED,		// 0x05
			ATR_LBA_EXT	|	CCL_OTHER,			// 0x06	Data Set Management
							CCL_UNDEFINED,		// 0x07
			ATR_LOCK_FREE |	CCL_OTHER,			// 0x08	Device Reset
							CCL_UNDEFINED,		// 0x09
							CCL_UNDEFINED,		// 0x0A
							CCL_UNDEFINED,		// 0x0B
							CCL_UNDEFINED,		// 0x0C
							CCL_UNDEFINED,		// 0x0D
							CCL_UNDEFINED,		// 0x0E
							CCL_UNDEFINED,		// 0x0F
							CCL_OTHER,			// 0x10	Recalibrate
							CCL_UNDEFINED,		// 0x11
							CCL_UNDEFINED,		// 0x12
							CCL_UNDEFINED,		// 0x13
							CCL_UNDEFINED,		// 0x14
							CCL_UNDEFINED,		// 0x15
							CCL_UNDEFINED,		// 0x16
							CCL_UNDEFINED,		// 0x17
							CCL_UNDEFINED,		// 0x18
							CCL_UNDEFINED,		// 0x19
							CCL_UNDEFINED,		// 0x1A
							CCL_UNDEFINED,		// 0x1B
							CCL_UNDEFINED,		// 0x1C
							CCL_UNDEFINED,		// 0x1D
							CCL_UNDEFINED,		// 0x1E
							CCL_UNDEFINED,		// 0x1F
			ATR_LBA_NOR |	CCL_FTL_D2H,		// 0x20	Read Sectors
							CCL_UNDEFINED,		// 0x21
							CCL_UNDEFINED,		// 0x22
							CCL_UNDEFINED,		// 0x23
			ATR_LBA_EXT |	CCL_FTL_D2H,		// 0x24	Read Sectors Ext
			ATR_LBA_EXT |	CCL_FTL_D2H,		// 0x25	Read DMA Ext
							CCL_UNDEFINED,		// 0x26
			ATR_LOCK_FREE|	CCL_OTHER,			// 0x27	Read Native Max Address Ext
							CCL_UNDEFINED,		// 0x28
			ATR_LBA_EXT |	CCL_FTL_D2H,		// 0x29	Read Multiple Ext
							CCL_UNDEFINED,		// 0x2A
							CCL_UNDEFINED,		// 0x2B
							CCL_UNDEFINED,		// 0x2C
							CCL_UNDEFINED,		// 0x2D
							CCL_UNDEFINED,		// 0x2E
			ATR_LBA_EXT	| 	CCL_OTHER,			// 0x2F	Read Log Ext
			ATR_LBA_NOR |	CCL_FTL_H2D,		// 0x30	Write Sectors
							CCL_UNDEFINED,		// 0x31
							CCL_UNDEFINED,		// 0x32
							CCL_UNDEFINED,		// 0x33
			ATR_LBA_EXT |	CCL_FTL_H2D,		// 0x34	Write Sectors Ext
			ATR_LBA_EXT |	CCL_FTL_H2D,		// 0x35	Write DMA Ext
							CCL_UNDEFINED,		// 0x36
ATR_NO_SECT|ATR_LBA_EXT	| 	CCL_OTHER,			// 0x37	Set Max Address Ext
							CCL_UNDEFINED,		// 0x38
			ATR_LBA_EXT |	CCL_FTL_H2D,		// 0x39	Write Multiple Ext
							CCL_UNDEFINED,		// 0x3A
							CCL_UNDEFINED,		// 0x3B
							CCL_UNDEFINED,		// 0x3C
							CCL_UNDEFINED,		// 0x3D
							CCL_UNDEFINED,		// 0x3E
			ATR_LBA_EXT	| 	CCL_OTHER,			// 0x3F	Write Log Ext
			ATR_LBA_NOR	| 	CCL_OTHER,			// 0x40	Read Verify Sectors
							CCL_UNDEFINED,		// 0x41
			ATR_LBA_EXT	| 	CCL_OTHER,			// 0x42	Read Verify Sectors Ext
							CCL_UNDEFINED,		// 0x43
							CCL_UNDEFINED,		// 0x44
							CCL_UNDEFINED,		// 0x45
							CCL_UNDEFINED,		// 0x46
							CCL_UNDEFINED,		// 0x47
							CCL_UNDEFINED,		// 0x48
							CCL_UNDEFINED,		// 0x49
							CCL_UNDEFINED,		// 0x4A
							CCL_UNDEFINED,		// 0x4B
							CCL_UNDEFINED,		// 0x4C
							CCL_UNDEFINED,		// 0x4D
							CCL_UNDEFINED,		// 0x4E
							CCL_UNDEFINED,		// 0x4F
							CCL_UNDEFINED,		// 0x50
							CCL_UNDEFINED,		// 0x51
							CCL_UNDEFINED,		// 0x52
							CCL_UNDEFINED,		// 0x53
							CCL_UNDEFINED,		// 0x54
							CCL_UNDEFINED,		// 0x55
							CCL_UNDEFINED,		// 0x56
							CCL_UNDEFINED,		// 0x57
							CCL_UNDEFINED,		// 0x58
							CCL_UNDEFINED,		// 0x59
							CCL_UNDEFINED,		// 0x5A
							CCL_UNDEFINED,		// 0x5B
							CCL_UNDEFINED,		// 0x5C
							CCL_UNDEFINED,		// 0x5D
							CCL_UNDEFINED,		// 0x5E
							CCL_UNDEFINED,		// 0x5F
							CCL_FTL_D2H,		// 0x60	Read FPDMA Queued
							CCL_FTL_H2D,		// 0x61	Write FPDMA Queued
							CCL_UNDEFINED,		// 0x62
							CCL_UNDEFINED,		// 0x63
							CCL_UNDEFINED,		// 0x64
							CCL_UNDEFINED,		// 0x65
							CCL_UNDEFINED,		// 0x66
							CCL_UNDEFINED,		// 0x67
							CCL_UNDEFINED,		// 0x68
							CCL_UNDEFINED,		// 0x69
							CCL_UNDEFINED,		// 0x6A
							CCL_UNDEFINED,		// 0x6B
							CCL_UNDEFINED,		// 0x6C
							CCL_UNDEFINED,		// 0x6D
							CCL_UNDEFINED,		// 0x6E
							CCL_UNDEFINED,		// 0x6F
							CCL_OTHER,			// 0x70	SEEK
							CCL_UNDEFINED,		// 0x71
							CCL_UNDEFINED,		// 0x72
							CCL_UNDEFINED,		// 0x73
							CCL_UNDEFINED,		// 0x74
							CCL_UNDEFINED,		// 0x75
							CCL_UNDEFINED,		// 0x76
							CCL_UNDEFINED,		// 0x77
							CCL_UNDEFINED,		// 0x78
							CCL_UNDEFINED,		// 0x79
							CCL_UNDEFINED,		// 0x7A
							CCL_UNDEFINED,		// 0x7B
							CCL_UNDEFINED,		// 0x7C
							CCL_UNDEFINED,		// 0x7D
							CCL_UNDEFINED,		// 0x7E
							CCL_UNDEFINED,		// 0x7F
							CCL_UNDEFINED,		// 0x80
							CCL_UNDEFINED,		// 0x81
							CCL_UNDEFINED,		// 0x82
							CCL_UNDEFINED,		// 0x83
							CCL_UNDEFINED,		// 0x84
							CCL_UNDEFINED,		// 0x85
							CCL_UNDEFINED,		// 0x86
							CCL_UNDEFINED,		// 0x87
							CCL_UNDEFINED,		// 0x88
							CCL_UNDEFINED,		// 0x89
							CCL_UNDEFINED,		// 0x8A
							CCL_UNDEFINED,		// 0x8B
							CCL_UNDEFINED,		// 0x8C
							CCL_UNDEFINED,		// 0x8D
							CCL_UNDEFINED,		// 0x8E
							CCL_UNDEFINED,		// 0x8F
			ATR_LOCK_FREE |	CCL_OTHER,			// 0x90	Execute Drive Diagnostics
							CCL_OTHER,			// 0x91	Initialize Device Parameters
			ATR_LBA_NOR	|	CCL_OTHER,			// 0x92	Download Microcode
							CCL_UNDEFINED,		// 0x93
			ATR_LOCK_FREE |	CCL_OTHER,			// 0x94 Standby Immediate
			ATR_LOCK_FREE |	CCL_OTHER,			// 0x95 Idle Immediate
			ATR_LOCK_FREE |	CCL_OTHER,			// 0x96 Standby
			ATR_LOCK_FREE |	CCL_OTHER,			// 0x97 Idle
							CCL_UNDEFINED,		// 0x98
			ATR_LOCK_FREE |	CCL_OTHER,			// 0x99 Sleep
							CCL_UNDEFINED,		// 0x9A
							CCL_UNDEFINED,		// 0x9B
							CCL_UNDEFINED,		// 0x9C
							CCL_UNDEFINED,		// 0x9D
							CCL_UNDEFINED,		// 0x9E
							CCL_UNDEFINED,		// 0x9F
							CCL_UNDEFINED,		// 0xA0
							CCL_UNDEFINED,		// 0xA1
							CCL_UNDEFINED,		// 0xA2
							CCL_UNDEFINED,		// 0xA3
							CCL_UNDEFINED,		// 0xA4
							CCL_UNDEFINED,		// 0xA5
							CCL_UNDEFINED,		// 0xA6
							CCL_UNDEFINED,		// 0xA7
							CCL_UNDEFINED,		// 0xA8
							CCL_UNDEFINED,		// 0xA9
							CCL_UNDEFINED,		// 0xAA
							CCL_UNDEFINED,		// 0xAB
							CCL_UNDEFINED,		// 0xAC
							CCL_UNDEFINED,		// 0xAD
							CCL_UNDEFINED,		// 0xAE
							CCL_UNDEFINED,		// 0xAF
			ATR_LOCK_FREE |	CCL_OTHER,			// 0xB0	SMART
							CCL_OTHER,			// 0xB1 Device Configuration
							CCL_UNDEFINED,		// 0xB2
							CCL_UNDEFINED,		// 0xB3
							CCL_UNDEFINED,		// 0xB4
							CCL_UNDEFINED,		// 0xB5
							CCL_UNDEFINED,		// 0xB6
							CCL_UNDEFINED,		// 0xB7
							CCL_UNDEFINED,		// 0xB8
							CCL_UNDEFINED,		// 0xB9
							CCL_UNDEFINED,		// 0xBA
							CCL_UNDEFINED,		// 0xBB
							CCL_UNDEFINED,		// 0xBC
							CCL_UNDEFINED,		// 0xBD
							CCL_UNDEFINED,		// 0xBE
							CCL_UNDEFINED,		// 0xBF
							CCL_UNDEFINED,		// 0xC0
							CCL_UNDEFINED,		// 0xC1
							CCL_UNDEFINED,		// 0xC2
							CCL_UNDEFINED,		// 0xC3
			ATR_LBA_NOR |	CCL_FTL_D2H,		// 0xC4	Read Multiple
			ATR_LBA_NOR |	CCL_FTL_H2D,		// 0xC5	Write Multiple
			ATR_LOCK_FREE |	CCL_OTHER,			// 0xC6	Set Multiple Mode
							CCL_UNDEFINED,		// 0xC7
			ATR_LBA_NOR |	CCL_FTL_D2H,		// 0xC8	Read DMA
							CCL_UNDEFINED,		// 0xC9
			ATR_LBA_NOR |	CCL_FTL_H2D,		// 0xCA	Write DMA
							CCL_UNDEFINED,		// 0xCB
							CCL_UNDEFINED,		// 0xCC
							CCL_UNDEFINED,		// 0xCD
							CCL_UNDEFINED,		// 0xCE
							CCL_UNDEFINED,		// 0xCF
							CCL_UNDEFINED,		// 0xD0
							CCL_UNDEFINED,		// 0xD1
							CCL_UNDEFINED,		// 0xD2
							CCL_UNDEFINED,		// 0xD3
							CCL_UNDEFINED,		// 0xD4
							CCL_UNDEFINED,		// 0xD5
							CCL_UNDEFINED,		// 0xD6
							CCL_UNDEFINED,		// 0xD7
							CCL_UNDEFINED,		// 0xD8
							CCL_UNDEFINED,		// 0xD9
							CCL_UNDEFINED,		// 0xDA
							CCL_UNDEFINED,		// 0xDB
							CCL_UNDEFINED,		// 0xDC
							CCL_UNDEFINED,		// 0xDD
							CCL_UNDEFINED,		// 0xDE
							CCL_UNDEFINED,		// 0xDF
			ATR_LOCK_FREE |	CCL_OTHER,			// 0xE0	Standby Immediate
			ATR_LOCK_FREE |	CCL_OTHER,			// 0xE1	Idle Immediate
			ATR_LOCK_FREE |	CCL_OTHER,			// 0xE2	Standby
			ATR_LOCK_FREE |	CCL_OTHER,			// 0xE3	Idle
			ATR_LOCK_FREE |	CCL_OTHER,			// 0xE4	Read Buffer
			ATR_LOCK_FREE |	CCL_OTHER,			// 0xE5	Check Power Mode
			ATR_LOCK_FREE |	CCL_OTHER,			// 0xE6	Sleep
							CCL_OTHER,			// 0xE7	Flush Cache
			ATR_LOCK_FREE |	CCL_OTHER,			// 0xE8	Write Buffer
							CCL_UNDEFINED,		// 0xE9
							CCL_OTHER,			// 0xEA	Flush Cache Ext
							CCL_UNDEFINED,		// 0xEB
			ATR_LOCK_FREE |	CCL_OTHER,			// 0xEC	Identify Device
							CCL_UNDEFINED,		// 0xED
							CCL_UNDEFINED,		// 0xEE
			ATR_LOCK_FREE |	CCL_OTHER,			// 0xEF	Set Features
							CCL_UNDEFINED,		// 0xF0
							CCL_OTHER,			// 0xF1 Security Set Password
			ATR_LOCK_FREE |	CCL_OTHER,			// 0xF2 Security Uniock
			ATR_LOCK_FREE |	CCL_OTHER,			// 0xF3 Security Erase Prepare
			ATR_LOCK_FREE |	CCL_OTHER,			// 0xF4 Security Erase Unit
							CCL_OTHER,			// 0xF5	Security Freeze Lock
							CCL_OTHER,			// 0xF6 Security Disable Password
							CCL_UNDEFINED,		// 0xF7
			ATR_LOCK_FREE |	CCL_OTHER,			// 0xF8	Read Native Max Address
ATR_NO_SECT|ATR_LBA_NOR	| 	CCL_OTHER,			// 0xF9	Set Max Address
							CCL_UNDEFINED,		// 0xFA
							CCL_UNDEFINED,		// 0xFB
			ATR_LBA_NOR |	CCL_OTHER,			// 0xFC Delete
			ATR_LBA_EXT	|	CCL_OTHER,			// 0xFD Delete Ext
							CCL_UNDEFINED,		// 0xFE
ATR_NO_SECT|ATR_LOCK_FREE | CCL_OTHER 			// 0xFF SRST
};


const UINT8 ata_command_code_table[] =
{
	0x00,	// 00 NOP
	0x06,	// 01 DATA SET MANAGEMENT
    0x08,   // 02 DEVICE RESET
    0x10,   // 03 RECALIBRATE
    0x25,   // 04 READ DMA EXT
    0x24,   // 05 READ SECTOR(S) EXT
    0x29,   // 06 READ MULTIPLE EXT
    0x20,   // 07 READ SECTOR(S)
    0x27,   // 08 READ NATIVE MAX ADDRESS EXT
    0x2F,   // 09 READ LOG EXT
    0x35,   // 10 WRITE DMA EXT
    0x34,   // 11 WRITE SECTOR(S) EXT
    0x39,   // 12 WRITE MULTIPLE EXT
    0x30,   // 13 WRITE SECTOR(S)
    0x37,   // 14 SET MAX ADDRESS EXT
    0x3F,   // 15 WRITE LOG EXT
    0x40,   // 16 READ VERIFY SECTOR(S)
    0x42,   // 17 READ VERIFY SECTOR(S) EXT
    0x70,   // 18 SEEK
    0x90,   // 19 EXECUTE DRIVE DIAGNOSTICS
    0x91,   // 20 INITIALIZE DEVICE PARAMETERS
    0x92,   // 21 DOWNLOAD MICROCODE
    0x94,	// 22 STANDBY IMMEDIATE
    0x95,	// 23 IDLE IMMEDIATE
    0x96,	// 24 STANDBY
    0x97,	// 25 IDLE
    0x98,	// 26 CHECK POWER MODE
    0x99,	// 27 SLEEP
    0xB0,   // 28 SMART
    0xB1,   // 29 DEVICE CONFIGURATION
    0xC8,   // 30 READ DMA
    0xCA,   // 31 WRITE DMA
    0xC4,   // 32 READ MULTIPLE
    0xC5,   // 33 WRITE MULTIPLE
    0xC6,   // 34 SET MULTIPLE MODE
    0xD0,   // 35 SANITARY ERASE
    0xE0,   // 36 STANDBY IMMEDIATE
    0xE1,   // 37 IDLE IMMEDIATE
    0xE2,   // 38 STANDBY
    0xE3,   // 39 IDLE
    0xE4,   // 40 READ BUFFER
    0xE5,   // 41 CHECK POWER MODE
    0xE6,   // 42 SLEEP
    0xE7,   // 43 FLUSH CACHE
    0xE8,   // 44 WRITE BUFFER
    0xEA,   // 45 FLUSH CACHE EXT
    0xEC,   // 46 IDENTIFY DEVICE
    0xEF,   // 47 SET FEATURES
    0xF1,   // 48 SECURITY SET PASSWORD
    0xF2,   // 49 SECURITY UNLOCK
    0xF3,   // 50 SECURITY ERASE PREPARE
    0xF4,   // 51 SECURITY ERASE UNIT
    0xF5,   // 52 SECURITY FREEZE LOCK
    0xF6,   // 53 SECURITY DISABLE PASSWORD
    0xF8,   // 54 READ NATIVE MAX ADDRESS
    0xF9,   // 55 SET MAX ADDRESS
    0xFC,  // 56 DELETE
    0xFD,  // 57 DELETE EXT
    0xFF,   // 58 SRST
    0xFF,   // 59 Dummy for 4 byte alignment
};

const ATA_FUNCTION_T ata_function_table[] =
{
	ata_nop,							// NOP
	(ATA_FUNCTION_T) INVALID32,			// DATA SET MANAGEMENT
	(ATA_FUNCTION_T) INVALID32,			// DEVICE RESET
	ata_recalibrate,					// RECALIBRATE
	(ATA_FUNCTION_T) INVALID32,			// READ DMA EXT
	(ATA_FUNCTION_T) INVALID32,			// READ SECTOR(S) EXT
	(ATA_FUNCTION_T) INVALID32,			// READ MULTIPLE EXT
	(ATA_FUNCTION_T) INVALID32,			// READ SECTOR(S)
	ata_read_native_max_address,		// READ NATIVE MAX ADDRESS EXT
	(ATA_FUNCTION_T) INVALID32,			// READ LOG EXT
	(ATA_FUNCTION_T) INVALID32,			// WRITE DMA EXT
	(ATA_FUNCTION_T) INVALID32,			// WRITE SECTOR(S) EXT
	(ATA_FUNCTION_T) INVALID32,			// WRITE MULTIPLE EXT
	(ATA_FUNCTION_T) INVALID32,			// WRITE SECTOR(S)
	(ATA_FUNCTION_T) INVALID32,			// SET MAX ADDRESS EXT
	(ATA_FUNCTION_T) INVALID32,			// WRITE LOG EXT
	ata_read_verify_sectors,			// READ VERIFY SECTOR(S)
	ata_read_verify_sectors,			// READ VERIFY SECTOR(S) EXT
	ata_seek, 							// SEEK
	ata_execute_drive_diagnostics,		// EXECUTE DRIVE DIAGNOSTICS
	ata_initialize_device_parameters,	// INITIALIZE DEVICE PARAMETERS
	(ATA_FUNCTION_T) INVALID32,			// DOWNLOAD MICROCODE
	ata_standby_immediate,				// STANDBY IMMEDIATE
	ata_idle_immediate,					// IDLE IMMEDIATE
	ata_standby,						// STANDBY
	ata_idle,							// IDLE
	ata_check_power_mode,				// CHECK POWER MODE
	ata_sleep,							// SLEEP
	(ATA_FUNCTION_T) INVALID32,			// SMART
	(ATA_FUNCTION_T) INVALID32,			// DEVICE CONFIGURATION
	(ATA_FUNCTION_T) INVALID32,			// READ DMA
	(ATA_FUNCTION_T) INVALID32,			// WRITE DMA
	(ATA_FUNCTION_T) INVALID32,			// READ MULTIPLE
	(ATA_FUNCTION_T) INVALID32,			// WRITE MULTIPLE
	ata_set_multiple_mode,				// SET MULTIPLE MODE
	(ATA_FUNCTION_T) INVALID32,			// SANITARY ERASE
	ata_standby_immediate,				// STANDBY IMMEDIATE
	ata_idle_immediate,					// IDLE IMMEDIATE
	ata_standby,						// STANDBY
	ata_idle,							// IDLE
	ata_read_buffer,					// READ BUFFER
	ata_check_power_mode,				// CHECK POWER MODE
	ata_sleep,							// SLEEP
	ata_flush_cache,					// FLUSH CACHE
	ata_write_buffer,					// WRITE BUFFER
	ata_flush_cache,					// FLUSH CACHE EXT
	ata_identify_device, 				// IDENTIFY DEVICE
	ata_set_features,					// SET FEATURES
	(ATA_FUNCTION_T) INVALID32,			// SECURITY SET PASSWORD
	(ATA_FUNCTION_T) INVALID32,			// SECURITY UNLOCK
	(ATA_FUNCTION_T) INVALID32,			// SECURITY ERASE PREPARE
	(ATA_FUNCTION_T) INVALID32,			// SECURITY ERASE UNIT
	(ATA_FUNCTION_T) INVALID32,			// SECURITY FREEZE LOCK
	(ATA_FUNCTION_T) INVALID32,			// SECURITY DISABLE PASSWORD
	ata_read_native_max_address,		// READ NATIVE MAX ADDRESS
	(ATA_FUNCTION_T) INVALID32,			// SET MAX ADDRESS
	(ATA_FUNCTION_T) INVALID32,
	(ATA_FUNCTION_T) INVALID32,
	ata_srst,							// SRST
};

