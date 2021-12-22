/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
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
#include <plat/common/platform.h>

void mcucfg_disable_gic_wakeup(unsigned int cluster, unsigned int cpu)
{
	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, GIC_WAKEUP_IGNORE(cpu));
}

void mcucfg_enable_gic_wakeup(unsigned int cluster, unsigned int cpu)
{
	mmio_clrbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, GIC_WAKEUP_IGNORE(cpu));
	/* Clear cpu's cpc sw hint */
	mmio_write_32(CPC_MCUSYS_CPU_ON_SW_HINT_CLR, BIT(cpu));
}

void mcucfg_set_bootaddr(unsigned int cluster, unsigned int cpu, uintptr_t bootaddr)
{
	assert(cluster == 0U);

	mmio_write_32(per_cpu(cluster, cpu, MCUCFG_BOOTADDR), bootaddr);
}

uintptr_t mcucfg_get_bootaddr(unsigned int cluster, unsigned int cpu)
{
	assert(cluster == 0U);

	return (uintptr_t)mmio_read_32(per_cpu(cluster, cpu, MCUCFG_BOOTADDR));
}

void mcucfg_init_archstate(unsigned int cluster, unsigned int cpu, bool arm64)
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

/*
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
	return (mmio_read_32(SPM_CPU_PWR_STATUS) & mask);
}

bool spm_get_cluster_powerstate(unsigned int cluster)
{
	assert(cluster == 0U);

	return spm_get_powerstate(MP0_CPUTOP);
}

bool spm_get_cpu_powerstate(unsigned int cluster, unsigned int cpu)
{
	uint32_t mask = BIT(cpu);

	assert(cluster == 0U);

	return spm_get_powerstate(mask);
}

int spmc_init(void)
{
	unsigned int cpu = plat_my_core_pos();


	INFO("SPM: enable CPC mode\n");

	mmio_write_32(SPM_POWERON_CONFIG_EN, PROJECT_CODE | BCLK_CG_EN);

	mmio_setbits_32(per_cpu(0, 1, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 2, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 3, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 4, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 5, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 6, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 7, SPM_CPU_PWR), PWR_RST_B);

	mmio_clrbits_32(SPM_MCUSYS_PWR_CON, RESETPWRON_CONFIG);
	mmio_clrbits_32(SPM_MP0_CPUTOP_PWR_CON, RESETPWRON_CONFIG);
	mmio_clrbits_32(per_cpu(0, 0, SPM_CPU_PWR), RESETPWRON_CONFIG);

	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, CPC_CTRL_ENABLE);

	/* Clear bootup cpu's cpc sw hint */
	mmio_write_32(CPC_MCUSYS_CPU_ON_SW_HINT_CLR, BIT(cpu));

	return 0;
}

/*
 * Power on a core with specified cluster and core index
 *
 * @cluster: the cluster ID of the CPU which to be powered on
 * @cpu: the CPU ID of the CPU which to be powered on
 */
void spm_poweron_cpu(unsigned int cluster, unsigned int cpu)
{
	/* info CPC that CPU hotplug on */
	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, SSPM_ALL_PWR_CTRL_EN);

	/* Set mp0_spmc_pwr_on_cpuX = 1 */
	mmio_setbits_32(per_cpu(cluster, cpu, SPM_CPU_PWR), PWR_ON);

	/* wait for power on ack */
	while (!spm_get_cpu_powerstate(cluster, cpu))
		;

	/* info CPC that CPU hotplug off */
	mmio_clrbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, SSPM_ALL_PWR_CTRL_EN);
}

/*
 * Power off a core with specified cluster and core index
 *
 * @cluster: the cluster ID of the CPU which to be powered off
 * @cpu: the CPU ID of the CPU which to be powered off
 */
void spm_poweroff_cpu(unsigned int cluster, unsigned int cpu)
{
	/* Set mp0_spmc_pwr_on_cpuX = 0 */
	mmio_clrbits_32(per_cpu(cluster, cpu, SPM_CPU_PWR), PWR_ON);
}

/*
 * Power off a cluster with specified index
 *
 * @cluster: the cluster index which to be powered off
 */
void spm_poweroff_cluster(unsigned int cluster)
{
	/* No need to power on/off cluster on single cluster platform */
	assert(false);
}

/*
 * Power on a cluster with specified index
 *
 * @cluster: the cluster index which to be powered on
 */
void spm_poweron_cluster(unsigned int cluster)
{
	/* No need to power on/off cluster on single cluster platform */
	assert(false);
}
