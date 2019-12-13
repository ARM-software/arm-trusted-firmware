/*
 * Copyright (c) 2015 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CMN_PLAT_DEF_H
#define CMN_PLAT_DEF_H

/* Print file and line number on assert */
#define PLAT_LOG_LEVEL_ASSERT LOG_LEVEL_INFO

/*
 * The number of regions like RO(code), coherent and data required by
 * different BL stages which need to be mapped in the MMU.
 */
#if USE_COHERENT_MEM
#define CMN_BL_REGIONS	3
#else
#define CMN_BL_REGIONS	2
#endif

/*
 * FIP definitions
 */
#define PLAT_FIP_ATTEMPT_OFFSET		0x20000
#define PLAT_FIP_NUM_ATTEMPTS		128

#define PLAT_BRCM_FIP_QSPI_BASE		QSPI_BASE_ADDR
#define PLAT_BRCM_FIP_NAND_BASE		NAND_BASE_ADDR
#define PLAT_BRCM_FIP_MAX_SIZE		0x01000000

#define PLAT_BRCM_FIP_BASE	PLAT_BRCM_FIP_QSPI_BASE
#endif
