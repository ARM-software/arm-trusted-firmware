/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * Platform overrides for the LX2160A shared soc.c weak hooks.
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <nxp_smmu.h>

#include "flash_bootstrap.h"
#include "nbxv3_runtime_mode.h"
#include <soc.h>

/*
 * Called near the end of soc_early_init()
 *
 * nbxv3-specific platform overrides go here.
 */
void plat_soc_early_init_hook(void)
{
	nbxv3_mode_t mode = nbxv3_runtime_mode();

	NOTICE("nbxv3: runtime mode = %s\n", nbxv3_mode_name(mode));

	switch (mode) {
	case NBXV3_MODE_FLASH_BOOTSTRAP:
		nbxv3_flash_bootstrap();
		/* no return */
	case NBXV3_MODE_FLASH_NO_HOST:
		nbxv3_wait_or_por_retry();
		/* no return */
	case NBXV3_MODE_PROD:
	default:
		break;
	}

	/*
	 * Baremetal packet processing: SMMU bypass for the whole bus.
	 *
	 * Sets SCR0.CLIENTPD = 1 and SCR0.USFCFG = 0 on both the secure
	 * (SCR0) and non-secure (NSCR0) global control banks. After this
	 * point every transaction passes through with IOVA == PA: no
	 * stream-table walk, no TLB pressure, no GFSR fault traffic.
	 *
	 * Symmetric with the existing secure-boot path. Upstream
	 * soc.c::soc_init() already calls bypass_smmu() when
	 * TRUSTED_BOARD_BOOT=1 AND boot mode is secure. We're not adding
	 * any new behaviour to the SMMU hardware: we're widening the
	 * "when do we call it" condition to "always on nbxv3", because
	 * the baremetal use cases has the same need (no IOMMU isolation
	 * required) as the secure-boot path that upstream already covers.
	 *
	 * Threat model: there is no untrusted DMA path on this product
	 * (no PCIe slot exposed to user-pluggable cards, no virtualised
	 * guests, no untrusted device firmware). The SMMU's IOMMU
	 * isolation defends against threats that do not exist here, at
	 * the cost of stream-table-walk latency on every DPAA2 / WRIOP /
	 * qDMA descriptor in the 100GE packet path.
	 *
	 * If the threat model ever changes (ex: a board revision adds a
	 * user-facing PCIe slot, or virt), revert this call site: no other
	 * change is required to reinstate SMMU protection.
	 */
	bypass_smmu(NXP_SMMU_ADDR);
	NOTICE("nbxv3: SMMU bypass on (IOVA == PA): baremetal packet path\n");
}
