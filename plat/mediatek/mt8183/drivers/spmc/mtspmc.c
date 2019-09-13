/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <arch_helpers.h>
#include <cortex_a53.h>
#include <cortex_a73.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <platform_def.h>
#include <mcucfg.h>
#include <spm.h>
#include <drivers/delay_timer.h>
#include <mtspmc.h>

#include "mtspmc_private.h"


static void set_retention(int cluster, int tick)
{
	uint64_t cpuectlr;

	if (cluster)
		cpuectlr = read_a73_cpuectlr_el1();
	else
		cpuectlr = read_a53_cpuectlr_el1();

	cpuectlr &= ~0x7ULL;
	cpuectlr |= tick & 0x7;

	if (cluster)
		write_a73_cpuectlr_el1(cpuectlr);
	else
		write_a53_cpuectlr_el1(cpuectlr);
}

void spm_enable_cpu_auto_off(int cluster, int cpu)
{
	uintptr_t reg = per_cpu(cluster, cpu, MCUCFG_SPARK);

	set_retention(cluster, 1);
	mmio_clrbits_32(reg, SW_NO_WAIT_Q);
}

void spm_disable_cpu_auto_off(int cluster, int cpu)
{
	uintptr_t reg = per_cpu(cluster, cpu, MCUCFG_SPARK);

	mmio_setbits_32(reg, SW_NO_WAIT_Q);
	set_retention(cluster, 0);
}

void spm_set_cpu_power_off(int cluster, int cpu)
{
	mmio_clrbits_32(per_cpu(cluster, cpu, SPM_CPU_PWR), PWRCTRL_PWR_ON);
}

void spm_enable_cluster_auto_off(int cluster)
{
	assert(cluster);

	mmio_clrbits_32(MCUCFG_MP2_SPMC, SW_NO_WAIT_Q);
	mmio_clrbits_32(MCUCFG_MP2_COQ, BIT(0));

	mmio_clrbits_32(SPM_SPMC_DORMANT_ENABLE, MP1_SPMC_SRAM_DORMANT_EN);

	mmio_clrbits_32(per_cluster(cluster, SPM_CLUSTER_PWR), PWRCTRL_PWR_ON);
}

void mcucfg_set_bootaddr(int cluster, int cpu, uintptr_t bootaddr)
{
	uintptr_t reg;
	const uintptr_t mp2_bootreg[] = {
			MCUCFG_MP2_RVADDR0, MCUCFG_MP2_RVADDR1,
			MCUCFG_MP2_RVADDR2, MCUCFG_MP2_RVADDR3 };

	if (cluster) {
		assert(cpu >= 0 && cpu < 4);
		reg = mp2_bootreg[cpu];
	} else {
		reg = per_cpu(cluster, cpu, MCUCFG_BOOTADDR);
	}

	mmio_write_32(reg, bootaddr);
}

uintptr_t mcucfg_get_bootaddr(int cluster, int cpu)
{
	uintptr_t reg;
	const uintptr_t mp2_bootreg[] = {
			MCUCFG_MP2_RVADDR0, MCUCFG_MP2_RVADDR1,
			MCUCFG_MP2_RVADDR2, MCUCFG_MP2_RVADDR3 };

	if (cluster) {
		assert(cpu >= 0 && cpu < 4);
		reg = mp2_bootreg[cpu];
	} else {
		reg = per_cpu(cluster, cpu, MCUCFG_BOOTADDR);
	}

	return mmio_read_32(reg);
}

void mcucfg_init_archstate(int cluster, int cpu, int arm64)
{
	uintptr_t reg;
	int i;

	reg = per_cluster(cluster, MCUCFG_INITARCH);
	i = cluster ? 16 : 12;

	mmio_setbits_32(reg, (arm64 & 1) << (i + cpu));
}

/**
 * Return power state of specified subsystem
 *
 * @mask: mask to SPM_PWR_STATUS to query the power state
 *        of one subsystem.
 * RETURNS:
 * 0 (the subsys was powered off)
 * 1 (the subsys was powered on)
 */
int spm_get_powerstate(uint32_t mask)
{
	return mmio_read_32(SPM_PWR_STATUS) & mask;
}

int spm_get_cluster_powerstate(int cluster)
{
	uint32_t mask;

	mask = cluster ? PWR_STATUS_MP1_CPUTOP : PWR_STATUS_MP0_CPUTOP;

	return spm_get_powerstate(mask);
}

int spm_get_cpu_powerstate(int cluster, int cpu)
{
	uint32_t i;

	/*
	 * a quick way to specify the mask of cpu[0-3]/cpu[4-7] in PWR_STATUS
	 * register which are the BITS[9:12](MP0_CPU0~3) and
	 * BITS[16:19](MP1_CPU0~3)
	 */
	i = (cluster) ? 16 : 9;
	i = 1 << (i + cpu);

	return spm_get_powerstate(i);
}

int spmc_init(void)
{
	/* enable SPM register control */
	mmio_write_32(SPM_POWERON_CONFIG_EN,
		      PROJECT_CODE | MD_BCLK_CG_EN | BCLK_CG_EN);

#if SPMC_MODE == 1
	INFO("SPM: enable SPMC mode\n");

	/* 0: SPMC mode  1: Legacy mode */
	mmio_write_32(SPM_BYPASS_SPMC, 0);

	mmio_clrbits_32(per_cluster(0, SPM_CLUSTER_PWR), PWRCTRL_PWR_ON_2ND);

	mmio_clrbits_32(per_cpu(0, 0, SPM_CPU_PWR), PWRCTRL_PWR_ON_2ND);
	mmio_clrbits_32(per_cpu(0, 1, SPM_CPU_PWR), PWRCTRL_PWR_ON_2ND);
	mmio_clrbits_32(per_cpu(0, 2, SPM_CPU_PWR), PWRCTRL_PWR_ON_2ND);
	mmio_clrbits_32(per_cpu(0, 3, SPM_CPU_PWR), PWRCTRL_PWR_ON_2ND);

	mmio_setbits_32(per_cpu(0, 1, SPM_CPU_PWR), PWRCTRL_PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 2, SPM_CPU_PWR), PWRCTRL_PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 3, SPM_CPU_PWR), PWRCTRL_PWR_RST_B);
#endif

	mmio_clrbits_32(per_cluster(1, SPM_CLUSTER_PWR), PWRCTRL_PWR_ON_2ND);
	mmio_setbits_32(per_cluster(1, SPM_CLUSTER_PWR), PWRCTRL_PWR_RST_B);
	mmio_clrbits_32(per_cluster(1, SPM_CLUSTER_PWR), PWRCTRL_PWR_CLK_DIS);

	mmio_clrbits_32(per_cpu(1, 0, SPM_CPU_PWR), PWRCTRL_PWR_ON_2ND);
	mmio_clrbits_32(per_cpu(1, 1, SPM_CPU_PWR), PWRCTRL_PWR_ON_2ND);
	mmio_clrbits_32(per_cpu(1, 2, SPM_CPU_PWR), PWRCTRL_PWR_ON_2ND);
	mmio_clrbits_32(per_cpu(1, 3, SPM_CPU_PWR), PWRCTRL_PWR_ON_2ND);

	mmio_setbits_32(per_cpu(1, 0, SPM_CPU_PWR), PWRCTRL_PWR_RST_B);
	mmio_setbits_32(per_cpu(1, 1, SPM_CPU_PWR), PWRCTRL_PWR_RST_B);
	mmio_setbits_32(per_cpu(1, 2, SPM_CPU_PWR), PWRCTRL_PWR_RST_B);
	mmio_setbits_32(per_cpu(1, 3, SPM_CPU_PWR), PWRCTRL_PWR_RST_B);

	return 0;
}

/**
 * Power on a core with specified cluster and core index
 *
 * @cluster: the cluster ID of the CPU which to be powered on
 * @cpu: the CPU ID of the CPU which to be powered on
 */
void spm_poweron_cpu(int cluster, int cpu)
{
	INFO("spmc: power on core %d.%d\n", cluster, cpu);

	/* STA_POWER_ON */
	/* Start to turn on MP0_CPU0 */

	/* Set PWR_RST_B = 1 */
	mmio_setbits_32(per_cpu(cluster, cpu, SPM_CPU_PWR), PWRCTRL_PWR_RST_B);

	/* Set PWR_ON = 1 */
	mmio_setbits_32(per_cpu(cluster, cpu, SPM_CPU_PWR), PWRCTRL_PWR_ON);

	/* Wait until MP0_CPU0_PWR_STA_MASK = 1 */
	while (!spm_get_cpu_powerstate(cluster, cpu))
		;

	/* Finish to turn on MP0_CPU0 */
	INFO("spmc: power on core %d.%d successfully\n", cluster, cpu);
}

/**
 * Power off a core with specified cluster and core index
 *
 * @cluster: the cluster ID of the CPU which to be powered off
 * @cpu: the CPU ID of the CPU which to be powered off
 */
void spm_poweroff_cpu(int cluster, int cpu)
{
	INFO("spmc: power off core %d.%d\n", cluster, cpu);

	/* Start to turn off MP0_CPU0 */
	/* Set PWR_ON_2ND = 0 */
	mmio_clrbits_32(per_cpu(cluster, cpu, SPM_CPU_PWR), PWRCTRL_PWR_ON_2ND);

	/* Set PWR_ON = 0 */
	mmio_clrbits_32(per_cpu(cluster, cpu, SPM_CPU_PWR), PWRCTRL_PWR_ON);

	/* Wait until MP0_CPU0_PWR_STA_MASK = 0 */
	while (spm_get_cpu_powerstate(cluster, cpu))
		;

	/* Set PWR_RST_B = 0 */
	mmio_clrbits_32(per_cpu(cluster, cpu, SPM_CPU_PWR), PWRCTRL_PWR_RST_B);

	/* Finish to turn off MP0_CPU0 */
	INFO("spmc: power off core %d.%d successfully\n", cluster, cpu);
}

/**
 * Power off a cluster with specified index
 *
 * @cluster: the cluster index which to be powered off
 */
void spm_poweroff_cluster(int cluster)
{
	uint32_t mask;
	uint32_t pwr_rst_ctl;

	INFO("spmc: power off cluster %d\n", cluster);

	/* Start to turn off MP0_CPUTOP */
	/* Set bus protect - step1 : 0 */
	mask = (cluster) ? MP1_CPUTOP_PROT_STEP1_0_MASK :
			   MP0_CPUTOP_PROT_STEP1_0_MASK;
	mmio_write_32(INFRA_TOPAXI_PROTECTEN_1_SET, mask);

	while ((mmio_read_32(INFRA_TOPAXI_PROTECTEN_STA1_1) & mask) != mask)
		;

	/* Set PWR_ON_2ND = 0 */
	mmio_clrbits_32(per_cluster(cluster, SPM_CLUSTER_PWR),
			PWRCTRL_PWR_ON_2ND);

	/* SPMC_DORMANT_ENABLE[0]=0 */
	mask = (cluster) ? MP1_SPMC_SRAM_DORMANT_EN : MP0_SPMC_SRAM_DORMANT_EN;
	mmio_clrbits_32(SPM_SPMC_DORMANT_ENABLE, mask);

	/* Set PWR_ON = 0" */
	mmio_clrbits_32(per_cluster(cluster, SPM_CLUSTER_PWR), PWRCTRL_PWR_ON);

	/* Wait until MP0_CPUTOP_PWR_STA_MASK = 0 */
	while (spm_get_cluster_powerstate(cluster))
		;

	/* NOTE
	 * Following flow only for BIG core cluster. It was from
	 * application note but not covered in mtcmos_ctrl.c
	 */
	if (cluster) {
		pwr_rst_ctl = mmio_read_32(MCUCFG_MP2_PWR_RST_CTL);
		mmio_write_32(MCUCFG_MP2_PWR_RST_CTL,
				(pwr_rst_ctl & ~SW_RST_B) | TOPAON_APB_MASK);
	}

	/* CPU_EXT_BUCK_ISO[0]=1 */
	if (cluster)
		mmio_setbits_32(SPM_CPU_EXT_BUCK_ISO, MP1_EXT_BUCK_ISO);

	/* Finish to turn off MP0_CPUTOP */
	INFO("spmc: power off cluster %d successfully\n", cluster);
}

/**
 * Power on a cluster with specified index
 *
 * @cluster: the cluster index which to be powered on
 */
void spm_poweron_cluster(int cluster)
{
	uint32_t mask;
	uint32_t pwr_rst_ctl;

	INFO("spmc: power on cluster %d\n", cluster);

	/* Start to turn on MP1_CPUTOP */

	/* NOTE
	 * Following flow only for BIG core cluster. It was from
	 * application note but not covered in mtcmos_ctrl.c
	 */
	if (cluster) {
		mmio_clrbits_32(MCUCFG_MP2_PWR_RST_CTL, SW_RST_B);

		/* CPU_EXT_BUCK_ISO[1]=0 */
		/* Set mp<n>_vproc_ext_off to 0 to release vproc isolation control */
		mmio_clrbits_32(SPM_CPU_EXT_BUCK_ISO, MP1_EXT_BUCK_ISO);

		/* NOTE
		 * Following flow only for BIG core cluster. It was from
		 * application note but not covered in mtcmos_ctrl.c
		 */
		pwr_rst_ctl = mmio_read_32(MCUCFG_MP2_PWR_RST_CTL);
		mmio_write_32(MCUCFG_MP2_PWR_RST_CTL,
				(pwr_rst_ctl | SW_RST_B) & ~TOPAON_APB_MASK);
	}

	/* Set PWR_ON_2ND = 0 */
	mmio_clrbits_32(per_cluster(cluster, SPM_CLUSTER_PWR),
			PWRCTRL_PWR_ON_2ND);

	/* Set PWR_RST_B = 1 */
	mmio_setbits_32(per_cluster(cluster, SPM_CLUSTER_PWR),
			PWRCTRL_PWR_RST_B);

	/* Set PWR_CLK_DIS = 0 */
	mmio_clrbits_32(per_cluster(cluster, SPM_CLUSTER_PWR),
			PWRCTRL_PWR_CLK_DIS);

	/* Set PWR_ON = 1 */
	mmio_setbits_32(per_cluster(cluster, SPM_CLUSTER_PWR), PWRCTRL_PWR_ON);

	/* Wait until MP1_CPUTOP_PWR_STA_MASK = 1 */
	while (!spm_get_cluster_powerstate(cluster))
		;

	/* Release bus protect - step1 : 0 */
	mask = (cluster) ? MP1_CPUTOP_PROT_STEP1_0_MASK :
			   MP0_CPUTOP_PROT_STEP1_0_MASK;
	mmio_write_32(INFRA_TOPAXI_PROTECTEN_1_CLR, mask);

	/* Finish to turn on MP1_CPUTOP */
	INFO("spmc: power on cluster %d successfully\n", cluster);
}
