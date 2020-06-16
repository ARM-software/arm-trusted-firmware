/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <mcucfg.h>
#include <mtspmc.h>
#include <mtspmc_private.h>


void mcucfg_disable_gic_wakeup(uint32_t cluster, uint32_t cpu)
{
	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, GIC_WAKEUP_IGNORE(cpu));
}

void mcucfg_enable_gic_wakeup(uint32_t cluster, uint32_t cpu)
{
	mmio_clrbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, GIC_WAKEUP_IGNORE(cpu));
}

void mcucfg_set_bootaddr(uint32_t cluster, uint32_t cpu, uintptr_t bootaddr)
{
	assert(cluster == 0U);

	mmio_write_32(per_cpu(cluster, cpu, MCUCFG_BOOTADDR), bootaddr);
}

uintptr_t mcucfg_get_bootaddr(uint32_t cluster, uint32_t cpu)
{
	assert(cluster == 0U);

	return (uintptr_t)mmio_read_32(per_cpu(cluster, cpu, MCUCFG_BOOTADDR));
}

void mcucfg_init_archstate(uint32_t cluster, uint32_t cpu, bool arm64)
{
	uint32_t reg;

	assert(cluster == 0U);

	reg = per_cluster(cluster, MCUCFG_INITARCH);

	if (arm64) {
		mmio_setbits_32(reg, MCUCFG_INITARCH_CPU_BIT(cpu));
	} else {
		mmio_clrbits_32(reg, MCUCFG_INITARCH_CPU_BIT(cpu));
	}
}

/**
 * Return subsystem's power state.
 *
 * @mask: mask to SPM_CPU_PWR_STATUS to query the power state
 *        of one subsystem.
 * RETURNS:
 * 0 (the subsys was powered off)
 * 1 (the subsys was powered on)
 */
bool spm_get_powerstate(uint32_t mask)
{
	return (mmio_read_32(SPM_CPU_PWR_STATUS) & mask) != 0U;
}

bool spm_get_cluster_powerstate(uint32_t cluster)
{
	assert(cluster == 0U);

	return spm_get_powerstate(MP0_CPUTOP);
}

bool spm_get_cpu_powerstate(uint32_t cluster, uint32_t cpu)
{
	uint32_t mask = BIT(cpu);

	assert(cluster == 0U);

	return spm_get_powerstate(mask);
}

int spmc_init(void)
{
	INFO("SPM: enable CPC mode\n");

	mmio_write_32(SPM_POWERON_CONFIG_EN, PROJECT_CODE | BCLK_CG_EN);

	mmio_setbits_32(per_cpu(0, 1, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 2, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 3, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 4, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 5, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 6, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 7, SPM_CPU_PWR), PWR_RST_B);

	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, GIC_WAKEUP_IGNORE(1));
	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, GIC_WAKEUP_IGNORE(2));
	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, GIC_WAKEUP_IGNORE(3));
	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, GIC_WAKEUP_IGNORE(4));
	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, GIC_WAKEUP_IGNORE(5));
	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, GIC_WAKEUP_IGNORE(6));
	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, GIC_WAKEUP_IGNORE(7));

	mmio_clrbits_32(SPM_MCUSYS_PWR_CON, RESETPWRON_CONFIG);
	mmio_clrbits_32(SPM_MP0_CPUTOP_PWR_CON, RESETPWRON_CONFIG);
	mmio_clrbits_32(per_cpu(0, 0, SPM_CPU_PWR), RESETPWRON_CONFIG);

	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, CPC_CTRL_ENABLE);

	return 0;
}

/**
 * Power on a core with specified cluster and core index
 *
 * @cluster: the cluster ID of the CPU which to be powered on
 * @cpu: the CPU ID of the CPU which to be powered on
 */
void spm_poweron_cpu(uint32_t cluster, uint32_t cpu)
{
	/* set to 0 after BIG VPROC bulk on & before B-core power on seq. */
	if (cpu >= 4U) {
		mmio_write_32(DREQ20_BIG_VPROC_ISO, 0U);
	}

	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, SSPM_ALL_PWR_CTRL_EN);
	mmio_setbits_32(per_cpu(cluster, cpu, SPM_CPU_PWR), PWR_ON);

	while (!spm_get_cpu_powerstate(cluster, cpu)) {
	}

	mmio_clrbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, SSPM_ALL_PWR_CTRL_EN);

	/* Enable Big CPU Last PC */
	if (cpu >= 4U) {
		mmio_clrbits_32(LAST_PC_REG(cpu), BIT(3));
	}
}

/**
 * Power off a core with specified cluster and core index
 *
 * @cluster: the cluster ID of the CPU which to be powered off
 * @cpu: the CPU ID of the CPU which to be powered off
 */
void spm_poweroff_cpu(uint32_t cluster, uint32_t cpu)
{
	/* Set mp0_spmc_pwr_on_cpuX = 0 */
	mmio_clrbits_32(per_cpu(cluster, cpu, SPM_CPU_PWR), PWR_ON);
}

/**
 * Power off a cluster with specified index
 *
 * @cluster: the cluster index which to be powered off
 */
void spm_poweroff_cluster(uint32_t cluster)
{
	/* No need to power on/off cluster on single cluster platform */
	assert(false);
}

/**
 * Power on a cluster with specified index
 *
 * @cluster: the cluster index which to be powered on
 */
void spm_poweron_cluster(uint32_t cluster)
{
	/* No need to power on/off cluster on single cluster platform */
	assert(false);
}
