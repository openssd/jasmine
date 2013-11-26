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


#ifndef SATA_H
#define SATA_H

#define BSO_RX_SSC		FALSE
#define BSO_TX_SSC		FALSE

#define CHS_MAX_ADDR				16514064

#if NUM_LSECTORS - 1 <= CHS_MAX_ADDR
#define CHS_HEADS	((NUM_LSECTORS - 1) / (0x3FFF * 0x3F))
#else
#define CHS_HEADS	16
#endif

#define MAX_LBA		(NUM_LSECTORS - 1)

#define NCQ_SIZE	32

typedef struct
{
	UINT32	queue[NCQ_SIZE][8];
	UINT32	gtag;
} sata_ncq_t;

extern sata_ncq_t g_sata_ncq;

#define CCL_FTL_D2H		0x01
#define CCL_FTL_H2D		0x02
#define CCL_OTHER		0x04
#define	CCL_UNDEFINED	0x08

#define	ATR_LBA_NOR		0x10
#define	ATR_LBA_EXT		0x20
#define	ATR_NO_SECT		0x40
#define ATR_LOCK_FREE	0x80

#define READ      0
#define WRITE     1

typedef struct
{
	UINT32	lba;
	UINT32	sector_count;
	UINT32	cmd_type;
} CMD_T;

// slow_cmd_t status
#define SLOW_CMD_STATUS_NONE		0
#define SLOW_CMD_STATUS_PENDING		1
#define SLOW_CMD_STATUS_BUSY		2

typedef struct
{
	UINT16	status;
	UINT16	code;
	UINT32	lba;
	UINT32	sector_count;
} slow_cmd_t;

typedef struct
{
	slow_cmd_t slow_cmd;
	volatile BOOL32 eq_full;

	UINT16	chs_cur_cylinders;
	UINT8	chs_cur_heads;
	UINT8	chs_cur_sectors;

	BOOL8	lba_48_mode;
	BOOL8	dma_setup_auto_activate;
	BOOL8	srst;
	BOOL8	write_cache_enabled;

	BOOL8	read_look_ahead_enabled;
	UINT8	unused1;
	UINT8	unused2;
	UINT8	unused3;
} sata_context_t;

extern sata_context_t	g_sata_context;

#define	B_ERR	BIT0
#define	B_DRQ	BIT3
#define	B_DF	BIT5
#define	B_DRDY	BIT6
#define	B_BSY	BIT7
#define B_IRQ	BIT6
#define B_AA	BIT7
#define	B_ABRT	BIT2
#define	B_IDNF	BIT4
#define	B_UNC	BIT6
#define B_ICRC	BIT7
#define	B_HOB	BIT7
#define	B_SRST	BIT2

typedef enum
{
	FISTYPE_REGISTER_H2D	= 0x27,
	FISTYPE_REGISTER_D2H	= 0x34,
	FISTYPE_DMA_ACTIVATE	= 0x39,
	FISTYPE_DMA_SETUP		= 0x41,
	FISTYPE_DATA			= 0x46,
	FISTYPE_BIST_ACTIVATE	= 0x58,
	FISTYPE_PIO_SETUP		= 0x5F,
	FISTYPE_SET_DEVICE_BITS	= 0xA1
} T_FISTYPES;

typedef enum
{
	PIO_H2D	= 0x00,
	PIO_D2H = 0x01,
} T_PROTOCOLS;

enum
{
	ATA_NOP							= 0x00,	/* Nop                       */
	ATA_DATA_SET_MANAGEMENT			= 0x06,	/* Data Set Management       */
	ATA_DEVICE_RESET				= 0x08,	/* Device Reset		 		 */
	ATA_RECALIBRATE					= 0x10,	/* Recalibrate - for booting */
	ATA_READ_SECTORS				= 0x20,	/* Read Sectors		 		 */
	ATA_READ_SECTORS_EXT			= 0x24,	/* Read Sectors Ext		 	 */
	ATA_READ_DMA_EXT				= 0x25,	/* Read DMA Ext 			 */
	ATA_READ_NATIVE_MAX_ADDRESS_EXT	= 0x27,	/* Read Native Max Address Ext */
	ATA_READ_MULTIPLE_EXT			= 0x29,	/* Read Multiple Ext		 */
	ATA_READ_LOG_EXT				= 0x2F,	/* Read Log Ext			 	 */
	ATA_WRITE_SECTORS				= 0x30,	/* Write Sectors	 		 */
	ATA_WRITE_SECTORS_EXT			= 0x34,	/* Write Sectors Ext		 */
	ATA_WRITE_DMA_EXT				= 0x35,	/* Write DMA Ext 			 */
	ATA_SET_MAX_ADDRESS_EXT			= 0x37,	/* Set Max Address Ext 		 */
	ATA_WRITE_MULTIPLE_EXT			= 0x39,	/* Write Multiple Ext		 */
	ATA_WRITE_LOG_EXT				= 0x3F,	/* Write Log Ext			 */
	ATA_READ_VERIFY_SECTORS			= 0x40, /* Read Verify Sectors		 */
	ATA_READ_VERIFY_SECTORS_EXT		= 0x42, /* Read Verify Sectors Ext	 */
	ATA_READ_FPDMA_QUEUED			= 0x60,	/* Read FPDMA Queued		 */
	ATA_WRITE_FPDMA_QUEUED			= 0x61,	/* Write FPDMA Queued		 */
	ATA_EXEDIAG						= 0x90,	/* Execute Drive Diagnostics */
	ATA_INITIALIZE_DEV_PARA			= 0x91,	/* Initialize Device Parameters */
	ATA_DOWNLOAD_MICROCODE			= 0x92,	/* Download Microcode		 */
	ATA_SMART						= 0xB0,	/* Smart					 */
	ATA_DEVICE_CONFIGURATION		= 0xB1,	/* Device Configuration		 */
	ATA_READ_MULTIPLE				= 0xC4,	/* Read Multiple			 */
	ATA_WRITE_MULTIPLE				= 0xC5,	/* Write Multiple			 */
	ATA_SET_MULTIPLE_MODE			= 0xC6,	/* Set Multiple Mode		 */
	ATA_READ_DMA					= 0xC8,	/* Read DMA 				 */
	ATA_WRITE_DMA					= 0xCA,	/* Write DMA 				 */
	ATA_STANDBY_IMMEDIATE			= 0xE0,	/* Standby Immediate		 */
	ATA_IDLE_IMMEDIATE				= 0xE1, /* Idle Immediate 			 */
	ATA_STANDBY						= 0xE2,	/* Standby					 */
	ATA_IDLE						= 0xE3,	/* Idle			 			 */
	ATA_READ_BUFFER					= 0xE4,	/* Read Buffer 				 */
	ATA_CHECK_POWER_MODE			= 0xE5,	/* Check Power Mode			 */
	ATA_SLEEP						= 0xE6, /* Sleep			 		 */
	ATA_FLUSH_CACHE					= 0xE7,	/* Flush Cache 				 */
	ATA_WRITE_BUFFER				= 0xE8,	/* Write Buffer 			 */
	ATA_FLUSH_CACHE_EXT				= 0xEA,	/* Flush Cache Ext			 */
	ATA_IDENT						= 0xEC,	/* Identify Device           */
	ATA_SET_FEATURE					= 0xEF,	/* Set Feature               */
	ATA_SECURITY_SET_PASSWORD		= 0xF1,	/* Security Set Password     */
	ATA_SECURITY_UNLOCK 			= 0xF2,	/* Security Unlock           */
	ATA_SECURITY_ERASE_PREPARE		= 0xF3, /* Security Erase Prepare    */
	ATA_SECURITY_ERASE_UNIT			= 0xF4, /* Security Erase Unlock     */
	ATA_SECURITY_FREEZE_LOCK		= 0xF5, /* Security Freeze Lock		 */
	ATA_SECURITY_DISABLE_PASSWORD	= 0xF6, /* Security Disable Password */
	ATA_READ_NATIVE_MAX_ADDRESS		= 0xF8,	/* Read Native Max Address   */
	ATA_SET_MAX_ADDRESS				= 0xF9,	/* Set Max Address   		 */
	ATA_SRST						= 0xFF	/* SRST request is regarded as if it were an ATA command. */
};


#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80
#define BIT8 0x0100
#define BIT9 0x0200
#define BIT10 0x0400
#define BIT11 0x0800
#define BIT12 0x1000
#define BIT13 0x2000
#define BIT14 0x4000
#define BIT15 0x8000
#define BIT16 0x00010000
#define BIT17 0x00020000
#define BIT18 0x00040000
#define BIT19 0x00080000
#define BIT20 0x00100000
#define BIT21 0x00200000
#define BIT22 0x00400000
#define BIT23 0x00800000
#define BIT24 0x01000000
#define BIT25 0x02000000
#define BIT26 0x04000000
#define BIT27 0x08000000
#define BIT28 0x10000000
#define BIT29 0x20000000
#define BIT30 0x40000000
#define BIT31 0x80000000

#define SETREGBOR(ADDR, VAL)	*(volatile UINT32*)(ADDR) |= (UINT32)(VAL)
#define SETREGBND(ADDR, VAL)	*(volatile UINT32*)(ADDR) &= (UINT32)(VAL)

#include "sata_registers.h"
#include "sata_cmd.h"

void send_status_to_host(UINT32 const err_code);
void sata_reset(void);
void pio_sector_transfer(UINT32 const dram_addr, UINT32 const protocol);

extern volatile UINT32 g_sata_action_flags;


#endif	// #ifndef SATA_H
