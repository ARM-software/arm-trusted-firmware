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
	 * Return the primary CPU ID from SDS Structure
	 * Returns CPUID on success or -1 on failure
	 */
func sds_get_primary_cpu_id
	ldr	r0, =PLAT_ARM_SDS_MEM_BASE
	ldr	r2, =SDS_REGION_SIGNATURE
	ldr	r1, [r0]
	ubfx	r3, r1, #0, #16

	/* Check if the SDS region signature found */
	cmp	r2, r3
	bne	2f

	/* Get the structure count from region descriptor in r1 */
	ubfx	r1, r1, #SDS_REGION_STRUCT_COUNT_SHIFT, #SDS_REGION_STRUCT_COUNT_WIDTH
	cmp	r1, #0
	beq	2f
	add	r0, r0, #SDS_REGION_DESC_SIZE

	/* Initialize the loop iterator count in r3 */
	mov	r3, #0
loop_begin:
	ldrh	r2, [r0]
	cmp	r2, #SDS_AP_CPU_INFO_STRUCT_ID
	bne	continue_loop

	/* We have found the required structure */
	ldr	r0, [r0,#(SDS_HEADER_SIZE + SDS_AP_CPU_INFO_PRIMARY_CPUID_OFFSET)]
	bx	lr
continue_loop:
	/* Increment the loop counter and exit loop if counter == structure count */
	add	r3, r3, #0x1
	cmp	r1, r3
	beq	2f

	/* Read the 2nd word in header */
	ldr	r2, [r0,#4]
	/* Get the structure size from header */
	ubfx	r2, r2, #SDS_HEADER_STRUCT_SIZE_SHIFT, #SDS_HEADER_STRUCT_SIZE_WIDTH
	/* Add the structure size and SDS HEADER SIZE to point to next header */
	add	r2, r2, #SDS_HEADER_SIZE
	add	r0, r0, r2
	b	loop_begin
2:
	mov	r0, #0xffffffff
	bx	lr
endfunc sds_get_primary_cpu_id
