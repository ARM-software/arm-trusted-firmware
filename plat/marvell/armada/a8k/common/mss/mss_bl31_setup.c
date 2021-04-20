/*
 * Copyright (C) 2021 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <platform_def.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/mmio.h>

#include <armada_common.h>

#include "mss_defs.h"

void mss_start_cp_cm3(int cp)
{
	uint32_t magic;
	uintptr_t sram = MVEBU_CP_REGS_BASE(cp) + MSS_CP_SRAM_OFFSET;
	uintptr_t regs = MVEBU_CP_REGS_BASE(cp) + MSS_CP_REGS_OFFSET;

	magic = mmio_read_32(sram);

	/* Make sure the FW was loaded */
	if (magic != MSS_FW_READY_MAGIC) {
		return;
	}

	NOTICE("Starting CP%d MSS CPU\n", cp);
	/* remove the magic */
	mmio_write_32(sram, 0);
	/* Release M3 from reset */
	mmio_write_32(MSS_M3_RSTCR(regs),
		      (MSS_M3_RSTCR_RST_OFF << MSS_M3_RSTCR_RST_OFFSET));
}
