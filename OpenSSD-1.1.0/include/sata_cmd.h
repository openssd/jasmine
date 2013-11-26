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


#ifndef SATA_CMD_H
#define SATA_CMD_H

#define	ATA_CMD_NUM			59
#define	CMD_TABLE_SIZE		60


typedef void (* ATA_FUNCTION_T)(UINT32, UINT32);


enum tag_FEATURES_subcommands
{
	FEATURE_ENABLE_WRITE_CACHE							= 0x02,
	FEATURE_SET_TRANSFER_MODE							= 0x03,
	FEATURE_ENABLE_ADVANCED_POWER_MANAGEMENT			= 0x05,
	FEATURE_ENABLE_POWERUP_IN_STANDBY					= 0x06,
	FEATURE_POWRUP_IN_STANDBY_FEATURE_SET_DEVICE_SPINUP	= 0x07,
	FEATURE_ENABLE_USE_OF_SATA							= 0x10,
	FEATURE_DISABLE_READ_LOOK_AHEAD						= 0x55,
	FEATURE_DISABLE_REVERTING_TO_POWER_ON_DEFAULTS		= 0x66,
	FEATURE_DISABLE_WRITE_CACHE							= 0x82,
	FEATURE_DISABLE_ADVANCED_POWER_MANAGEMENT			= 0x85,
	FEATURE_DISABLE_POWERUP_IN_STANDBY					= 0x86,
	FEATURE_DISABLE_USE_OF_SATA							= 0x90,
	FEATURE_ENABLE_READ_LOOK_AHEAD						= 0xAA,
	FEATURE_ENABLE_REVERTING_TO_POWER_ON_DEFAULTS		= 0xCC
};

#define MAXNUM_DRQ_SECTORS		0x01	/* using const UINT8 ht_identify_data[IDENTIFY_VALLEN] */

extern const UINT8 ata_cmd_class_table[];
extern const UINT8 ata_index_table[];
extern const UINT8 ata_command_code_table[];
extern const ATA_FUNCTION_T ata_function_table[];


void ata_identify_device(UINT32 lba, UINT32 sector_count);
void ata_set_features(UINT32 lba, UINT32 sector_count);
void ata_execute_drive_diagnostics(UINT32 lba, UINT32 sector_count);
void ata_check_power_mode(UINT32 lba, UINT32 sector_count);
void ata_flush_cache(UINT32 lba, UINT32 sector_count);
void ata_read_verify_sectors(UINT32 lba, UINT32 sector_count);
void ata_set_multiple_mode(UINT32 lba, UINT32 sector_count);
void ata_read_buffer(UINT32 lba, UINT32 sector_count);
void ata_write_buffer(UINT32 lba, UINT32 sector_count);
void ata_seek(UINT32 lba, UINT32 sector_count);
void ata_standby(UINT32 lba, UINT32 sector_count);
void ata_recalibrate(UINT32 lba, UINT32 sector_count);
void ata_standby_immediate(UINT32 lba, UINT32 sector_count);
void ata_idle(UINT32 lba, UINT32 sector_count);
void ata_idle_immediate(UINT32 lba, UINT32 sector_count);
void ata_sleep(UINT32 lba, UINT32 sector_count);
void ata_read_native_max_address(UINT32 lba, UINT32 sector_count);
void ata_read_native_max_address(UINT32 lba, UINT32 sector_count);
void ata_nop(UINT32 lba, UINT32 sector_count);
void ata_initialize_device_parameters(UINT32 lba, UINT32 sector_count);
void ata_not_supported(UINT32 lba, UINT32 sector_count);
void ata_srst(UINT32 lba, UINT32 sector_count);


#endif	// SATA_CMD_H

