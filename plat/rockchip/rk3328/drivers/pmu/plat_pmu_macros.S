/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <platform_def.h>

.globl	clst_warmboot_data

.macro	func_rockchip_clst_warmboot
.endm

.macro rockchip_clst_warmboot_data
clst_warmboot_data:
	.rept	PLATFORM_CLUSTER_COUNT
	.word	0
	.endr
.endm
