/*
   Copyright 2011 INDILINX Co., Ltd.

   This file is part of Jasmine.

   Jasmine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Jasmine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Jasmine. See the file COPYING.
   If not, see <http://www.gnu.org/licenses/>.
*/

.section INIT, "x"

		B reset_handler	@ reset
		B .				@ undefined instruction
		B .			  	@ SWI
		B .				@ prefetch abort
		B .				@ data abort
		NOP				@ reserved vector
		B irq_handler  	@ IRQ
		B fiq_handler	@ FIQ

reset_handler:

		/* IRQ mode stack */

		MSR			CPSR_c,	#(0x12 | 0xC0)
		LDR			R13, =93696

		/* FIQ mode stack */

		MSR			CPSR_c,	#(0x11 | 0xC0)
		LDR			R13, =94720

		/* SYSTEM mode stack
		SYSTEM mode is the main mode. */

		MSR			CPSR_c,	#(0x1F | 0xC0)
		LDR			R13, =95744

	    BL			init_jasmine
    	BL			Main
		B			.		@ should not reach here

disable_irq:

		MRS			R0, CPSR
		TST			R0, #0x80
		ORREQ		R0,R0,#0x80
		MSREQ		CPSR_c,R0
		MOVNE		R0,#1
		MOVEQ		R0,#0
		BX			LR

enable_irq:

		MRS			R0, CPSR
		BIC			R0, R0, #0x80
		MSR			CPSR_c, R0
		BX			LR

disable_fiq:

		MRS			R0, CPSR
		TST			R0, #0x40
		ORREQ		R0,R0,#0x40
		MSREQ		CPSR_c,R0
		MOVNE		R0,#1
		MOVEQ		R0,#0
		BX			LR

enable_fiq:

		MRS			R0, CPSR
		BIC			R0, R0, #0x40
		MSR			CPSR_c, R0
		BX			LR

disable_interrupt:

		MRS			R0, CPSR
		ORR			R0, R0, #0xC0
		MSR			CPSR_c, R0
		BX			LR

enable_interrupt:

		MRS			R0, CPSR
		BIC			R0, R0, #0xC0
		MSR			CPSR_c, R0
		BX			LR

		.global disable_irq, enable_irq, disable_fiq, enable_fiq, disable_interrupt, enable_interrupt

