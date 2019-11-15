/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "smcall.h"

#define	SMC_ENTITY_PLATFORM_MONITOR	61

/*
 * SMC calls implemented by EL3 monitor
 */

/*
 * Write character in r1 to debug console
 */
#define SMC_FC_DEBUG_PUTC	SMC_FASTCALL_NR(SMC_ENTITY_PLATFORM_MONITOR, 0x0)

/*
 * Get register base address
 * r1: SMC_GET_GIC_BASE_GICD or SMC_GET_GIC_BASE_GICC
 */
#define SMC_GET_GIC_BASE_GICD	0
#define SMC_GET_GIC_BASE_GICC	1
#define SMC_GET_GIC_BASE_GICR	2
#define SMC_FC_GET_REG_BASE	SMC_FASTCALL_NR(SMC_ENTITY_PLATFORM_MONITOR, 0x1)
#define SMC_FC64_GET_REG_BASE	SMC_FASTCALL64_NR(SMC_ENTITY_PLATFORM_MONITOR, 0x1)
