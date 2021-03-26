/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BOARD_DEF_H
#define BOARD_DEF_H

#include <lib/utils_def.h>

/* The ports must be in order and contiguous */
#define K3_CLUSTER0_CORE_COUNT		U(4)
#define K3_CLUSTER1_CORE_COUNT		U(0)
#define K3_CLUSTER2_CORE_COUNT		U(0)
#define K3_CLUSTER3_CORE_COUNT		U(0)

/*
 * This RAM will be used for the bootloader including code, bss, and stacks.
 * It may need to be increased if BL31 grows in size.
 * Current computation assumes data structures necessary for GIC and ARM for
 * a single cluster of 4 processor.
 *
 * The link addresses are determined by SEC_SRAM_BASE + offset.
 * When ENABLE_PIE is set, the TF images can be loaded anywhere, so
 * SEC_SRAM_BASE is really arbitrary.
 *
 * When ENABLE_PIE is unset, SEC_SRAM_BASE should be chosen so that
 * it matches to the physical address where BL31 is loaded, that is,
 * SEC_SRAM_BASE should be the base address of the RAM region.
 *
 * Lets make things explicit by mapping SRAM_BASE to 0x0 since ENABLE_PIE is
 * defined as default for our platform.
 */
#define SEC_SRAM_BASE			UL(0x00000000) /* PIE remapped on fly */
#define SEC_SRAM_SIZE			UL(0x0001c000) /* 112k */

#define PLAT_MAX_OFF_STATE		U(2)
#define PLAT_MAX_RET_STATE		U(1)

#define PLAT_PROC_START_ID		U(32)
#define PLAT_PROC_DEVICE_START_ID	U(135)
#define PLAT_CLUSTER_DEVICE_START_ID	U(134)

#endif /* BOARD_DEF_H */
