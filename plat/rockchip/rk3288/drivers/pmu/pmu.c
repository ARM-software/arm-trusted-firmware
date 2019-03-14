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

#include <plat_private.h>
#include <pmu.h>
#include <pmu_com.h>
#include <rk3288_def.h>
#include <secure.h>
#include <soc.h>

DEFINE_BAKERY_LOCK(rockchip_pd_lock);

static uint32_t cpu_warm_boot_addr;

static uint32_t store_pmu_pwrmode_con;
static uint32_t store_sgrf_soc_con0;
static uint32_t store_sgrf_cpu_con0;

/* These enum are variants of low power mode */
enum {
	ROCKCHIP_ARM_OFF_LOGIC_NORMAL = 0,
	ROCKCHIP_ARM_OFF_LOGIC_DEEP = 1,
};

static inline int rk3288_pmu_bus_idle(uint32_t req, uint32_t idle)
{
	uint32_t mask = BIT(req);
	uint32_t idle_mask = 0;
	uint32_t idle_target = 0;
	uint32_t val;
	uint32_t wait_cnt = 0;

	switch (req) {
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
	case bus_ide_req_cpup:
		idle_mask = BIT(pmu_idle_ack_cpup) | BIT(pmu_idle_cpup);
		idle_target = (idle << pmu_idle_ack_cpup) |
			      (idle << pmu_idle_cpup);
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
	case bus_ide_req_hevc:
		idle_mask = BIT(pmu_idle_ack_hevc) | BIT(pmu_idle_hevc);
		idle_target = (idle << pmu_idle_ack_hevc) |
			      (idle << pmu_idle_hevc);
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
	default:
		ERROR("%s: Unsupported the idle request\n", __func__);
		break;
	}

	val = mmio_read_32(PMU_BASE + PMU_BUS_IDE_REQ);
	if (idle)
		val |= mask;
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

static bool rk3288_sleep_disable_osc(void)
{
	static const uint32_t reg_offset[] = { GRF_UOC0_CON0, GRF_UOC1_CON0,
					       GRF_UOC2_CON0 };
	uint32_t reg, i;

	/*
	 * if any usb phy is still on(GRF_SIDDQ==0), that means we need the
	 * function of usb wakeup, so do not switch to 32khz, since the usb phy
	 * clk does not connect to 32khz osc
	 */
	for (i = 0; i < ARRAY_SIZE(reg_offset); i++) {
		reg = mmio_read_32(GRF_BASE + reg_offset[i]);
		if (!(reg & GRF_SIDDQ))
			return false;
	}

	return true;
}

static void pmu_set_sleep_mode(int level)
{
	uint32_t mode_set, mode_set1;
	bool osc_disable = rk3288_sleep_disable_osc();

	mode_set = BIT(pmu_mode_glb_int_dis) | BIT(pmu_mode_l2_flush_en) |
		   BIT(pmu_mode_sref0_enter) | BIT(pmu_mode_sref1_enter) |
		   BIT(pmu_mode_ddrc0_gt) | BIT(pmu_mode_ddrc1_gt) |
		   BIT(pmu_mode_en) | BIT(pmu_mode_chip_pd) |
		   BIT(pmu_mode_scu_pd);

	mode_set1 = BIT(pmu_mode_clr_core) | BIT(pmu_mode_clr_cpup);

	if (level == ROCKCHIP_ARM_OFF_LOGIC_DEEP) {
		/* arm off, logic deep sleep */
		mode_set |= BIT(pmu_mode_bus_pd) | BIT(pmu_mode_pmu_use_lf) |
			    BIT(pmu_mode_ddrio1_ret) |
			    BIT(pmu_mode_ddrio0_ret) |
			    BIT(pmu_mode_pmu_alive_use_lf) |
			    BIT(pmu_mode_pll_pd);

		if (osc_disable)
			mode_set |= BIT(pmu_mode_osc_dis);

		mode_set1 |= BIT(pmu_mode_clr_alive) | BIT(pmu_mode_clr_bus) |
			     BIT(pmu_mode_clr_peri) | BIT(pmu_mode_clr_dma);

		mmio_write_32(PMU_BASE + PMU_WAKEUP_CFG1,
			      pmu_armint_wakeup_en);

		/*
		 * In deep suspend we use PMU_PMU_USE_LF to let the rk3288
		 * switch its main clock supply to the alternative 32kHz
		 * source. Therefore set 30ms on a 32kHz clock for pmic
		 * stabilization. Similar 30ms on 24MHz for the other
		 * mode below.
		 */
		mmio_write_32(PMU_BASE + PMU_STABL_CNT, 32 * 30);

		/* only wait for stabilization, if we turned the osc off */
		mmio_write_32(PMU_BASE + PMU_OSC_CNT,
					 osc_disable ? 32 * 30 : 0);
	} else {
		/*
		 * arm off, logic normal
		 * if pmu_clk_core_src_gate_en is not set,
		 * wakeup will be error
		 */
		mode_set |= BIT(pmu_mode_core_src_gt);

		mmio_write_32(PMU_BASE + PMU_WAKEUP_CFG1,
			      BIT(pmu_armint_wakeup_en) |
			      BIT(pmu_gpioint_wakeup_en));

		/* 30ms on a 24MHz clock for pmic stabilization */
		mmio_write_32(PMU_BASE + PMU_STABL_CNT, 24000 * 30);

		/* oscillator is still running, so no need to wait */
		mmio_write_32(PMU_BASE + PMU_OSC_CNT, 0);
	}

	mmio_write_32(PMU_BASE + PMU_PWRMODE_CON, mode_set);
	mmio_write_32(PMU_BASE + PMU_PWRMODE_CON1, mode_set1);
}

static int cpus_power_domain_on(uint32_t cpu_id)
{
	uint32_t cpu_pd;

	cpu_pd = PD_CPU0 + cpu_id;

	/* if the core has been on, power it off first */
	if (pmu_power_domain_st(cpu_pd) == pmu_pd_on) {
		/* put core in reset - some sort of A12/A17 bug */
		mmio_write_32(CRU_BASE + CRU_SOFTRSTS_CON(0),
			      BIT(cpu_id) | (BIT(cpu_id) << 16));

		pmu_power_domain_ctr(cpu_pd, pmu_pd_off);
	}

	pmu_power_domain_ctr(cpu_pd, pmu_pd_on);

	/* pull core out of reset */
	mmio_write_32(CRU_BASE + CRU_SOFTRSTS_CON(0), BIT(cpu_id) << 16);

	return 0;
}

static int cpus_power_domain_off(uint32_t cpu_id)
{
	uint32_t cpu_pd = PD_CPU0 + cpu_id;

	if (pmu_power_domain_st(cpu_pd) == pmu_pd_off)
		return 0;

	if (check_cpu_wfie(cpu_id, CKECK_WFEI_MSK))
		return -EINVAL;

	/* put core in reset - some sort of A12/A17 bug */
	mmio_write_32(CRU_BASE + CRU_SOFTRSTS_CON(0),
		      BIT(cpu_id) | (BIT(cpu_id) << 16));

	pmu_power_domain_ctr(cpu_pd, pmu_pd_off);

	return 0;
}

static void nonboot_cpus_off(void)
{
	uint32_t boot_cpu, cpu;

	boot_cpu = plat_my_core_pos();
	boot_cpu = MPIDR_AFFLVL0_VAL(read_mpidr());

	/* turn off noboot cpus */
	for (cpu = 0; cpu < PLATFORM_CORE_COUNT; cpu++) {
		if (cpu == boot_cpu)
			continue;

		cpus_power_domain_off(cpu);
	}
}

void sram_save(void)
{
	/* TODO: support the sdram save for rk3288 SoCs*/
}

void sram_restore(void)
{
	/* TODO: support the sdram restore for rk3288 SoCs */
}

int rockchip_soc_cores_pwr_dm_on(unsigned long mpidr, uint64_t entrypoint)
{
	uint32_t cpu_id = plat_core_pos_by_mpidr(mpidr);

	assert(cpu_id < PLATFORM_CORE_COUNT);
	assert(cpuson_flags[cpu_id] == 0);
	cpuson_flags[cpu_id] = PMU_CPU_HOTPLUG;
	cpuson_entry_point[cpu_id] = entrypoint;
	dsb();

	cpus_power_domain_on(cpu_id);

	/*
	 * We communicate with the bootrom to active the cpus other
	 * than cpu0, after a blob of initialize code, they will
	 * stay at wfe state, once they are actived, they will check
	 * the mailbox:
	 * sram_base_addr + 4: 0xdeadbeaf
	 * sram_base_addr + 8: start address for pc
	 * The cpu0 need to wait the other cpus other than cpu0 entering
	 * the wfe state.The wait time is affected by many aspects.
	 * (e.g: cpu frequency, bootrom frequency, sram frequency, ...)
	 */
	mdelay(1); /* ensure the cpus other than cpu0 to startup */

	/* tell the bootrom mailbox where to start from */
	mmio_write_32(SRAM_BASE + 8, cpu_warm_boot_addr);
	mmio_write_32(SRAM_BASE + 4, 0xDEADBEAF);
	dsb();
	sev();

	return 0;
}

int rockchip_soc_cores_pwr_dm_on_finish(void)
{
	return 0;
}

int rockchip_soc_sys_pwr_dm_resume(void)
{
	mmio_write_32(PMU_BASE + PMU_PWRMODE_CON, store_pmu_pwrmode_con);
	mmio_write_32(SGRF_BASE + SGRF_CPU_CON(0),
		      store_sgrf_cpu_con0 | SGRF_DAPDEVICE_MSK);

	/* disable fastboot mode */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(0),
		      store_sgrf_soc_con0 | SGRF_FAST_BOOT_DIS);

	secure_watchdog_ungate();
	clk_gate_con_restore();
	clk_sel_con_restore();
	clk_plls_resume();

	secure_gic_init();
	plat_rockchip_gic_init();

	return 0;
}

int rockchip_soc_sys_pwr_dm_suspend(void)
{
	nonboot_cpus_off();

	store_sgrf_cpu_con0 = mmio_read_32(SGRF_BASE + SGRF_CPU_CON(0));
	store_sgrf_soc_con0 = mmio_read_32(SGRF_BASE + SGRF_SOC_CON(0));
	store_pmu_pwrmode_con = mmio_read_32(PMU_BASE + PMU_PWRMODE_CON);

	/* save clk-gates and ungate all for suspend */
	clk_gate_con_save();
	clk_gate_con_disable();
	clk_sel_con_save();

	pmu_set_sleep_mode(ROCKCHIP_ARM_OFF_LOGIC_NORMAL);

	clk_plls_suspend();
	secure_watchdog_gate();

	/*
	 * The dapswjdp can not auto reset before resume, that cause it may
	 * access some illegal address during resume. Let's disable it before
	 * suspend, and the MASKROM will enable it back.
	 */
	mmio_write_32(SGRF_BASE + SGRF_CPU_CON(0), SGRF_DAPDEVICE_MSK);

	/*
	 * SGRF_FAST_BOOT_EN - system to boot from FAST_BOOT_ADDR
	 */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(0), SGRF_FAST_BOOT_ENA);

	/* boot-address of resuming system is from this register value */
	mmio_write_32(SGRF_BASE + SGRF_FAST_BOOT_ADDR,
		      (uint32_t)&pmu_cpuson_entrypoint);

	/* flush all caches - otherwise we might loose the resume address */
	dcsw_op_all(DC_OP_CISW);

	return 0;
}

void rockchip_plat_mmu_svc_mon(void)
{
}

void plat_rockchip_pmu_init(void)
{
	uint32_t cpu;

	cpu_warm_boot_addr = (uint32_t)platform_cpu_warmboot;

	/* on boot all power-domains are on */
	for (cpu = 0; cpu < PLATFORM_CORE_COUNT; cpu++)
		cpuson_flags[cpu] = pmu_pd_on;

	nonboot_cpus_off();
}
