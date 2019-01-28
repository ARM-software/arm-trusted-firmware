/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <drivers/arm/css/sds.h>
#include <platform_def.h>

#include "../sds_private.h"

	.globl	sds_get_primary_cpu_id

	/*
	 * int sds_get_primary_cpu_id(void);
	 * Return the primary CPI ID from SDS Structure
	 * Returns CPUID on success or -1 on failure
	 */
func sds_get_primary_cpu_id
	mov_imm	x0, PLAT_ARM_SDS_MEM_BASE
	mov	w2, #SDS_REGION_SIGNATURE
	ldr	w1, [x0]

	/* Check if the SDS region signature found */
	cmp	w2, w1, uxth
	b.ne	2f

	/* Get the structure count from region descriptor in `w1 */
	ubfx	w1, w1, #SDS_REGION_STRUCT_COUNT_SHIFT, #SDS_REGION_STRUCT_COUNT_WIDTH
	cbz	w1, 2f
	add	x0, x0, #SDS_REGION_DESC_SIZE

	/* Initialize the loop iterator count in w3 */
	mov	w3, #0
loop_begin:
	ldrh	w2, [x0]
	cmp	w2, #SDS_AP_CPU_INFO_STRUCT_ID
	b.ne	continue_loop

	/* We have found the required structure */
	ldr	w0, [x0,#(SDS_HEADER_SIZE + SDS_AP_CPU_INFO_PRIMARY_CPUID_OFFSET)]
	ret
continue_loop:
	/* Increment the loop counter and exit loop if counter == structure count */
	add	w3, w3, #0x1
	cmp	w1, w3
	b.eq	2f

	/* Read the 2nd word in header */
	ldr	w2, [x0,#4]
	/* Get the structure size from header */
	ubfx	x2, x2, #SDS_HEADER_STRUCT_SIZE_SHIFT, #SDS_HEADER_STRUCT_SIZE_WIDTH
	/* Add the structure size and SDS HEADER SIZE to point to next header */
	add	x2, x2, #SDS_HEADER_SIZE
	add	x0, x0, x2
	b	loop_begin
2:
	mov	w0, #0xffffffff
	ret
endfunc sds_get_primary_cpu_id
