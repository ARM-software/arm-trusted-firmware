/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BL1_H
#define BL1_H

#include <common/bl_common.h>

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

#include <lib/cassert.h>

struct entry_point_info;

register_t bl1_smc_wrapper(uint32_t smc_fid,
	void *cookie,
	void *handle,
	unsigned int flags);

register_t bl1_smc_handler(unsigned int smc_fid,
	register_t x1,
	register_t x2,
	register_t x3,
	register_t x4,
	void *cookie,
	void *handle,
	unsigned int flags);

void bl1_print_next_bl_ep_info(const struct entry_point_info *bl_ep_info);

void bl1_setup(void);
void bl1_main(void);
void bl1_plat_prepare_exit(entry_point_info_t *ep_info);

/*
 * Check if the total number of FWU SMC calls are as expected.
 */
CASSERT(FWU_NUM_SMC_CALLS == 	\
		(FWU_SMC_FID_END - FWU_SMC_FID_START + 1),\
		assert_FWU_NUM_SMC_CALLS_mismatch);

/* Utility functions */
void bl1_calc_bl2_mem_layout(const meminfo_t *bl1_mem_layout,
			meminfo_t *bl2_mem_layout);

#endif /* __ASSEMBLY__ */
#endif /* BL1_H */
