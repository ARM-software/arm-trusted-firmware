/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
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
#include "mt_ppu.h"
#include "mt_smp.h"

#define is_core_power_status_on(_pwr_ctrl)\
	(!!((mmio_read_32(_pwr_ctrl->pwr.ppu_pwsr)) & (PPU_PWSR_STATE_ON)))

#ifndef CPU_PM_CORE_ARCH64_ONLY
void mt_smp_core_init_arch(int cluster,
			   int cpu,
			   int arm64,
			   struct cpu_pwr_ctrl *pwr_ctrl)
{
	CPU_PM_ASSERT(cluster == 0);
	CPU_PM_ASSERT(pwr_ctrl);

	/* aa64naa32 in bits[16:23] */
	if (arm64)
		mmio_setbits_32(pwr_ctrl->arch_addr,
				BIT(AA64NAA32_FLAG_START_BIT + cpu));
	else
		mmio_clrbits_32(pwr_ctrl->arch_addr,
				BIT(AA64NAA32_FLAG_START_BIT + cpu));
}
#endif /* CPU_PM_CORE_ARCH64_ONLY */

void mt_smp_core_bootup_address_set(int cluster,
				    int cpu,
				    struct cpu_pwr_ctrl *pwr_ctrl,
				    uintptr_t entry)
{
	CPU_PM_ASSERT(pwr_ctrl);

	/* Set bootup address */
	mmio_write_32(pwr_ctrl->rvbaraddr_l, entry);
	mmio_write_32(pwr_ctrl->rvbaraddr_h, 0);
}

int mt_smp_power_core_on(unsigned int cpu_id, struct cpu_pwr_ctrl *pwr_ctrl)
{
	unsigned int val = 0;

	CPU_PM_ASSERT(pwr_ctrl);

	mt_smp_ppu_pwr_set(&pwr_ctrl->pwr, PPU_PWPR_DYNAMIC_MODE, PPU_PWPR_OFF);
	val = is_core_power_status_on(pwr_ctrl);
	if (!val) {
		mmio_clrbits_32(CPC_MCUSYS_CPC_FLOW_CTRL_CFG,
				GIC_WAKEUP_IGNORE(cpu_id));
		mmio_setbits_32(SPM_EXT_INT_WAKEUP_REQ_SET, BIT(cpu_id));

		mmio_clrbits_32(SPMC_CONTROL_CONFIG,
				SPMC_CPU_RESET_PWRON_CONFIG << (cpu_id));
		dsbsy();
		isb();

		while (!is_core_power_status_on(pwr_ctrl))
			DO_SMP_CORE_ON_WAIT_TIMEOUT(cpu_id, val);
		mmio_setbits_32(SPM_EXT_INT_WAKEUP_REQ_CLR, BIT(cpu_id));
	} else {
		mmio_clrbits_32(SPMC_CONTROL_CONFIG,
				SPMC_CPU_RESET_PWRON_CONFIG << (cpu_id));
		INFO("[%s:%d] - core_%u have been power on\n",
		     __func__, __LINE__, cpu_id);
	}

	return MTK_CPUPM_E_OK;
}

int mt_smp_power_core_off(unsigned int cpu_id, struct cpu_pwr_ctrl *pwr_ctrl)
{
	mmio_setbits_32(CPC_MCUSYS_CPC_FLOW_CTRL_CFG,
			GIC_WAKEUP_IGNORE(cpu_id));
	return MTK_CPUPM_E_OK;
}

void mt_smp_init(void)
{
	mmio_write_32(SPM_POWERON_CONFIG_EN, PROJECT_CODE | BCLK_CG_EN);

	/* INFO=SPMC_INIT: clear resetpwron of mcusys/cluster/core0 */
	mmio_clrbits_32(SPMC_CONTROL_CONFIG, SPMC_MCUSYS_RESET_PWRON_CONFIG);
	mmio_clrbits_32(SPMC_CONTROL_CONFIG, SPMC_CPUTOP_RESET_PWRON_CONFIG);

	/* Switch DSU ISO/CKDIS control from PCSM to PPU */
	mmio_setbits_32(CPC_FCM_SPMC_SW_CFG2,
			(CPUSYS_PPU_CLK_EN_CTRL | CPUSYS_PPU_ISO_CTRL));

#ifdef SPM_CPU_BUCK_ISO_CON
	/* Make sure that buck iso have been released before power on */
	mmio_write_32(SPM_CPU_BUCK_ISO_CON, SPM_CPU_BUCK_ISO_DEFAUT);
#endif /* SPM_CPU_BUCK_ISO_CON */
}
