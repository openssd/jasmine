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


#ifndef SATA_REGISTERS_H
#define SATA_REGISTERS_H

#define SATA_CTRL_1					(BS_BASE+0x0000)
#define SATA_INT_ENABLE				(BS_BASE+0x0008)
#define SATA_INT_STAT				(BS_BASE+0x000C)
#define SATA_CTRL_2					(BS_BASE+0x0010)
#define SATA_FIS_D2H_0				(BS_BASE+0x0014)
#define SATA_FIS_D2H_1				(BS_BASE+0x0018)
#define SATA_FIS_D2H_2				(BS_BASE+0x001C)
#define SATA_FIS_D2H_3				(BS_BASE+0x0020)
#define SATA_FIS_D2H_4				(BS_BASE+0x0024)
#define SATA_FIS_D2H_5				(BS_BASE+0x0028)
#define SATA_FIS_D2H_6				(BS_BASE+0x002C)
#define SATA_FIS_D2H_7				(BS_BASE+0x0030)
#define SATA_FIS_D2H_LEN			(BS_BASE+0x0034)
#define SATA_FIS_H2D_0				(BS_BASE+0x0038)
#define SATA_FIS_H2D_1				(BS_BASE+0x003C)
#define SATA_FIS_H2D_2				(BS_BASE+0x0040)
#define SATA_FIS_H2D_3				(BS_BASE+0x0044)
#define SATA_FIS_H2D_4				(BS_BASE+0x0048)
#define SATA_FIS_H2D_5				(BS_BASE+0x004C)
#define SATA_FIS_H2D_6				(BS_BASE+0x0050)
#define SATA_FIS_H2D_7				(BS_BASE+0x0054)
#define SATA_XFER_BYTES				(BS_BASE+0x005C)
#define SATA_XFER_BYTES_REMAIN		(BS_BASE+0x0060)
#define SATA_XFER_SECTORS_REMAIN	(BS_BASE+0x0064)
#define SATA_SACTIVE				(BS_BASE+0x0070)
#define SATA_NCQ_BMP_1				(BS_BASE+0x0074)
#define SATA_NCQ_BMP_2				(BS_BASE+0x0078)
#define SATA_STATUS					(BS_BASE+0x00A0)	// 0=idle, 1/2=read, 3/4=write
#define SATA_SECT_OFFSET			(BS_BASE+0x00A4)
#define SATA_BUF_PAGE_SIZE			(BS_BASE+0x00B4)	// size of each buffer
#define SATA_RESET_FIFO_1			(BS_BASE+0x00D4)	// reset command layer FIFO
#define SATA_MANUAL_MODE_ADDR		(BS_BASE+0x00D8)
#define SATA_CTRL_3					(BS_BASE+0x00DC)	// Buffer Manager Mode or Manual Mode
#define SATA_FIFO_1_STATUS			(BS_BASE+0x00E4)	// command layer FIFO status
#define SATA_FIFO_2_STATUS			(BS_BASE+0x00E8)	// transport layer FIFO status
#define SATA_FIFO_S_STATUS			(BS_BASE+0x00EC)	// slave FIFO status
#define SATA_PHY_STATUS				(BS_BASE+0x0100)
#define SATA_ERROR					(BS_BASE+0x0104)
#define SATA_PHY_CTRL				(BS_BASE+0x0108)
#define SATA_CFG_1					(BS_BASE+0x0140)
#define SATA_CFG_2					(BS_BASE+0x0148)
#define SATA_CFG_3					(BS_BASE+0x014C)
#define SATA_CFG_4					(BS_BASE+0x0150)
#define SATA_CFG_5					(BS_BASE+0x015C)
#define SATA_CFG_6					(BS_BASE+0x0160)
#define SATA_CFG_7					(BS_BASE+0x0164)
#define SATA_WBUF_BASE				(BS_BASE+0x0170)	// should be DRAM
#define SATA_RBUF_BASE				(BS_BASE+0x0174)	// should be DRAM
#define SATA_WBUF_SIZE				(BS_BASE+0x0178)	// number of write buffers
#define SATA_RBUF_SIZE				(BS_BASE+0x017C)	// number of read buffers
#define SATA_WBUF_MARGIN			(BS_BASE+0x0180)
#define SATA_RESET_WBUF_PTR			(BS_BASE+0x0184)
#define SATA_RESET_RBUF_PTR			(BS_BASE+0x0188)
#define SATA_NCQ_BASE				(BS_BASE+0x0190)	// should be SRAM
#define SATA_WBUF_PTR				(BS_BASE+0x0194)
#define SATA_RBUF_PTR				(BS_BASE+0x0198)
#define SATA_WBUF_FREE				(BS_BASE+0x019C)	// number of free write buffers
#define SATA_RBUF_PENDING			(BS_BASE+0x01A0)	// number of pending read buffers
#define SATA_CFG_8					(BS_BASE+0x01A8)
#define SATA_MAX_LBA				(BS_BASE+0x01B0)
#define SATA_CFG_9					(BS_BASE+0x01C8)
#define SATA_INSERT_EQ_W			(BS_BASE+0x01D0)
#define SATA_LBA					(BS_BASE+0x01D4)
#define SATA_SECT_CNT				(BS_BASE+0x01D8)
#define SATA_INSERT_EQ_R			(BS_BASE+0x01DC)
#define SATA_NCQ_ORDER				(BS_BASE+0x0220)
#define SATA_NCQ_CTRL				(BS_BASE+0x0224)
#define SATA_CFG_10					(BS_BASE+0x022C)	// bm sector read enable
#define SATA_EQ_CFG_1				(BS_BASE+0x0230)
#define SATA_EQ_CTRL				(BS_BASE+0x0234)
#define SATA_EQ_PTR					(BS_BASE+0x0238)
#define SATA_EQ_STATUS				(BS_BASE+0x023C)
#define SATA_EQ_DATA_0				(BS_BASE+0x0240)
#define SATA_EQ_DATA_1				(BS_BASE+0x0244)
#define SATA_EQ_DATA_2				(BS_BASE+0x0248)
#define SATA_EQ_CFG_2				(BS_BASE+0x026C)

#define	AUTOINC						(BIT3)
#define FLUSH_NCQ					(BIT2)

#define SEND_NON_DATA_FIS	(1 << 0)
#define SEND_DATA_FIS		(1 << 1)
#define GET_DATA_FIS		(1 << 2)
#define SEND_GOOD_RESP		(1 << 3)
#define SEND_SACTIVE		(1 << 4)
#define COMPLETE			(1 << 5)		// register FIS D2H is sent automatically upon successful protocol completion
#define PIO_READ			(1 << 8)
#define CLR_STAT_PIO_SETUP	(1 << 9)
#define DMA_READ			(1 << 10)
#define FPDMA_READ			(1 << 11)
#define PIO_WRITE			(1 << 12)
#define DMA_WRITE			(1 << 14)
#define FPDMA_WRITE			(1 << 15)
#define SEND_R_OK			(1 << 24)
#define SEND_R_ERR			(1 << 25)

#define	CMD_RECV		0x00000001		// non-NCQ command received
#define	REG_FIS_RECV	0x00000002		// register FIS H2D without C flag or unknown type of FIS received
#define	OPERATION_OK	0x00000008		// SATA operation completed successfully
#define	OPERATION_ERR	0x00400040		// SATA operation terminated due to an error
#define	PHY_ONLINE		0x00000080		// Successful OOB
#define	PHY_OFFLINE		0x00000100		// Connection lost
#define	NCQ_CMD_RECV	0x00000200		// NCQ command received
#define	NCQ_CMD_ERR		0x00000400		// wrong NCQ command received
#define NCQ_INVALID_LBA	0x00200000		// NCQ command with an invalid LBA received (included by NCQ_CMD_ERR)

#endif	// SATA_REGISTERS_H
