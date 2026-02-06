/*
 * Copyright (c) 2025-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <drivers/arm/css/css_mhu_doorbell.h>
#include <drivers/arm/css/css_scp.h>
#include <drivers/arm/css/scmi.h>
#include <drivers/arm/dsu.h>
#include <lib/psci/psci.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <services/pfdi.h>

#include <rdaspen_ras.h>

#if PFDI_SUPPORT
void rdaspen_pfdi_setup(void);
#endif

static scmi_channel_plat_info_t plat_rd_scmi_info[] = {
	{
		.scmi_mbx_mem = CSS_SCMI_PAYLOAD_BASE,
		.db_reg_addr = PLAT_CSS_MHU_BASE + MHU_V3_SENDER_REG_SET(0),
		.db_preserve_mask = 0xfffffffe,
		.db_modify_mask = 0x1,
		.ring_doorbell = &mhu_ring_doorbell,
	},
};

scmi_channel_plat_info_t *plat_css_get_scmi_info(unsigned int channel_id)
{
	assert(channel_id == 0U);
	return &plat_rd_scmi_info[channel_id];
}

#if PFDI_SUPPORT
void pfdi_enable(void)
{
	int cpu_num;
	int psci_rc;
	pfdi_status_t rc;

	cpu_num = plat_my_core_pos();
	rc = pfdi_pe_oor_test_run();
	if (rc != 0) {
		ERROR("PFDI: OoR tests on core %d failed.\n", cpu_num);
	}

	/* This hook runs only on secondary warm boot. Power the PE back off. */
	psci_rc = psci_cpu_off();
	ERROR("PFDI: Failed to power off core %d (%d).\n", cpu_num, psci_rc);
	panic();
}

u_register_t plat_pfdi_mpidr_by_core_pos(unsigned int core_pos)
{
	unsigned int cluster_id = core_pos / PLAT_MAX_CPUS_PER_CLUSTER;
	unsigned int core_id = core_pos % PLAT_MAX_CPUS_PER_CLUSTER;

	return ((u_register_t)((core_id & MPIDR_AFFLVL_MASK) << MPIDR_AFF1_SHIFT) |
	       ((u_register_t)((cluster_id & MPIDR_AFFLVL_MASK) << MPIDR_AFF2_SHIFT)));
}

static int rdaspen_pwr_domain_on(u_register_t mpidr)
{
	uint64_t ft_id;
	uint64_t cpu_num = plat_core_pos_by_mpidr(mpidr);
	pfdi_status_t pfdi_status = pfdi_func_desc.result(cpu_num, &ft_id);
	/*
	 * The core can only boot if the OoR PFDI tests have not failed
	 * or if the OoR PFDI tests have not been run yet.
	 */
	if ((pfdi_status != 0) && (pfdi_status != PFDI_RET_NOT_RUN)) {
		return PSCI_E_INTERN_FAIL;
	}

	css_scp_on(mpidr);

	return PSCI_E_SUCCESS;
}
#endif /* PFDI_SUPPORT */

const plat_psci_ops_t *plat_arm_psci_override_pm_ops(plat_psci_ops_t *ops)
{
#if PFDI_SUPPORT
	ops->pwr_domain_on = rdaspen_pwr_domain_on;
#endif /* PFDI_SUPPORT */
	ops->pwr_domain_on_finish = rdaspen_css_pwr_domain_on_finish;
	return css_scmi_override_pm_ops(ops);
}

const dsu_driver_data_t plat_dsu_data = {
	.clusterpwrdwn_pwrdn = false,
	.clusterpwrdwn_memret = false,
	.clusterpwrctlr_cachepwr = CLUSTERPWRCTLR_CACHEPWR_RESET,
	.clusterpwrctlr_funcret = CLUSTERPWRCTLR_FUNCRET_RESET
};

void bl31_platform_setup(void)
{
	arm_bl31_platform_setup();
#if PFDI_SUPPORT
	rdaspen_pfdi_setup();
#endif
	rdaspen_ras_init_per_cpu();
}

#if defined(SPD_spmd) && (SPMC_AT_EL3 == 0)
/*
 * A dummy implementation of the platform handler for Group0 secure interrupt.
 */
int plat_spmd_handle_group0_interrupt(uint32_t intid)
{
	(void)intid;
	return -1;
}
#endif /* defined(SPD_spmd) && (SPMC_AT_EL3 == 0) */
