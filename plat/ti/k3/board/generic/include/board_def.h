/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __BOARD_DEF_H__
#define __BOARD_DEF_H__

/* The ports must be in order and contiguous */
#define K3_CLUSTER0_CORE_COUNT		2
#define K3_CLUSTER0_MSMC_PORT		0

#define K3_CLUSTER1_CORE_COUNT		2
#define K3_CLUSTER1_MSMC_PORT		1

#define K3_CLUSTER2_CORE_COUNT		2
#define K3_CLUSTER2_MSMC_PORT		2

#define K3_CLUSTER3_CORE_COUNT		2
#define K3_CLUSTER3_MSMC_PORT		3

/*
 * This RAM will be used for the bootloader including code, bss, and stacks.
 * It may need to be increased if BL31 grows in size.
 */
#define SEC_SRAM_BASE			0x70000000 /* Base of MSMC SRAM */
#define SEC_SRAM_SIZE			0x00020000 /* 128k */

#define PLAT_MAX_OFF_STATE		2
#define PLAT_MAX_RET_STATE		1

#endif /* __BOARD_DEF_H__ */
