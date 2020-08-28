/*
 * Copyright (c) 2019-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/bl_common.h>
#include <mce.h>
#include <memctrl_v2.h>
#include <tegra_platform.h>
#include <tegra_private.h>

/*******************************************************************************
 * Array to hold MC context for Tegra194
 ******************************************************************************/
static __attribute__((aligned(16))) mc_regs_t tegra194_mc_context[] = {
	_START_OF_TABLE_,
	mc_smmu_bypass_cfg,	/* TBU settings */
	_END_OF_TABLE_,
};

/*******************************************************************************
 * Handler to return the pointer to the MC's context struct
 ******************************************************************************/
mc_regs_t *plat_memctrl_get_sys_suspend_ctx(void)
{
	/* index of _END_OF_TABLE_ */
	tegra194_mc_context[0].val = (uint32_t)ARRAY_SIZE(tegra194_mc_context) - 1U;

	return tegra194_mc_context;
}

/*******************************************************************************
 * Handler to restore platform specific settings to the memory controller
 ******************************************************************************/
void plat_memctrl_restore(void)
{
	UNUSED_FUNC_NOP(); /* do nothing */
}

/*******************************************************************************
 * Handler to program platform specific settings to the memory controller
 ******************************************************************************/
void plat_memctrl_setup(void)
{
	UNUSED_FUNC_NOP(); /* do nothing */
}

/*******************************************************************************
 * Handler to program the scratch registers with TZDRAM settings for the
 * resume firmware
 ******************************************************************************/
void plat_memctrl_tzdram_setup(uint64_t phys_base, uint64_t size_in_bytes)
{
	uint32_t sec_reg_ctrl = tegra_mc_read_32(MC_SECURITY_CFG_REG_CTRL_0);
	uint32_t phys_base_lo = (uint32_t)phys_base & 0xFFF00000;
	uint32_t phys_base_hi = (uint32_t)(phys_base >> 32);

	/*
	 * Check TZDRAM carveout register access status. Setup TZDRAM fence
	 * only if access is enabled.
	 */
	if ((sec_reg_ctrl & SECURITY_CFG_WRITE_ACCESS_BIT) ==
	     SECURITY_CFG_WRITE_ACCESS_ENABLE) {

		/*
		 * Setup the Memory controller to allow only secure accesses to
		 * the TZDRAM carveout
		 */
		INFO("Configuring TrustZone DRAM Memory Carveout\n");

		tegra_mc_write_32(MC_SECURITY_CFG0_0, phys_base_lo);
		tegra_mc_write_32(MC_SECURITY_CFG3_0, phys_base_hi);
		tegra_mc_write_32(MC_SECURITY_CFG1_0, (uint32_t)(size_in_bytes >> 20));

		/*
		 * MCE propagates the security configuration values across the
		 * CCPLEX.
		 */
		(void)mce_update_gsc_tzdram();
	}
}
