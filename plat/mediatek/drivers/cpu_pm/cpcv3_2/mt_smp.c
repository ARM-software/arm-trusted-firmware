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
	unsigned int val = is_core_power_status_on(cpu_id);

	CPU_PM_ASSERT(pwr_ctrl);

	mmio_clrbits_32(pwr_ctrl->pwpr, RESETPWRON_CONFIG);
	if (val == 0) {
		/*
		 * Set to 0 after BIG VPROC bulk powered on (configure in MCUPM) and
		 * before big core power-on sequence.
		 */
		if (cpu_id >= PLAT_CPU_PM_B_BUCK_ISO_ID) {
			mmio_write_32(DREQ20_BIG_VPROC_ISO, 0);
		}

		mmio_setbits_32(pwr_ctrl->pwpr, PWR_RST_B);
		dsbsy();

		/* set mp0_spmc_pwr_on_cpuX = 1 */
		mmio_setbits_32(pwr_ctrl->pwpr, PWR_ON);

		val = 0;
		while (is_core_power_status_on(cpu_id) == 0) {
			DO_SMP_CORE_ON_WAIT_TIMEOUT(val);
			mmio_clrbits_32(pwr_ctrl->pwpr, PWR_ON);
			mmio_setbits_32(pwr_ctrl->pwpr, PWR_ON);
		}
	} else {
		INFO("[%s:%d] - core_%u haven been power on\n", __func__, __LINE__, cpu_id);
	}

	return MTK_CPUPM_E_OK;
}

int mt_smp_power_core_off(struct cpu_pwr_ctrl *pwr_ctrl)
{
	/* set mp0_spmc_pwr_on_cpuX = 1 */
	mmio_clrbits_32(pwr_ctrl->pwpr, PWR_ON);
	return MTK_CPUPM_E_OK;
}

void mt_smp_init(void)
{
	/* clear RESETPWRON_CONFIG of mcusys/cluster/core0 */
	mmio_clrbits_32(SPM_MCUSYS_PWR_CON, RESETPWRON_CONFIG);
	mmio_clrbits_32(SPM_MP0_CPUTOP_PWR_CON, RESETPWRON_CONFIG);
}
