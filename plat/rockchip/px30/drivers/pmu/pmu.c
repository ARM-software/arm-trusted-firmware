/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <common/debug.h>
#include <drivers/console.h>
#include <drivers/delay_timer.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include <cpus_on_fixed_addr.h>
#include <plat_private.h>
#include <pmu.h>
#include <px30_def.h>
#include <soc.h>

DEFINE_BAKERY_LOCK(rockchip_pd_lock);
#define rockchip_pd_lock_init()	bakery_lock_init(&rockchip_pd_lock)
#define rockchip_pd_lock_get()	bakery_lock_get(&rockchip_pd_lock)
#define rockchip_pd_lock_rls()	bakery_lock_release(&rockchip_pd_lock)

static struct psram_data_t *psram_boot_cfg =
	(struct psram_data_t *)&sys_sleep_flag_sram;

/*
 * There are two ways to powering on or off on core.
 * 1) Control it power domain into on or off in PMU_PWRDN_CON reg,
 *    it is core_pwr_pd mode
 * 2) Enable the core power manage in PMU_CORE_PM_CON reg,
 *     then, if the core enter into wfi, it power domain will be
 *     powered off automatically. it is core_pwr_wfi or core_pwr_wfi_int mode
 * so we need core_pm_cfg_info to distinguish which method be used now.
 */

static uint32_t cores_pd_cfg_info[PLATFORM_CORE_COUNT]
#if USE_COHERENT_MEM
__attribute__ ((section("tzfw_coherent_mem")))
#endif
;

struct px30_sleep_ddr_data {
	uint32_t clk_sel0;
	uint32_t cru_mode_save;
	uint32_t cru_pmu_mode_save;
	uint32_t ddrc_hwlpctl;
	uint32_t ddrc_pwrctrl;
	uint32_t ddrgrf_con0;
	uint32_t ddrgrf_con1;
	uint32_t ddrstdby_con0;
	uint32_t gpio0b_iomux;
	uint32_t gpio0c_iomux;
	uint32_t pmu_pwrmd_core_l;
	uint32_t pmu_pwrmd_core_h;
	uint32_t pmu_pwrmd_cmm_l;
	uint32_t pmu_pwrmd_cmm_h;
	uint32_t pmu_wkup_cfg2_l;
	uint32_t pmu_cru_clksel_con0;
	uint32_t pmugrf_soc_con0;
	uint32_t pmusgrf_soc_con0;
	uint32_t pmic_slp_iomux;
	uint32_t pgrf_pvtm_con[2];
	uint32_t cru_clk_gate[CRU_CLKGATES_CON_CNT];
	uint32_t cru_pmu_clk_gate[CRU_PMU_CLKGATE_CON_CNT];
	uint32_t cru_plls_con_save[END_PLL_ID][PLL_CON_CNT];
	uint32_t cpu_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t gpu_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t isp_128m_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t isp_rd_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t isp_wr_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t isp_m1_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t vip_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t rga_rd_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t rga_wr_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t vop_m0_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t vop_m1_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t vpu_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t vpu_r128_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t dcf_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t dmac_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t crypto_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t gmac_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t emmc_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t nand_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t sdio_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t sfc_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t sdmmc_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t usb_host_qos[CPU_AXI_QOS_NUM_REGS];
	uint32_t usb_otg_qos[CPU_AXI_QOS_NUM_REGS];
};

static struct px30_sleep_ddr_data ddr_data
#if USE_COHERENT_MEM
__attribute__ ((section("tzfw_coherent_mem")))
#endif
;

static inline uint32_t get_cpus_pwr_domain_cfg_info(uint32_t cpu_id)
{
	assert(cpu_id < PLATFORM_CORE_COUNT);
	return cores_pd_cfg_info[cpu_id];
}

static inline void set_cpus_pwr_domain_cfg_info(uint32_t cpu_id, uint32_t value)
{
	assert(cpu_id < PLATFORM_CORE_COUNT);
	cores_pd_cfg_info[cpu_id] = value;
#if !USE_COHERENT_MEM
	flush_dcache_range((uintptr_t)&cores_pd_cfg_info[cpu_id],
			   sizeof(uint32_t));
#endif
}

static inline uint32_t pmu_power_domain_st(uint32_t pd)
{
	return mmio_read_32(PMU_BASE + PMU_PWRDN_ST) & BIT(pd) ?
	       pmu_pd_off :
	       pmu_pd_on;
}

static int pmu_power_domain_ctr(uint32_t pd, uint32_t pd_state)
{
	uint32_t loop = 0;
	int ret = 0;

	rockchip_pd_lock_get();

	mmio_write_32(PMU_BASE + PMU_PWRDN_CON,
		      BITS_WITH_WMASK(pd_state, 0x1, pd));
	dsb();

	while ((pmu_power_domain_st(pd) != pd_state) && (loop < PD_CTR_LOOP)) {
		udelay(1);
		loop++;
	}

	if (pmu_power_domain_st(pd) != pd_state) {
		WARN("%s: %d, %d, error!\n", __func__, pd, pd_state);
		ret = -EINVAL;
	}

	rockchip_pd_lock_rls();

	return ret;
}

static inline uint32_t pmu_bus_idle_st(uint32_t bus)
{
	return !!((mmio_read_32(PMU_BASE + PMU_BUS_IDLE_ST) & BIT(bus)) &&
		  (mmio_read_32(PMU_BASE + PMU_BUS_IDLE_ST) & BIT(bus + 16)));
}

static void pmu_bus_idle_req(uint32_t bus, uint32_t state)
{
	uint32_t wait_cnt = 0;

	mmio_write_32(PMU_BASE + PMU_BUS_IDLE_REQ,
		      BITS_WITH_WMASK(state, 0x1, bus));

	while (pmu_bus_idle_st(bus) != state &&
	       wait_cnt < BUS_IDLE_LOOP) {
		udelay(1);
		wait_cnt++;
	}

	if (pmu_bus_idle_st(bus) != state)
		WARN("%s:idle_st=0x%x, bus_id=%d\n",
		     __func__, mmio_read_32(PMU_BASE + PMU_BUS_IDLE_ST), bus);
}

static void qos_save(void)
{
	/* scu powerdomain will power off, so cpu qos should be saved */
	SAVE_QOS(ddr_data.cpu_qos, CPU);

	if (pmu_power_domain_st(PD_GPU) == pmu_pd_on)
		SAVE_QOS(ddr_data.gpu_qos, GPU);
	if (pmu_power_domain_st(PD_VI) == pmu_pd_on) {
		SAVE_QOS(ddr_data.isp_128m_qos, ISP_128M);
		SAVE_QOS(ddr_data.isp_rd_qos, ISP_RD);
		SAVE_QOS(ddr_data.isp_wr_qos, ISP_WR);
		SAVE_QOS(ddr_data.isp_m1_qos, ISP_M1);
		SAVE_QOS(ddr_data.vip_qos, VIP);
	}
	if (pmu_power_domain_st(PD_VO) == pmu_pd_on) {
		SAVE_QOS(ddr_data.rga_rd_qos, RGA_RD);
		SAVE_QOS(ddr_data.rga_wr_qos, RGA_WR);
		SAVE_QOS(ddr_data.vop_m0_qos, VOP_M0);
		SAVE_QOS(ddr_data.vop_m1_qos, VOP_M1);
	}
	if (pmu_power_domain_st(PD_VPU) == pmu_pd_on) {
		SAVE_QOS(ddr_data.vpu_qos, VPU);
		SAVE_QOS(ddr_data.vpu_r128_qos, VPU_R128);
	}
	if (pmu_power_domain_st(PD_MMC_NAND) == pmu_pd_on) {
		SAVE_QOS(ddr_data.emmc_qos, EMMC);
		SAVE_QOS(ddr_data.nand_qos, NAND);
		SAVE_QOS(ddr_data.sdio_qos, SDIO);
		SAVE_QOS(ddr_data.sfc_qos, SFC);
	}
	if (pmu_power_domain_st(PD_GMAC) == pmu_pd_on)
		SAVE_QOS(ddr_data.gmac_qos, GMAC);
	if (pmu_power_domain_st(PD_CRYPTO) == pmu_pd_on)
		SAVE_QOS(ddr_data.crypto_qos, CRYPTO);
	if (pmu_power_domain_st(PD_SDCARD) == pmu_pd_on)
		SAVE_QOS(ddr_data.sdmmc_qos, SDMMC);
	if (pmu_power_domain_st(PD_USB) == pmu_pd_on) {
		SAVE_QOS(ddr_data.usb_host_qos, USB_HOST);
		SAVE_QOS(ddr_data.usb_otg_qos, USB_OTG);
	}
}

static void qos_restore(void)
{
	RESTORE_QOS(ddr_data.cpu_qos, CPU);

	if (pmu_power_domain_st(PD_GPU) == pmu_pd_on)
		RESTORE_QOS(ddr_data.gpu_qos, GPU);
	if (pmu_power_domain_st(PD_VI) == pmu_pd_on) {
		RESTORE_QOS(ddr_data.isp_128m_qos, ISP_128M);
		RESTORE_QOS(ddr_data.isp_rd_qos, ISP_RD);
		RESTORE_QOS(ddr_data.isp_wr_qos, ISP_WR);
		RESTORE_QOS(ddr_data.isp_m1_qos, ISP_M1);
		RESTORE_QOS(ddr_data.vip_qos, VIP);
	}
	if (pmu_power_domain_st(PD_VO) == pmu_pd_on) {
		RESTORE_QOS(ddr_data.rga_rd_qos, RGA_RD);
		RESTORE_QOS(ddr_data.rga_wr_qos, RGA_WR);
		RESTORE_QOS(ddr_data.vop_m0_qos, VOP_M0);
		RESTORE_QOS(ddr_data.vop_m1_qos, VOP_M1);
	}
	if (pmu_power_domain_st(PD_VPU) == pmu_pd_on) {
		RESTORE_QOS(ddr_data.vpu_qos, VPU);
		RESTORE_QOS(ddr_data.vpu_r128_qos, VPU_R128);
	}
	if (pmu_power_domain_st(PD_MMC_NAND) == pmu_pd_on) {
		RESTORE_QOS(ddr_data.emmc_qos, EMMC);
		RESTORE_QOS(ddr_data.nand_qos, NAND);
		RESTORE_QOS(ddr_data.sdio_qos, SDIO);
		RESTORE_QOS(ddr_data.sfc_qos, SFC);
	}
	if (pmu_power_domain_st(PD_GMAC) == pmu_pd_on)
		RESTORE_QOS(ddr_data.gmac_qos, GMAC);
	if (pmu_power_domain_st(PD_CRYPTO) == pmu_pd_on)
		RESTORE_QOS(ddr_data.crypto_qos, CRYPTO);
	if (pmu_power_domain_st(PD_SDCARD) == pmu_pd_on)
		RESTORE_QOS(ddr_data.sdmmc_qos, SDMMC);
	if (pmu_power_domain_st(PD_USB) == pmu_pd_on) {
		RESTORE_QOS(ddr_data.usb_host_qos, USB_HOST);
		RESTORE_QOS(ddr_data.usb_otg_qos, USB_OTG);
	}
}

static int pmu_set_power_domain(uint32_t pd_id, uint32_t pd_state)
{
	uint32_t state;

	if (pmu_power_domain_st(pd_id) == pd_state)
		goto out;

	if (pd_state == pmu_pd_on)
		pmu_power_domain_ctr(pd_id, pd_state);

	state = (pd_state == pmu_pd_off) ? bus_idle : bus_active;

	switch (pd_id) {
	case PD_GPU:
		pmu_bus_idle_req(BUS_ID_GPU, state);
		break;
	case PD_VI:
		pmu_bus_idle_req(BUS_ID_VI, state);
		break;
	case PD_VO:
		pmu_bus_idle_req(BUS_ID_VO, state);
		break;
	case PD_VPU:
		pmu_bus_idle_req(BUS_ID_VPU, state);
		break;
	case PD_MMC_NAND:
		pmu_bus_idle_req(BUS_ID_MMC, state);
		break;
	case PD_GMAC:
		pmu_bus_idle_req(BUS_ID_GMAC, state);
		break;
	case PD_CRYPTO:
		pmu_bus_idle_req(BUS_ID_CRYPTO, state);
		break;
	case PD_SDCARD:
		pmu_bus_idle_req(BUS_ID_SDCARD, state);
		break;
	case PD_USB:
		pmu_bus_idle_req(BUS_ID_USB, state);
		break;
	default:
		break;
	}

	if (pd_state == pmu_pd_off)
		pmu_power_domain_ctr(pd_id, pd_state);

out:
	return 0;
}

static uint32_t pmu_powerdomain_state;

static void pmu_power_domains_suspend(void)
{
	uint32_t clkgt_save[CRU_CLKGATES_CON_CNT + CRU_PMU_CLKGATE_CON_CNT];

	clk_gate_con_save(clkgt_save);
	clk_gate_con_disable();
	qos_save();

	pmu_powerdomain_state = mmio_read_32(PMU_BASE + PMU_PWRDN_ST);
	pmu_set_power_domain(PD_GPU, pmu_pd_off);
	pmu_set_power_domain(PD_VI, pmu_pd_off);
	pmu_set_power_domain(PD_VO, pmu_pd_off);
	pmu_set_power_domain(PD_VPU, pmu_pd_off);
	pmu_set_power_domain(PD_MMC_NAND, pmu_pd_off);
	pmu_set_power_domain(PD_GMAC, pmu_pd_off);
	pmu_set_power_domain(PD_CRYPTO, pmu_pd_off);
	pmu_set_power_domain(PD_SDCARD, pmu_pd_off);
	pmu_set_power_domain(PD_USB, pmu_pd_off);

	clk_gate_con_restore(clkgt_save);
}

static void pmu_power_domains_resume(void)
{
	uint32_t clkgt_save[CRU_CLKGATES_CON_CNT + CRU_PMU_CLKGATE_CON_CNT];

	clk_gate_con_save(clkgt_save);
	clk_gate_con_disable();

	if (!(pmu_powerdomain_state & BIT(PD_USB)))
		pmu_set_power_domain(PD_USB, pmu_pd_on);
	if (!(pmu_powerdomain_state & BIT(PD_SDCARD)))
		pmu_set_power_domain(PD_SDCARD, pmu_pd_on);
	if (!(pmu_powerdomain_state & BIT(PD_CRYPTO)))
		pmu_set_power_domain(PD_CRYPTO, pmu_pd_on);
	if (!(pmu_powerdomain_state & BIT(PD_GMAC)))
		pmu_set_power_domain(PD_GMAC, pmu_pd_on);
	if (!(pmu_powerdomain_state & BIT(PD_MMC_NAND)))
		pmu_set_power_domain(PD_MMC_NAND, pmu_pd_on);
	if (!(pmu_powerdomain_state & BIT(PD_VPU)))
		pmu_set_power_domain(PD_VPU, pmu_pd_on);
	if (!(pmu_powerdomain_state & BIT(PD_VO)))
		pmu_set_power_domain(PD_VO, pmu_pd_on);
	if (!(pmu_powerdomain_state & BIT(PD_VI)))
		pmu_set_power_domain(PD_VI, pmu_pd_on);
	if (!(pmu_powerdomain_state & BIT(PD_GPU)))
		pmu_set_power_domain(PD_GPU, pmu_pd_on);

	qos_restore();
	clk_gate_con_restore(clkgt_save);
}

static int check_cpu_wfie(uint32_t cpu)
{
	uint32_t loop = 0, wfie_msk = CKECK_WFEI_MSK << cpu;

	while (!(mmio_read_32(GRF_BASE + GRF_CPU_STATUS1) & wfie_msk) &&
	       (loop < WFEI_CHECK_LOOP)) {
		udelay(1);
		loop++;
	}

	if ((mmio_read_32(GRF_BASE + GRF_CPU_STATUS1) & wfie_msk) == 0) {
		WARN("%s: %d, %d, error!\n", __func__, cpu, wfie_msk);
		return -EINVAL;
	}

	return 0;
}

static int cpus_power_domain_on(uint32_t cpu_id)
{
	uint32_t cpu_pd, apm_value, cfg_info, loop = 0;

	cpu_pd = PD_CPU0 + cpu_id;
	cfg_info = get_cpus_pwr_domain_cfg_info(cpu_id);

	if (cfg_info == core_pwr_pd) {
		/* disable apm cfg */
		mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(cpu_id),
			      WITH_16BITS_WMSK(CORES_PM_DISABLE));
		if (pmu_power_domain_st(cpu_pd) == pmu_pd_on) {
			mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(cpu_id),
				      WITH_16BITS_WMSK(CORES_PM_DISABLE));
			pmu_power_domain_ctr(cpu_pd, pmu_pd_off);
		}
		pmu_power_domain_ctr(cpu_pd, pmu_pd_on);
	} else {
		/* wait cpu down */
		while (pmu_power_domain_st(cpu_pd) == pmu_pd_on && loop < 100) {
			udelay(2);
			loop++;
		}

		/* return error if can't wait cpu down */
		if (pmu_power_domain_st(cpu_pd) == pmu_pd_on) {
			WARN("%s:can't wait cpu down\n", __func__);
			return -EINVAL;
		}

		/* power up cpu in power down state */
		apm_value = BIT(core_pm_sft_wakeup_en);
		mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(cpu_id),
			      WITH_16BITS_WMSK(apm_value));
	}

	return 0;
}

static int cpus_power_domain_off(uint32_t cpu_id, uint32_t pd_cfg)
{
	uint32_t cpu_pd, apm_value;

	cpu_pd = PD_CPU0 + cpu_id;
	if (pmu_power_domain_st(cpu_pd) == pmu_pd_off)
		return 0;

	if (pd_cfg == core_pwr_pd) {
		if (check_cpu_wfie(cpu_id))
			return -EINVAL;
		/* disable apm cfg */
		mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(cpu_id),
			      WITH_16BITS_WMSK(CORES_PM_DISABLE));
		set_cpus_pwr_domain_cfg_info(cpu_id, pd_cfg);
		pmu_power_domain_ctr(cpu_pd, pmu_pd_off);
	} else {
		set_cpus_pwr_domain_cfg_info(cpu_id, pd_cfg);
		apm_value = BIT(core_pm_en) | BIT(core_pm_dis_int);
		if (pd_cfg == core_pwr_wfi_int)
			apm_value |= BIT(core_pm_int_wakeup_en);
		mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(cpu_id),
			      WITH_16BITS_WMSK(apm_value));
	}

	return 0;
}

static void nonboot_cpus_off(void)
{
	uint32_t boot_cpu, cpu;

	boot_cpu = plat_my_core_pos();

	for (cpu = 0; cpu < PLATFORM_CORE_COUNT; cpu++) {
		if (cpu == boot_cpu)
			continue;
		cpus_power_domain_off(cpu, core_pwr_pd);
	}
}

int rockchip_soc_cores_pwr_dm_on(unsigned long mpidr,
				 uint64_t entrypoint)
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

int rockchip_soc_cores_pwr_dm_on_finish(void)
{
	uint32_t cpu_id = plat_my_core_pos();

	mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(cpu_id),
		      WITH_16BITS_WMSK(CORES_PM_DISABLE));
	return PSCI_E_SUCCESS;
}

int rockchip_soc_cores_pwr_dm_off(void)
{
	uint32_t cpu_id = plat_my_core_pos();

	cpus_power_domain_off(cpu_id, core_pwr_wfi);

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
	mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(cpu_id),
		      WITH_16BITS_WMSK(CORES_PM_DISABLE));

	return PSCI_E_SUCCESS;
}

#define CLK_MSK_GATING(msk, con) \
	mmio_write_32(CRU_BASE + (con), ((msk) << 16) | 0xffff)
#define CLK_MSK_UNGATING(msk, con) \
	mmio_write_32(CRU_BASE + (con), ((~(msk)) << 16) | 0xffff)

static uint32_t clk_ungt_msk[CRU_CLKGATES_CON_CNT] = {
	0xe0ff, 0xffff, 0x0000, 0x0000,
	0x0000, 0x0380, 0x0000, 0x0000,
	0x07c0, 0x0000, 0x0000, 0x000f,
	0x0061, 0x1f02, 0x0440, 0x1801,
	0x004b, 0x0000
};

static uint32_t clk_pmu_ungt_msk[CRU_PMU_CLKGATE_CON_CNT] = {
	0xf1ff, 0x0310
};

void clk_gate_suspend(void)
{
	int i;

	for (i = 0; i < CRU_CLKGATES_CON_CNT; i++) {
		ddr_data.cru_clk_gate[i] =
			mmio_read_32(CRU_BASE + CRU_CLKGATES_CON(i));
			mmio_write_32(CRU_BASE + CRU_CLKGATES_CON(i),
				      WITH_16BITS_WMSK(~clk_ungt_msk[i]));
	}

	for (i = 0; i < CRU_PMU_CLKGATE_CON_CNT; i++) {
		ddr_data.cru_pmu_clk_gate[i] =
			mmio_read_32(PMUCRU_BASE + CRU_PMU_CLKGATES_CON(i));
			mmio_write_32(PMUCRU_BASE + CRU_PMU_CLKGATES_CON(i),
				      WITH_16BITS_WMSK(~clk_pmu_ungt_msk[i]));
	}
}

void clk_gate_resume(void)
{
	int i;

	for (i = 0; i < CRU_PMU_CLKGATE_CON_CNT; i++)
		mmio_write_32(PMUCRU_BASE + CRU_PMU_CLKGATES_CON(i),
			      WITH_16BITS_WMSK(ddr_data.cru_pmu_clk_gate[i]));

	for (i = 0; i < CRU_CLKGATES_CON_CNT; i++)
		mmio_write_32(CRU_BASE + CRU_CLKGATES_CON(i),
			      WITH_16BITS_WMSK(ddr_data.cru_clk_gate[i]));
}

static void pvtm_32k_config(void)
{
	uint32_t  pvtm_freq_khz, pvtm_div;

	ddr_data.pmu_cru_clksel_con0 =
		mmio_read_32(PMUCRU_BASE + CRU_PMU_CLKSELS_CON(0));

	ddr_data.pgrf_pvtm_con[0] =
		mmio_read_32(PMUGRF_BASE + PMUGRF_PVTM_CON0);
	ddr_data.pgrf_pvtm_con[1] =
		mmio_read_32(PMUGRF_BASE + PMUGRF_PVTM_CON1);

	mmio_write_32(PMUGRF_BASE + PMUGRF_PVTM_CON0,
		      BITS_WITH_WMASK(0, 0x3, pgrf_pvtm_st));
	dsb();
	mmio_write_32(PMUGRF_BASE + PMUGRF_PVTM_CON0,
		      BITS_WITH_WMASK(1, 0x1, pgrf_pvtm_en));
	dsb();
	mmio_write_32(PMUGRF_BASE + PMUGRF_PVTM_CON1, PVTM_CALC_CNT);
	dsb();

	mmio_write_32(PMUGRF_BASE + PMUGRF_PVTM_CON0,
		      BITS_WITH_WMASK(1, 0x1, pgrf_pvtm_st));

	/* pmugrf_pvtm_st0 will be clear after PVTM start,
	 * which will cost about 6 cycles of pvtm at least.
	 * So we wait 30 cycles of pvtm for security.
	 */
	while (mmio_read_32(PMUGRF_BASE + PMUGRF_PVTM_ST1) < 30)
		;

	dsb();
	while (!(mmio_read_32(PMUGRF_BASE + PMUGRF_PVTM_ST0) & 0x1))
		;

	pvtm_freq_khz =
		(mmio_read_32(PMUGRF_BASE + PMUGRF_PVTM_ST1) * 24000 +
		PVTM_CALC_CNT / 2) / PVTM_CALC_CNT;
	pvtm_div = (pvtm_freq_khz + 16) / 32;

	/* pvtm_div = div_factor << 2 + 1,
	 * so div_factor = (pvtm_div - 1) >> 2.
	 * But the operation ">> 2" will clear the low bit of pvtm_div,
	 * so we don't have to do "- 1" for compasation
	 */
	pvtm_div = pvtm_div >> 2;
	if (pvtm_div > 0x3f)
		pvtm_div = 0x3f;

	mmio_write_32(PMUGRF_BASE + PMUGRF_PVTM_CON0,
		      BITS_WITH_WMASK(pvtm_div, 0x3f, pgrf_pvtm_div));

	/* select pvtm as 32k source */
	mmio_write_32(PMUCRU_BASE + CRU_PMU_CLKSELS_CON(0),
		      BITS_WITH_WMASK(1, 0x3U, 14));
}

static void pvtm_32k_config_restore(void)
{
	mmio_write_32(PMUCRU_BASE + CRU_PMU_CLKSELS_CON(0),
		      ddr_data.pmu_cru_clksel_con0 | BITS_WMSK(0x3U, 14));

	mmio_write_32(PMUGRF_BASE + PMUGRF_PVTM_CON0,
		      WITH_16BITS_WMSK(ddr_data.pgrf_pvtm_con[0]));
	mmio_write_32(PMUGRF_BASE + PMUGRF_PVTM_CON1,
		      ddr_data.pgrf_pvtm_con[1]);
}

static void ddr_sleep_config(void)
{
	/* disable ddr pd, sr */
	ddr_data.ddrc_pwrctrl = mmio_read_32(DDR_UPCTL_BASE + 0x30);
	mmio_write_32(DDR_UPCTL_BASE + 0x30, BITS_WITH_WMASK(0x0, 0x3, 0));

	/* disable ddr auto gt */
	ddr_data.ddrgrf_con1 = mmio_read_32(DDRGRF_BASE + 0x4);
	mmio_write_32(DDRGRF_BASE + 0x4, BITS_WITH_WMASK(0x0, 0x1f, 0));

	/* disable ddr standby */
	ddr_data.ddrstdby_con0 = mmio_read_32(DDR_STDBY_BASE + 0x0);
	mmio_write_32(DDR_STDBY_BASE + 0x0, BITS_WITH_WMASK(0x0, 0x1, 0));
	while ((mmio_read_32(DDR_UPCTL_BASE + 0x4) & 0x7) != 1)
		;

	/* ddr pmu ctrl */
	ddr_data.ddrgrf_con0 = mmio_read_32(DDRGRF_BASE + 0x0);
	mmio_write_32(DDRGRF_BASE + 0x0, BITS_WITH_WMASK(0x0, 0x1, 5));
	dsb();
	mmio_write_32(DDRGRF_BASE + 0x0, BITS_WITH_WMASK(0x1, 0x1, 4));

	/* ddr ret sel */
	ddr_data.pmugrf_soc_con0 =
		mmio_read_32(PMUGRF_BASE + PMUGRF_SOC_CON(0));
	mmio_write_32(PMUGRF_BASE + PMUGRF_SOC_CON(0),
		      BITS_WITH_WMASK(0x0, 0x1, 12));
}

static void ddr_sleep_config_restore(void)
{
	/* restore ddr ret sel */
	mmio_write_32(PMUGRF_BASE + PMUGRF_SOC_CON(0),
		      ddr_data.pmugrf_soc_con0 | BITS_WMSK(0x1, 12));

	/* restore ddr pmu ctrl */
	mmio_write_32(DDRGRF_BASE + 0x0,
		      ddr_data.ddrgrf_con0 | BITS_WMSK(0x1, 4));
	dsb();
	mmio_write_32(DDRGRF_BASE + 0x0,
		      ddr_data.ddrgrf_con0 | BITS_WMSK(0x1, 5));

	/* restore ddr standby */
	mmio_write_32(DDR_STDBY_BASE + 0x0,
		      ddr_data.ddrstdby_con0 | BITS_WMSK(0x1, 0));

	/* restore ddr auto gt */
	mmio_write_32(DDRGRF_BASE + 0x4,
		      ddr_data.ddrgrf_con1 | BITS_WMSK(0x1f, 0));

	/* restore ddr pd, sr */
	mmio_write_32(DDR_UPCTL_BASE + 0x30,
		      ddr_data.ddrc_pwrctrl | BITS_WMSK(0x3, 0));
}

static void pmu_sleep_config(void)
{
	uint32_t pwrmd_core_lo, pwrmd_core_hi, pwrmd_com_lo, pwrmd_com_hi;
	uint32_t pmu_wkup_cfg2_lo;
	uint32_t clk_freq_khz;

	/* save pmic_sleep iomux gpio0_a4 */
	ddr_data.pmic_slp_iomux = mmio_read_32(PMUGRF_BASE + GPIO0A_IOMUX);

	ddr_data.pmu_pwrmd_core_l =
			mmio_read_32(PMU_BASE + PMU_PWRMODE_CORE_LO);
	ddr_data.pmu_pwrmd_core_h =
			mmio_read_32(PMU_BASE + PMU_PWRMODE_CORE_HI);
	ddr_data.pmu_pwrmd_cmm_l =
			mmio_read_32(PMU_BASE + PMU_PWRMODE_COMMON_CON_LO);
	ddr_data.pmu_pwrmd_cmm_h =
			mmio_read_32(PMU_BASE + PMU_PWRMODE_COMMON_CON_HI);
	ddr_data.pmu_wkup_cfg2_l = mmio_read_32(PMU_BASE + PMU_WKUP_CFG2_LO);

	pwrmd_core_lo = BIT(pmu_global_int_dis) |
			BIT(pmu_core_src_gt) |
			BIT(pmu_cpu0_pd) |
			BIT(pmu_clr_core) |
			BIT(pmu_scu_pd) |
			BIT(pmu_l2_idle) |
			BIT(pmu_l2_flush) |
			BIT(pmu_clr_bus2main) |
			BIT(pmu_clr_peri2msch);

	pwrmd_core_hi = BIT(pmu_dpll_pd_en) |
			BIT(pmu_apll_pd_en) |
			BIT(pmu_cpll_pd_en) |
			BIT(pmu_gpll_pd_en) |
			BIT(pmu_npll_pd_en);

	pwrmd_com_lo = BIT(pmu_mode_en) |
		       BIT(pmu_pll_pd) |
		       BIT(pmu_pmu_use_if) |
		       BIT(pmu_alive_use_if) |
		       BIT(pmu_osc_dis) |
		       BIT(pmu_sref_enter) |
		       BIT(pmu_ddrc_gt) |
		       BIT(pmu_clr_pmu) |
		       BIT(pmu_clr_peri_pmu);

	pwrmd_com_hi = BIT(pmu_clr_bus) |
		       BIT(pmu_clr_msch) |
		       BIT(pmu_wakeup_begin_cfg);

	pmu_wkup_cfg2_lo = BIT(pmu_cluster_wkup_en) |
			   BIT(pmu_gpio_wkup_en) |
			   BIT(pmu_timer_wkup_en);

	/* set pmic_sleep iomux gpio0_a4 */
	mmio_write_32(PMUGRF_BASE + GPIO0A_IOMUX,
		      BITS_WITH_WMASK(1, 0x3, 8));

	clk_freq_khz = 32;

	mmio_write_32(PMU_BASE + PMU_OSC_CNT_LO,
		      WITH_16BITS_WMSK(clk_freq_khz * 32 & 0xffff));
	mmio_write_32(PMU_BASE + PMU_OSC_CNT_HI,
		      WITH_16BITS_WMSK(clk_freq_khz * 32 >> 16));

	mmio_write_32(PMU_BASE + PMU_STABLE_CNT_LO,
		      WITH_16BITS_WMSK(clk_freq_khz * 32 & 0xffff));
	mmio_write_32(PMU_BASE + PMU_STABLE_CNT_HI,
		      WITH_16BITS_WMSK(clk_freq_khz * 32 >> 16));

	mmio_write_32(PMU_BASE + PMU_WAKEUP_RST_CLR_LO,
		      WITH_16BITS_WMSK(clk_freq_khz * 2 & 0xffff));
	mmio_write_32(PMU_BASE + PMU_WAKEUP_RST_CLR_HI,
		      WITH_16BITS_WMSK(clk_freq_khz * 2 >> 16));

	/* Pmu's clk has switched to 24M back When pmu FSM counts
	 * the follow counters, so we should use 24M to calculate
	 * these counters.
	 */
	mmio_write_32(PMU_BASE + PMU_SCU_PWRDN_CNT_LO,
		      WITH_16BITS_WMSK(24000 * 2 & 0xffff));
	mmio_write_32(PMU_BASE + PMU_SCU_PWRDN_CNT_HI,
		      WITH_16BITS_WMSK(24000 * 2 >> 16));

	mmio_write_32(PMU_BASE + PMU_SCU_PWRUP_CNT_LO,
		      WITH_16BITS_WMSK(24000 * 2 & 0xffff));
	mmio_write_32(PMU_BASE + PMU_SCU_PWRUP_CNT_HI,
		      WITH_16BITS_WMSK(24000 * 2 >> 16));

	mmio_write_32(PMU_BASE + PMU_PLLLOCK_CNT_LO,
		      WITH_16BITS_WMSK(24000 * 5 & 0xffff));
	mmio_write_32(PMU_BASE + PMU_PLLLOCK_CNT_HI,
		      WITH_16BITS_WMSK(24000 * 5 >> 16));

	mmio_write_32(PMU_BASE + PMU_PLLRST_CNT_LO,
		      WITH_16BITS_WMSK(24000 * 2 & 0xffff));
	mmio_write_32(PMU_BASE + PMU_PLLRST_CNT_HI,
		      WITH_16BITS_WMSK(24000 * 2 >> 16));

	/* Config pmu power mode and pmu wakeup source */
	mmio_write_32(PMU_BASE + PMU_PWRMODE_CORE_LO,
		      WITH_16BITS_WMSK(pwrmd_core_lo));
	mmio_write_32(PMU_BASE + PMU_PWRMODE_CORE_HI,
		      WITH_16BITS_WMSK(pwrmd_core_hi));

	mmio_write_32(PMU_BASE + PMU_PWRMODE_COMMON_CON_LO,
		      WITH_16BITS_WMSK(pwrmd_com_lo));
	mmio_write_32(PMU_BASE + PMU_PWRMODE_COMMON_CON_HI,
		      WITH_16BITS_WMSK(pwrmd_com_hi));

	mmio_write_32(PMU_BASE + PMU_WKUP_CFG2_LO,
		      WITH_16BITS_WMSK(pmu_wkup_cfg2_lo));
}

static void pmu_sleep_restore(void)
{
	mmio_write_32(PMU_BASE + PMU_PWRMODE_CORE_LO,
		      WITH_16BITS_WMSK(ddr_data.pmu_pwrmd_core_l));
	mmio_write_32(PMU_BASE + PMU_PWRMODE_CORE_HI,
		      WITH_16BITS_WMSK(ddr_data.pmu_pwrmd_core_h));
	mmio_write_32(PMU_BASE + PMU_PWRMODE_COMMON_CON_LO,
		      WITH_16BITS_WMSK(ddr_data.pmu_pwrmd_cmm_l));
	mmio_write_32(PMU_BASE + PMU_PWRMODE_COMMON_CON_HI,
		      WITH_16BITS_WMSK(ddr_data.pmu_pwrmd_cmm_h));
	mmio_write_32(PMU_BASE + PMU_WKUP_CFG2_LO,
		      WITH_16BITS_WMSK(ddr_data.pmu_wkup_cfg2_l));

	/* restore pmic_sleep iomux */
	mmio_write_32(PMUGRF_BASE + GPIO0A_IOMUX,
		      WITH_16BITS_WMSK(ddr_data.pmic_slp_iomux));
}

static void soc_sleep_config(void)
{
	ddr_data.gpio0c_iomux = mmio_read_32(PMUGRF_BASE + GPIO0C_IOMUX);

	pmu_sleep_config();

	ddr_sleep_config();

	pvtm_32k_config();
}

static void soc_sleep_restore(void)
{
	secure_timer_init();

	pvtm_32k_config_restore();

	ddr_sleep_config_restore();

	pmu_sleep_restore();

	mmio_write_32(PMUGRF_BASE + GPIO0C_IOMUX,
		      WITH_16BITS_WMSK(ddr_data.gpio0c_iomux));
}

static inline void pm_pll_wait_lock(uint32_t pll_base, uint32_t pll_id)
{
	uint32_t delay = PLL_LOCKED_TIMEOUT;

	while (delay > 0) {
		if (mmio_read_32(pll_base + PLL_CON(1)) &
		    PLL_LOCK_MSK)
			break;
		delay--;
	}

	if (delay == 0)
		ERROR("Can't wait pll:%d lock\n", pll_id);
}

static inline void pll_pwr_ctr(uint32_t pll_base, uint32_t pll_id, uint32_t pd)
{
	mmio_write_32(pll_base + PLL_CON(1),
		      BITS_WITH_WMASK(1, 1U, 15));
	if (pd)
		mmio_write_32(pll_base + PLL_CON(1),
			      BITS_WITH_WMASK(1, 1, 14));
	else
		mmio_write_32(pll_base + PLL_CON(1),
			      BITS_WITH_WMASK(0, 1, 14));
}

static inline void pll_set_mode(uint32_t pll_id, uint32_t mode)
{
	uint32_t val = BITS_WITH_WMASK(mode, 0x3, PLL_MODE_SHIFT(pll_id));

	if (pll_id != GPLL_ID)
		mmio_write_32(CRU_BASE + CRU_MODE, val);
	else
		mmio_write_32(PMUCRU_BASE + CRU_PMU_MODE,
			      BITS_WITH_WMASK(mode, 0x3, 0));
}

static inline void pll_suspend(uint32_t pll_id)
{
	int i;
	uint32_t pll_base;

	if (pll_id != GPLL_ID)
		pll_base = CRU_BASE + CRU_PLL_CONS(pll_id, 0);
	else
		pll_base = PMUCRU_BASE + CRU_PLL_CONS(0, 0);

	/* save pll con */
	for (i = 0; i < PLL_CON_CNT; i++)
		ddr_data.cru_plls_con_save[pll_id][i] =
				mmio_read_32(pll_base + PLL_CON(i));

	/* slow mode */
	pll_set_mode(pll_id, SLOW_MODE);
}

static inline void pll_resume(uint32_t pll_id)
{
	uint32_t mode, pll_base;

	if (pll_id != GPLL_ID) {
		pll_base = CRU_BASE + CRU_PLL_CONS(pll_id, 0);
		mode = (ddr_data.cru_mode_save >> PLL_MODE_SHIFT(pll_id)) & 0x3;
	} else {
		pll_base = PMUCRU_BASE + CRU_PLL_CONS(0, 0);
		mode = ddr_data.cru_pmu_mode_save & 0x3;
	}

	/* if pll locked before suspend, we should wait atfer resume */
	if (ddr_data.cru_plls_con_save[pll_id][1] & PLL_LOCK_MSK)
		pm_pll_wait_lock(pll_base, pll_id);

	pll_set_mode(pll_id, mode);
}

static void pm_plls_suspend(void)
{
	ddr_data.cru_mode_save = mmio_read_32(CRU_BASE + CRU_MODE);
	ddr_data.cru_pmu_mode_save = mmio_read_32(PMUCRU_BASE + CRU_PMU_MODE);
	ddr_data.clk_sel0 = mmio_read_32(CRU_BASE + CRU_CLKSELS_CON(0));

	pll_suspend(GPLL_ID);
	pll_suspend(NPLL_ID);
	pll_suspend(CPLL_ID);
	pll_suspend(APLL_ID);

	/* core */
	mmio_write_32(CRU_BASE + CRU_CLKSELS_CON(0),
		      BITS_WITH_WMASK(0, 0xf, 0));

	/* pclk_dbg */
	mmio_write_32(CRU_BASE + CRU_CLKSELS_CON(0),
		      BITS_WITH_WMASK(0, 0xf, 8));
}

static void pm_plls_resume(void)
{
	/* pclk_dbg */
	mmio_write_32(CRU_BASE + CRU_CLKSELS_CON(0),
		      ddr_data.clk_sel0 | BITS_WMSK(0xf, 8));

	/* core */
	mmio_write_32(CRU_BASE + CRU_CLKSELS_CON(0),
		      ddr_data.clk_sel0 | BITS_WMSK(0xf, 0));

	pll_resume(APLL_ID);
	pll_resume(CPLL_ID);
	pll_resume(NPLL_ID);
	pll_resume(GPLL_ID);
}

int rockchip_soc_sys_pwr_dm_suspend(void)
{
	pmu_power_domains_suspend();

	clk_gate_suspend();

	soc_sleep_config();

	pm_plls_suspend();

	psram_boot_cfg->pm_flag &= ~PM_WARM_BOOT_BIT;

	return 0;
}

int rockchip_soc_sys_pwr_dm_resume(void)
{
	psram_boot_cfg->pm_flag |= PM_WARM_BOOT_BIT;

	pm_plls_resume();

	soc_sleep_restore();

	clk_gate_resume();

	pmu_power_domains_resume();

	plat_rockchip_gic_cpuif_enable();

	return 0;
}

void __dead2 rockchip_soc_soft_reset(void)
{
	pll_set_mode(GPLL_ID, SLOW_MODE);
	pll_set_mode(CPLL_ID, SLOW_MODE);
	pll_set_mode(NPLL_ID, SLOW_MODE);
	pll_set_mode(APLL_ID, SLOW_MODE);
	dsb();

	mmio_write_32(CRU_BASE + CRU_GLB_SRST_FST, CRU_GLB_SRST_FST_VALUE);
	dsb();

	/*
	 * Maybe the HW needs some times to reset the system,
	 * so we do not hope the core to execute valid codes.
	 */
	psci_power_down_wfi();
}

void __dead2 rockchip_soc_system_off(void)
{
	uint32_t val;

	/* set pmic_sleep pin(gpio0_a4) to gpio mode */
	mmio_write_32(PMUGRF_BASE + GPIO0A_IOMUX, BITS_WITH_WMASK(0, 0x3, 8));

	/* config output */
	val = mmio_read_32(GPIO0_BASE + SWPORTA_DDR);
	val |= BIT(4);
	mmio_write_32(GPIO0_BASE + SWPORTA_DDR, val);

	/* config output high level */
	val = mmio_read_32(GPIO0_BASE);
	val |= BIT(4);
	mmio_write_32(GPIO0_BASE, val);
	dsb();

	/*
	 * Maybe the HW needs some times to reset the system,
	 * so we do not hope the core to execute valid codes.
	 */
	psci_power_down_wfi();
}

void rockchip_plat_mmu_el3(void)
{
	/* TODO: support the el3 for px30 SoCs */
}

void plat_rockchip_pmu_init(void)
{
	uint32_t cpu;

	rockchip_pd_lock_init();

	for (cpu = 0; cpu < PLATFORM_CORE_COUNT; cpu++)
		cpuson_flags[cpu] = 0;

	psram_boot_cfg->ddr_func = (uint64_t)0;
	psram_boot_cfg->ddr_data = (uint64_t)0;
	psram_boot_cfg->sp = PSRAM_SP_TOP;
	psram_boot_cfg->ddr_flag = 0x0;
	psram_boot_cfg->boot_mpidr = read_mpidr_el1() & 0xffff;
	psram_boot_cfg->pm_flag = PM_WARM_BOOT_BIT;

	nonboot_cpus_off();

	/* Remap pmu_sram's base address to boot address */
	mmio_write_32(PMUSGRF_BASE + PMUSGRF_SOC_CON(0),
		      BITS_WITH_WMASK(1, 0x1, 13));

	INFO("%s: pd status %x\n",
	     __func__, mmio_read_32(PMU_BASE + PMU_PWRDN_ST));
}
