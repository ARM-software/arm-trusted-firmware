/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

.section .text, "ax"

#include <asm_macros.S>

#ifndef NXP_COINED_BB
#include <flash_info.h>
#include <fspi.h>
#endif
#include <regs.h>
#ifdef NXP_COINED_BB
#include <snvs.h>
#endif

#include <plat_warm_rst.h>
#include <platform_def.h>

#define SDRAM_CFG	0x110
#define SDRAM_CFG_2	0x114
#define SDRAM_MD_CNTL	0x120
#define SDRAM_INTERVAL	0x124
#define TIMING_CFG_10	0x258
#define DEBUG_2		0xF04
#define DEBUG_26	0xF64
#define DDR_DSR2	0xB24

#define DDR_CNTRLR_2	0x2
#define COUNT_100	1000

	.globl	_soc_sys_warm_reset
	.align 12

func _soc_sys_warm_reset
	mov  x3, xzr
	b    touch_line0
start_line0:
	mov  x3, #1
	mov  x2, #NUM_OF_DDRC
	ldr x1, =NXP_DDR_ADDR
1:
	ldr w0, [x1, #SDRAM_CFG]
	orr w0, w0, #SDRAM_CFG_MEM_HLT
	str w0, [x1, #SDRAM_CFG]
2:
	ldr w0, [x1, #DEBUG_2]
	and w0, w0, #DDR_DBG_2_MEM_IDLE
	cbz w0, 2b

	ldr w0, [x1, #DEBUG_26]
	orr w0, w0, #DDR_DEBUG_26_BIT_12
	orr w0, w0, #DDR_DEBUG_26_BIT_13
	orr w0, w0, #DDR_DEBUG_26_BIT_14
touch_line0:
	cbz x3, touch_line1

	orr w0, w0, #DDR_DEBUG_26_BIT_15
	orr w0, w0, #DDR_DEBUG_26_BIT_16
	str w0, [x1, #DEBUG_26]

	ldr w0, [x1, #SDRAM_CFG_2]
	orr w0, w0, #SDRAM_CFG2_FRC_SR
	str w0,  [x1, #SDRAM_CFG_2]

3:
	ldr w0, [x1, #DDR_DSR2]
	orr w0, w0, #DDR_DSR_2_PHY_INIT_CMPLT
	str w0, [x1, #DDR_DSR2]
	ldr w0, [x1, #DDR_DSR2]
        and w0, w0, #DDR_DSR_2_PHY_INIT_CMPLT
	cbnz w0, 3b

	ldr w0, [x1, #SDRAM_INTERVAL]
	and w0, w0, #SDRAM_INTERVAL_REFINT_CLEAR
	str w0, [x1, #SDRAM_INTERVAL]
touch_line1:
	cbz x3, touch_line2

	ldr w0, [x1, #SDRAM_MD_CNTL]
	orr w0, w0, #MD_CNTL_CKE(1)
	orr w0, w0, #MD_CNTL_MD_EN
	str w0, [x1, #SDRAM_MD_CNTL]

	ldr w0, [x1, #TIMING_CFG_10]
	orr w0, w0, #DDR_TIMING_CFG_10_T_STAB
	str w0, [x1, #TIMING_CFG_10]

	ldr w0, [x1, #SDRAM_CFG_2]
	and w0, w0, #SDRAM_CFG2_FRC_SR_CLEAR
	str w0, [x1, #SDRAM_CFG_2]

4:
	ldr w0, [x1, #DDR_DSR2]
        and w0, w0, #DDR_DSR_2_PHY_INIT_CMPLT
        cbz w0, 4b
	nop
touch_line2:
	cbz x3, touch_line3

	ldr w0, [x1, #DEBUG_26]
	orr w0, w0, #DDR_DEBUG_26_BIT_25
	and w0, w0, #DDR_DEBUG_26_BIT_24_CLEAR
	str w0, [x1, #DEBUG_26]

	cmp x2, #DDR_CNTRLR_2
	b.ne 5f
	ldr x1, =NXP_DDR2_ADDR
	mov x2, xzr
	b 1b

5:
	mov x5, xzr
6:
	add x5, x5, #1
	cmp x5, #COUNT_100
	b.ne 6b
	nop
touch_line3:
	cbz x3, touch_line4
#ifdef NXP_COINED_BB
        ldr  x1, =NXP_SNVS_ADDR
        ldr  w0, [x1, #NXP_APP_DATA_LP_GPR_OFFSET]

	/* On Warm Boot is enabled, then zeroth bit
	 * of SNVS LP GPR register 0 will used
	 * to save the status of warm-reset as a cause.
	 */
        orr  w0, w0, #(1 << NXP_LPGPR_ZEROTH_BIT)

        /* write back */
        str  w0, [x1, #NXP_APP_DATA_LP_GPR_OFFSET]
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
touch_line4:
	cbz x3, touch_line6
#elif !(ERLY_WRM_RST_FLG_FLSH_UPDT)
        ldr  x1, =NXP_FLEXSPI_ADDR
        ldr  w0, [x1, #FSPI_IPCMD]
        orr  w0, w0, #FSPI_IPCMD_TRG_MASK
        str  w0, [x1, #FSPI_IPCMD]
7:
        ldr  w0, [x1, #FSPI_INTR]
        and  w0, w0, #FSPI_INTR_IPCMDDONE_MASK
        cmp  w0, #0
        b.eq 7b

        ldr  w0, [x1, #FSPI_IPTXFCR]
        orr  w0, w0, #FSPI_IPTXFCR_CLR
        str  w0, [x1, #FSPI_IPTXFCR]

        ldr  w0, [x1, #FSPI_INTR]
        orr  w0, w0, #FSPI_INTR_IPCMDDONE_MASK
        str  w0, [x1, #FSPI_INTR]
	nop
touch_line4:
        cbz x3, touch_line5
        /* flexspi driver has an api
         * is_flash_busy().
         * Impelementation of the api will not
         * fit-in in 1 cache line.
         * instead a nop-cycles are introduced to
         * simulate the wait time for flash write
         * completion.
         *
         * Note: This wait time varies from flash to flash.
         */

        mov    x0, #FLASH_WR_COMP_WAIT_BY_NOP_COUNT
8:
        sub x0, x0, #1
        nop
        cmp x0, #0
        b.ne    8b
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
touch_line5:
        cbz x3, touch_line6
#endif
        ldr  x2, =NXP_RST_ADDR
	/* clear the RST_REQ_MSK and SW_RST_REQ */
	mov  w0, #0x00000000
	str  w0, [x2, #RSTCNTL_OFFSET]

	/* initiate the sw reset request */
	mov  w0, #SW_RST_REQ_INIT
        str  w0, [x2, #RSTCNTL_OFFSET]

        /* In case this address range is mapped as cacheable,
         * flush the write out of the dcaches.
         */
        add  x2, x2, #RSTCNTL_OFFSET
        dc   cvac, x2
        dsb  st
        isb

        /* Function does not return */
        b  .
	nop
	nop
	nop
	nop
	nop
	nop
	nop
touch_line6:
	cbz x3, start_line0

endfunc _soc_sys_warm_reset
