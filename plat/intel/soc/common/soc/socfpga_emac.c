/*
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>
#include <platform_def.h>

#include "socfpga_emac.h"
#include "socfpga_reset_manager.h"
#include "socfpga_system_manager.h"

void socfpga_emac_init(void)
{
	mmio_setbits_32(SOCFPGA_RSTMGR(PER0MODRST),
		RSTMGR_PER0MODRST_EMAC0 |
		RSTMGR_PER0MODRST_EMAC1 |
		RSTMGR_PER0MODRST_EMAC2);

	mmio_clrsetbits_32(SOCFPGA_SYSMGR(EMAC_0),
		PHY_INTF_SEL_MSK, EMAC0_PHY_MODE);
	mmio_clrsetbits_32(SOCFPGA_SYSMGR(EMAC_1),
		PHY_INTF_SEL_MSK, EMAC1_PHY_MODE);
	mmio_clrsetbits_32(SOCFPGA_SYSMGR(EMAC_2),
		PHY_INTF_SEL_MSK, EMAC2_PHY_MODE);

	mmio_clrbits_32(SOCFPGA_SYSMGR(FPGAINTF_EN_3),
		FPGAINTF_EN_3_EMAC_MSK(0) |
		FPGAINTF_EN_3_EMAC_MSK(1) |
		FPGAINTF_EN_3_EMAC_MSK(2));

	mmio_clrbits_32(SOCFPGA_RSTMGR(PER0MODRST),
		RSTMGR_PER0MODRST_EMAC0 |
		RSTMGR_PER0MODRST_EMAC1 |
		RSTMGR_PER0MODRST_EMAC2);
}

