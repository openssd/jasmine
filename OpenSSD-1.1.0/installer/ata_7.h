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


#define NOP									0x00
#define CHECK_POWER_MODE					0xE5
#define SLEEP								0xE6
#define IDLE								0xE3
#define IDLE_IMMEDIATE						0xE1
#define STANDBY								0xE2
#define STANDBY_IMMEDIATE					0xE0
#define READ_BUFFER							0xE4
#define READ_DMA							0xC8
#define READ_DMA_EXT						0x25
#define READ_DMA_QUEUED						0xC7
#define READ_DMA_QUEUED_EXT					0x26
#define READ_LOG_EXT						0x2F
#define READ_MULTIPLE						0xC4
#define READ_MULTIPLE_EXT					0x29
#define READ_NATIVE_MAX_ADDRESS				0xF8
#define READ_NATIVE_MAX_ADDRESS_EXT			0x27
#define READ_SECTORS						0x20
#define READ_SECTORS_EXT					0x24
#define READ_STREAM_DMA_EXT					0x2A
#define READ_STREAM_EXT						0x2B
#define READ_VERIFY_SECTORS					0x40
#define READ_VERIFY_SECTORS_EXT				0x42
#define WRITE_BUFFER						0xE8
#define WRITE_DMA							0xCA
#define WRITE_DMA_EXT						0x35
#define WRITE_DMA_FUA_EXT					0x3D
#define WRITE_DMA_QUEUED					0xCC
#define WRITE_DMA_QUEUED_EXT				0x36
#define WRITE_DMA_QUEUED_FUA_EXT			0x3E
#define WRITE_LOG_EXT						0x3F
#define WRITE_MULTIPLE						0xC5
#define WRITE_MULTIPLE_EXT					0x39
#define WRITE_MULTIPLE_FUA_EXT				0xCE
#define WRITE_SECTORS						0x30
#define WRITE_SECTORS_EXT					0x34
#define WRITE_STREAM_DMA_EXT				0x3A
#define WRITE_STREAM_EXT					0x3B
/*
#define SMART_DISABLE_OPERATIONS			0xB0
#define SMART_ENABLE_DISABLE_AUTOSAVE		0xB0
#define SMART_ENABLE_OPERATIONS				0xB0
#define SMART_EXECUTE_OFF_LINE_IMMEDIATE	0xB0
#define SMART_READ_DATA						0xB0
#define SMART_READ_LOG						0xB0
#define SMART_RETURN_STATUS					0xB0
#define SMART_WRITE_LOG						0xB0
*/
#define SECURITY_DISABLE_PASSWORD			0xF6
#define SECURITY_ERASE_PREPARE				0xF3
#define SECURITY_ERASE_UNIT					0xF4
#define SECURITY_FREEZE_LOCK				0xF5
#define SECURITY_SET_PASSWORD				0xF1
#define SECURITY_UNLOCK						0xF2
#define CONFIGURE_STREAM					0x51
#define SERVICE								0xA2
#define EXECUTE_DEVICE_DIAGNOSTIC			0x90
#define IDENTIFY_DEVICE						0xEC
#define SET_FEATURES						0xEF
#define SET_MAX								0xF9
#define SET_MAX_ADDRESS_EXT					0x37
#define SET_MULTIPLE_MODE					0xC6
#define FLUSH_CACHE							0xE7
#define FLUSH_CACHE_EXT						0xEA
#define DEVICE_CONFIGURATION_FREEZE_LOCK	0xB1
#define DEVICE_CONFIGURATION_IDENTIFY		0xB1
#define DEVICE_CONFIGURATION_RESTORE		0xB1
#define DEVICE_CONFIGURATION_SET			0xB1
#define DOWNLOAD_MICROCODE					0x92

#define PACKET								0xA0
#define DEVICE_RESET						0x08
#define IDENTIFY_PACKET_DEVICE				0xA1
#define CFA_ERASE_SECTORS					0xC0
#define CFA_REQUEST_EXTENDED_ERROR			0x03
#define CFA_TRANSLATE_SECTOR				0x87
#define CFA_WRITE_MULTIPLE_WITHOUT_ERASE	0xCD
#define CFA_WRITE_SECTORS_WITHOUT_ERASE		0x38
#define MEDIA_EJECT							0xED
#define MEDIA_LOCK							0xDE
#define MEDIA_UNLOCK						0xDF
#define CHECK_MEDIA_CARD_TYPE				0xD1
#define GET_MEDIA_STATUS					0xDA


// IDENTIFY_DEVICE information format

#define ID_SERIAL_NUMBER		10
#define ID_SERIAL_NUMBER_SZ		10
#define ID_FIRMWARE_REVISION	23
#define ID_FIRMWARE_REVISION_SZ	4
#define	ID_MODEL_NUMBER			27
#define ID_MODEL_NUMBER_SZ		20
#define ID_USER_SECTORS_28		60	// total number of sectors for 28-bit mode addressing
#define ID_MAJOR_VERSION		80
#define ID_USER_SECTORS_48		100	// total number of sectors for 48-bit mode addressing
