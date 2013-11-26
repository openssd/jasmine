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

#define SERIAL		0,0,0,0,0,0,0,0,0,0
#define FW_REV		0,0,0,0
#define MODEL		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

#ifdef __GNUC__
UINT16 ht_identify_data[256] __attribute__((aligned(4))) =
#else
__align(4) UINT16 ht_identify_data[256] =
#endif

{						// Word	Contents (MSB first)
	0x0C5A,				//	0:	General configuration bit-significant information:
						//		15	:ATA device
						//		14-8:Retired
						//		7	:removable media device
						//		6	:not removable controller and/or device
						//		5-3	:reserved
						//		2	:Response incomplete
						//		1	:Retired
						//		0	:Reserved
	0x3FFF,				//	1:	Number of logical cylinders (ATA5)
	0xC837,				//	2:	Specific configuration
	CHS_HEADS,			//	3:	Number of logical heads (ATA5)
	0x0000,0x0000,		//	4-5: Retired
	0x003F,				//	6:	Number of logical sectors per logical track (ATA5)
	0x0000,0x0000,		//	7-8: Reserved
	0x0000,				//	9:	Retired
	SERIAL, 			//	10-19: Serial number (20 ASCII characters)
	0x0000,				//	20: Retired
	0xFFFF,				//	21: Retired (32MB Cache)
	0x3000,				//	22:	Obsolete
	FW_REV,				//	23-26:	Firmware revision (8 ASCII characters)
	MODEL, 				//	27-46:	Model number (40 ASCII characters)
	0x8001,				//	47:	Maximum number of sectors on READ/WRITE MULTIPLE
	0x0000,				//	48: Reserved
	0x2F00,				//	49:	Capabilities
						//		15-14:Reseved for the IDENTIFY PACKET DEVICE
						//		13	:Standby timer values are supported
						//		12	:Reseved for the IDENTIFY PACKET DEVICE
						//		11	:IORDY supported
						//		10	:IORDY may be disabled
						// 		9	:LBA supported
						//		8	:DMA supported
						//		7-0	:Retired
	0x4000,				//	50:	Capabilities
						//		15	:Shall be cleared to zero
						//		14	:Shall be set to one
						//		0	:Standby timer value minimum
	0x0200,0x0200,		//	51-52:	Obsolete
	0x0007,				//	53:	Field validity
						//		15-3:Reserved
						//		2	:the fields reported in word 88 are valid
						//		1	:the fields reported in words 64-70 are valid
						//		0	:the fields reported in words 54-58 are valid
	0x3FFF,0x0010,		//	54-58: Obsolete
	0x003F,0xFC10,
	0x00FB,
	0x0101,				//	59:	Multiple Sector Setting
	0xFFFF,0x0FFF,		//	60-61:	Total number of user addressable sectors
	0x0000,				//	62:	Obsolete (ATA-2: Single word DMA transfer mode)
	0x0407,				//	63:	Multi word DMA transfer mode
						//		15-11:Reserved
						//		10-8:Multiword DMA mode selected
						//		7-3:Reserved
						//		2-0	:Multiword DMA modes supported
	0x0003,				//	64:	PIO modes supported
						//		15-2:Reserved
						//		1-0	:PIO modes supported
	120,				//	65:	Minimum Multiword DMA transfer cycle time per word
						//		(ns)
	120,				//	66:	Manufacturer's recommended Multiword DMA transfer
						//		cycle time(ns)
	120,				//	67:	Minimum PIO transfer cycle time without flow control
						//		(ns)
	120,				//	68:	Minimum PIO transfer cycle time with IORDY flow
						//		control(ns)
	0x0000,0x0000,  	//	69-70:	Reserved (for future command overlap and queuing)
	0x0000,0x0000,  	//	71-74:	Reserved for IDENTIFY PACKET DEVICE command
	0x0000,0x0000,
	(NCQ_SIZE - 1),		//	75:	Queue depth (= 32)
						//		15-5:Reserved
						//		4-0	:Maximum queue depth supported
	0x0006,				//	76:	Serial ATA capabilities
						//		15-11:Reserved
						//		10	 :Supports Phy event counters
						//		9	 :Supports receipt of host-initiated interface power management requests
						//		8	 :Supports Native Command Queuing
						//		7-3	 :Reserved for future Serial ATA signaling speed grades
						//		2	 :Supports Serial ATA Gen2 signaling speed (3.0Gbps)
						//		1	 :Supports Serial ATA Gen1 signaling speed (1.5Gbps)
						//		0	 :Shall be cleared to zero
	0x0000,  			//  77: Reserved
	0x0014,  			//  78: Serial ATA features supported
						//  	15-7: Reserved
						//  	6   : Supports software settings preservation
						//  	5   : Reserved
						//  	4   : In-order data delivery
						//  	3   : Device initiating interface power management
						//  	2   : DMA Setup Auto-Activate optimization
						//  	1   : Non-zero buffer offsets in DMA SetupFIS
						//  	0   : Shall be cleared to zero
	0x0010,  			//  79: Serial ATA features enabled
						//  	15-7: Reserved
						//  	6   : Software settings preservation enabled
						//  	5   : Reserved
						//  	4   : In-order data delivery enabled
						//  	3   : Device initiating interface power management enabled
						//  	2   : DMA Setup Auto-Activate optimization enabled
						//  	1   : Non-zero buffer offsets in DMA SetupFIS enabled
						//  	0   : Shall be cleared to zero
	0x01E0,				//	80:	Major version number
						//		15-9:Reserved
						//		8	:supports ATA8-ACS
						//		7	:supports ATA/ATAPI-7
						//		6	:supports ATA/ATAPI-6
						//		5	:supports ATA/ATAPI-5	(LBA supported)
						//		4	:supports ATA/ATAPI-4
						//		3	:supports ATA-3
	0x0000,				//	81:	Minor version number
						//			0000h or FFFFh=device does not report version
	0x3068,				//	82:	Command set supported. If words 82 and 83 =0000h or
						//		FFFFh command set notification not supported.
						//		15	:Obsolete
						//		14	:NOP command supported
						//		13	:READ BUFFER command supported
						//		12	:WRITE BUFFER command supported
						//		11	:Obsolete
						//		10	:Host Protected Area feature set supported
						//		9	:DEVICE RESET command supported
						//		8	:SERVICE interrupt supported
						//		7	:release interrupt supported
						//		6	:look-ahead supported
						//		5	:write cache supported
						//		4	:Shall be cleared to zero
						//		3	:mandatory Power Management feature set supported
						//		2	:supports Removable Media feature set
						//		1	:supports Security Mode feature set
						//		0	:supports SMART feature set
	0x7400,				//	83:	Command sets supported. If words 82 and 83 =0000h or
						//		FFFFh command set notification not supported.
						//		15	:Shall be cleared to zero
						//		14	:Shall be set to one
						//		13	:FLUSH CACHE EXT command supported
						//		12	:mandatory FLUSH CACHE command supported
  						//		11	:Device Configuration Overly feature set
  						//		10	:48bit Address feature set is supported
  						//		9	:Automatic Acoustic Management feature set
						//		8	:SET MAX security extension supported
						//		7	:Reserved
						//		6	:SET FEATURES subcommand required to spinup after power-up
						//		5	:Power-Up In Stanby seature set supported
						//		4	:Removable Media Status Notification feature set supported
						//		3	:Advanced Power Management feature set supported
						//		2	:CFA feature set supported
						//		1	:READ/WRITE DMA QUEUED supported
						//		0	:DOWNLOAD MICROCODE command supported
	0x4000,				//	84:	Command set/feature supported extension. If words
						//		82, 83, and 84 = 0000h or FFFFh command set notification extension is not supported.
						//		15	:Shall be cleared to zero
						//		14	:Shall be set to one
						//		13	:IDLE IMMEDIATE with UNLOAD REATURE supported
						//		12-7:Reserved
						//		6	:WRITE DMA FUA EXT and WRITE MULTIPLE FUA EXT commands supported
						//		5	:General Purpose Logging feature set supported
						//		1	:SMART self-test supported
						//		0	:SMART error logging supported
	0x3060,				//	85:	Command set/feature enabled. If words 85, 86, and 87
						//		= 0000h or FFFFh command set enabled notification is not supported.
						//		15	:Obsolete
						//		14	:NOP command supported
						//		13	:READ BUFFER command supported
						//		12	:WRITE BUFFER command supported
						//		11	:Obsolete
						//		10	:Host Protected Area feature set supported
						//		9	:DEVICE RESET command supported
						//		8	:SERVICE interrupt enabled
						//		7	:release interrupt enabled
						//		6	:look-ahead enabled
						//		5	:write cache enabled
						//		4	:supports PACKET Command feature set
						//		3	:Power Management feature set enabled
						//		2	:supports Removable Media feature set
						//		1	:Security Mode feature set enabled
						//		0	:SMART feature set enabled
	0x3400,				//	86:	Command set/feature enabled. If words 85, 86, and 87
						//		= 0000h or FFFFh command set enabled notification is not supported.
						//		15-14:Reserved
						//		13	:FLUSH CACHE EXT command supported
						//		12	:FLUSH CACHE command supported
						//		11	:Device Configuration Overlay supported
						// 		10	:48bit Address features set supported
						//		9	:Automatic Acoustic Management feature set enabled
						//		8	:SET MAX security extension enabled by SET MAX SET PASSWORD
						//		7	:Reserved
						//		6	:SET FEATURES subcommand required to spin-up after power-up
						//		5	:Power-Up In Standby feature set enabled
						//		4	:Removable Media Status Notification feature set enabled via the SET FEATURES command
						//		3	:Advanced Power Management feature set enabled
						//		2	:CFA feature set enabled
						//		1	:READ/WRITE DMA QUEUED command enabled
						//		0	:DOWNLOAD MICROCODE command enabled
	0x4000,				//	87:	Command set/feature default. If words 85, 86, and 87
						//		= 0000h or FFFFh command set default notification is
						//		not supported.
						//		15	:Shall be cleared to zero
						//		14	:Shall be set to one
						//		13	:IDLE IMMEDIATE with UNLOAD REATURE enabled
						//		12-7:Reserved
						//		6	:WRITE DMA FUA EXT and WRITE MULTIPLE FUA EXT commands enabled
						//		5	:General Purpose Logging feature set enabled
						//		1	:SMART self-test enabled
						//		0	:SMART error logging enabled
	0x007F,				//	88:	Ultra DMA modes
						//			15	:Reserved
						//			14	:Ultra DMA mode 6 is selected
						//			13	:Ultra DMA mode 5 is selected
						//			12	:Ultra DMA mode 4 is selected
						//			11	:Ultra DMA mode 3 is selected
						//			10	:Ultra DMA mode 2 is selected
						//			9	:Ultra DMA mode 1 is selected
						//			8	:Ultra DMA mode 0 is selected
						//			7	:Reserved
						//			6	:Ultra DMA mode 6 and below are supported
						//			5	:Ultra DMA mode 5 and below are supported
						//			4	:Ultra DMA mode 4 and below are supported
						//			3	:Ultra DMA mode 3 and below are supported
						//			2	:Ultra DMA mode 2 and below are supported
						//			1	:Ultra DMA mode 1 and below are supported
						//			0	:Ultra DMA mode 0 is supported
	0x0000				//	89:	Time required for security erase unit completion
						//	90:	Time required for Enhanced security erase completion
						//	91:	Current advanced power management value
						//  92: Master Password Revision Code - not supported
						//	93-126:	Reserved
						//	127:Media Status Notification feature set support
						//			15-2:Reserved
						//			1-0	:00=Removable Media Status Notification feature set not supported
						//				:01=Removable Media Status Notification feature set supported
						//				:10=Reserved, 11=Reserved
						//	128:Security status
						//			15-9:Reserved
						//			8	:Security level 0=High, 1=Maximum
						//			7-6	:Reserved
						//			5	:Enhanced security erase supported
						//			4	:Security count expired
						//			3	:Security frozen
						//			2	:Security locked
						//			1	:Security enabled
						//			0	:Security supported
						//	129-159:Vendor specific
};						//	160-255:Reserved

static UINT16 get_integrity_word(void)
{
	UINT8	checksum = 0;
	UINT16	i;
	UINT16	retval;

	for (i = 0; i < BYTES_PER_SECTOR - 2; i++)
	{
		checksum += g_temp_mem[i];
	}

	checksum += 0xA5;

	retval = ((UINT16)(~checksum + 1)) << 8;
	retval |= 0xA5;
	return	retval;
}

static void set_string_data(UINT16 *id_buffer, char* src_data, UINT32 whole_word_size)
{
	BOOL32 fill_space = FALSE;
	UINT32 i=0;

	for(i=0; i<whole_word_size; i++)
	{
		if(FALSE == fill_space)
		{
			if(NULL == src_data[i*2])
			{
				fill_space = TRUE;
				id_buffer[i] = 0x2020;
			}
			else if(NULL == src_data[i*2+1])
			{
				fill_space = TRUE;
				id_buffer[i] = 0x20 | (src_data[i*2]<<8);
			}
			else
			{
				id_buffer[i] = src_data[i*2+1] | (src_data[i*2]<<8);
			}
		}
		else
		{
			id_buffer[i]= 0x2020;
		}
	}
}

void ata_identify_device(UINT32 lba, UINT32 sector_count)
{
	UINT16* const addr = ht_identify_data;
	UINT32 capacity;

	set_string_data(&addr[10], "0123456789", 10);
	set_string_data(&addr[23], "0001", 4);
	set_string_data(&addr[27], "OpenSSD Jasmine     ", 20);

	addr[54] = g_sata_context.chs_cur_cylinders;
	addr[55] = g_sata_context.chs_cur_heads;
	addr[56] = g_sata_context.chs_cur_sectors;

	capacity = g_sata_context.chs_cur_cylinders * g_sata_context.chs_cur_heads * g_sata_context.chs_cur_sectors;
	addr[57] = capacity & 0xFFFF;
	addr[58] = capacity >> 16;

	#if DRAM_SIZE == 16268800
	addr[21] = 0x8000;	// 16MB Cache
	#endif

	#if NUM_LSECTORS >= 0x0FFFFFFF
	{
		addr[60] = 0xFFFF;
		addr[61] = 0x0FFF;
	}
	#else
	{
		addr[60] = (UINT16) (NUM_LSECTORS & 0xFFFF);
		addr[61] = (UINT16) (NUM_LSECTORS >> 16);
	}
	#endif

	#if OPTION_SLOW_SATA
	addr[76] &= (UINT16)(~BIT2);
	#endif

	#if OPTION_SUPPORT_NCQ
	addr[76] |= (UINT16) BIT8;
	#endif

	if(g_sata_context.dma_setup_auto_activate)
	{
		addr[79] |= (UINT16)(BIT2);
	}

	addr[100] = (UINT16) (NUM_LSECTORS & 0xFFFF);
	addr[101] = (UINT16) (NUM_LSECTORS >> 16);
	addr[106] = 0x4000;
	addr[217] = 0x0001;

	addr[255] = get_integrity_word();

	mem_copy(HIL_BUF_ADDR, addr, BYTES_PER_SECTOR);

	pio_sector_transfer(HIL_BUF_ADDR, PIO_D2H);
}

