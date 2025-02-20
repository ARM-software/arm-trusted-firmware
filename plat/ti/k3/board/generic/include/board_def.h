/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BOARD_DEF_H
#define BOARD_DEF_H

#include <lib/utils_def.h>

/* The ports must be in order and contiguous */
#define K3_CLUSTER0_CORE_COUNT		U(2)
#define K3_CLUSTER1_CORE_COUNT		U(2)
#define K3_CLUSTER2_CORE_COUNT		U(0)
#define K3_CLUSTER3_CORE_COUNT		U(0)

#define PLAT_PROC_START_ID		U(32)
#define PLAT_PROC_DEVICE_START_ID	U(202)
#define PLAT_CLUSTER_DEVICE_START_ID	U(198)
#define PLAT_BOARD_DEVICE_ID		U(157)

/*******************************************************************************
 * Memory layout constants
 ******************************************************************************/

/*
 * This RAM will be used for the bootloader including code, bss, and stacks.
 * It may need to be increased if BL31 grows in size.
 *
 * The link addresses are determined by BL31_BASE + offset.
 * When ENABLE_PIE is set, the TF images can be loaded anywhere, so
 * BL31_BASE is really arbitrary.
 *
 * When ENABLE_PIE is unset, BL31_BASE should be chosen so that
 * it matches to the physical address where BL31 is loaded, that is,
 * BL31_BASE should be the base address of the RAM region.
 *
 * Lets make things explicit by mapping BL31_BASE to 0x0 since ENABLE_PIE is
 * defined as default for our platform.
 */
#define BL31_BASE	UL(0x00000000) /* PIE remapped on fly */
#define BL31_SIZE	UL(0x00020000) /* 128k */
#define BL31_LIMIT	(BL31_BASE + BL31_SIZE)

#endif /* BOARD_DEF_H */
