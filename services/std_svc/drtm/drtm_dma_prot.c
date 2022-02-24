/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier:    BSD-3-Clause
 *
 * DRTM DMA protection.
 *
 * Authors:
 *      Lucian Paul-Trifu <lucian.paultrifu@gmail.com>
 *
 */

#include <stdint.h>
#include <string.h>

#include <common/debug.h>

#include "drtm_dma_prot.h"
#include <plat/common/platform.h>

/*
 * This function checks that platform supports complete DMA protection.
 * and returns false - if the platform supports complete DMA protection.
 * and returns true - if the platform does not support complete DMA protection.
 */
bool drtm_dma_prot_init(void)
{
	bool must_init_fail = false;
	const uintptr_t *smmus;
	size_t num_smmus = 0;
	unsigned int total_smmus;

	/* Warns presence of non-host platforms */
	if (plat_has_non_host_platforms()) {
		WARN("DRTM: the platform includes trusted DMA-capable devices"
				" (non-host platforms)\n");
	}

	/*
	 * DLME protection is uncertain on platforms with peripherals whose
	 * DMA is not managed by an SMMU. DRTM doesn't work on such platforms.
	 */
	if (plat_has_unmanaged_dma_peripherals()) {
		ERROR("DRTM: this platform does not provide DMA protection\n");
		must_init_fail = true;
	}

	/*
	 * Check that the platform reported all SMMUs.
	 * It is acceptable if the platform doesn't have any SMMUs when it
	 * doesn't have any DMA-capable devices.
	 */
	total_smmus = plat_get_total_smmus();
	plat_enumerate_smmus(&smmus, &num_smmus);
	if (num_smmus != total_smmus) {
		ERROR("DRTM: could not discover all SMMUs\n");
		must_init_fail = true;
	}

	return must_init_fail;
}
