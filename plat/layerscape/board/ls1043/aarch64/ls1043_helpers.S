/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <platform_def.h>

	.globl	plat_reset_handler
	.globl  plat_my_core_pos
	.globl  platform_mem_init

func plat_my_core_pos
	mrs x0, mpidr_el1
	and x1, x0, #MPIDR_CPU_MASK   //reserve the last 8 bits
	and x0, x0, #MPIDR_CLUSTER_MASK
	add x0, x1, x0, LSR #4  //4 cores
	ret
endfunc plat_my_core_pos

func platform_mem_init
	mov x29, x30
	bl  inv_dcache_range

//SDRAM_CFG
	ldr w0, =0x1080000
	ldr w1, =0x0c000c45
	str w1, [x0, #0x110]
//CS0_BNDS
	ldr w1, =0x7f000000
	str w1, [x0, #0x000]
//CS0_CONFIG
	ldr w1, =0x22030480
	str w1, [x0, #0x080]
//TIMING_CFG_0
	ldr w1, =0x18005591
	str w1, [x0, #0x104]
//TIMING_CFG_1
	ldr w1, =0x428cb4bb
	str w1, [x0, #0x108]
//TIMING_CFG_2
	ldr w1, =0x11c14800
	str w1, [x0, #0x10C]
//TIMING_CFG_3
	ldr w1, =0x00100c01
	str w1, [x0, #0x100]
//TIMING_CFG_4
	ldr w1, =0x02000000
	str w1, [x0, #0x160]
//TIMING_CFG_5
	ldr w1, =0x00144003
	str w1, [x0, #0x164]
//TIMING_CFG_7
	ldr w1, =0x00003013
	str w1, [x0, #0x16C]
//TIMING_CFG_8
	ldr w1, =0x00561102
	str w1, [x0, #0x250]
//SDRAM_CFG_2
	ldr w1, =0x00114000
	str w1, [x0, #0x114]
//SDRAM_MODE
	ldr w1, =0x10020103
	str w1, [x0, #0x118]
//SDRAM_MODE_2
	ldr w1, =0x0
	str w1, [x0, #0x11C]
//SDRAM_INTERVAL
	ldr w1, =0x18066018
	str w1, [x0, #0x124]
//DDR_WRLVL_CNTL
	ldr w1, =0x07f675c6
	str w1, [x0, #0x174]
//DDR_WRLVL_CNTL_2
	ldr w1, =0x00080907
	str w1, [x0, #0x190]
//DDR_WRLVL_CNTL_3
	ldr w1, =0x0
	str w1, [x0, #0x194]
//DDR_CDR1
	ldr w1, =0x00000480
	str w1, [x0, #0xB28]
//DDR_CDR2
	ldr w1, =0x81a10000
	str w1, [x0, #0xB2C]
//SDRAM_CLK_CNTL
	ldr w1, =0x00000003
	str w1, [x0, #0x130]
//DDR_ZQ_CNTL
	ldr w1, =0x0507098a
	str w1, [x0, #0x170]
//SDRAM_MODE_9
	ldr w1, =0x00050000
	str w1, [x0, #0x220]
//SDRAM_MODE_10
	ldr w1, =0x00000004
	str w1, [x0, #0x224]
//CS0_CONFIG_2
	ldr w1, =0x0
	str w1, [x0, #0x0C0]
//SDRAM_CFG
	ldr w1, =0x08000cc5
	str w1, [x0, #0x110]

	mov w3,#0
	ldr w4,=0xffffff01
z_loop:
delay_loop1:
	sub w4, w4, #1
	cmp w4, #0
	b.gt    delay_loop1

	ldr w1, [x0, #0x114]
	add w3, w3, #1
	cmp       w1, #0 //'\n'
	b.eq       1f
	cmp w3, #20
	b.gt    1f
	b z_loop

1:
	ldr w4,=0xffffff02
delay_loop2:
	sub w4, w4, #1
	cmp w4, #0
	b.gt        delay_loop2

	ldr w1, =0x00000000
	str w1, [x0]

	ret x29
endfunc	platform_mem_init

func apply_platform_errata
	/*TODO if needed*/
	ret
endfunc apply_platform_errata

func plat_reset_handler
	mov x29, x30
	bl  apply_platform_errata

	mov x30, x29
	ret
endfunc plat_reset_handler
