/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bakery_lock.h>
#include <debug.h>
#include <delay_timer.h>
#include <errno.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <plat_private.h>
#include <rk3399_def.h>
#include <pmu_sram.h>
#include <soc.h>
#include <pmu.h>
#include <pmu_com.h>

static struct psram_data_t *psram_sleep_cfg =
	(struct psram_data_t *)PSRAM_DT_BASE;

/*
 * There are two ways to powering on or off on core.
 * 1) Control it power domain into on or off in PMU_PWRDN_CON reg,
 *    it is core_pwr_pd mode
 * 2) Enable the core power manage in PMU_CORE_PM_CON reg,
 *     then, if the core enter into wfi, it power domain will be
 *     powered off automatically. it is core_pwr_wfi or core_pwr_wfi_int mode
 * so we need core_pm_cfg_info to distinguish which method be used now.
 */

static uint32_t core_pm_cfg_info[PLATFORM_CORE_COUNT]
#if USE_COHERENT_MEM
__attribute__ ((section("tzfw_coherent_mem")))
#endif
;/* coheront */

void plat_rockchip_pmusram_prepare(void)
{
	uint32_t *sram_dst, *sram_src;
	size_t sram_size = 2;

	/*
	 * pmu sram code and data prepare
	 */
	sram_dst = (uint32_t *)PMUSRAM_BASE;
	sram_src = (uint32_t *)&pmu_cpuson_entrypoint_start;
	sram_size = (uint32_t *)&pmu_cpuson_entrypoint_end -
		    (uint32_t *)sram_src;

	u32_align_cpy(sram_dst, sram_src, sram_size);

	psram_sleep_cfg->sp = PSRAM_DT_BASE;
}

static inline uint32_t get_cpus_pwr_domain_cfg_info(uint32_t cpu_id)
{
	return core_pm_cfg_info[cpu_id];
}

static inline void set_cpus_pwr_domain_cfg_info(uint32_t cpu_id, uint32_t value)
{
	core_pm_cfg_info[cpu_id] = value;
#if !USE_COHERENT_MEM
	flush_dcache_range((uintptr_t)&core_pm_cfg_info[cpu_id],
			   sizeof(uint32_t));
#endif
}

static int cpus_power_domain_on(uint32_t cpu_id)
{
	uint32_t cfg_info;
	uint32_t cpu_pd = PD_CPUL0 + cpu_id;
	/*
	  * There are two ways to powering on or off on core.
	  * 1) Control it power domain into on or off in PMU_PWRDN_CON reg
	  * 2) Enable the core power manage in PMU_CORE_PM_CON reg,
	  *     then, if the core enter into wfi, it power domain will be
	  *     powered off automatically.
	  */

	cfg_info = get_cpus_pwr_domain_cfg_info(cpu_id);

	if (cfg_info == core_pwr_pd) {
		/* disable core_pm cfg */
		mmio_write_32(PMU_BASE + PMU_CORE_PM_CON(cpu_id),
			      CORES_PM_DISABLE);
		/* if the cores have be on, power off it firstly */
		if (pmu_power_domain_st(cpu_pd) == pmu_pd_on) {
			mmio_write_32(PMU_BASE + PMU_CORE_PM_CON(cpu_id), 0);
			pmu_power_domain_ctr(cpu_pd, pmu_pd_off);
		}

		pmu_power_domain_ctr(cpu_pd, pmu_pd_on);
	} else {
		if (pmu_power_domain_st(cpu_pd) == pmu_pd_on) {
			WARN("%s: cpu%d is not in off,!\n", __func__, cpu_id);
			return -EINVAL;
		}

		mmio_write_32(PMU_BASE + PMU_CORE_PM_CON(cpu_id),
			      BIT(core_pm_sft_wakeup_en));
	}

	return 0;
}

static int cpus_power_domain_off(uint32_t cpu_id, uint32_t pd_cfg)
{
	uint32_t cpu_pd;
	uint32_t core_pm_value;

	cpu_pd = PD_CPUL0 + cpu_id;
	if (pmu_power_domain_st(cpu_pd) == pmu_pd_off)
		return 0;

	if (pd_cfg == core_pwr_pd) {
		if (check_cpu_wfie(cpu_id, CKECK_WFEI_MSK))
			return -EINVAL;

		/* disable core_pm cfg */
		mmio_write_32(PMU_BASE + PMU_CORE_PM_CON(cpu_id),
			      CORES_PM_DISABLE);

		set_cpus_pwr_domain_cfg_info(cpu_id, pd_cfg);
		pmu_power_domain_ctr(cpu_pd, pmu_pd_off);
	} else {
		set_cpus_pwr_domain_cfg_info(cpu_id, pd_cfg);

		core_pm_value = BIT(core_pm_en);
		if (pd_cfg == core_pwr_wfi_int)
			core_pm_value |= BIT(core_pm_int_wakeup_en);
		mmio_write_32(PMU_BASE + PMU_CORE_PM_CON(cpu_id),
			      core_pm_value);
	}

	return 0;
}

static void nonboot_cpus_off(void)
{
	uint32_t boot_cpu, cpu;

	boot_cpu = plat_my_core_pos();

	/* turn off noboot cpus */
	for (cpu = 0; cpu < PLATFORM_CORE_COUNT; cpu++) {
		if (cpu == boot_cpu)
			continue;
		cpus_power_domain_off(cpu, core_pwr_pd);
	}
}

static int cores_pwr_domain_on(unsigned long mpidr, uint64_t entrypoint)
{
	uint32_t cpu_id = plat_core_pos_by_mpidr(mpidr);

	assert(cpuson_flags[cpu_id] == 0);
	cpuson_flags[cpu_id] = PMU_CPU_HOTPLUG;
	cpuson_entry_point[cpu_id] = entrypoint;
	dsb();

	cpus_power_domain_on(cpu_id);

	return 0;
}

static int cores_pwr_domain_off(void)
{
	uint32_t cpu_id = plat_my_core_pos();

	cpus_power_domain_off(cpu_id, core_pwr_wfi);

	return 0;
}

static int cores_pwr_domain_suspend(void)
{
	uint32_t cpu_id = plat_my_core_pos();

	assert(cpuson_flags[cpu_id] == 0);
	cpuson_flags[cpu_id] = PMU_CPU_AUTO_PWRDN;
	cpuson_entry_point[cpu_id] = (uintptr_t)psci_entrypoint;
	dsb();

	cpus_power_domain_off(cpu_id, core_pwr_wfi_int);

	return 0;
}

static int cores_pwr_domain_on_finish(void)
{
	uint32_t cpu_id = plat_my_core_pos();

	cpuson_flags[cpu_id] = 0;
	cpuson_entry_point[cpu_id] = 0;

	/* Disable core_pm */
	mmio_write_32(PMU_BASE + PMU_CORE_PM_CON(cpu_id), CORES_PM_DISABLE);

	return 0;
}

static int cores_pwr_domain_resume(void)
{
	uint32_t cpu_id = plat_my_core_pos();

	cpuson_flags[cpu_id] = 0;
	cpuson_entry_point[cpu_id] = 0;

	/* Disable core_pm */
	mmio_write_32(PMU_BASE + PMU_CORE_PM_CON(cpu_id), CORES_PM_DISABLE);

	return 0;
}

static void sys_slp_config(void)
{
	uint32_t slp_mode_cfg = 0;

	slp_mode_cfg = PMU_PWR_MODE_EN |
				   PMU_CPU0_PD_EN |
				   PMU_L2_FLUSH_EN |
				   PMU_L2_IDLE_EN |
				   PMU_SCU_PD_EN |
				   PMU_CLK_CORE_SRC_GATE_EN;
	mmio_setbits_32(PMU_BASE + PMU_WKUP_CFG4, PMU_CLUSTER_L_WKUP_EN);
	mmio_setbits_32(PMU_BASE + PMU_WKUP_CFG4, PMU_CLUSTER_B_WKUP_EN);
	mmio_clrbits_32(PMU_BASE + PMU_WKUP_CFG4, PMU_GPIO_WKUP_EN);
	mmio_write_32(PMU_BASE + PMU_PWRMODE_CON, slp_mode_cfg);
}

static int sys_pwr_domain_suspend(void)
{
	sys_slp_config();
	plls_suspend();
	psram_sleep_cfg->sys_mode = PMU_SYS_SLP_MODE;
	pmu_sgrf_rst_hld();
	return 0;
}

static int sys_pwr_domain_resume(void)
{
	pmu_sgrf_rst_hld_release();
	psram_sleep_cfg->sys_mode = PMU_SYS_ON_MODE;
	plls_resume();

	return 0;
}

static struct rockchip_pm_ops_cb pm_ops = {
	.cores_pwr_dm_on = cores_pwr_domain_on,
	.cores_pwr_dm_off = cores_pwr_domain_off,
	.cores_pwr_dm_on_finish = cores_pwr_domain_on_finish,
	.cores_pwr_dm_suspend = cores_pwr_domain_suspend,
	.cores_pwr_dm_resume = cores_pwr_domain_resume,
	.sys_pwr_dm_suspend = sys_pwr_domain_suspend,
	.sys_pwr_dm_resume = sys_pwr_domain_resume,
	.sys_gbl_soft_reset = soc_global_soft_reset,
};

void plat_rockchip_pmu_init(void)
{
	uint32_t cpu;

	rockchip_pd_lock_init();
	plat_setup_rockchip_pm_ops(&pm_ops);

	for (cpu = 0; cpu < PLATFORM_CORE_COUNT; cpu++)
		cpuson_flags[cpu] = 0;

	psram_sleep_cfg->sys_mode = PMU_SYS_ON_MODE;

	psram_sleep_cfg->boot_mpidr = read_mpidr_el1() & 0xffff;

	/* cpu boot from pmusram */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON0_1(1),
		      (PMUSRAM_BASE >> CPU_BOOT_ADDR_ALIGN) |
		      CPU_BOOT_ADDR_WMASK);

	nonboot_cpus_off();
	INFO("%s(%d): pd status %x\n", __func__, __LINE__,
	     mmio_read_32(PMU_BASE + PMU_PWRDN_ST));
}
