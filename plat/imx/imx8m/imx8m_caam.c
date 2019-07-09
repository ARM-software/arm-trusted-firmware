/*
 * Copyright (c) 2019, NXP. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>

#include <imx8m_caam.h>

void imx8m_caam_init(void)
{
	uint32_t sm_cmd;

	/* Dealloc part 0 and 2 with current DID */
	sm_cmd = (0 << SMC_PART_SHIFT | SMC_CMD_DEALLOC_PART);
	mmio_write_32(SM_CMD, sm_cmd);

	sm_cmd = (2 << SMC_PART_SHIFT | SMC_CMD_DEALLOC_PART);
	mmio_write_32(SM_CMD, sm_cmd);

	/* config CAAM JRaMID set MID to Cortex A */
	mmio_write_32(CAAM_JR0MID, CAAM_NS_MID);
	mmio_write_32(CAAM_JR1MID, CAAM_NS_MID);
	mmio_write_32(CAAM_JR2MID, CAAM_NS_MID);

	/* Alloc partition 0 writing SMPO and SMAGs */
	mmio_write_32(SM_P0_PERM, 0xff);
	mmio_write_32(SM_P0_SMAG2, 0xffffffff);
	mmio_write_32(SM_P0_SMAG1, 0xffffffff);

	/* Allocate page 0 and 1 to partition 0 with DID set */
	sm_cmd = (0 << SMC_PAGE_SHIFT | 0 << SMC_PART_SHIFT |
			SMC_CMD_ALLOC_PAGE);
	mmio_write_32(SM_CMD, sm_cmd);

	sm_cmd = (1 << SMC_PAGE_SHIFT | 0 << SMC_PART_SHIFT |
			SMC_CMD_ALLOC_PAGE);
	mmio_write_32(SM_CMD, sm_cmd);
}
