/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __BL1_FWU_H__
#define __BL1_FWU_H__

#include <bl_common.h>

/*
 * Defines for BL1 SMC function ids.
 */
#define BL1_SMC_CALL_COUNT		0x0
#define BL1_SMC_UID			0x1
/* SMC #0x2 reserved */
#define BL1_SMC_VERSION			0x3

/*
 * Corresponds to the function ID of the SMC that
 * the BL1 exception handler service to execute BL31.
 */
#define BL1_SMC_RUN_IMAGE		0x4

/*
 * BL1 SMC version
 */
#define BL1_SMC_MAJOR_VER		0x0
#define BL1_SMC_MINOR_VER		0x1

/*
 * Defines for FWU SMC function ids.
 */

#define FWU_SMC_IMAGE_COPY		0x10
#define FWU_SMC_IMAGE_AUTH		0x11
#define FWU_SMC_IMAGE_EXECUTE		0x12
#define FWU_SMC_IMAGE_RESUME		0x13
#define FWU_SMC_SEC_IMAGE_DONE		0x14
#define FWU_SMC_UPDATE_DONE		0x15
#define FWU_SMC_IMAGE_RESET		0x16

/*
 * Number of FWU calls (above) implemented
 */
#define FWU_NUM_SMC_CALLS		7

#if TRUSTED_BOARD_BOOT
# define BL1_NUM_SMC_CALLS		(FWU_NUM_SMC_CALLS + 4)
#else
# define BL1_NUM_SMC_CALLS		4
#endif

/*
 * The macros below are used to identify FWU
 * calls from the SMC function ID
 */
#define FWU_SMC_FID_START		FWU_SMC_IMAGE_COPY
#define FWU_SMC_FID_END			FWU_SMC_IMAGE_RESET
#define is_fwu_fid(_fid) \
    ((_fid >= FWU_SMC_FID_START) && (_fid <= FWU_SMC_FID_END))

#ifndef __ASSEMBLY__
#include <cassert.h>

/*
 * Check if the total number of FWU SMC calls are as expected.
 */
CASSERT(FWU_NUM_SMC_CALLS == 	\
		(FWU_SMC_FID_END - FWU_SMC_FID_START + 1),\
		assert_FWU_NUM_SMC_CALLS_mismatch);

#endif /* __ASSEMBLY__ */
#endif /* __BL1_FWU_H__ */
