// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2025, Rockchip Electronics Co., Ltd.
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
#include <platform_def.h>
#include <pmu.h>

#include <cpus_on_fixed_addr.h>
#include <dmc_rk3576.h>
#include <plat_pm_helpers.h>
#include <plat_private.h>
#include <pm_pd_regs.h>
#include <secure.h>
#include <soc.h>

static struct psram_data_t *psram_sleep_cfg =
	(struct psram_data_t *)&sys_sleep_flag_sram;

struct rk3576_sleep_ddr_data {
	uint32_t cru_mode_con, secure_cru_mode;
	uint32_t gpio0a_iomux_l, gpio0a_iomux_h, gpio0b_iomux_l;
	uint32_t pmu2_bisr_glb_con;
	uint32_t pmu2_c0_ack_sel_con0, pmu2_c1_ack_sel_con0, pmu2_c2_ack_sel_con0;
	uint32_t pmu2_fast_pwr_con, pmu2_pwrgt_sft_con0;
	uint32_t pmu0grf_soc_con0, pmu0grf_soc_con1, pmu0grf_soc_con5;
	uint32_t pmu_pd_st, bus_idle_st;
	uint32_t sys_sgrf_soc_con0;
	uint32_t ddrgrf_cha_con2, ddrgrf_chb_con2;
};

static struct rk3576_sleep_ddr_data ddr_data;

void rockchip_plat_mmu_el3(void)
{
#if PLAT_EXTRA_LD_SCRIPT
	size_t sram_size;

	sram_size = (char *)&__bl31_pmusram_text_end -
		    (char *)PMUSRAM_BASE;
	mmap_add_region(PMUSRAM_BASE, PMUSRAM_BASE,
			sram_size, MT_MEMORY | MT_RO | MT_SECURE);

	sram_size = (char *)&__bl31_pmusram_data_end -
		    (char *)&__bl31_pmusram_data_start;
	mmap_add_region((unsigned long)&__bl31_pmusram_data_start,
			(unsigned long)&__bl31_pmusram_data_start,
			sram_size, MT_DEVICE | MT_RW | MT_SECURE);
#endif
}

static int check_cpu_wfie(uint32_t cpu)
{
	uint32_t loop = 0;

	while ((mmio_read_32(SYS_GRF_BASE + SYSGRF_STATUS0) & BIT(cpu + 12)) == 0 &&
	       (mmio_read_32(PMU_BASE + PMU2_CLUSTER_PWR_ST) & BIT(cpu)) == 0 &&
	       (loop < WFEI_CHECK_LOOP)) {
		udelay(1);
		loop++;
	}

	if (loop >= WFEI_CHECK_LOOP) {
		WARN("%s: error, cpu%d (0x%x 0x%x)!\n", __func__, cpu,
		     mmio_read_32(SYS_GRF_BASE + SYSGRF_STATUS0),
		     mmio_read_32(PMU_BASE + PMU2_CLUSTER_PWR_ST));
		return -EINVAL;
	}

	return 0;
}

static inline uint32_t cpu_power_domain_st(uint32_t cpu)
{
	return !!(mmio_read_32(PMU_BASE + PMU2_CLUSTER_PWR_ST) &
		  BIT(cpu + 16));
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

	if ((mmio_read_32(PMU_BASE + PMU2_CPU_PWR_SFTCON(cpu_id)) & BIT(0)) != 0)
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

static inline void set_cpus_pwr_domain_cfg_info(uint32_t cpu_id, uint32_t value)
{
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
		if (check_cpu_wfie(cpu_id))
			return -EINVAL;

		/* disable core_pm cfg */
		mmio_write_32(PMU_BASE + PMU2_CPU_AUTO_PWR_CON(cpu_id),
			      BITS_WITH_WMASK(0, 0xf, 0));

		set_cpus_pwr_domain_cfg_info(cpu_id, pd_cfg);
		cpu_power_domain_ctr(cpu_id, pmu_pd_off);
	} else {
		set_cpus_pwr_domain_cfg_info(cpu_id, pd_cfg);

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

static __pmusramfunc void ddr_resume(void)
{
	uint32_t key_upd_msk =
		mmio_read_32(PMU_BASE + PMU2_PWR_GATE_ST) & BIT(pmu_pd_vop) ? 0x3 : 0x7;

	/* hptimer is 24M here */
	write_cntfrq_el0(24000000);

	/* release cpu1~cpu7 to make pmu1_fsm exit */
	mmio_write_32(CCI_GRF_BASE + CCIGRF_CON(4), 0xffffffff);
	mmio_write_32(LITCORE_GRF_BASE + COREGRF_CPU_CON(1),
		      BITS_WITH_WMASK(0x77, 0xff, 4));

	/* wait pmu1 fsm over */
	while ((mmio_read_32(PMU_BASE + PMU1_PWR_FSM) & 0xf) != 0)
		;

	/* SOC_CON19.vop/center/cci_ddr_hash_key_update_en */
	mmio_write_32(SYS_SGRF_BASE + SYSSGRF_SOC_CON(19), 0x00070000);
	dsb();

	/* SOC_CON19.vop/center/cci_ddr_hash_key_update_en */
	mmio_write_32(SYS_SGRF_BASE + SYSSGRF_SOC_CON(19), 0x00070000 | key_upd_msk);

	/* SOC_STATUS.center/cci_ddr_hash_key_shift_ready */
	while (((mmio_read_32(SYS_SGRF_BASE + SYSSGRF_SOC_STATUS) >> 12) & key_upd_msk) != key_upd_msk)
		;

	/* CCI_BASE.ctrl_override_reg Attr:W1C addrmap strobe */
	mmio_setbits_32(CCI_BASE + 0x0, 0x1 << 29);

	/* SOC_CON19.vop/center/cci_ddr_hash_key_auto_update_en */
	mmio_write_32(SYS_SGRF_BASE + SYSSGRF_SOC_CON(19), 0x00700070);
}

static uint32_t clk_save[CRU_CLKGATE_CON_CNT + PHP_CRU_CLKGATE_CON_CNT +
			 SECURE_CRU_CLKGATE_CON_CNT + SECURE_SCRU_CLKGATE_CON_CNT +
			 PMU1CRU_CLKGATE_CON_CNT + PMU1SCRU_CLKGATE_CON_CNT];

void clk_gate_con_disable(void)
{
	int i;

	for (i = 0; i < CRU_CLKGATE_CON_CNT; i++) {
		/* Don't open wdt0 clk (cru_gate16[7:8] */
		if (i == 16) {
			mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(i),
				      0xfe7f0000);
		} else {
			mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(i),
				      0xffff0000);
		}
	}

	for (i = 0; i < PHP_CRU_CLKGATE_CON_CNT; i++)
		mmio_write_32(PHP_CRU_BASE + PHP_CRU_CLKGATE_CON(i), 0xffff0000);

	for (i = 0; i < SECURE_CRU_CLKGATE_CON_CNT; i++)
		mmio_write_32(SECURE_CRU_BASE + SECURE_CRU_CLKGATE_CON(i), 0xffff0000);

	for (i = 0; i < SECURE_SCRU_CLKGATE_CON_CNT; i++)
		mmio_write_32(SECURE_CRU_BASE + SECURE_SCRU_CLKGATE_CON(i), 0xffff0000);

	for (i = 0; i < PMU1CRU_CLKGATE_CON_CNT; i++)
		mmio_write_32(PMU1_CRU_BASE + PMU1CRU_CLKGATE_CON(i), 0xffff0000);

	for (i = 0; i < PMU1SCRU_CLKGATE_CON_CNT; i++)
		mmio_write_32(PMU1_CRU_BASE + PMU1SCRU_CLKGATE_CON(i), 0xffff0000);
}

void clk_gate_con_save(void)
{
	int i, j = 0;

	for (i = 0; i < CRU_CLKGATE_CON_CNT; i++, j++)
		clk_save[j] = mmio_read_32(CRU_BASE + CRU_CLKGATE_CON(i));

	for (i = 0; i < PHP_CRU_CLKGATE_CON_CNT; i++, j++)
		clk_save[j] = mmio_read_32(PHP_CRU_BASE + PHP_CRU_CLKGATE_CON(i));

	for (i = 0; i < SECURE_CRU_CLKGATE_CON_CNT; i++, j++)
		clk_save[j] = mmio_read_32(SECURE_CRU_BASE + SECURE_CRU_CLKGATE_CON(i));

	for (i = 0; i < SECURE_SCRU_CLKGATE_CON_CNT; i++, j++)
		clk_save[j] = mmio_read_32(SECURE_CRU_BASE + SECURE_SCRU_CLKGATE_CON(i));

	for (i = 0; i < PMU1CRU_CLKGATE_CON_CNT; i++, j++)
		clk_save[j] = mmio_read_32(PMU1_CRU_BASE + PMU1CRU_CLKGATE_CON(i));

	for (i = 0; i < PMU1SCRU_CLKGATE_CON_CNT; i++, j++)
		clk_save[j] = mmio_read_32(PMU1_CRU_BASE + PMU1SCRU_CLKGATE_CON(i));
}

void clk_gate_con_restore(void)
{
	int i, j = 0;

	for (i = 0; i < CRU_CLKGATE_CON_CNT; i++, j++)
		mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(i),
			      WITH_16BITS_WMSK(clk_save[j]));

	for (i = 0; i < PHP_CRU_CLKGATE_CON_CNT; i++, j++)
		mmio_write_32(PHP_CRU_BASE + PHP_CRU_CLKGATE_CON(i),
			      WITH_16BITS_WMSK(clk_save[j]));

	for (i = 0; i < SECURE_CRU_CLKGATE_CON_CNT; i++, j++)
		mmio_write_32(SECURE_CRU_BASE + SECURE_CRU_CLKGATE_CON(i),
			      WITH_16BITS_WMSK(clk_save[j]));

	for (i = 0; i < SECURE_SCRU_CLKGATE_CON_CNT; i++, j++)
		mmio_write_32(SECURE_CRU_BASE + SECURE_SCRU_CLKGATE_CON(i),
			      WITH_16BITS_WMSK(clk_save[j]));

	for (i = 0; i < PMU1CRU_CLKGATE_CON_CNT; i++, j++)
		mmio_write_32(PMU1_CRU_BASE + PMU1CRU_CLKGATE_CON(i),
			      WITH_16BITS_WMSK(clk_save[j]));

	for (i = 0; i < PMU1SCRU_CLKGATE_CON_CNT; i++, j++)
		mmio_write_32(PMU1_CRU_BASE + PMU1SCRU_CLKGATE_CON(i),
			      WITH_16BITS_WMSK(clk_save[j]));
}

void pmu_bus_idle_req(uint32_t bus, uint32_t state)
{
	uint32_t wait_cnt = 0;

	mmio_write_32(PMU_BASE + PMU2_BUS_IDLE_SFTCON(bus / 16),
		      BITS_WITH_WMASK(state, 0x1, bus % 16));

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

	mmio_write_32(PMU_BASE + PMU2_PWR_GATE_SFTCON(pd / 16),
		      BITS_WITH_WMASK(pd_state, 0x1, pd % 16));
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
		pmu_bus_idle_req(pmu_bus_id_npusys, state);
		break;
	case pmu_pd_secure:
		pmu_bus_idle_req(pmu_bus_id_secure, state);
		break;
	case pmu_pd_nvm:
		pmu_bus_idle_req(pmu_bus_id_nvm, state);
		break;
	case pmu_pd_sd_gmac:
		pmu_bus_idle_req(pmu_bus_id_gmac, state);
		break;
	case pmu_pd_audio:
		pmu_bus_idle_req(pmu_bus_id_audio, state);
		break;
	case pmu_pd_php:
		pmu_bus_idle_req(pmu_bus_id_php, state);
		break;
	case pmu_pd_subphp:
		break;
	case pmu_pd_vop:
		pmu_bus_idle_req(pmu_bus_id_vop, state);
		break;
	case pmu_pd_vop_smart:
		break;
	case pmu_pd_vop_clst:
		break;
	case pmu_pd_vo1:
		pmu_bus_idle_req(pmu_bus_id_vo1, state);
		break;
	case pmu_pd_vo0:
		pmu_bus_idle_req(pmu_bus_id_vo0, state);
		break;
	case pmu_pd_usb:
		pmu_bus_idle_req(pmu_bus_id_usb, state);
		break;
	case pmu_pd_vi:
		pmu_bus_idle_req(pmu_bus_id_vi, state);
		break;
	case pmu_pd_vepu0:
		pmu_bus_idle_req(pmu_bus_id_vepu0, state);
		break;
	case pmu_pd_vepu1:
		pmu_bus_idle_req(pmu_bus_id_vepu1, state);
		break;
	case pmu_pd_vdec:
		pmu_bus_idle_req(pmu_bus_id_vdec, state);
		break;
	case pmu_pd_vpu:
		pmu_bus_idle_req(pmu_bus_id_vpu, state);
		break;
	case pmu_pd_nputop:
		pmu_bus_idle_req(pmu_bus_id_nputop, state);
		break;
	case pmu_pd_npu0:
		pmu_bus_idle_req(pmu_bus_id_npu0, state);
		break;
	case pmu_pd_npu1:
		pmu_bus_idle_req(pmu_bus_id_npu1, state);
		break;
	case pmu_pd_gpu:
		pmu_bus_idle_req(pmu_bus_id_gpu, state);
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
	ddr_data.pmu2_pwrgt_sft_con0 = mmio_read_32(PMU_BASE + PMU2_PWR_GATE_SFTCON(0));

	qos_save();

	pd_usb2phy_save();

	if ((ddr_data.pmu_pd_st & BIT(pmu_pd_php)) == 0)
		pd_php_save();
}

static void pmu_power_domains_resume(void)
{
	int i;

	for (i = 0; i < pmu_pd_id_max; i++) {
		/* vop smart/clst pd is not controlled by pmu */
		if (i == pmu_pd_vop_smart || i == pmu_pd_vop_clst)
			continue;

		pmu_set_power_domain(i, !!(ddr_data.pmu_pd_st & BIT(i)));
	}

	/* restore vop smart/clst pd of pmu2_pwrgt_sft_con0 */
	mmio_write_32(PMU_BASE + PMU2_PWR_GATE_SFTCON(0),
		      0x30000000 | ddr_data.pmu2_pwrgt_sft_con0);

	for (i = pmu_bus_id_max - 1; i >= 0; i--)
		pmu_bus_idle_req(i, !!(ddr_data.bus_idle_st & BIT(i)));

	if ((ddr_data.pmu_pd_st & BIT(pmu_pd_php)) == 0)
		pd_php_restore();

	pd_usb2phy_restore();

	qos_restore();
}

static void ddr_sleep_config(void)
{
	ddr_data.ddrgrf_cha_con2 =
		mmio_read_32(DDR_GRF_BASE + DDRGRF_CHA_CON(2));
	ddr_data.ddrgrf_chb_con2 =
		mmio_read_32(DDR_GRF_BASE + DDRGRF_CHB_CON(2));

	mmio_write_32(DDR_GRF_BASE + DDRGRF_CHA_CON(2), 0x0a000a00);
	mmio_write_32(DDR_GRF_BASE + DDRGRF_CHB_CON(2), 0x0a000a00);
}

static void ddr_sleep_config_restore(void)
{
	mmio_write_32(DDR_GRF_BASE + DDRGRF_CHA_CON(2),
		      WITH_16BITS_WMSK(ddr_data.ddrgrf_cha_con2));
	mmio_write_32(DDR_GRF_BASE + DDRGRF_CHB_CON(2),
		      WITH_16BITS_WMSK(ddr_data.ddrgrf_chb_con2));
}

static void sleep_pin_config(void)
{
	/* pwr0 sleep: gpio0_a3 */
	mmio_write_32(PMU0_GRF_BASE + PMU0GRF_SOC_CON(1),
		      BITS_WITH_WMASK(0x7, 0xf, 0));
	mmio_write_32(PMU0_GRF_BASE + PMU0GRF_SOC_CON(0),
		      BITS_WITH_WMASK(0, 0x1, 7));
	mmio_write_32(PMU0_IOC_BASE + PMUIO0_IOC_GPIO0A_IOMUX_SEL_L,
		      BITS_WITH_WMASK(9, 0xfu, 12));
}

static void pmu_sleep_config(void)
{
	uint32_t pmu1_wkup_int_con;
	uint32_t pmu1_pwr_con, pmu1_ddr_pwr_con, pmu1cru_pwr_con, pmu1_pll_pd_con;
	uint32_t pmu2_bus_idle_con[2], pmu2_pwr_gt_con[2];
	uint32_t key_upd_msk = ddr_data.pmu_pd_st & BIT(pmu_pd_vop) ? 0x3 : 0x7;
	uint32_t fw_lkp_upd_msk = ddr_data.pmu_pd_st & BIT(pmu_pd_npu) ? 0x3 : 0x7;
	uint32_t fw_ddr_upd_msk = key_upd_msk;
	uint32_t pmu_pd_st = mmio_read_32(PMU_BASE + PMU2_PWR_GATE_ST);
	uint32_t bus_idle_st = mmio_read_32(PMU_BASE + PMU2_BUS_IDLE_ST);

	ddr_data.pmu2_bisr_glb_con = mmio_read_32(PMU_BASE + PMU2_BISR_GLB_CON);

	ddr_data.pmu2_fast_pwr_con =
		mmio_read_32(PMU_BASE + PMU2_FAST_POWER_CON);

	ddr_data.pmu2_c0_ack_sel_con0 =
		mmio_read_32(PMU_BASE + PMU2_C0_PWRACK_BYPASS_CON(0));
	ddr_data.pmu2_c1_ack_sel_con0 =
		mmio_read_32(PMU_BASE + PMU2_C1_PWRACK_BYPASS_CON(0));
	ddr_data.pmu2_c2_ack_sel_con0 =
		mmio_read_32(PMU_BASE + PMU2_C2_PWRACK_BYPASS_CON(0));
	ddr_data.pmu0grf_soc_con5 =
		mmio_read_32(PMU0_GRF_BASE + PMU0GRF_SOC_CON(5));

	/* set tsadc_shut_m0 pin iomux to gpio */
	mmio_write_32(PMU0_IOC_BASE + PMUIO0_IOC_GPIO0A_IOMUX_SEL_L,
		      BITS_WITH_WMASK(0, 0xf, 4));

	pmu1_wkup_int_con =
		BIT(pmu_wkup_cpu0_int) |
		BIT(pmu_wkup_gpio0_int);

	pmu1_pwr_con =
		BIT(pmu_powermode_en) |
		/* BIT(pmu_scu0_byp) | */
		/* BIT(pmu_scu1_byp) | */
		/* BIT(pmu_cci_byp) | */
		/* BIT(pmu_bus_byp) | */
		/* BIT(pmu_ddr_byp) | */
		/* BIT(pmu_pwrgt_byp) | */
		/* BIT(pmu_cru_byp) | */
		BIT(pmu_qch_byp) |
		/* BIT(pmu_wfi_byp) | */
		BIT(pmu_slp_cnt_en);

	pmu1_ddr_pwr_con = 0;

	pmu1_pll_pd_con =
		BIT(pmu_bpll_pd_en) |
		BIT(pmu_lpll_pd_en) |
		BIT(pmu_spll_pd_en) |
		BIT(pmu_gpll_pd_en) |
		BIT(pmu_cpll_pd_en) |
		BIT(pmu_ppll_pd_en) |
		BIT(pmu_aupll_pd_en) |
		BIT(pmu_vpll_pd_en);

	pmu1cru_pwr_con =
		BIT(pmu_alive_osc_mode_en) |
		BIT(pmu_io_sleep_en) |
		BIT(pmu_power_off_en);

	pmu2_bus_idle_con[0] = 0xffff & ~(bus_idle_st & 0xffff);
	pmu2_bus_idle_con[1] = 0x3fff & ~(bus_idle_st >> 16);

	pmu2_pwr_gt_con[0] = 0xffff & ~(pmu_pd_st & 0xffff);
	pmu2_pwr_gt_con[1] = 0x03ff & ~(pmu_pd_st >> 16);

	pmu2_pwr_gt_con[0] &=
		~(BIT(pmu_pd_secure) |
		  BIT(pmu_pd_bus) |
		  BIT(pmu_pd_center) |
		  BIT(pmu_pd_ddr));

	mmio_write_32(PMU_BASE + PMU2_BUS_IDLEACK_BYPASS_CON, 0x00030003);
	mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_KEYUPD_CON0, 0x03ff0000);

	/* disable repair */
	mmio_write_32(PMU_BASE + PMU2_BISR_GLB_CON, 0x00010000);

	/* disable ddr_hash_key update.
	 * enable disable ddr_hash_key auto update.
	 * wait ddr_hash_key auto update.
	 */
	mmio_write_32(SYS_SGRF_BASE + SYSSGRF_SOC_CON(19),
		      BITS_WITH_WMASK(key_upd_msk, 0x7, 8));
	mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_KEYUPD_CON1,
		      BITS_WITH_WMASK(fw_lkp_upd_msk, 0x7, 10));
	mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_KEYUPD_CON1,
		      BITS_WITH_WMASK(fw_ddr_upd_msk, 0x7u, 13));

	mmio_write_32(PMU_BASE + PMU0_PMIC_STABLE_CNT_THRES, 24000 * 5);
	mmio_write_32(PMU_BASE + PMU0_OSC_STABLE_CNT_THRES, 24000 * 5);

	mmio_write_32(PMU_BASE + PMU1_OSC_STABLE_CNT_THRESH, 24000 * 5);
	mmio_write_32(PMU_BASE + PMU1_STABLE_CNT_THRESH, 24000 * 5);

	mmio_write_32(PMU_BASE + PMU1_SLEEP_CNT_THRESH, 24000 * 15);

	/* Pmu's clk has switched to 24M back When pmu FSM counts
	 * the follow counters, so we should use 24M to calculate
	 * these counters.
	 */
	mmio_write_32(PMU_BASE + PMU0_WAKEUP_RST_CLR_CNT_THRES, 12000);

	mmio_write_32(PMU_BASE + PMU1_WAKEUP_RST_CLR_CNT_THRESH, 12000);
	mmio_write_32(PMU_BASE + PMU1_PLL_LOCK_CNT_THRESH, 12000);
	mmio_write_32(PMU_BASE + PMU1_PWM_SWITCH_CNT_THRESH,
		      24000 * 2);

	mmio_write_32(PMU_BASE + PMU2_SCU0_PWRUP_CNT_THRESH, 0);
	mmio_write_32(PMU_BASE + PMU2_SCU0_PWRDN_CNT_THRESH, 0);
	mmio_write_32(PMU_BASE + PMU2_SCU0_STABLE_CNT_THRESH, 0);

	mmio_write_32(PMU_BASE + PMU2_FAST_PWRUP_CNT_THRESH_0, 0);
	mmio_write_32(PMU_BASE + PMU2_FAST_PWRDN_CNT_THRESH_0, 0);
	mmio_write_32(PMU_BASE + PMU2_FAST_PWRUP_CNT_THRESH_1, 0);
	mmio_write_32(PMU_BASE + PMU2_FAST_PWRDN_CNT_THRESH_1, 0);
	mmio_write_32(PMU_BASE + PMU2_FAST_PWRUP_CNT_THRESH_2, 0);
	mmio_write_32(PMU_BASE + PMU2_FAST_PWRDN_CNT_THRESH_2, 0);
	mmio_write_32(PMU_BASE + PMU2_FAST_POWER_CON, 0xffff0007);

	/* pmu_clst_idle_con */
	mmio_write_32(PMU_BASE + PMU2_CLUSTER0_IDLE_CON, 0xffff0007);
	mmio_write_32(PMU_BASE + PMU2_CLUSTER1_IDLE_CON, 0xffff0007);

	/* pmu_scu_pwr_con */
	/* L2's flush and idle by hardware, so need to enable wfil2 bypass */
	mmio_write_32(PMU_BASE + PMU2_SCU0_PWR_CON, 0xffff020f);
	mmio_write_32(PMU_BASE + PMU2_SCU1_PWR_CON, 0xffff020f);
	mmio_write_32(PMU_BASE + PMU2_SCU0_AUTO_PWR_CON, 0x00070000);
	mmio_write_32(PMU_BASE + PMU2_SCU1_AUTO_PWR_CON, 0x00070000);

	mmio_write_32(PMU_BASE + PMU2_CCI_PWR_CON, 0xffff0009);

	/* pmu_int_msk_con */
	/* mmio_write_32(PMU_BASE + PMU1_INT_MASK_CON, BITS_WITH_WMASK(1, 0x1, 0)); */

	/* pmu_pwr_con */
	mmio_write_32(PMU_BASE + PMU1_PWR_CON, WITH_16BITS_WMSK(pmu1_pwr_con));

	/* pmu_cru_pwr_conx */
	mmio_write_32(PMU_BASE + PMU1_CRU_PWR_CON(0), WITH_16BITS_WMSK(pmu1cru_pwr_con));

	/* pmu_ddr_pwr_con */
	mmio_write_32(PMU_BASE + PMU0_DDR_RET_CON(1), 0xffff0000);
	mmio_write_32(PMU_BASE + PMU1_DDR_PWR_CON(0), WITH_16BITS_WMSK(pmu1_ddr_pwr_con));
	mmio_write_32(PMU_BASE + PMU1_DDR_PWR_CON(1), WITH_16BITS_WMSK(pmu1_ddr_pwr_con));
	mmio_write_32(PMU_BASE + PMU1_DDR_AXIPWR_CON(0), 0x03ff03ff);
	mmio_write_32(PMU_BASE + PMU1_DDR_AXIPWR_CON(1), 0x03ff03ff);

	/* pll_pd */
	mmio_write_32(PMU_BASE + PMU1_PLLPD_CON(0), WITH_16BITS_WMSK(pmu1_pll_pd_con));

	/* bus idle */
	mmio_write_32(PMU_BASE + PMU2_BUS_IDLE_CON(0), WITH_16BITS_WMSK(pmu2_bus_idle_con[0]));
	mmio_write_32(PMU_BASE + PMU2_BUS_IDLE_CON(1), WITH_16BITS_WMSK(pmu2_bus_idle_con[1]));

	/* power gate */
	mmio_write_32(PMU_BASE + PMU2_PWR_GATE_CON(0), WITH_16BITS_WMSK(pmu2_pwr_gt_con[0]));
	mmio_write_32(PMU_BASE + PMU2_PWR_GATE_CON(1), WITH_16BITS_WMSK(pmu2_pwr_gt_con[1]));

	/* vol gate */
	mmio_write_32(PMU_BASE + PMU2_VOL_GATE_SFTCON(0), 0xffff0031);
	mmio_write_32(PMU_BASE + PMU2_VOL_GATE_SFTCON(1), 0xffff0200);

	/* wakeup source */
	mmio_write_32(PMU_BASE + PMU1_WAKEUP_INT_CON, pmu1_wkup_int_con);

	/* ppll clamp */
	mmio_write_32(PMU0_GRF_BASE + PMU0GRF_SOC_CON(5), 0x00400040);

	/* usbphy clamp */
	mmio_write_32(PMU0_GRF_BASE + PMU0GRF_SOC_CON(5),
		      BITS_WITH_WMASK(0x9, 0x9, 2));

	/* big core pwr ack bypass */
	mmio_write_32(PMU_BASE + PMU2_C0_PWRACK_BYPASS_CON(0), 0x01000100);
	mmio_write_32(PMU_BASE + PMU2_C1_PWRACK_BYPASS_CON(0), 0x01000100);
	mmio_write_32(PMU_BASE + PMU2_C2_PWRACK_BYPASS_CON(0), 0x01000100);
}

static void pmu_sleep_restore(void)
{
	mmio_write_32(PMU_BASE + PMU0_INFO_TX_CON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU2_DEBUG_INFO_SEL, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU2_CLUSTER0_IDLE_CON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU2_SCU0_PWR_CON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU2_CCI_PWR_CON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU1_INT_MASK_CON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU1_PWR_CON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU1_CRU_PWR_CON(0), 0xffff0000);
	mmio_write_32(PMU_BASE + PMU1_DDR_PWR_CON(0), 0xffff0000);
	mmio_write_32(PMU_BASE + PMU1_DDR_PWR_CON(1), 0xffff0000);
	mmio_write_32(PMU_BASE + PMU1_DDR_AXIPWR_CON(0), 0xffff0000);
	mmio_write_32(PMU_BASE + PMU1_DDR_AXIPWR_CON(1), 0xffff0000);
	mmio_write_32(PMU_BASE + PMU1_PLLPD_CON(0), 0xffff0000);
	mmio_write_32(PMU_BASE + PMU2_BUS_IDLE_CON(0), 0xffff0000);
	mmio_write_32(PMU_BASE + PMU2_BUS_IDLE_CON(1), 0xffff0000);
	mmio_write_32(PMU_BASE + PMU2_PWR_GATE_CON(0), 0xffff0000);
	mmio_write_32(PMU_BASE + PMU2_PWR_GATE_CON(1), 0xffff0000);
	mmio_write_32(PMU_BASE + PMU2_VOL_GATE_SFTCON(0), 0xffff0000);
	mmio_write_32(PMU_BASE + PMU2_VOL_GATE_SFTCON(1), 0xffff0000);
	mmio_write_32(PMU_BASE + PMU2_BUS_IDLEACK_BYPASS_CON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU1_WAKEUP_INT_CON, 0);
	mmio_write_32(PMU_BASE + PMU2_FAST_POWER_CON,
		      WITH_16BITS_WMSK(ddr_data.pmu2_fast_pwr_con));
	mmio_write_32(PMU_BASE + PMU2_BISR_GLB_CON,
		      WITH_16BITS_WMSK(ddr_data.pmu2_bisr_glb_con));

	mmio_write_32(PMU_BASE + PMU2_C0_PWRACK_BYPASS_CON(0),
		      WITH_16BITS_WMSK(ddr_data.pmu2_c0_ack_sel_con0));
	mmio_write_32(PMU_BASE + PMU2_C1_PWRACK_BYPASS_CON(0),
		      WITH_16BITS_WMSK(ddr_data.pmu2_c1_ack_sel_con0));
	mmio_write_32(PMU_BASE + PMU2_C2_PWRACK_BYPASS_CON(0),
		      WITH_16BITS_WMSK(ddr_data.pmu2_c2_ack_sel_con0));

	mmio_write_32(PMU0_GRF_BASE + PMU0GRF_SOC_CON(5),
		      WITH_16BITS_WMSK(ddr_data.pmu0grf_soc_con5));
}

static void secure_watchdog_disable(void)
{
	ddr_data.sys_sgrf_soc_con0 =
		mmio_read_32(SYS_SGRF_BASE + SYSSGRF_SOC_CON(0));

	/* pause wdt_s */
	mmio_write_32(SYS_SGRF_BASE + SYSSGRF_SOC_CON(0),
		      BITS_WITH_WMASK(1, 0x1, 14));
}

static void secure_watchdog_restore(void)
{
	mmio_write_32(SYS_SGRF_BASE + SYSSGRF_SOC_CON(0),
		      ddr_data.sys_sgrf_soc_con0 |
		      BITS_WMSK(0x1, 14));

	if (mmio_read_32(WDT_S_BASE + WDT_CR) & WDT_EN)
		mmio_write_32(WDT_S_BASE + WDT_CRR, 0x76);
}

static void soc_sleep_config(void)
{
	ddr_data.pmu0grf_soc_con0 =
		mmio_read_32(PMU0_GRF_BASE + PMU0GRF_SOC_CON(0));
	ddr_data.pmu0grf_soc_con1 =
		mmio_read_32(PMU0_GRF_BASE + PMU0GRF_SOC_CON(1));

	ddr_data.gpio0a_iomux_l =
		mmio_read_32(PMU0_IOC_BASE + PMUIO0_IOC_GPIO0A_IOMUX_SEL_L);
	ddr_data.gpio0a_iomux_h =
		mmio_read_32(PMU0_IOC_BASE + PMUIO0_IOC_GPIO0A_IOMUX_SEL_H);
	ddr_data.gpio0b_iomux_l =
		mmio_read_32(PMU0_IOC_BASE + PMUIO0_IOC_GPIO0B_IOMUX_SEL_L);

	sleep_pin_config();
	pmu_sleep_config();
	ddr_sleep_config();
	secure_watchdog_disable();
}

static void soc_sleep_restore(void)
{
	secure_watchdog_restore();
	ddr_sleep_config_restore();
	pmu_sleep_restore();

	mmio_write_32(PMU0_IOC_BASE + PMUIO0_IOC_GPIO0A_IOMUX_SEL_L,
		      WITH_16BITS_WMSK(ddr_data.gpio0a_iomux_l));
	mmio_write_32(PMU0_IOC_BASE + PMUIO0_IOC_GPIO0A_IOMUX_SEL_H,
		      WITH_16BITS_WMSK(ddr_data.gpio0a_iomux_h));
	mmio_write_32(PMU0_IOC_BASE + PMUIO0_IOC_GPIO0B_IOMUX_SEL_L,
		      WITH_16BITS_WMSK(ddr_data.gpio0b_iomux_l));

	mmio_write_32(PMU0_GRF_BASE + PMU0GRF_SOC_CON(1),
		      WITH_16BITS_WMSK(ddr_data.pmu0grf_soc_con1));
	mmio_write_32(PMU0_GRF_BASE + PMU0GRF_SOC_CON(0),
		      WITH_16BITS_WMSK(ddr_data.pmu0grf_soc_con0));
}

static void pm_pll_suspend(void)
{
	ddr_data.cru_mode_con = mmio_read_32(CRU_BASE + 0x280);
	ddr_data.secure_cru_mode = mmio_read_32(SECURE_CRU_BASE + 0x4280);

	/* bpll gpll vpll aupll cpll spll switch to slow mode */
	mmio_write_32(CRU_BASE + 0x280, 0x03ff0000);
	mmio_write_32(SECURE_CRU_BASE + 0x4280, 0x00030000);

	/* hclk_pmu_cm0_root_i_sel to 24M */
	mmio_write_32(PMU1_CRU_BASE + PMU1CRU_CLKSEL_CON(4),
		      BITS_WITH_WMASK(0x3, 0x3, 2));
}

static void pm_pll_restore(void)
{
	mmio_write_32(CRU_BASE + 0x280, WITH_16BITS_WMSK(ddr_data.cru_mode_con));
	mmio_write_32(SECURE_CRU_BASE + 0x4280,
		      WITH_16BITS_WMSK(ddr_data.secure_cru_mode));
}

int rockchip_soc_sys_pwr_dm_suspend(void)
{
	psram_sleep_cfg->pm_flag &= ~PM_WARM_BOOT_BIT;

	clk_gate_con_save();
	clk_gate_con_disable();
	dmc_save();
	pmu_power_domains_suspend();
	soc_sleep_config();
	pm_pll_suspend();
	pd_core_save();

	return 0;
}

int rockchip_soc_sys_pwr_dm_resume(void)
{
	pd_core_restore();
	pm_pll_restore();
	soc_sleep_restore();
	pmu_power_domains_resume();
	plat_rockchip_gic_cpuif_enable();
	dmc_restore();
	clk_gate_con_restore();

	psram_sleep_cfg->pm_flag |= PM_WARM_BOOT_BIT;

	return 0;
}

static int rockchip_reboot_is_rbrom(void)
{
	return mmio_read_32(PMU0_GRF_BASE + PMU0GRF_OS_REG(16)) ==
	       BOOT_BROM_DOWNLOAD;
}

static void rockchip_soc_soft_reset_check_rstout(void)
{
	/*
	 * Maskrom enter maskrom-usb mode according to os_reg0 which
	 * will be reset by NPOR. So disable tsadc_shut_m0 if we want
	 * to maskrom-usb mode.
	 */
	if (rockchip_reboot_is_rbrom() != 0) {
		/* write BOOT_BROM_DOWNLOAD to os_reg0 */
		mmio_write_32(PMU1_GRF_BASE + PMU1GRF_OS_REG(0), BOOT_BROM_DOWNLOAD);

		/* disable first/tsadc/wdt reset output */
		mmio_write_32(PMU1SGRF_BASE + PMU1SGRF_SOC_CON(0), 0x00070000);

		/* clear reset hold */
		mmio_write_32(PMU0SGRF_BASE + PMU0SGRF_SOC_CON(1), 0xffff0000);
		mmio_write_32(PMU1SGRF_BASE + PMU1SGRF_SOC_CON(16), 0xffff0000);
		mmio_write_32(PMU1SGRF_BASE + PMU1SGRF_SOC_CON(17), 0xffff0000);
	}
}

void __dead2 rockchip_soc_soft_reset(void)
{
	rockchip_soc_soft_reset_check_rstout();

	/* pll slow mode */
	mmio_write_32(CRU_BASE + CRU_MODE_CON, 0x003f0000);

	dsb();
	isb();

	INFO("system reset......\n");
	mmio_write_32(CRU_BASE + CRU_GLB_SRST_FST, GLB_SRST_FST_CFG_VAL);

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

	/* gpio0_a3 config output */
	mmio_write_32(GPIO0_BASE + GPIO_SWPORT_DDR_L,
		      BITS_WITH_WMASK(1, 0x1, 3));

	/* gpio0_a3 config output high level */
	mmio_write_32(GPIO0_BASE + GPIO_SWPORT_DR_L,
		      BITS_WITH_WMASK(1, 0x1, 3));
	dsb();

	/*
	 * Maybe the HW needs some times to reset the system,
	 * so we do not hope the core to execute valid codes.
	 */
	while (1) {
		wfi();
	}
}

static void rockchip_pmu_pd_repair_init(void)
{
	INFO("enable memory repair\n");
	/* Enable gpu and npu repair */
	mmio_write_32(PMU_BASE + PMU2_BISR_PDGEN_CON(1),
		      BITS_WITH_WMASK(0xf, 0xf, 6));
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

	/*
	 * When perform idle operation, corresponding clock can be
	 * opened or gated automatically.
	 */
	mmio_write_32(PMU_BASE + PMU2_NOC_AUTO_CON(0), 0xffffffff);
	mmio_write_32(PMU_BASE + PMU2_NOC_AUTO_CON(1), 0xffffffff);

	/* remap pmusram to 0x00000000 */
	mmio_write_32(PMU0SGRF_BASE + PMU0SGRF_SOC_CON(2), BITS_WITH_WMASK(1, 0x3, 0));

	/* enable power off VD_NPU by hrdware */
	mmio_write_32(PMU_BASE + PMU2_VOL_GATE_SFTCON(0),
		      BITS_WITH_WMASK(0x1, 0x1, 0));

	rockchip_pmu_pd_repair_init();

	pm_reg_rgns_init();
}
