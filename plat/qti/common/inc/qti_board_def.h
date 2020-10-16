/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef QTI_BOARD_DEF_H
#define QTI_BOARD_DEF_H

/*
 * Required platform porting definitions common to all ARM
 * development platforms
 */

/*
 * Defines used to retrieve QTI SOC Version
 */
#define JEDEC_QTI_BKID	U(0x0)
#define JEDEC_QTI_MFID	U(0x70)
#define QTI_SOC_CONTINUATION_SHIFT	U(24)
#define QTI_SOC_IDENTIFICATION_SHIFT	U(16)

/* Size of cacheable stacks */
#define PLATFORM_STACK_SIZE	0x1000

/*
 * PLAT_QTI_MMAP_ENTRIES depends on the number of entries in the
 * plat_qti_mmap array defined for each BL stage.
 */
#define PLAT_QTI_MMAP_ENTRIES	12

/*
 * Platform specific page table and MMU setup constants
 */
#define MAX_XLAT_TABLES		12

#endif /* QTI_BOARD_DEF_H */
