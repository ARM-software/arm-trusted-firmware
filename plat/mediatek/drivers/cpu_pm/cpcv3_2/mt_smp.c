/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <plat/common/platform.h>

#include <lib/pm/mtk_pm.h>
#include <mcucfg.h>
#include "mt_cpu_pm.h"
#include "mt_smp.h"

static inline int is_core_power_status_on(unsigned int cpuid)
{
	return !!(mmio_read_32(CPU_PWR_STATUS) & BIT(cpuid));
}

void mt_smp_core_init_arch(unsigned int cluster, unsigned int cpu, int arm64,
			   struct cpu_pwr_ctrl *pwr_ctrl)
{
	CPU_PM_ASSERT(cluster == 0);
	CPU_PM_ASSERT(pwr_ctrl != NULL);

	/* aa64naa32 in bits[16:23] */
	if (arm64 != 0) {
		mmio_setbits_32(pwr_ctrl->arch_addr, 1 << (16 + cpu));
	} else {
		mmio_clrbits_32(pwr_ctrl->arch_addr, 1 << (16 + cpu));
	}
}

void mt_smp_core_bootup_address_set(struct cpu_pwr_ctrl *pwr_ctrl, uintptr_t entry)
{
	CPU_PM_ASSERT(pwr_ctrl != NULL);

	/* Set bootup address */
	mmio_write_32(pwr_ctrl->rvbaraddr_l, entry);
}

int mt_smp_power_core_on(unsigned int cpu_id, struct cpu_pwr_ctrl *pwr_ctrl)
{
	uint32_t pwpr_reg;
	unsigned int val = is_core_power_status_on(cpu_id);

	CPU_PM_ASSERT(pwr_ctrl);

#ifdef CPU_PM_SPM_CORE_POWERON
	pwpr_reg = pwr_ctrl->pwpr_intermediate;
#else
	pwpr_reg = pwr_ctrl->pwpr;
#endif

	mmio_clrbits_32(pwpr_reg, RESETPWRON_CONFIG);

	if (val == 0) {
		/*
		 * Set to 0 after BIG VPROC bulk powered on (configure in MCUPM) and
		 * before big core power-on sequence.
		 */
		if (cpu_id >= PLAT_CPU_PM_B_BUCK_ISO_ID) {
			mmio_write_32(DREQ20_BIG_VPROC_ISO, 0);
		}

#ifdef CPU_PM_SPM_CORE_POWERON
		mmio_setbits_32(CPC_MCUSYS_CPC_FLOW_CTRL_CFG,
				SSPM_ALL_PWR_CTRL_EN);
#endif

		mmio_setbits_32(pwr_ctrl->pwpr, PWR_RST_B);
		dsbsy();

		/* Set mp0_spmc_pwr_on_cpuX = 1 */
		mmio_setbits_32(pwpr_reg, PWR_ON);

		val = 0;
		while (is_core_power_status_on(cpu_id) == 0) {
			DO_SMP_CORE_ON_WAIT_TIMEOUT(val);
			mmio_clrbits_32(pwpr_reg, PWR_ON);
			mmio_setbits_32(pwpr_reg, PWR_ON);
		}
	} else {
		INFO("[%s:%d] - core_%u haven been power on\n", __func__, __LINE__, cpu_id);
	}

	return MTK_CPUPM_E_OK;
}

int mt_smp_power_core_off(struct cpu_pwr_ctrl *pwr_ctrl)
{
	/* Set mp0_spmc_pwr_on_cpuX = 1 */
#ifdef CPU_PM_SPM_CORE_POWERON
	mmio_clrbits_32(pwr_ctrl->pwpr_intermediate, PWR_ON);
#else
	mmio_clrbits_32(pwr_ctrl->pwpr, PWR_ON);
#endif
	return MTK_CPUPM_E_OK;
}

void mt_smp_init(void)
{
	/* INFO=SPMC_INIT: clear resetpwron of mcusys/cluster/core0 */
#ifdef CPU_PM_SPM_CORE_POWERON
	mmio_write_32(SPM_POWERON_CONFIG_EN, PROJECT_CODE | BCLK_CG_EN);
	mmio_clrbits_32(SPM_VLP_MCUSYS_PWR_CON, RESETPWRON_CONFIG);
	mmio_clrbits_32(SPM_VLP_MP0_CPUTOP_PWR_CON, RESETPWRON_CONFIG);
#else
	mmio_clrbits_32(SPM_MCUSYS_PWR_CON, RESETPWRON_CONFIG);
	mmio_clrbits_32(SPM_MP0_CPUTOP_PWR_CON, RESETPWRON_CONFIG);
#endif
}
