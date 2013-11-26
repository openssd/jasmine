; Copyright 2011 INDILINX Co., Ltd.
;
; This file is part of Jasmine.
;
; Jasmine is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; Jasmine is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with Jasmine. See the file COPYING.
; If not, see <http://www.gnu.org/licenses/>.

		IMPORT	|Load$$ER_RW$$Base|
		IMPORT	|Image$$ER_RW$$Base|
		IMPORT	|Image$$ER_RW$$Length|
		IMPORT	|Image$$ER_RW$$Limit|
		IMPORT	|Image$$ER_ZI$$ZI$$Base|
		IMPORT	|Image$$ER_ZI$$ZI$$Length|
		IMPORT	|Image$$ER_ZI$$ZI$$Limit|
		IMPORT	|Image$$ER_IRQ_STACK$$ZI$$Base|
		IMPORT	|Image$$ER_IRQ_STACK$$ZI$$Length|
		IMPORT	|Image$$ER_IRQ_STACK$$ZI$$Limit|
		IMPORT	|Image$$ER_FIQ_STACK$$ZI$$Base|
		IMPORT	|Image$$ER_FIQ_STACK$$ZI$$Length|
		IMPORT	|Image$$ER_FIQ_STACK$$ZI$$Limit|
		IMPORT	|Image$$ER_SYS_STACK$$ZI$$Base|
		IMPORT	|Image$$ER_SYS_STACK$$ZI$$Length|
		IMPORT	|Image$$ER_SYS_STACK$$ZI$$Limit|
		IMPORT	init_jasmine
		IMPORT	Main
		IMPORT	fiq_handler
		IMPORT	irq_handler
		EXPORT	enable_interrupt
		EXPORT	disable_interrupt

MODE_USR        EQU     0x10
MODE_FIQ        EQU     0x11
MODE_IRQ        EQU     0x12
MODE_SVC        EQU     0x13
MODE_ABT        EQU     0x17
MODE_UND        EQU     0x1B
MODE_SYS        EQU     0x1F

I_BIT           EQU     0x80
F_BIT           EQU     0x40

		PRESERVE8
		AREA	init, CODE, READONLY

		ENTRY

		B reset_handler	; reset
		B .				; undefined instruction
		B .			  	; SWI
		B .				; prefetch abort
		B .				; data abort
		NOP				; reserved vector
		B irq_handler  	; IRQ
		B fiq_handler	; FIQ

reset_handler

		; IRQ mode stack

		MSR			CPSR_c,	#MODE_IRQ:OR:I_BIT:OR:F_BIT
		LDR			R13, =|Image$$ER_IRQ_STACK$$ZI$$Limit|

		; FIQ mode stack

		MSR			CPSR_c,	#MODE_FIQ:OR:I_BIT:OR:F_BIT
		LDR			R13, =|Image$$ER_FIQ_STACK$$ZI$$Limit|

		; SYSTEM mode stack
		; SYSTEM mode is the main mode of Barefoot firmware.

		MSR			CPSR_c,	#MODE_SYS:OR:I_BIT:OR:F_BIT
		LDR			R13, =|Image$$ER_SYS_STACK$$ZI$$Limit|

		BL			init_jasmine
		BL			Main
		B			.		; should not reach here

disable_interrupt

		MRS			R0, CPSR
		ORR			R0, R0, #0xC0
		MSR			CPSR_c, R0
		BX			LR

enable_interrupt

		MRS			R0, CPSR
		BIC			R0, R0, #0xC0
		MSR			CPSR_c, R0
		BX			LR

		END

