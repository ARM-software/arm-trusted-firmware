/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include <ddr_rk3368.h>
#include <plat_private.h>
#include <pmu.h>
#include <pmu_com.h>
#include <rk3368_def.h>
#include <soc.h>

DEFINE_BAKERY_LOCK(rockchip_pd_lock);

static uint32_t cpu_warm_boot_addr;

void rk3368_flash_l2_b(void)
{
	uint32_t wait_cnt = 0;

	regs_updata_bit_set(PMU_BASE + PMU_SFT_CON, pmu_sft_l2flsh_clst_b);
	dsb();

	while (!(mmio_read_32(PMU_BASE + PMU_CORE_PWR_ST)
		& BIT(clst_b_l2_flsh_done))) {
		wait_cnt++;
		if (!(wait_cnt % MAX_WAIT_CONUT))
			WARN("%s:reg %x,wait\n", __func__,
			     mmio_read_32(PMU_BASE + PMU_CORE_PWR_ST));
	}

	regs_updata_bit_clr(PMU_BASE + PMU_SFT_CON, pmu_sft_l2flsh_clst_b);
}

static inline int rk3368_pmu_bus_idle(uint32_t req, uint32_t idle)
{
	uint32_t mask = BIT(req);
	uint32_t idle_mask = 0;
	uint32_t idle_target = 0;
	uint32_t val;
	uint32_t wait_cnt = 0;

	switch (req) {
	case bus_ide_req_clst_l:
		idle_mask = BIT(pmu_idle_ack_cluster_l);
		idle_target = (idle << pmu_idle_ack_cluster_l);
		break;

	case bus_ide_req_clst_b:
		idle_mask = BIT(pmu_idle_ack_cluster_b);
		idle_target = (idle << pmu_idle_ack_cluster_b);
		break;

	case bus_ide_req_cxcs:
		idle_mask = BIT(pmu_idle_ack_cxcs);
		idle_target = ((!idle) << pmu_idle_ack_cxcs);
		break;

	case bus_ide_req_cci400:
		idle_mask = BIT(pmu_idle_ack_cci400);
		idle_target = ((!idle) << pmu_idle_ack_cci400);
		break;

	case bus_ide_req_gpu:
		idle_mask = BIT(pmu_idle_ack_gpu) | BIT(pmu_idle_gpu);
		idle_target = (idle << pmu_idle_ack_gpu) |
			      (idle << pmu_idle_gpu);
		break;

	case bus_ide_req_core:
		idle_mask = BIT(pmu_idle_ack_core) | BIT(pmu_idle_core);
		idle_target = (idle << pmu_idle_ack_core) |
			      (idle << pmu_idle_core);
		break;

	case bus_ide_req_bus:
		idle_mask = BIT(pmu_idle_ack_bus) | BIT(pmu_idle_bus);
		idle_target = (idle << pmu_idle_ack_bus) |
			      (idle << pmu_idle_bus);
		break;
	case bus_ide_req_dma:
		idle_mask = BIT(pmu_idle_ack_dma) | BIT(pmu_idle_dma);
		idle_target = (idle << pmu_idle_ack_dma) |
			      (idle << pmu_idle_dma);
		break;

	case bus_ide_req_peri:
		idle_mask = BIT(pmu_idle_ack_peri) | BIT(pmu_idle_peri);
		idle_target = (idle << pmu_idle_ack_peri) |
			      (idle << pmu_idle_peri);
		break;

	case bus_ide_req_video:
		idle_mask = BIT(pmu_idle_ack_video) | BIT(pmu_idle_video);
		idle_target = (idle << pmu_idle_ack_video) |
			      (idle << pmu_idle_video);
		break;

	case bus_ide_req_vio:
		idle_mask = BIT(pmu_idle_ack_vio) | BIT(pmu_idle_vio);
		idle_target = (pmu_idle_ack_vio) |
			      (idle << pmu_idle_vio);
		break;

	case bus_ide_req_alive:
		idle_mask = BIT(pmu_idle_ack_alive) | BIT(pmu_idle_alive);
		idle_target = (idle << pmu_idle_ack_alive) |
			      (idle << pmu_idle_alive);
		break;

	case bus_ide_req_pmu:
		idle_mask = BIT(pmu_idle_ack_pmu) | BIT(pmu_idle_pmu);
		idle_target = (idle << pmu_idle_ack_pmu) |
			      (idle << pmu_idle_pmu);
		break;

	case bus_ide_req_msch:
		idle_mask = BIT(pmu_idle_ack_msch) | BIT(pmu_idle_msch);
		idle_target = (idle << pmu_idle_ack_msch) |
			      (idle << pmu_idle_msch);
		break;

	case bus_ide_req_cci:
		idle_mask = BIT(pmu_idle_ack_cci) | BIT(pmu_idle_cci);
		idle_target = (idle << pmu_idle_ack_cci) |
			      (idle << pmu_idle_cci);
		break;

	default:
		ERROR("%s: Unsupported the idle request\n", __func__);
		break;
	}

	val = mmio_read_32(PMU_BASE + PMU_BUS_IDE_REQ);
	if (idle)
		val |=	mask;
	else
		val &= ~mask;

	mmio_write_32(PMU_BASE + PMU_BUS_IDE_REQ, val);

	while ((mmio_read_32(PMU_BASE +
	       PMU_BUS_IDE_ST) & idle_mask) != idle_target) {
		wait_cnt++;
		if (!(wait_cnt % MAX_WAIT_CONUT))
			WARN("%s:st=%x(%x)\n", __func__,
			     mmio_read_32(PMU_BASE + PMU_BUS_IDE_ST),
			     idle_mask);
	}

	return 0;
}

void pmu_scu_b_pwrup(void)
{
	regs_updata_bit_clr(PMU_BASE + PMU_SFT_CON, pmu_sft_acinactm_clst_b);
	rk3368_pmu_bus_idle(bus_ide_req_clst_b, 0);
}

static void pmu_scu_b_pwrdn(void)
{
	uint32_t wait_cnt = 0;

	if ((mmio_read_32(PMU_BASE + PMU_PWRDN_ST) &
	     PM_PWRDM_CPUSB_MSK) != PM_PWRDM_CPUSB_MSK) {
		ERROR("%s: not all cpus is off\n", __func__);
		return;
	}

	rk3368_flash_l2_b();

	regs_updata_bit_set(PMU_BASE + PMU_SFT_CON, pmu_sft_acinactm_clst_b);

	while (!(mmio_read_32(PMU_BASE +
	       PMU_CORE_PWR_ST) & BIT(clst_b_l2_wfi))) {
		wait_cnt++;
		if (!(wait_cnt % MAX_WAIT_CONUT))
			ERROR("%s:wait cluster-b l2(%x)\n", __func__,
			      mmio_read_32(PMU_BASE + PMU_CORE_PWR_ST));
	}
	rk3368_pmu_bus_idle(bus_ide_req_clst_b, 1);
}

static void pmu_sleep_mode_config(void)
{
	uint32_t pwrmd_core, pwrmd_com;

	pwrmd_core = BIT(pmu_mdcr_cpu0_pd) |
		     BIT(pmu_mdcr_scu_l_pd) |
		     BIT(pmu_mdcr_l2_flush) |
		     BIT(pmu_mdcr_l2_idle) |
		     BIT(pmu_mdcr_clr_clst_l) |
		     BIT(pmu_mdcr_clr_core) |
		     BIT(pmu_mdcr_clr_cci) |
		     BIT(pmu_mdcr_core_pd);

	pwrmd_com = BIT(pmu_mode_en) |
		    BIT(pmu_mode_sref_enter) |
		    BIT(pmu_mode_pwr_off);

	regs_updata_bit_set(PMU_BASE + PMU_WKUP_CFG2, pmu_cluster_l_wkup_en);
	regs_updata_bit_set(PMU_BASE + PMU_WKUP_CFG2, pmu_cluster_b_wkup_en);
	regs_updata_bit_clr(PMU_BASE + PMU_WKUP_CFG2, pmu_gpio_wkup_en);

	mmio_write_32(PMU_BASE + PMU_PLLLOCK_CNT, CYCL_24M_CNT_MS(2));
	mmio_write_32(PMU_BASE + PMU_PLLRST_CNT, CYCL_24M_CNT_US(100));
	mmio_write_32(PMU_BASE + PMU_STABLE_CNT, CYCL_24M_CNT_MS(2));
	mmio_write_32(PMU_BASE + PMU_PWRMD_CORE, pwrmd_core);
	mmio_write_32(PMU_BASE + PMU_PWRMD_COM, pwrmd_com);
	dsb();
}

static void pmu_set_sleep_mode(void)
{
	pmu_sleep_mode_config();
	soc_sleep_config();
	regs_updata_bit_set(PMU_BASE + PMU_PWRMD_CORE, pmu_mdcr_global_int_dis);
	regs_updata_bit_set(PMU_BASE + PMU_SFT_CON, pmu_sft_glbl_int_dis_b);
	pmu_scu_b_pwrdn();
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(1),
		      ((uintptr_t)&pmu_cpuson_entrypoint >>
			CPU_BOOT_ADDR_ALIGN) | CPU_BOOT_ADDR_WMASK);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(2),
		      ((uintptr_t)&pmu_cpuson_entrypoint >>
			CPU_BOOT_ADDR_ALIGN) | CPU_BOOT_ADDR_WMASK);
}

static int cpus_id_power_domain(uint32_t cluster,
				uint32_t cpu,
				uint32_t pd_state,
				uint32_t wfie_msk)
{
	uint32_t pd;
	uint64_t mpidr;

	if (cluster)
		pd = PD_CPUB0 + cpu;
	else
		pd = PD_CPUL0 + cpu;

	if (pmu_power_domain_st(pd) == pd_state)
		return 0;

	if (pd_state == pmu_pd_off) {
		mpidr = (cluster << MPIDR_AFF1_SHIFT) | cpu;
		if (check_cpu_wfie(mpidr, wfie_msk))
			return -EINVAL;
	}

	return pmu_power_domain_ctr(pd, pd_state);
}

static void nonboot_cpus_off(void)
{
	uint32_t boot_cpu, boot_cluster, cpu;

	boot_cpu = MPIDR_AFFLVL0_VAL(read_mpidr_el1());
	boot_cluster = MPIDR_AFFLVL1_VAL(read_mpidr_el1());

	/* turn off noboot cpus */
	for (cpu = 0; cpu < PLATFORM_CLUSTER0_CORE_COUNT; cpu++) {
		if (!boot_cluster && (cpu == boot_cpu))
			continue;
		cpus_id_power_domain(0, cpu, pmu_pd_off, CKECK_WFEI_MSK);
	}

	for (cpu = 0; cpu < PLATFORM_CLUSTER1_CORE_COUNT; cpu++) {
		if (boot_cluster && (cpu == boot_cpu))
			continue;
		cpus_id_power_domain(1, cpu, pmu_pd_off, CKECK_WFEI_MSK);
	}
}

void sram_save(void)
{
	/* TODO: support the sdram save for rk3368 SoCs*/
}

void sram_restore(void)
{
	/* TODO: support the sdram restore for rk3368 SoCs */
}

int rockchip_soc_cores_pwr_dm_on(unsigned long mpidr, uint64_t entrypoint)
{
	uint32_t cpu, cluster;
	uint32_t cpuon_id;

	cpu = MPIDR_AFFLVL0_VAL(mpidr);
	cluster = MPIDR_AFFLVL1_VAL(mpidr);

	/* Make sure the cpu is off,Before power up the cpu! */
	cpus_id_power_domain(cluster, cpu, pmu_pd_off, CKECK_WFEI_MSK);

	cpuon_id = (cluster * PLATFORM_CLUSTER0_CORE_COUNT) + cpu;
	assert(cpuon_id < PLATFORM_CORE_COUNT);
	assert(cpuson_flags[cpuon_id] == 0);
	cpuson_flags[cpuon_id] = PMU_CPU_HOTPLUG;
	cpuson_entry_point[cpuon_id] = entrypoint;

	/* Switch boot addr to pmusram */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(1 + cluster),
		      (cpu_warm_boot_addr >> CPU_BOOT_ADDR_ALIGN) |
		      CPU_BOOT_ADDR_WMASK);
	dsb();

	cpus_id_power_domain(cluster, cpu, pmu_pd_on, CKECK_WFEI_MSK);

	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(1 + cluster),
		      (COLD_BOOT_BASE >> CPU_BOOT_ADDR_ALIGN) |
		      CPU_BOOT_ADDR_WMASK);

	return 0;
}

int rockchip_soc_cores_pwr_dm_on_finish(void)
{
	return 0;
}

int rockchip_soc_sys_pwr_dm_resume(void)
{
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(1),
		      (COLD_BOOT_BASE >> CPU_BOOT_ADDR_ALIGN) |
		      CPU_BOOT_ADDR_WMASK);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(2),
		      (COLD_BOOT_BASE >> CPU_BOOT_ADDR_ALIGN) |
		      CPU_BOOT_ADDR_WMASK);
	pm_plls_resume();
	pmu_scu_b_pwrup();

	return 0;
}

int rockchip_soc_sys_pwr_dm_suspend(void)
{
	nonboot_cpus_off();
	pmu_set_sleep_mode();

	return 0;
}

void rockchip_plat_mmu_el3(void)
{
	/* TODO: support the el3 for rk3368 SoCs */
}

void plat_rockchip_pmu_init(void)
{
	uint32_t cpu;

	/* register requires 32bits mode, switch it to 32 bits */
	cpu_warm_boot_addr = (uint64_t)platform_cpu_warmboot;

	for (cpu = 0; cpu < PLATFORM_CORE_COUNT; cpu++)
		cpuson_flags[cpu] = 0;

	nonboot_cpus_off();
	INFO("%s(%d): pd status %x\n", __func__, __LINE__,
	     mmio_read_32(PMU_BASE + PMU_PWRDN_ST));
}
