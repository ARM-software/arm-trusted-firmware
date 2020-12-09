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
 */
#define SEC_SRAM_BASE			0x70000000 /* Base of SRAM */
#define SEC_SRAM_SIZE			0x0001a000 /* 104k */

#define PLAT_MAX_OFF_STATE		U(2)
#define PLAT_MAX_RET_STATE		U(1)

#define PLAT_PROC_START_ID		32
#define PLAT_PROC_DEVICE_START_ID	135
#define PLAT_CLUSTER_DEVICE_START_ID	134

#endif /* BOARD_DEF_H */
