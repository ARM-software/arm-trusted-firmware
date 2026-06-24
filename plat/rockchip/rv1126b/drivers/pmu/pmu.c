// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2026, Rockchip Electronics Co., Ltd.
 */

#include <assert.h>
#include <errno.h>

#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <common/debug.h>
#include <drivers/console.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include <cpus_on_fixed_addr.h>
#include <dmc_rv1126b.h>
#include <plat_pm_helpers.h>
#include <plat_private.h>
#include <platform_def.h>
#include <pm_pd_regs.h>
#include <pmu.h>
#include <secure.h>
#include <soc.h>

static struct psram_data_t *psram_sleep_cfg =
	(struct psram_data_t *)&sys_sleep_flag_sram;

struct rv1126b_sleep_ddr_data {
	uint32_t cru_mode_con, secure_cru_mode;
	uint32_t gpio0a_iomux_l, gpio0a_iomux_h, gpio0b_iomux_l, gpio0b_iomux_h,
		 gpio0c_iomux_l, gpio0c_iomux_h, gpio0d_iomux_l;
	uint32_t gpio0_ddr_l, gpio0_ddr_h, gpio0_dr_l, gpio0_dr_h;
	uint32_t gpio0a_pull, gpio0b_pull, gpio0c_pull, gpio0d_pull;
	uint32_t gpio0_dbclk_div_con;
	uint32_t pmu2_pwrgt_sft_con0;
	uint32_t pmugrf_soc_con0, pmugrf_soc_con1, pmugrf_soc_con4, pmugrf_soc_con5,
		 pmugrf_soc_con6, pmugrf_soc_con7;
	uint32_t pmu0cru_sel_con1;
	uint32_t pmu_pd_st, bus_idle_st;
	uint32_t sys_sgrf_con;
	uint32_t sleep_clk_freq_khz, sleep_pin_en_msk, sleep_pin_act_low_msk;
	uint32_t ddrgrf_con21;
};

static struct rv1126b_sleep_ddr_data ddr_data;

struct rv1126b_sleep_pmusram_data {
	uint32_t idle_msk[2];
};

static __pmusramdata struct rv1126b_sleep_pmusram_data pmusram_data;

void rockchip_plat_mmu_el3(void)
{
}

static int check_cpu_wfie(uint32_t cpu)
{
	uint32_t loop = 0;
	uint32_t msk = 0x11 << cpu;

	while ((mmio_read_32(COREGRF_BASE + CPUGRF_STATUS(0)) & msk) == 0 &&
	       loop < WFEI_CHECK_LOOP) {
		udelay(1);
		loop++;
	}

	if (loop >= WFEI_CHECK_LOOP) {
		WARN("%s: error, cpu%d (0x%x)!\n", __func__, cpu,
		     mmio_read_32(COREGRF_BASE + CPUGRF_STATUS(0)));
		return -EINVAL;
	}

	return 0;
}

static inline uint32_t cpu_power_domain_st(uint32_t cpu)
{
	return !!(mmio_read_32(PMU_BASE + PMU2_CLUSTER_PWR_ST) &
		  BIT(cpu + 8));
}

static int cpu_power_domain_ctr(uint32_t cpu, uint32_t pd_state)
{
	uint32_t loop = 0;
	int ret = 0;

	mmio_write_32(PMU_BASE + PMU2_CPU_PWR_SFTCON(cpu),
		      BITS_WITH_WMASK(pd_state, 0x1, 0));

	dsb();
	while ((cpu_power_domain_st(cpu) != pd_state) && (loop < PD_CTR_LOOP)) {
		udelay(1);
		loop++;
	}

	if (cpu_power_domain_st(cpu) != pd_state) {
		WARN("%s: %d, %d, error!\n", __func__, cpu, pd_state);
		ret = -EINVAL;
	}

	return ret;
}

static inline uint32_t get_cpus_pwr_domain_cfg_info(uint32_t cpu_id)
{
	uint32_t val;

	if (mmio_read_32(PMU_BASE + PMU2_CPU_PWR_SFTCON(cpu_id)) & BIT(0))
		return core_pwr_pd;

	val = mmio_read_32(PMU_BASE + PMU2_CPU_AUTO_PWR_CON(cpu_id));
	if ((val & BIT(pmu_cpu_pm_en)) != 0) {
		if ((val & BIT(core_pwr_wfi_int)) != 0)
			return core_pwr_wfi_int;
		else if ((val & BIT(pmu_cpu_pm_sft_wakeup_en)) != 0)
			return core_pwr_wfi_reset;
		else
			return core_pwr_wfi;
	} else {
		return -1;
	}
}

static int cpus_power_domain_on(uint32_t cpu_id)
{
	uint32_t cfg_info;
	/*
	 * There are two ways to powering on or off on core.
	 * 1) Control it power domain into on or off in PMU_PWRDN_CON reg
	 * 2) Enable the core power manage in PMU_CORE_PM_CON reg,
	 *	then, if the core enter into wfi, it power domain will be
	 *	powered off automatically.
	 */

	cfg_info = get_cpus_pwr_domain_cfg_info(cpu_id);

	if (cfg_info == core_pwr_pd) {
		/* disable core_pm cfg */
		mmio_write_32(PMU_BASE + PMU2_CPU_AUTO_PWR_CON(cpu_id),
			      BITS_WITH_WMASK(0, 0xf, 0));
		/* if the cores have be on, power off it firstly */
		if (cpu_power_domain_st(cpu_id) == pmu_pd_on)
			cpu_power_domain_ctr(cpu_id, pmu_pd_off);

		cpu_power_domain_ctr(cpu_id, pmu_pd_on);
	} else {
		if (cpu_power_domain_st(cpu_id) == pmu_pd_on) {
			WARN("%s: cpu%d is not in off,!\n", __func__, cpu_id);
			return -EINVAL;
		}

		mmio_write_32(PMU_BASE + PMU2_CPU_AUTO_PWR_CON(cpu_id),
			      BITS_WITH_WMASK(1, 0x1, pmu_cpu_pm_sft_wakeup_en));
		dsb();
	}

	return 0;
}

static int cpus_power_domain_off(uint32_t cpu_id, uint32_t pd_cfg)
{
	uint32_t core_pm_value;

	if (cpu_power_domain_st(cpu_id) == pmu_pd_off)
		return 0;

	if (pd_cfg == core_pwr_pd) {
		if (check_cpu_wfie(cpu_id) != 0)
			return -EINVAL;

		/* disable core_pm cfg */
		mmio_write_32(PMU_BASE + PMU2_CPU_AUTO_PWR_CON(cpu_id),
			      BITS_WITH_WMASK(0, 0xf, 0));

		cpu_power_domain_ctr(cpu_id, pmu_pd_off);
	} else {
		core_pm_value = BIT(pmu_cpu_pm_en) | BIT(pmu_cpu_pm_dis_int);
		if (pd_cfg == core_pwr_wfi_int)
			core_pm_value |= BIT(pmu_cpu_pm_int_wakeup_en);
		else if (pd_cfg == core_pwr_wfi_reset)
			core_pm_value |= BIT(pmu_cpu_pm_sft_wakeup_en);

		mmio_write_32(PMU_BASE + PMU2_CPU_AUTO_PWR_CON(cpu_id),
			      BITS_WITH_WMASK(core_pm_value, 0xf, 0));
		dsb();
	}

	return 0;
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

	return PSCI_E_SUCCESS;
}

int rockchip_soc_cores_pwr_dm_off(void)
{
	uint32_t cpu_id = plat_my_core_pos();

	cpus_power_domain_off(cpu_id, core_pwr_wfi);

	return PSCI_E_SUCCESS;
}

int rockchip_soc_cores_pwr_dm_on_finish(void)
{
	uint32_t cpu_id = plat_my_core_pos();

	mmio_write_32(PMU_BASE + PMU2_CPU_AUTO_PWR_CON(cpu_id),
		      BITS_WITH_WMASK(0, 0xf, 0));

	return PSCI_E_SUCCESS;
}

int rockchip_soc_cores_pwr_dm_suspend(void)
{
	uint32_t cpu_id = plat_my_core_pos();

	assert(cpu_id < PLATFORM_CORE_COUNT);
	assert(cpuson_flags[cpu_id] == 0);
	cpuson_flags[cpu_id] = PMU_CPU_AUTO_PWRDN;
	cpuson_entry_point[cpu_id] = plat_get_sec_entrypoint();
	dsb();

	cpus_power_domain_off(cpu_id, core_pwr_wfi_int);

	return PSCI_E_SUCCESS;
}

int rockchip_soc_cores_pwr_dm_resume(void)
{
	uint32_t cpu_id = plat_my_core_pos();

	/* Disable core_pm */
	mmio_write_32(PMU_BASE + PMU2_CPU_AUTO_PWR_CON(cpu_id),
		      BITS_WITH_WMASK(0, 0xf, 0));

	return PSCI_E_SUCCESS;
}

void nonboot_cpus_off(void)
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

__pmusramfunc void pmusram_pmu_bus_idle_req_msk(uint32_t msk, uint32_t state)
{
	if (state == pmu_bus_idle)
		state = msk;
	else
		state = 0x0;

	mmio_write_32(PMU_BASE + PMU2_BUS_IDLE_SFTCON,
		      BITS_WITH_WMASK(state, msk, 0));

	while (pmu_bus_idle_st_msk(msk) != state ||
	       pmu_bus_idle_ack_msk(msk) != state)
		;
}

static __pmusramfunc void ddr_resume(void)
{
	/* switch to pll */
	mmio_write_32(TOPCRU_BASE + 0x280, 0x003f0015);

	/* hptimer 32k disable */
	mmio_write_32(PMUSGRF_BASE + PMUSGRF_SOC_CON(0),
		      BITS_WITH_WMASK(0, 0x1, 8));

	/* hptimer is 24M here */
	write_cntfrq_el0(24000000);

	pmusram_pmu_bus_idle_req_msk(pmusram_data.idle_msk[1], pmu_bus_active);
	pmusram_pmu_bus_idle_req_msk(pmusram_data.idle_msk[0], pmu_bus_active);

	dmc_restore();
}

static uint32_t clk_save[CRU_CLKGATE_CON_CNT + PERI_CRU_CLKGATE_CON_CNT +
			 BUS_CRU_CLKGATE_CON_CNT + BUS_SCRU_CLKGATE_CON_CNT +
			 PMU0CRU_CLKGATE_CON_CNT + PMU1CRU_CLKGATE_CON_CNT +
			 CORE_CRU_CLKGATE_CON_CNT + VI_CRU_CLKGATE_CON_CNT +
			 VEPU_CRU_CLKGATE_CON_CNT + VCP_CRU_CLKGATE_CON_CNT +
			 PMUSCRU_CLKGATE_CON_CNT];

static const uint16_t top_clk_ungt_msk[CRU_CLKGATE_CON_CNT] = {
	0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff,
	0xfffc, 0xffff, 0xffff, 0xffff, /* Don't open wdt clk (cru_gt8[0:1]) */
	0xffff, 0xffff, 0xffff, 0xff87, /* Don't open mipi clk (cru_gt15[0:1]) */
};

void clk_gate_con_disable(void)
{
	int i;

	for (i = 0; i < CRU_CLKGATE_CON_CNT; i++) {
		mmio_write_32(TOPCRU_BASE + CRU_CLKGATE_CON(i),
			      (uint32_t)top_clk_ungt_msk[i] << 16);
	}

	for (i = 0; i < PERI_CRU_CLKGATE_CON_CNT; i++)
		mmio_write_32(PERICRU_BASE + PERI_CRU_CLKGATE_CON(i), 0xffff0000);

	for (i = 0; i < BUS_CRU_CLKGATE_CON_CNT; i++)
		mmio_write_32(BUSCRU_BASE + BUS_CRU_CLKGATE_CON(i), 0xffff0000);

	for (i = 0; i < BUS_SCRU_CLKGATE_CON_CNT; i++)
		mmio_write_32(BUSCRU_BASE + BUS_SCRU_CLKGATE_CON(i), 0xffff0000);

	for (i = 0; i < PMU0CRU_CLKGATE_CON_CNT; i++)
		mmio_write_32(PMU0CRU_BASE + PMU0CRU_CLKGATE_CON(i), 0xffff0000);

	for (i = 0; i < PMU1CRU_CLKGATE_CON_CNT; i++)
		mmio_write_32(PMU1CRU_BASE + PMU1CRU_CLKGATE_CON(i), 0xffff0000);

	for (i = 0; i < CORE_CRU_CLKGATE_CON_CNT; i++)
		mmio_write_32(CORECRU_BASE + CORE_CRU_CLKGATE_CON(i), 0xffff0000);

	for (i = 0; i < VI_CRU_CLKGATE_CON_CNT; i++)
		mmio_write_32(VICRU_BASE + VI_CRU_CLKGATE_CON(i), 0xffff0000);

	for (i = 0; i < VEPU_CRU_CLKGATE_CON_CNT; i++)
		mmio_write_32(VEPUCRU_BASE + VEPU_CRU_CLKGATE_CON(i), 0xffff0000);

	for (i = 0; i < VCP_CRU_CLKGATE_CON_CNT; i++)
		mmio_write_32(VCPCRU_BASE + VCP_CRU_CLKGATE_CON(i), 0xffff0000);

	mmio_write_32(PMUSCRU_BASE + PMUSCRU_CLKGATE_CON0, 0xffff0000);
}

void clk_gate_con_save(void)
{
	int i, j = 0;

	for (i = 0; i < CRU_CLKGATE_CON_CNT; i++, j++)
		clk_save[j] = mmio_read_32(TOPCRU_BASE + CRU_CLKGATE_CON(i));

	for (i = 0; i < PERI_CRU_CLKGATE_CON_CNT; i++, j++)
		clk_save[j] = mmio_read_32(PERICRU_BASE + PERI_CRU_CLKGATE_CON(i));

	for (i = 0; i < BUS_CRU_CLKGATE_CON_CNT; i++, j++)
		clk_save[j] = mmio_read_32(BUSCRU_BASE + BUS_CRU_CLKGATE_CON(i));

	for (i = 0; i < BUS_SCRU_CLKGATE_CON_CNT; i++, j++)
		clk_save[j] = mmio_read_32(BUSCRU_BASE + BUS_SCRU_CLKGATE_CON(i));

	for (i = 0; i < PMU0CRU_CLKGATE_CON_CNT; i++, j++)
		clk_save[j] = mmio_read_32(PMU0CRU_BASE + PMU0CRU_CLKGATE_CON(i));

	for (i = 0; i < PMU1CRU_CLKGATE_CON_CNT; i++, j++)
		clk_save[j] = mmio_read_32(PMU1CRU_BASE + PMU1CRU_CLKGATE_CON(i));

	for (i = 0; i < CORE_CRU_CLKGATE_CON_CNT; i++, j++)
		clk_save[j] = mmio_read_32(CORECRU_BASE + CORE_CRU_CLKGATE_CON(i));

	for (i = 0; i < VI_CRU_CLKGATE_CON_CNT; i++, j++)
		clk_save[j] = mmio_read_32(VICRU_BASE + VI_CRU_CLKGATE_CON(i));

	for (i = 0; i < VEPU_CRU_CLKGATE_CON_CNT; i++, j++)
		clk_save[j] = mmio_read_32(VEPUCRU_BASE + VEPU_CRU_CLKGATE_CON(i));

	for (i = 0; i < VCP_CRU_CLKGATE_CON_CNT; i++, j++)
		clk_save[j] = mmio_read_32(VCPCRU_BASE + VCP_CRU_CLKGATE_CON(i));

	clk_save[j] = mmio_read_32(PMUSCRU_BASE + PMUSCRU_CLKGATE_CON0);
}

void clk_gate_con_restore(void)
{
	int i, j = 0;

	for (i = 0; i < CRU_CLKGATE_CON_CNT; i++, j++)
		mmio_write_32(TOPCRU_BASE + CRU_CLKGATE_CON(i),
			      WITH_16BITS_WMSK(clk_save[j]));

	for (i = 0; i < PERI_CRU_CLKGATE_CON_CNT; i++, j++)
		mmio_write_32(PERICRU_BASE + PERI_CRU_CLKGATE_CON(i),
			      WITH_16BITS_WMSK(clk_save[j]));

	for (i = 0; i < BUS_CRU_CLKGATE_CON_CNT; i++, j++)
		mmio_write_32(BUSCRU_BASE + BUS_CRU_CLKGATE_CON(i),
			      WITH_16BITS_WMSK(clk_save[j]));

	for (i = 0; i < BUS_SCRU_CLKGATE_CON_CNT; i++, j++)
		mmio_write_32(BUSCRU_BASE + BUS_SCRU_CLKGATE_CON(i),
			      WITH_16BITS_WMSK(clk_save[j]));

	for (i = 0; i < PMU0CRU_CLKGATE_CON_CNT; i++, j++)
		mmio_write_32(PMU0CRU_BASE + PMU0CRU_CLKGATE_CON(i),
			      WITH_16BITS_WMSK(clk_save[j]));

	for (i = 0; i < PMU1CRU_CLKGATE_CON_CNT; i++, j++)
		mmio_write_32(PMU1CRU_BASE + PMU1CRU_CLKGATE_CON(i),
			      WITH_16BITS_WMSK(clk_save[j]));

	for (i = 0; i < CORE_CRU_CLKGATE_CON_CNT; i++, j++)
		mmio_write_32(CORECRU_BASE + CORE_CRU_CLKGATE_CON(i),
			      WITH_16BITS_WMSK(clk_save[j]));

	for (i = 0; i < VI_CRU_CLKGATE_CON_CNT; i++, j++)
		mmio_write_32(VICRU_BASE + VI_CRU_CLKGATE_CON(i),
			      WITH_16BITS_WMSK(clk_save[j]));

	for (i = 0; i < VEPU_CRU_CLKGATE_CON_CNT; i++, j++)
		mmio_write_32(VEPUCRU_BASE + VEPU_CRU_CLKGATE_CON(i),
			      WITH_16BITS_WMSK(clk_save[j]));

	for (i = 0; i < VCP_CRU_CLKGATE_CON_CNT; i++, j++)
		mmio_write_32(VCPCRU_BASE + VCP_CRU_CLKGATE_CON(i),
			      WITH_16BITS_WMSK(clk_save[j]));

	mmio_write_32(PMUSCRU_BASE + PMUSCRU_CLKGATE_CON0, WITH_16BITS_WMSK(clk_save[j]));
}

void pmu_bus_idle_req(uint32_t bus, uint32_t state)
{
	uint32_t wait_cnt = 0;

	mmio_write_32(PMU_BASE + PMU2_BUS_IDLE_SFTCON,
		      BITS_WITH_WMASK(state, 0x1, bus));

	while (pmu_bus_idle_st(bus) != state ||
	       pmu_bus_idle_ack(bus) != state) {
		if (++wait_cnt > BUS_IDLE_LOOP)
			break;
		udelay(1);
	}

	if (wait_cnt > BUS_IDLE_LOOP)
		WARN("%s: can't  wait state %d for bus %d (0x%x)\n",
		     __func__, state, bus,
		     mmio_read_32(PMU_BASE + PMU2_BUS_IDLE_ST));
}

static inline uint32_t pmu_power_domain_st(uint32_t pd)
{
	return mmio_read_32(PMU_BASE + PMU2_PWR_GATE_ST) & BIT(pd) ?
	       pmu_pd_off :
	       pmu_pd_on;
}

int pmu_power_domain_ctr(uint32_t pd, uint32_t pd_state)
{
	uint32_t loop = 0;
	int ret = 0;

	mmio_write_32(PMU_BASE + PMU2_PWR_GATE_SFTCON,
		      BITS_WITH_WMASK(pd_state, 0x1, pd));
	dsb();

	while ((pmu_power_domain_st(pd) != pd_state) && (loop < PD_CTR_LOOP)) {
		udelay(1);
		loop++;
	}

	if (pmu_power_domain_st(pd) != pd_state) {
		WARN("%s: %d, %d, (0x%x) error!\n", __func__, pd, pd_state,
		     mmio_read_32(PMU_BASE + PMU2_PWR_GATE_ST));
		ret = -EINVAL;
	}

	return ret;
}

static int pmu_set_power_domain(uint32_t pd_id, uint32_t pd_state)
{
	uint32_t state;

	if (pmu_power_domain_st(pd_id) == pd_state)
		goto out;

	if (pd_state == pmu_pd_on)
		pmu_power_domain_ctr(pd_id, pd_state);

	state = (pd_state == pmu_pd_off) ? pmu_bus_idle : pmu_bus_active;

	switch (pd_id) {
	case pmu_pd_npu:
		pmu_bus_idle_req(pmu_bus_id_npu, state);
		break;
	case pmu_pd_vdo:
		pmu_bus_idle_req(pmu_bus_id_vdo, state);
		break;
	case pmu_pd_aiisp:
		pmu_bus_idle_req(pmu_bus_id_aisp, state);
		break;
	default:
		break;
	}

	if (pd_state == pmu_pd_off)
		pmu_power_domain_ctr(pd_id, pd_state);

out:
	return 0;
}

static void pmu_power_domains_suspend(void)
{
	ddr_data.pmu_pd_st = mmio_read_32(PMU_BASE + PMU2_PWR_GATE_ST);
	ddr_data.bus_idle_st = mmio_read_32(PMU_BASE + PMU2_BUS_IDLE_ST);
	ddr_data.pmu2_pwrgt_sft_con0 = mmio_read_32(PMU_BASE + PMU2_PWR_GATE_SFTCON);

	if ((ddr_data.pmu_pd_st & BIT(pmu_pd_aiisp)) == 0) {
		pd_aiisp_save();
		pmu_set_power_domain(pmu_pd_aiisp, pmu_pd_off);
	}

	if ((ddr_data.pmu_pd_st & BIT(pmu_pd_vdo)) == 0) {
		pd_vdo_save();
		pmu_set_power_domain(pmu_pd_vdo, pmu_pd_off);
	}

	if ((ddr_data.pmu_pd_st & BIT(pmu_pd_npu)) == 0) {
		vd_npu_save();
		pmu_set_power_domain(pmu_pd_npu, pmu_pd_off);
	}
}

static void pmu_power_domains_resume(void)
{
	int i;

	for (i = 0; i < pmu_pd_id_max; i++)
		pmu_set_power_domain(i, !!(ddr_data.pmu_pd_st & BIT(i)));

	for (i = 0; i < pmu_bus_id_max; i++)
		pmu_bus_idle_req(i, !!(ddr_data.bus_idle_st & BIT(i)));

	if ((ddr_data.pmu_pd_st & BIT(pmu_pd_aiisp)) == 0)
		pd_aiisp_restore();

	if ((ddr_data.pmu_pd_st & BIT(pmu_pd_vdo)) == 0)
		pd_vdo_restore();

	if ((ddr_data.pmu_pd_st & BIT(pmu_pd_npu)) == 0)
		vd_npu_restore();
}

static void ddr_sleep_config(void)
{
	ddr_data.ddrgrf_con21 = mmio_read_32(DDRGRF_BASE + DDRGRF_CON(21));

	/* [7:4]-csysreq_aclk_pmu enable */
	mmio_write_32(DDRGRF_BASE + DDRGRF_CON(21), BITS_WITH_WMASK(0xf, 0xf, 4));
	/* [12 ]-csysreq_ddrc_pmu enable */
	mmio_write_32(DDRGRF_BASE + DDRGRF_CON(21), BITS_WITH_WMASK(0x1, 0x1, 12));

	mmio_write_32(PMUGRF_BASE + PMUGRF_SOC_CON(0), BITS_WITH_WMASK(0x0, 0x1, 9));
}

static void ddr_sleep_config_restore(void)
{
	mmio_write_32(DDRGRF_BASE + DDRGRF_CON(21), WITH_16BITS_WMSK(ddr_data.ddrgrf_con21));
}

static void pmu_sleep_config(void)
{
	uint32_t clk_freq_khz = 24000;
	uint32_t pmu1_wkup_int_con = 0;
	uint32_t pmu0_pwr_con, pmu1_pwr_con;
	uint32_t pmu1_pll_con, pmu1_cru_con[2], pmu1_ddr_ch_pwr_con, pmu1_ddr_axi_pwr_con;
	uint32_t pmu2_scu_con, pmu2_scu_sftcon, pmu2_cpu_auto_con;
	uint32_t pmu2_bus_idle_con, pmu2_pwr_gt_con;

	pmu1_wkup_int_con = BIT(pmu_wkup_cpu0_int) |
			    BIT(pmu_wkup_gpio0_int) |
			    0;

	if (mmio_read_32(PMU_BASE + PMU1_WAKEUP_TIMEOUT) != 0)
		pmu1_wkup_int_con |= BIT(pmu_wkup_timeout);

	pmu0_pwr_con = 0;

	pmu1_pwr_con =
		BIT(pmu_powermode_en) |
		/* BIT(pmu_scu0_byp) | */
		/* BIT(pmu_scu1_byp) | */
		/* BIT(pmu_bus_byp) | */
		/* BIT(pmu_ddr_byp) | */
		/* BIT(pmu_pwrgt_byp) | */
		/* BIT(pmu_cru_byp) | */
		BIT(pmu_pdpmu1_byp) |
		/* BIT(pmu_wfi_byp) | */
		BIT(pmu_slp_cnt_en) |
		/* BIT(pmu_wkup_pmu_sft_en) | */
		0;

	pmu1_ddr_ch_pwr_con =
		/* BIT(pmu_ddr_sref_c_en) | */
		/* BIT(pmu_ddr_ioret_en) | */
		/* BIT(pmu_ddr_ioret_exit_en) | */
		/* BIT(pmu_ddr_rstiov_en) | */
		/* BIT(pmu_ddr_rstiov_exit_en) | */
		BIT(pmu_ddrctl_c_auto_gating_en) |
		BIT(pmu_ddrphy_auto_gating_en) |
		0;

	pmu1_ddr_axi_pwr_con =
		/* BIT(pmu_ddr_sref_a_ch0_en) | */
		/* BIT(pmu_ddr_sref_a_ch1_en) | */
		/* BIT(pmu_ddr_sref_a_ch2_en) | */
		/* BIT(pmu_ddr_sref_a_ch3_en) | */
		0;

	pmu1_cru_con[0] =
		BIT(pmu_alive_osc_mode_en) |
		/* BIT(pmu_power_off_en) | */
		/* BIT(pmu_pwm_switch_en) | */
		/* BIT(pmu_pwm_gpio_ioe_en) | */
		/* BIT(pmu_pwm_switch_io) | */
		/* BIT(pmu_pwm_clkgt_en) | */
		0;

	pmu1_cru_con[1] =
		/* BIT(pmu_peri_clksrc_gt_en) | */
		BIT(pmu_vepu_clksrc_gt_en) |
		BIT(pmu_vcp_clksrc_gt_en) |
		BIT(pmu_vi_clksrc_gt_en) |
		BIT(pmu_npu_clksrc_gt_en) |
		BIT(pmu_vdo_clksrc_gt_en) |
		BIT(pmu_aisp_clksrc_gt_en) |
		BIT(pmu_core_clksrc_gt_en) |
		BIT(pmu_ddr_clksrc_gt_en) |
		/* BIT(pmu_bus_clksrc_gt_en) | */
		0;

	pmu1_pll_con =
		BIT(pmu_gpll_pd_en) |
		BIT(pmu_dpll_pd_en) |
		BIT(pmu_cpll_pd_en) |
		BIT(pmu_ppll_pd_en) |
		0;

	pmu2_scu_con =
		BIT(pmu_l2_flush_en) |
		BIT(pmu_l2_ilde_en) |
		BIT(pmu_scu_pd_en) |
		BIT(pmu_scu_pwroff_en) |
		BIT(pmu_clst_cpu_pd_en) |
		BIT(pmu_clst_clksrc_gt_en) |
		0;

	pmu2_scu_sftcon =
		BIT(pmu_scu_vol_gate) |
		BIT(pmu_scu_dwn_ack_sel) |
		0;

	pmu2_cpu_auto_con =
		BIT(pmu_cpu_pm_int_wakeup_en) |
		BIT(pmu_cpu_cluster_wakeup_en) |
		0;

	pmu2_bus_idle_con =
		/* BIT(pmu_bus_id_vepu) | */
		/* BIT(pmu_bus_id_vi) | */
		BIT(pmu_bus_id_vcp) |
		BIT(pmu_bus_id_cru) |
		/* BIT(pmu_bus_id_peri) | */
		BIT(pmu_bus_id_bus) |
		/* BIT(pmu_bus_id_cfgddr) | */
		/* BIT(pmu_bus_id_subddr) | */
		/* BIT(pmu_bus_id_ddr) | */
		0;

	pmu2_pwr_gt_con = 0;

	/* pmu count */
	mmio_write_32(PMU_BASE + PMU1_OSC_STABLE_CNT, clk_freq_khz * 4);
	mmio_write_32(PMU_BASE + PMU1_PMIC_STABLE_CNT, clk_freq_khz * 6);
	mmio_write_32(PMU_BASE + PMU1_SLEEP_CNT, clk_freq_khz * 15);

	/* Pmu's clk has switched to 24M back When pmu FSM counts
	 * the follow counters, so we should use 24M to calculate
	 * these counters.
	 */
	mmio_write_32(PMU_BASE + PMU1_WAKEUP_RST_CLR_CNT, 0);
	mmio_write_32(PMU_BASE + PMU1_PLL_LOCK_CNT, 1200);
	mmio_write_32(PMU_BASE + PMU1_PWM_SWITCH_CNT, 24000 * 2);

	mmio_write_32(PMU_BASE + PMU2_SCU_STABLE_CNT, 0);
	mmio_write_32(PMU_BASE + PMU2_SCU_PWRUP_CNT, 0);
	mmio_write_32(PMU_BASE + PMU2_SCU_PWRDN_CNT, 0);

	/* bus / pd */
	mmio_write_32(PMU_BASE + PMU2_BUS_IDLE_CON, WITH_16BITS_WMSK(pmu2_bus_idle_con));
	mmio_write_32(PMU_BASE + PMU2_PWR_GATE_CON, WITH_16BITS_WMSK(pmu2_pwr_gt_con));

	/* clust idle */
	mmio_write_32(PMU_BASE + PMU2_CLUSTER_IDLE_CON, 0x000f000f);
	mmio_write_32(PMU_BASE + PMU2_SCU_PWR_CON, WITH_16BITS_WMSK(pmu2_scu_con));
	mmio_write_32(PMU_BASE + PMU2_SCU_PWR_SFTCON, WITH_16BITS_WMSK(pmu2_scu_sftcon));
	mmio_write_32(PMU_BASE + PMU2_CPU_AUTO_PWR_CON(0), WITH_16BITS_WMSK(pmu2_cpu_auto_con));

	/* ddr pwr con0 */
	mmio_write_32(PMU_BASE + PMU1_DDR_CH0PWR_CON, WITH_16BITS_WMSK(pmu1_ddr_ch_pwr_con));
	mmio_write_32(PMU_BASE + PMU1_DDR_AXIPWR_CON, WITH_16BITS_WMSK(pmu1_ddr_axi_pwr_con));

	/* cru pwr con */
	mmio_write_32(PMU_BASE + PMU1_CRU_PWR_CON(0), WITH_16BITS_WMSK(pmu1_cru_con[0]));
	mmio_write_32(PMU_BASE + PMU1_CRU_PWR_CON(1), WITH_16BITS_WMSK(pmu1_cru_con[1]));

	/* pll con */
	mmio_write_32(PMU_BASE + PMU1_PLLPD_CON, WITH_16BITS_WMSK(pmu1_pll_con));

	/* global */
	mmio_write_32(PMU_BASE + PMU1_INT_MASK_CON, BITS_WITH_WMASK(1, 0x1, 0));
	mmio_write_32(PMU_BASE + PMU1_WAKEUP_INT_CON, pmu1_wkup_int_con);
	mmio_write_32(PMU_BASE + PMU1_PWR_CON, WITH_16BITS_WMSK(pmu1_pwr_con));
	mmio_write_32(PMU_BASE + PMU0_PWR_CON, WITH_16BITS_WMSK(pmu0_pwr_con));
}

static void pmu_sleep_restore(void)
{
	/* bus / pd */
	mmio_write_32(PMU_BASE + PMU2_BUS_IDLE_CON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU2_PWR_GATE_CON, 0xffff0000);

	/* clust idle */
	mmio_write_32(PMU_BASE + PMU2_CLUSTER_IDLE_CON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU2_SCU_PWR_CON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU2_SCU_PWR_SFTCON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU2_CPU_AUTO_PWR_CON(0), 0xffff0000);

	/* ddr pwr con0 */
	mmio_write_32(PMU_BASE + PMU1_DDR_CH0PWR_CON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU1_DDR_AXIPWR_CON, 0xffff0000);

	/* cru pwr con */
	mmio_write_32(PMU_BASE + PMU1_CRU_PWR_CON(0), 0xffff0000);
	mmio_write_32(PMU_BASE + PMU1_CRU_PWR_CON(1), 0xffff0000);

	/* pll con */
	mmio_write_32(PMU_BASE + PMU1_PLLPD_CON, 0xffff0000);

	mmio_write_32(PMU_BASE + PMU1_INT_MASK_CON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU1_WAKEUP_INT_CON, 0);
	mmio_write_32(PMU_BASE + PMU1_PWR_CON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU0_PWR_CON, 0xffff0000);
}

static void secure_watchdog_disable(void)
{
	ddr_data.sys_sgrf_con =
		mmio_read_32(SYSSGRF_BASE + SYSSGRF_CON);

	/* pause wdt_s */
	mmio_write_32(SYSSGRF_BASE + SYSSGRF_CON,
		      BITS_WITH_WMASK(1, 0x1, 0));
}

static void secure_watchdog_restore(void)
{
	mmio_write_32(SYSSGRF_BASE + SYSSGRF_CON,
		      ddr_data.sys_sgrf_con | BITS_WMSK(0x1, 0));

	if (mmio_read_32(WDT_S_BASE + WDT_CR) & WDT_EN)
		mmio_write_32(WDT_S_BASE + WDT_CRR, 0x76);
}

static void soc_sleep_config(void)
{
	ddr_data.pmugrf_soc_con0 = mmio_read_32(PMUGRF_BASE + PMUGRF_SOC_CON(0));
	ddr_data.pmugrf_soc_con1 = mmio_read_32(PMUGRF_BASE + PMUGRF_SOC_CON(1));
	ddr_data.pmugrf_soc_con4 = mmio_read_32(PMUGRF_BASE + PMUGRF_SOC_CON(4));
	ddr_data.pmugrf_soc_con5 = mmio_read_32(PMUGRF_BASE + PMUGRF_SOC_CON(5));
	ddr_data.pmugrf_soc_con6 = mmio_read_32(PMUGRF_BASE + PMUGRF_SOC_CON(6));
	ddr_data.pmugrf_soc_con7 = mmio_read_32(PMUGRF_BASE + PMUGRF_SOC_CON(7));

	pmu_sleep_config();
	ddr_sleep_config();
	secure_watchdog_disable();
}

static void soc_sleep_restore(void)
{
	secure_watchdog_restore();
	ddr_sleep_config_restore();
	pmu_sleep_restore();

	mmio_write_32(PMUGRF_BASE + PMUGRF_SOC_CON(0), WITH_16BITS_WMSK(ddr_data.pmugrf_soc_con0));
	mmio_write_32(PMUGRF_BASE + PMUGRF_SOC_CON(1), WITH_16BITS_WMSK(ddr_data.pmugrf_soc_con1));
	mmio_write_32(PMUGRF_BASE + PMUGRF_SOC_CON(4), WITH_16BITS_WMSK(ddr_data.pmugrf_soc_con4));
	mmio_write_32(PMUGRF_BASE + PMUGRF_SOC_CON(5), WITH_16BITS_WMSK(ddr_data.pmugrf_soc_con5));
	mmio_write_32(PMUGRF_BASE + PMUGRF_SOC_CON(6), WITH_16BITS_WMSK(ddr_data.pmugrf_soc_con6));
	mmio_write_32(PMUGRF_BASE + PMUGRF_SOC_CON(7), WITH_16BITS_WMSK(ddr_data.pmugrf_soc_con7));
}

static void pm_pll_suspend(void)
{
	ddr_data.cru_mode_con = mmio_read_32(TOPCRU_BASE + 0x280);

	/* pll switch to slow mode */
	mmio_write_32(TOPCRU_BASE + 0x280, 0x003f0000);
}

static void pm_pll_restore(void)
{
	mmio_write_32(TOPCRU_BASE + 0x280, WITH_16BITS_WMSK(ddr_data.cru_mode_con));
}

int rockchip_soc_sys_pwr_dm_suspend(void)
{
	psram_sleep_cfg->pm_flag &= ~PM_WARM_BOOT_BIT;

	clk_gate_con_save();
	clk_gate_con_disable();
	pmu_power_domains_suspend();
	soc_sleep_config();
	pm_pll_suspend();
	vd_core_save();

	return 0;
}

int rockchip_soc_sys_pwr_dm_resume(void)
{
	vd_core_restore();
	pm_pll_restore();
	soc_sleep_restore();
	pmu_power_domains_resume();
	plat_rockchip_gic_cpuif_enable();
	clk_gate_con_restore();

	psram_sleep_cfg->pm_flag |= PM_WARM_BOOT_BIT;

	return 0;
}

static __pmusramfunc void pmusram_wfi(void)
{
	pmusram_data.idle_msk[0] =
			BIT(pmu_bus_id_peri) |
			BIT(pmu_bus_id_vi) |
			BIT(pmu_bus_id_vepu) |
			0;

	pmusram_data.idle_msk[1] =
			BIT(pmu_bus_id_ddr) |
			BIT(pmu_bus_id_subddr) |
			BIT(pmu_bus_id_cfgddr) |
			0;

	pmusram_pmu_bus_idle_req_msk(pmusram_data.idle_msk[0], pmu_bus_idle);

	/* ddr enter self refresh */
	dmc_enter_lp();

	pmusram_pmu_bus_idle_req_msk(pmusram_data.idle_msk[1], pmu_bus_idle);

	/*
	 * The HW enters to sleep mode trigger by wfi.
	 */
	while (1) {
		wfi();
	}
}

void rockchip_soc_sys_pd_pwr_dn_wfi(void)
{
	disable_mmu_icache_el3();
	dsb();
	isb();

	rockchip_set_sp(PSRAM_SP_TOP);
	pmusram_wfi();
}

void __dead2 rockchip_soc_soft_reset(void)
{
	/* pll slow mode */
	mmio_write_32(TOPCRU_BASE + CRU_MODE_CON, 0x003f0000);

	dsb();
	isb();

	INFO("system reset......\n");
	mmio_write_32(TOPCRU_BASE + CRU_GLB_SRST_FST, GLB_SRST_FST_CFG_VAL);

	/*
	 * Maybe the HW needs some times to reset the system,
	 * so we do not hope the core to execute valid codes.
	 */
	while (1) {
		wfi();
	}
}

void __dead2 rockchip_soc_system_off(void)
{
	INFO("system poweroff......\n");
	dsb();

	/*
	 * Maybe the HW needs some times to power off the system,
	 * so we do not hope the core to execute valid codes.
	 */
	while (1) {
		wfi();
	}
}

static void rockchip_pmu_pd_init(void)
{
	pmu_set_power_domain(pmu_pd_npu, pmu_pd_on);

	INFO("idle_st=0x%x, pd_st=0x%x\n",
	     mmio_read_32(PMU_BASE + PMU2_BUS_IDLE_ST),
	     mmio_read_32(PMU_BASE + PMU2_PWR_GATE_ST));
}

void plat_rockchip_pmu_init(void)
{
	int cpu;

	for (cpu = 0; cpu < PLATFORM_CORE_COUNT; cpu++)
		cpuson_flags[cpu] = 0;

	psram_sleep_cfg->sp = PSRAM_SP_TOP;
	psram_sleep_cfg->ddr_func = (uint64_t)ddr_resume;
	psram_sleep_cfg->ddr_data = 0;
	psram_sleep_cfg->ddr_flag = 0;
	psram_sleep_cfg->boot_mpidr = read_mpidr_el1() & 0xffff;
	psram_sleep_cfg->pm_flag = PM_WARM_BOOT_BIT;

	nonboot_cpus_off();

	mmio_write_32(PMU_BASE + PMU2_NOC_AUTO_CON, 0xffffffff);

	/* pmusram remap to 0x0 */
	mmio_write_32(PMUSGRF_BASE + PMUSGRF_SOC_CON(1), BITS_WITH_WMASK(2, 0x3, 9));

	/* PMU_WAKEUP_TIMEOUT_CNT = 0, disable TIMEOUT_WAKEUP by default */
	mmio_write_32(PMU_BASE + PMU1_WAKEUP_TIMEOUT, 0);

	rockchip_pmu_pd_init();

	pm_reg_rgns_init();
}
