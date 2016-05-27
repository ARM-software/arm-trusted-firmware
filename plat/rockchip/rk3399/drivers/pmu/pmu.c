/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bakery_lock.h>
#include <debug.h>
#include <delay_timer.h>
#include <errno.h>
#include <gpio.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <plat_params.h>
#include <plat_private.h>
#include <rk3399_def.h>
#include <pmu_sram.h>
#include <soc.h>
#include <pmu.h>
#include <pmu_com.h>

static struct psram_data_t *psram_sleep_cfg =
	(struct psram_data_t *)PSRAM_DT_BASE;

static uint32_t cpu_warm_boot_addr;

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

void rk3399_flash_l2_b(void)
{
	uint32_t wait_cnt = 0;

	mmio_setbits_32(PMU_BASE + PMU_SFT_CON, BIT(L2_FLUSH_REQ_CLUSTER_B));
	dsb();

	while (!(mmio_read_32(PMU_BASE + PMU_CORE_PWR_ST) &
		 BIT(L2_FLUSHDONE_CLUSTER_B))) {
		wait_cnt++;
		if (!(wait_cnt % MAX_WAIT_CONUT))
			WARN("%s:reg %x,wait\n", __func__,
			     mmio_read_32(PMU_BASE + PMU_CORE_PWR_ST));
	}

	mmio_clrbits_32(PMU_BASE + PMU_SFT_CON, BIT(L2_FLUSH_REQ_CLUSTER_B));
}

static void pmu_scu_b_pwrdn(void)
{
	uint32_t wait_cnt = 0;

	if ((mmio_read_32(PMU_BASE + PMU_PWRDN_ST) &
	     (BIT(PMU_A72_B0_PWRDWN_ST) | BIT(PMU_A72_B1_PWRDWN_ST))) !=
	     (BIT(PMU_A72_B0_PWRDWN_ST) | BIT(PMU_A72_B1_PWRDWN_ST))) {
		ERROR("%s: not all cpus is off\n", __func__);
		return;
	}

	rk3399_flash_l2_b();

	mmio_setbits_32(PMU_BASE + PMU_SFT_CON, BIT(ACINACTM_CLUSTER_B_CFG));

	while (!(mmio_read_32(PMU_BASE + PMU_CORE_PWR_ST) &
		 BIT(STANDBY_BY_WFIL2_CLUSTER_B))) {
		wait_cnt++;
		if (!(wait_cnt % MAX_WAIT_CONUT))
			ERROR("%s:wait cluster-b l2(%x)\n", __func__,
			      mmio_read_32(PMU_BASE + PMU_CORE_PWR_ST));
	}
}

static void pmu_scu_b_pwrup(void)
{
	mmio_clrbits_32(PMU_BASE + PMU_SFT_CON, BIT(ACINACTM_CLUSTER_B_CFG));
}

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
	assert(cpu_id < PLATFORM_CORE_COUNT);
	return core_pm_cfg_info[cpu_id];
}

static inline void set_cpus_pwr_domain_cfg_info(uint32_t cpu_id, uint32_t value)
{
	assert(cpu_id < PLATFORM_CORE_COUNT);
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
		dsb();
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
		dsb();
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

	assert(cpu_id < PLATFORM_CORE_COUNT);
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

	assert(cpu_id < PLATFORM_CORE_COUNT);
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

	/* Disable core_pm */
	mmio_write_32(PMU_BASE + PMU_CORE_PM_CON(cpu_id), CORES_PM_DISABLE);

	return 0;
}

static int cores_pwr_domain_resume(void)
{
	uint32_t cpu_id = plat_my_core_pos();

	/* Disable core_pm */
	mmio_write_32(PMU_BASE + PMU_CORE_PM_CON(cpu_id), CORES_PM_DISABLE);

	return 0;
}

static void sys_slp_config(void)
{
	uint32_t slp_mode_cfg = 0;

	mmio_write_32(PMU_BASE + PMU_CCI500_CON,
		      BIT_WITH_WMSK(PMU_CLR_PREQ_CCI500_HW) |
		      BIT_WITH_WMSK(PMU_CLR_QREQ_CCI500_HW) |
		      BIT_WITH_WMSK(PMU_QGATING_CCI500_CFG));

	mmio_write_32(PMU_BASE + PMU_ADB400_CON,
		      BIT_WITH_WMSK(PMU_CLR_CORE_L_HW) |
		      BIT_WITH_WMSK(PMU_CLR_CORE_L_2GIC_HW) |
		      BIT_WITH_WMSK(PMU_CLR_GIC2_CORE_L_HW));

	mmio_write_32(PMUGRF_BASE + PMUGRF_GPIO1A_IOMUX,
		      BIT_WITH_WMSK(AP_PWROFF));

	slp_mode_cfg = BIT(PMU_PWR_MODE_EN) |
		       BIT(PMU_POWER_OFF_REQ_CFG) |
		       BIT(PMU_CPU0_PD_EN) |
		       BIT(PMU_L2_FLUSH_EN) |
		       BIT(PMU_L2_IDLE_EN) |
		       BIT(PMU_SCU_PD_EN);

	mmio_setbits_32(PMU_BASE + PMU_WKUP_CFG4, PMU_CLUSTER_L_WKUP_EN);
	mmio_setbits_32(PMU_BASE + PMU_WKUP_CFG4, PMU_CLUSTER_B_WKUP_EN);
	mmio_clrbits_32(PMU_BASE + PMU_WKUP_CFG4, PMU_GPIO_WKUP_EN);

	mmio_write_32(PMU_BASE + PMU_PWRMODE_CON, slp_mode_cfg);

	mmio_write_32(PMU_BASE + PMU_STABLE_CNT, CYCL_24M_CNT_MS(5));
	mmio_write_32(PMU_BASE + PMU_SCU_L_PWRDN_CNT, CYCL_24M_CNT_MS(2));
	mmio_write_32(PMU_BASE + PMU_SCU_L_PWRUP_CNT, CYCL_24M_CNT_MS(2));
	mmio_write_32(PMU_BASE + PMU_SCU_B_PWRDN_CNT, CYCL_24M_CNT_MS(2));
	mmio_write_32(PMU_BASE + PMU_SCU_B_PWRUP_CNT, CYCL_24M_CNT_MS(2));
}

static int sys_pwr_domain_suspend(void)
{
	sys_slp_config();
	plls_suspend();
	pmu_sgrf_rst_hld();

	mmio_write_32(SGRF_BASE + SGRF_SOC_CON0_1(1),
		      (PMUSRAM_BASE >> CPU_BOOT_ADDR_ALIGN) |
		      CPU_BOOT_ADDR_WMASK);

	pmu_scu_b_pwrdn();

	mmio_write_32(PMU_BASE + PMU_ADB400_CON,
		      BIT_WITH_WMSK(PMU_PWRDWN_REQ_CORE_B_2GIC_SW) |
		      BIT_WITH_WMSK(PMU_PWRDWN_REQ_CORE_B_SW) |
		      BIT_WITH_WMSK(PMU_PWRDWN_REQ_GIC2_CORE_B_SW));
	dsb();
	mmio_setbits_32(PMU_BASE + PMU_PWRDN_CON, BIT(PMU_SCU_B_PWRDWN_EN));

	return 0;
}

static int sys_pwr_domain_resume(void)
{
	pmu_sgrf_rst_hld();

	mmio_write_32(SGRF_BASE + SGRF_SOC_CON0_1(1),
		      (cpu_warm_boot_addr >> CPU_BOOT_ADDR_ALIGN) |
		      CPU_BOOT_ADDR_WMASK);

	plls_resume();

	mmio_write_32(PMU_BASE + PMU_CCI500_CON,
		      WMSK_BIT(PMU_CLR_PREQ_CCI500_HW) |
		      WMSK_BIT(PMU_CLR_QREQ_CCI500_HW) |
		      WMSK_BIT(PMU_QGATING_CCI500_CFG));

	mmio_write_32(PMU_BASE + PMU_ADB400_CON,
		      WMSK_BIT(PMU_CLR_CORE_L_HW) |
		      WMSK_BIT(PMU_CLR_CORE_L_2GIC_HW) |
		      WMSK_BIT(PMU_CLR_GIC2_CORE_L_HW));

	mmio_clrbits_32(PMU_BASE + PMU_PWRDN_CON,
			BIT(PMU_SCU_B_PWRDWN_EN));

	mmio_write_32(PMU_BASE + PMU_ADB400_CON,
		      WMSK_BIT(PMU_PWRDWN_REQ_CORE_B_2GIC_SW) |
		      WMSK_BIT(PMU_PWRDWN_REQ_CORE_B_SW) |
		      WMSK_BIT(PMU_PWRDWN_REQ_GIC2_CORE_B_SW));

	pmu_scu_b_pwrup();

	plat_rockchip_gic_cpuif_enable();
	return 0;
}

void __dead2 soc_soft_reset(void)
{
	struct gpio_info *rst_gpio;

	rst_gpio = (struct gpio_info *)plat_get_rockchip_gpio_reset();

	if (rst_gpio) {
		gpio_set_direction(rst_gpio->index, GPIO_DIR_OUT);
		gpio_set_value(rst_gpio->index, rst_gpio->polarity);
	} else {
		soc_global_soft_reset();
	}

	while (1)
		;
}

void __dead2 soc_system_off(void)
{
	struct gpio_info *poweroff_gpio;

	poweroff_gpio = (struct gpio_info *)plat_get_rockchip_gpio_poweroff();

	if (poweroff_gpio) {
		/*
		 * if use tsadc over temp pin(GPIO1A6) as shutdown gpio,
		 * need to set this pin iomux back to gpio function
		 */
		if (poweroff_gpio->index == TSADC_INT_PIN) {
			mmio_write_32(PMUGRF_BASE + PMUGRF_GPIO1A_IOMUX,
				      GPIO1A6_IOMUX);
		}
		gpio_set_direction(poweroff_gpio->index, GPIO_DIR_OUT);
		gpio_set_value(poweroff_gpio->index, poweroff_gpio->polarity);
	} else {
		WARN("Do nothing when system off\n");
	}

	while (1)
		;
}

static struct rockchip_pm_ops_cb pm_ops = {
	.cores_pwr_dm_on = cores_pwr_domain_on,
	.cores_pwr_dm_off = cores_pwr_domain_off,
	.cores_pwr_dm_on_finish = cores_pwr_domain_on_finish,
	.cores_pwr_dm_suspend = cores_pwr_domain_suspend,
	.cores_pwr_dm_resume = cores_pwr_domain_resume,
	.sys_pwr_dm_suspend = sys_pwr_domain_suspend,
	.sys_pwr_dm_resume = sys_pwr_domain_resume,
	.sys_gbl_soft_reset = soc_soft_reset,
	.system_off = soc_system_off,
};

void plat_rockchip_pmu_init(void)
{
	uint32_t cpu;

	rockchip_pd_lock_init();
	plat_setup_rockchip_pm_ops(&pm_ops);

	/* register requires 32bits mode, switch it to 32 bits */
	cpu_warm_boot_addr = (uint64_t)platform_cpu_warmboot;

	for (cpu = 0; cpu < PLATFORM_CORE_COUNT; cpu++)
		cpuson_flags[cpu] = 0;

	psram_sleep_cfg->ddr_func = 0x00;
	psram_sleep_cfg->ddr_data = 0x00;
	psram_sleep_cfg->ddr_flag = 0x00;
	psram_sleep_cfg->boot_mpidr = read_mpidr_el1() & 0xffff;

	/* cpu boot from pmusram */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON0_1(1),
		      (cpu_warm_boot_addr >> CPU_BOOT_ADDR_ALIGN) |
		      CPU_BOOT_ADDR_WMASK);

	nonboot_cpus_off();

	INFO("%s(%d): pd status %x\n", __func__, __LINE__,
	     mmio_read_32(PMU_BASE + PMU_PWRDN_ST));
}
