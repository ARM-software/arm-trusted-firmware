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
#include <pwm.h>
#include <bl31.h>
#include <rk3399m0.h>
#include <suspend.h>

DEFINE_BAKERY_LOCK(rockchip_pd_lock);

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

static void pmu_bus_idle_req(uint32_t bus, uint32_t state)
{
	uint32_t bus_id = BIT(bus);
	uint32_t bus_req;
	uint32_t wait_cnt = 0;
	uint32_t bus_state, bus_ack;

	if (state)
		bus_req = BIT(bus);
	else
		bus_req = 0;

	mmio_clrsetbits_32(PMU_BASE + PMU_BUS_IDLE_REQ, bus_id, bus_req);

	do {
		bus_state = mmio_read_32(PMU_BASE + PMU_BUS_IDLE_ST) & bus_id;
		bus_ack = mmio_read_32(PMU_BASE + PMU_BUS_IDLE_ACK) & bus_id;
		wait_cnt++;
	} while ((bus_state != bus_req || bus_ack != bus_req) &&
		 (wait_cnt < MAX_WAIT_COUNT));

	if (bus_state != bus_req || bus_ack != bus_req) {
		INFO("%s:st=%x(%x)\n", __func__,
		     mmio_read_32(PMU_BASE + PMU_BUS_IDLE_ST),
		     bus_state);
		INFO("%s:st=%x(%x)\n", __func__,
		     mmio_read_32(PMU_BASE + PMU_BUS_IDLE_ACK),
		     bus_ack);
	}
}

struct pmu_slpdata_s pmu_slpdata;

static void qos_save(void)
{
	if (pmu_power_domain_st(PD_GPU) == pmu_pd_on)
		RESTORE_QOS(pmu_slpdata.gpu_qos, GPU);
	if (pmu_power_domain_st(PD_ISP0) == pmu_pd_on) {
		RESTORE_QOS(pmu_slpdata.isp0_m0_qos, ISP0_M0);
		RESTORE_QOS(pmu_slpdata.isp0_m1_qos, ISP0_M1);
	}
	if (pmu_power_domain_st(PD_ISP1) == pmu_pd_on) {
		RESTORE_QOS(pmu_slpdata.isp1_m0_qos, ISP1_M0);
		RESTORE_QOS(pmu_slpdata.isp1_m1_qos, ISP1_M1);
	}
	if (pmu_power_domain_st(PD_VO) == pmu_pd_on) {
		RESTORE_QOS(pmu_slpdata.vop_big_r, VOP_BIG_R);
		RESTORE_QOS(pmu_slpdata.vop_big_w, VOP_BIG_W);
		RESTORE_QOS(pmu_slpdata.vop_little, VOP_LITTLE);
	}
	if (pmu_power_domain_st(PD_HDCP) == pmu_pd_on)
		RESTORE_QOS(pmu_slpdata.hdcp_qos, HDCP);
	if (pmu_power_domain_st(PD_GMAC) == pmu_pd_on)
		RESTORE_QOS(pmu_slpdata.gmac_qos, GMAC);
	if (pmu_power_domain_st(PD_CCI) == pmu_pd_on) {
		RESTORE_QOS(pmu_slpdata.cci_m0_qos, CCI_M0);
		RESTORE_QOS(pmu_slpdata.cci_m1_qos, CCI_M1);
	}
	if (pmu_power_domain_st(PD_SD) == pmu_pd_on)
		RESTORE_QOS(pmu_slpdata.sdmmc_qos, SDMMC);
	if (pmu_power_domain_st(PD_EMMC) == pmu_pd_on)
		RESTORE_QOS(pmu_slpdata.emmc_qos, EMMC);
	if (pmu_power_domain_st(PD_SDIOAUDIO) == pmu_pd_on)
		RESTORE_QOS(pmu_slpdata.sdio_qos, SDIO);
	if (pmu_power_domain_st(PD_GIC) == pmu_pd_on)
		RESTORE_QOS(pmu_slpdata.gic_qos, GIC);
	if (pmu_power_domain_st(PD_RGA) == pmu_pd_on) {
		RESTORE_QOS(pmu_slpdata.rga_r_qos, RGA_R);
		RESTORE_QOS(pmu_slpdata.rga_w_qos, RGA_W);
	}
	if (pmu_power_domain_st(PD_IEP) == pmu_pd_on)
		RESTORE_QOS(pmu_slpdata.iep_qos, IEP);
	if (pmu_power_domain_st(PD_USB3) == pmu_pd_on) {
		RESTORE_QOS(pmu_slpdata.usb_otg0_qos, USB_OTG0);
		RESTORE_QOS(pmu_slpdata.usb_otg1_qos, USB_OTG1);
	}
	if (pmu_power_domain_st(PD_PERIHP) == pmu_pd_on) {
		RESTORE_QOS(pmu_slpdata.usb_host0_qos, USB_HOST0);
		RESTORE_QOS(pmu_slpdata.usb_host1_qos, USB_HOST1);
		RESTORE_QOS(pmu_slpdata.perihp_nsp_qos, PERIHP_NSP);
	}
	if (pmu_power_domain_st(PD_PERILP) == pmu_pd_on) {
		RESTORE_QOS(pmu_slpdata.dmac0_qos, DMAC0);
		RESTORE_QOS(pmu_slpdata.dmac1_qos, DMAC1);
		RESTORE_QOS(pmu_slpdata.dcf_qos, DCF);
		RESTORE_QOS(pmu_slpdata.crypto0_qos, CRYPTO0);
		RESTORE_QOS(pmu_slpdata.crypto1_qos, CRYPTO1);
		RESTORE_QOS(pmu_slpdata.perilp_nsp_qos, PERILP_NSP);
		RESTORE_QOS(pmu_slpdata.perilpslv_nsp_qos, PERILPSLV_NSP);
		RESTORE_QOS(pmu_slpdata.peri_cm1_qos, PERI_CM1);
	}
	if (pmu_power_domain_st(PD_VDU) == pmu_pd_on)
		RESTORE_QOS(pmu_slpdata.video_m0_qos, VIDEO_M0);
	if (pmu_power_domain_st(PD_VCODEC) == pmu_pd_on) {
		RESTORE_QOS(pmu_slpdata.video_m1_r_qos, VIDEO_M1_R);
		RESTORE_QOS(pmu_slpdata.video_m1_w_qos, VIDEO_M1_W);
	}
}

static void qos_restore(void)
{
	if (pmu_power_domain_st(PD_GPU) == pmu_pd_on)
		SAVE_QOS(pmu_slpdata.gpu_qos, GPU);
	if (pmu_power_domain_st(PD_ISP0) == pmu_pd_on) {
		SAVE_QOS(pmu_slpdata.isp0_m0_qos, ISP0_M0);
		SAVE_QOS(pmu_slpdata.isp0_m1_qos, ISP0_M1);
	}
	if (pmu_power_domain_st(PD_ISP1) == pmu_pd_on) {
		SAVE_QOS(pmu_slpdata.isp1_m0_qos, ISP1_M0);
		SAVE_QOS(pmu_slpdata.isp1_m1_qos, ISP1_M1);
	}
	if (pmu_power_domain_st(PD_VO) == pmu_pd_on) {
		SAVE_QOS(pmu_slpdata.vop_big_r, VOP_BIG_R);
		SAVE_QOS(pmu_slpdata.vop_big_w, VOP_BIG_W);
		SAVE_QOS(pmu_slpdata.vop_little, VOP_LITTLE);
	}
	if (pmu_power_domain_st(PD_HDCP) == pmu_pd_on)
		SAVE_QOS(pmu_slpdata.hdcp_qos, HDCP);
	if (pmu_power_domain_st(PD_GMAC) == pmu_pd_on)
		SAVE_QOS(pmu_slpdata.gmac_qos, GMAC);
	if (pmu_power_domain_st(PD_CCI) == pmu_pd_on) {
		SAVE_QOS(pmu_slpdata.cci_m0_qos, CCI_M0);
		SAVE_QOS(pmu_slpdata.cci_m1_qos, CCI_M1);
	}
	if (pmu_power_domain_st(PD_SD) == pmu_pd_on)
		SAVE_QOS(pmu_slpdata.sdmmc_qos, SDMMC);
	if (pmu_power_domain_st(PD_EMMC) == pmu_pd_on)
		SAVE_QOS(pmu_slpdata.emmc_qos, EMMC);
	if (pmu_power_domain_st(PD_SDIOAUDIO) == pmu_pd_on)
		SAVE_QOS(pmu_slpdata.sdio_qos, SDIO);
	if (pmu_power_domain_st(PD_GIC) == pmu_pd_on)
		SAVE_QOS(pmu_slpdata.gic_qos, GIC);
	if (pmu_power_domain_st(PD_RGA) == pmu_pd_on) {
		SAVE_QOS(pmu_slpdata.rga_r_qos, RGA_R);
		SAVE_QOS(pmu_slpdata.rga_w_qos, RGA_W);
	}
	if (pmu_power_domain_st(PD_IEP) == pmu_pd_on)
		SAVE_QOS(pmu_slpdata.iep_qos, IEP);
	if (pmu_power_domain_st(PD_USB3) == pmu_pd_on) {
		SAVE_QOS(pmu_slpdata.usb_otg0_qos, USB_OTG0);
		SAVE_QOS(pmu_slpdata.usb_otg1_qos, USB_OTG1);
	}
	if (pmu_power_domain_st(PD_PERIHP) == pmu_pd_on) {
		SAVE_QOS(pmu_slpdata.usb_host0_qos, USB_HOST0);
		SAVE_QOS(pmu_slpdata.usb_host1_qos, USB_HOST1);
		SAVE_QOS(pmu_slpdata.perihp_nsp_qos, PERIHP_NSP);
	}
	if (pmu_power_domain_st(PD_PERILP) == pmu_pd_on) {
		SAVE_QOS(pmu_slpdata.dmac0_qos, DMAC0);
		SAVE_QOS(pmu_slpdata.dmac1_qos, DMAC1);
		SAVE_QOS(pmu_slpdata.dcf_qos, DCF);
		SAVE_QOS(pmu_slpdata.crypto0_qos, CRYPTO0);
		SAVE_QOS(pmu_slpdata.crypto1_qos, CRYPTO1);
		SAVE_QOS(pmu_slpdata.perilp_nsp_qos, PERILP_NSP);
		SAVE_QOS(pmu_slpdata.perilpslv_nsp_qos, PERILPSLV_NSP);
		SAVE_QOS(pmu_slpdata.peri_cm1_qos, PERI_CM1);
	}
	if (pmu_power_domain_st(PD_VDU) == pmu_pd_on)
		SAVE_QOS(pmu_slpdata.video_m0_qos, VIDEO_M0);
	if (pmu_power_domain_st(PD_VCODEC) == pmu_pd_on) {
		SAVE_QOS(pmu_slpdata.video_m1_r_qos, VIDEO_M1_R);
		SAVE_QOS(pmu_slpdata.video_m1_w_qos, VIDEO_M1_W);
	}
}

static int pmu_set_power_domain(uint32_t pd_id, uint32_t pd_state)
{
	uint32_t state;

	if (pmu_power_domain_st(pd_id) == pd_state)
		goto out;

	if (pd_state == pmu_pd_on)
		pmu_power_domain_ctr(pd_id, pd_state);

	state = (pd_state == pmu_pd_off) ? BUS_IDLE : BUS_ACTIVE;

	switch (pd_id) {
	case PD_GPU:
		pmu_bus_idle_req(BUS_ID_GPU, state);
		break;
	case PD_VIO:
		pmu_bus_idle_req(BUS_ID_VIO, state);
		break;
	case PD_ISP0:
		pmu_bus_idle_req(BUS_ID_ISP0, state);
		break;
	case PD_ISP1:
		pmu_bus_idle_req(BUS_ID_ISP1, state);
		break;
	case PD_VO:
		pmu_bus_idle_req(BUS_ID_VOPB, state);
		pmu_bus_idle_req(BUS_ID_VOPL, state);
		break;
	case PD_HDCP:
		pmu_bus_idle_req(BUS_ID_HDCP, state);
		break;
	case PD_TCPD0:
		break;
	case PD_TCPD1:
		break;
	case PD_GMAC:
		pmu_bus_idle_req(BUS_ID_GMAC, state);
		break;
	case PD_CCI:
		pmu_bus_idle_req(BUS_ID_CCIM0, state);
		pmu_bus_idle_req(BUS_ID_CCIM1, state);
		break;
	case PD_SD:
		pmu_bus_idle_req(BUS_ID_SD, state);
		break;
	case PD_EMMC:
		pmu_bus_idle_req(BUS_ID_EMMC, state);
		break;
	case PD_EDP:
		pmu_bus_idle_req(BUS_ID_EDP, state);
		break;
	case PD_SDIOAUDIO:
		pmu_bus_idle_req(BUS_ID_SDIOAUDIO, state);
		break;
	case PD_GIC:
		pmu_bus_idle_req(BUS_ID_GIC, state);
		break;
	case PD_RGA:
		pmu_bus_idle_req(BUS_ID_RGA, state);
		break;
	case PD_VCODEC:
		pmu_bus_idle_req(BUS_ID_VCODEC, state);
		break;
	case PD_VDU:
		pmu_bus_idle_req(BUS_ID_VDU, state);
		break;
	case PD_IEP:
		pmu_bus_idle_req(BUS_ID_IEP, state);
		break;
	case PD_USB3:
		pmu_bus_idle_req(BUS_ID_USB3, state);
		break;
	case PD_PERIHP:
		pmu_bus_idle_req(BUS_ID_PERIHP, state);
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
	clk_gate_con_save();
	clk_gate_con_disable();
	qos_save();
	pmu_powerdomain_state = mmio_read_32(PMU_BASE + PMU_PWRDN_ST);
	pmu_set_power_domain(PD_GPU, pmu_pd_off);
	pmu_set_power_domain(PD_TCPD0, pmu_pd_off);
	pmu_set_power_domain(PD_TCPD1, pmu_pd_off);
	pmu_set_power_domain(PD_VO, pmu_pd_off);
	pmu_set_power_domain(PD_ISP0, pmu_pd_off);
	pmu_set_power_domain(PD_ISP1, pmu_pd_off);
	pmu_set_power_domain(PD_HDCP, pmu_pd_off);
	pmu_set_power_domain(PD_SDIOAUDIO, pmu_pd_off);
	pmu_set_power_domain(PD_GMAC, pmu_pd_off);
	pmu_set_power_domain(PD_EDP, pmu_pd_off);
	pmu_set_power_domain(PD_IEP, pmu_pd_off);
	pmu_set_power_domain(PD_RGA, pmu_pd_off);
	pmu_set_power_domain(PD_VCODEC, pmu_pd_off);
	pmu_set_power_domain(PD_VDU, pmu_pd_off);
	clk_gate_con_restore();
}

static void pmu_power_domains_resume(void)
{
	clk_gate_con_save();
	clk_gate_con_disable();
	if (!(pmu_powerdomain_state & BIT(PD_VDU)))
		pmu_set_power_domain(PD_VDU, pmu_pd_on);
	if (!(pmu_powerdomain_state & BIT(PD_VCODEC)))
		pmu_set_power_domain(PD_VCODEC, pmu_pd_on);
	if (!(pmu_powerdomain_state & BIT(PD_RGA)))
		pmu_set_power_domain(PD_RGA, pmu_pd_on);
	if (!(pmu_powerdomain_state & BIT(PD_IEP)))
		pmu_set_power_domain(PD_IEP, pmu_pd_on);
	if (!(pmu_powerdomain_state & BIT(PD_EDP)))
		pmu_set_power_domain(PD_EDP, pmu_pd_on);
	if (!(pmu_powerdomain_state & BIT(PD_GMAC)))
		pmu_set_power_domain(PD_GMAC, pmu_pd_on);
	if (!(pmu_powerdomain_state & BIT(PD_SDIOAUDIO)))
		pmu_set_power_domain(PD_SDIOAUDIO, pmu_pd_on);
	if (!(pmu_powerdomain_state & BIT(PD_HDCP)))
		pmu_set_power_domain(PD_HDCP, pmu_pd_on);
	if (!(pmu_powerdomain_state & BIT(PD_ISP1)))
		pmu_set_power_domain(PD_ISP1, pmu_pd_on);
	if (!(pmu_powerdomain_state & BIT(PD_ISP0)))
		pmu_set_power_domain(PD_ISP0, pmu_pd_on);
	if (!(pmu_powerdomain_state & BIT(PD_VO)))
		pmu_set_power_domain(PD_VO, pmu_pd_on);
	if (!(pmu_powerdomain_state & BIT(PD_TCPD1)))
		pmu_set_power_domain(PD_TCPD1, pmu_pd_on);
	if (!(pmu_powerdomain_state & BIT(PD_TCPD0)))
		pmu_set_power_domain(PD_TCPD0, pmu_pd_on);
	if (!(pmu_powerdomain_state & BIT(PD_GPU)))
		pmu_set_power_domain(PD_GPU, pmu_pd_on);
	qos_restore();
	clk_gate_con_restore();
}

void rk3399_flash_l2_b(void)
{
	uint32_t wait_cnt = 0;

	mmio_setbits_32(PMU_BASE + PMU_SFT_CON, BIT(L2_FLUSH_REQ_CLUSTER_B));
	dsb();

	while (!(mmio_read_32(PMU_BASE + PMU_CORE_PWR_ST) &
		 BIT(L2_FLUSHDONE_CLUSTER_B))) {
		wait_cnt++;
		if (wait_cnt >= MAX_WAIT_COUNT)
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
		if (wait_cnt >= MAX_WAIT_COUNT)
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
	size_t sram_size;

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

static inline void clst_pwr_domain_suspend(plat_local_state_t lvl_state)
{
	uint32_t cpu_id = plat_my_core_pos();
	uint32_t pll_id, clst_st_msk, clst_st_chk_msk, pmu_st;

	assert(cpu_id < PLATFORM_CORE_COUNT);

	if (lvl_state == PLAT_MAX_OFF_STATE) {
		if (cpu_id < PLATFORM_CLUSTER0_CORE_COUNT) {
			pll_id = ALPLL_ID;
			clst_st_msk = CLST_L_CPUS_MSK;
		} else {
			pll_id = ABPLL_ID;
			clst_st_msk = CLST_B_CPUS_MSK <<
				       PLATFORM_CLUSTER0_CORE_COUNT;
		}

		clst_st_chk_msk = clst_st_msk & ~(BIT(cpu_id));

		pmu_st = mmio_read_32(PMU_BASE + PMU_PWRDN_ST);

		pmu_st &= clst_st_msk;

		if (pmu_st == clst_st_chk_msk) {
			mmio_write_32(CRU_BASE + CRU_PLL_CON(pll_id, 3),
				      PLL_SLOW_MODE);

			clst_warmboot_data[pll_id] = PMU_CLST_RET;

			pmu_st = mmio_read_32(PMU_BASE + PMU_PWRDN_ST);
			pmu_st &= clst_st_msk;
			if (pmu_st == clst_st_chk_msk)
				return;
			/*
			 * it is mean that others cpu is up again,
			 * we must resume the cfg at once.
			 */
			mmio_write_32(CRU_BASE + CRU_PLL_CON(pll_id, 3),
				      PLL_NOMAL_MODE);
			clst_warmboot_data[pll_id] = 0;
		}
	}
}

static int clst_pwr_domain_resume(plat_local_state_t lvl_state)
{
	uint32_t cpu_id = plat_my_core_pos();
	uint32_t pll_id, pll_st;

	assert(cpu_id < PLATFORM_CORE_COUNT);

	if (lvl_state == PLAT_MAX_OFF_STATE) {
		if (cpu_id < PLATFORM_CLUSTER0_CORE_COUNT)
			pll_id = ALPLL_ID;
		else
			pll_id = ABPLL_ID;

		pll_st = mmio_read_32(CRU_BASE + CRU_PLL_CON(pll_id, 3)) >>
				 PLL_MODE_SHIFT;

		if (pll_st != NORMAL_MODE) {
			WARN("%s: clst (%d) is in error mode (%d)\n",
			     __func__, pll_id, pll_st);
			return -1;
		}
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

static int hlvl_pwr_domain_off(uint32_t lvl, plat_local_state_t lvl_state)
{
	switch (lvl) {
	case MPIDR_AFFLVL1:
		clst_pwr_domain_suspend(lvl_state);
		break;
	default:
		break;
	}

	return 0;
}

static int cores_pwr_domain_suspend(void)
{
	uint32_t cpu_id = plat_my_core_pos();

	assert(cpu_id < PLATFORM_CORE_COUNT);
	assert(cpuson_flags[cpu_id] == 0);
	cpuson_flags[cpu_id] = PMU_CPU_AUTO_PWRDN;
	cpuson_entry_point[cpu_id] = plat_get_sec_entrypoint();
	dsb();

	cpus_power_domain_off(cpu_id, core_pwr_wfi_int);

	return 0;
}

static int hlvl_pwr_domain_suspend(uint32_t lvl, plat_local_state_t lvl_state)
{
	switch (lvl) {
	case MPIDR_AFFLVL1:
		clst_pwr_domain_suspend(lvl_state);
		break;
	default:
		break;
	}

	return 0;
}

static int cores_pwr_domain_on_finish(void)
{
	uint32_t cpu_id = plat_my_core_pos();

	mmio_write_32(PMU_BASE + PMU_CORE_PM_CON(cpu_id),
		      CORES_PM_DISABLE);
	return 0;
}

static int hlvl_pwr_domain_on_finish(uint32_t lvl,
				     plat_local_state_t lvl_state)
{
	switch (lvl) {
	case MPIDR_AFFLVL1:
		clst_pwr_domain_resume(lvl_state);
		break;
	default:
		break;
	}

	return 0;
}

static int cores_pwr_domain_resume(void)
{
	uint32_t cpu_id = plat_my_core_pos();

	/* Disable core_pm */
	mmio_write_32(PMU_BASE + PMU_CORE_PM_CON(cpu_id), CORES_PM_DISABLE);

	return 0;
}

static int hlvl_pwr_domain_resume(uint32_t lvl, plat_local_state_t lvl_state)
{
	switch (lvl) {
	case MPIDR_AFFLVL1:
		clst_pwr_domain_resume(lvl_state);
	default:
		break;
	}

	return 0;
}

/**
 * init_pmu_counts - Init timing counts in the PMU register area
 *
 * At various points when we power up or down parts of the system we need
 * a delay to wait for power / clocks to become stable.  The PMU has counters
 * to help software do the delay properly.  Basically, it works like this:
 * - Software sets up counter values
 * - When software turns on something in the PMU, the counter kicks off
 * - The hardware sets a bit automatically when the counter has finished and
 *   software knows that the initialization is done.
 *
 * It's software's job to setup these counters.  The hardware power on default
 * for these settings is conservative, setting everything to 0x5dc0
 * (750 ms in 32 kHz counts or 1 ms in 24 MHz counts).
 *
 * Note that some of these counters are only really used at suspend/resume
 * time (for instance, that's the only time we turn off/on the oscillator) and
 * others are used during normal runtime (like turning on/off a CPU or GPU) but
 * it doesn't hurt to init everything at boot.
 *
 * Also note that these counters can run off the 32 kHz clock or the 24 MHz
 * clock.  While the 24 MHz clock can give us more precision, it's not always
 * available (like when we turn the oscillator off at sleep time). The
 * pmu_use_lf (lf: low freq) is available in power mode.  Current understanding
 * is that counts work like this:
 *    IF (pmu_use_lf == 0) || (power_mode_en == 0)
 *      use the 24M OSC for counts
 *    ELSE
 *      use the 32K OSC for counts
 *
 * Notes:
 * - There is a separate bit for the PMU called PMU_24M_EN_CFG.  At the moment
 *   we always keep that 0.  This apparently choose between using the PLL as
 *   the source for the PMU vs. the 24M clock.  If we ever set it to 1 we
 *   should consider how it affects these counts (if at all).
 * - The power_mode_en is documented to auto-clear automatically when we leave
 *   "power mode".  That's why most clocks are on 24M.  Only timings used when
 *   in "power mode" are 32k.
 * - In some cases the kernel may override these counts.
 *
 * The PMU_STABLE_CNT / PMU_OSC_CNT / PMU_PLLLOCK_CNT are important CNTs
 * in power mode, we need to ensure that they are available.
 */
static void init_pmu_counts(void)
{
	/* COUNTS FOR INSIDE POWER MODE */

	/*
	 * From limited testing, need PMU stable >= 2ms, but go overkill
	 * and choose 30 ms to match testing on past SoCs.  Also let
	 * OSC have 30 ms for stabilization.
	 */
	mmio_write_32(PMU_BASE + PMU_STABLE_CNT, CYCL_32K_CNT_MS(30));
	mmio_write_32(PMU_BASE + PMU_OSC_CNT, CYCL_32K_CNT_MS(30));

	/* Unclear what these should be; try 3 ms */
	mmio_write_32(PMU_BASE + PMU_WAKEUP_RST_CLR_CNT, CYCL_32K_CNT_MS(3));

	/* Unclear what this should be, but set the default explicitly */
	mmio_write_32(PMU_BASE + PMU_TIMEOUT_CNT, 0x5dc0);

	/* COUNTS FOR OUTSIDE POWER MODE */

	/* Put something sorta conservative here until we know better */
	mmio_write_32(PMU_BASE + PMU_PLLLOCK_CNT, CYCL_24M_CNT_MS(3));
	mmio_write_32(PMU_BASE + PMU_DDRIO_PWRON_CNT, CYCL_24M_CNT_MS(1));
	mmio_write_32(PMU_BASE + PMU_CENTER_PWRDN_CNT, CYCL_24M_CNT_MS(1));
	mmio_write_32(PMU_BASE + PMU_CENTER_PWRUP_CNT, CYCL_24M_CNT_MS(1));

	/*
	 * Set CPU/GPU to 1 us.
	 *
	 * NOTE: Even though ATF doesn't configure the GPU we'll still setup
	 * counts here.  After all ATF controls all these other bits and also
	 * chooses which clock these counters use.
	 */
	mmio_write_32(PMU_BASE + PMU_SCU_L_PWRDN_CNT, CYCL_24M_CNT_US(1));
	mmio_write_32(PMU_BASE + PMU_SCU_L_PWRUP_CNT, CYCL_24M_CNT_US(1));
	mmio_write_32(PMU_BASE + PMU_SCU_B_PWRDN_CNT, CYCL_24M_CNT_US(1));
	mmio_write_32(PMU_BASE + PMU_SCU_B_PWRUP_CNT, CYCL_24M_CNT_US(1));
	mmio_write_32(PMU_BASE + PMU_GPU_PWRDN_CNT, CYCL_24M_CNT_US(1));
	mmio_write_32(PMU_BASE + PMU_GPU_PWRUP_CNT, CYCL_24M_CNT_US(1));
}

static uint32_t clk_ddrc_save;

static void sys_slp_config(void)
{
	uint32_t slp_mode_cfg = 0;

	/* keep enabling clk_ddrc_bpll_src_en gate for DDRC */
	clk_ddrc_save = mmio_read_32(CRU_BASE + CRU_CLKGATE_CON(3));
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(3), WMSK_BIT(1));

	prepare_abpll_for_ddrctrl();
	sram_func_set_ddrctl_pll(ABPLL_ID);

	mmio_write_32(GRF_BASE + GRF_SOC_CON4, CCI_FORCE_WAKEUP);
	mmio_write_32(PMU_BASE + PMU_CCI500_CON,
		      BIT_WITH_WMSK(PMU_CLR_PREQ_CCI500_HW) |
		      BIT_WITH_WMSK(PMU_CLR_QREQ_CCI500_HW) |
		      BIT_WITH_WMSK(PMU_QGATING_CCI500_CFG));

	mmio_write_32(PMU_BASE + PMU_ADB400_CON,
		      BIT_WITH_WMSK(PMU_CLR_CORE_L_HW) |
		      BIT_WITH_WMSK(PMU_CLR_CORE_L_2GIC_HW) |
		      BIT_WITH_WMSK(PMU_CLR_GIC2_CORE_L_HW));

	slp_mode_cfg = BIT(PMU_PWR_MODE_EN) |
		       BIT(PMU_POWER_OFF_REQ_CFG) |
		       BIT(PMU_CPU0_PD_EN) |
		       BIT(PMU_L2_FLUSH_EN) |
		       BIT(PMU_L2_IDLE_EN) |
		       BIT(PMU_SCU_PD_EN) |
		       BIT(PMU_CCI_PD_EN) |
		       BIT(PMU_CLK_CORE_SRC_GATE_EN) |
		       BIT(PMU_ALIVE_USE_LF) |
		       BIT(PMU_SREF0_ENTER_EN) |
		       BIT(PMU_SREF1_ENTER_EN) |
		       BIT(PMU_DDRC0_GATING_EN) |
		       BIT(PMU_DDRC1_GATING_EN) |
		       BIT(PMU_DDRIO0_RET_EN) |
		       BIT(PMU_DDRIO1_RET_EN) |
		       BIT(PMU_DDRIO_RET_HW_DE_REQ) |
		       BIT(PMU_CENTER_PD_EN) |
		       BIT(PMU_PLL_PD_EN) |
		       BIT(PMU_CLK_CENTER_SRC_GATE_EN) |
		       BIT(PMU_OSC_DIS) |
		       BIT(PMU_PMU_USE_LF);

	mmio_setbits_32(PMU_BASE + PMU_WKUP_CFG4, BIT(PMU_GPIO_WKUP_EN));
	mmio_write_32(PMU_BASE + PMU_PWRMODE_CON, slp_mode_cfg);

	mmio_write_32(PMU_BASE + PMU_PLL_CON, PLL_PD_HW);
	mmio_write_32(PMUGRF_BASE + PMUGRF_SOC_CON0, EXTERNAL_32K);
	mmio_write_32(PMUGRF_BASE, IOMUX_CLK_32K); /* 32k iomux */
}

static void set_hw_idle(uint32_t hw_idle)
{
	mmio_setbits_32(PMU_BASE + PMU_BUS_CLR, hw_idle);
}

static void clr_hw_idle(uint32_t hw_idle)
{
	mmio_clrbits_32(PMU_BASE + PMU_BUS_CLR, hw_idle);
}

static uint32_t iomux_status[12];
static uint32_t pull_mode_status[12];
static uint32_t gpio_direction[3];
static uint32_t gpio_2_4_clk_gate;

static void suspend_apio(void)
{
	struct apio_info *suspend_apio;
	int i;

	suspend_apio = plat_get_rockchip_suspend_apio();

	if (!suspend_apio)
		return;

	/* save gpio2 ~ gpio4 iomux and pull mode */
	for (i = 0; i < 12; i++) {
		iomux_status[i] = mmio_read_32(GRF_BASE +
				GRF_GPIO2A_IOMUX + i * 4);
		pull_mode_status[i] = mmio_read_32(GRF_BASE +
				GRF_GPIO2A_P + i * 4);
	}

	/* store gpio2 ~ gpio4 clock gate state */
	gpio_2_4_clk_gate = (mmio_read_32(CRU_BASE + CRU_CLKGATE_CON(31)) >>
				PCLK_GPIO2_GATE_SHIFT) & 0x07;

	/* enable gpio2 ~ gpio4 clock gate */
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(31),
		      BITS_WITH_WMASK(0, 0x07, PCLK_GPIO2_GATE_SHIFT));

	/* save gpio2 ~ gpio4 direction */
	gpio_direction[0] = mmio_read_32(GPIO2_BASE + 0x04);
	gpio_direction[1] = mmio_read_32(GPIO3_BASE + 0x04);
	gpio_direction[2] = mmio_read_32(GPIO4_BASE + 0x04);

	/* apio1 charge gpio3a0 ~ gpio3c7 */
	if (suspend_apio->apio1) {

		/* set gpio3a0 ~ gpio3c7 iomux to gpio */
		mmio_write_32(GRF_BASE + GRF_GPIO3A_IOMUX,
			      REG_SOC_WMSK | GRF_IOMUX_GPIO);
		mmio_write_32(GRF_BASE + GRF_GPIO3B_IOMUX,
			      REG_SOC_WMSK | GRF_IOMUX_GPIO);
		mmio_write_32(GRF_BASE + GRF_GPIO3C_IOMUX,
			      REG_SOC_WMSK | GRF_IOMUX_GPIO);

		/* set gpio3a0 ~ gpio3c7 pull mode to pull none */
		mmio_write_32(GRF_BASE + GRF_GPIO3A_P, REG_SOC_WMSK | 0);
		mmio_write_32(GRF_BASE + GRF_GPIO3B_P, REG_SOC_WMSK | 0);
		mmio_write_32(GRF_BASE + GRF_GPIO3C_P, REG_SOC_WMSK | 0);

		/* set gpio3a0 ~ gpio3c7 to input */
		mmio_clrbits_32(GPIO3_BASE + 0x04, 0x00ffffff);
	}

	/* apio2 charge gpio2a0 ~ gpio2b4 */
	if (suspend_apio->apio2) {

		/* set gpio2a0 ~ gpio2b4 iomux to gpio */
		mmio_write_32(GRF_BASE + GRF_GPIO2A_IOMUX,
			      REG_SOC_WMSK | GRF_IOMUX_GPIO);
		mmio_write_32(GRF_BASE + GRF_GPIO2B_IOMUX,
			      REG_SOC_WMSK | GRF_IOMUX_GPIO);

		/* set gpio2a0 ~ gpio2b4 pull mode to pull none */
		mmio_write_32(GRF_BASE + GRF_GPIO2A_P, REG_SOC_WMSK | 0);
		mmio_write_32(GRF_BASE + GRF_GPIO2B_P, REG_SOC_WMSK | 0);

		/* set gpio2a0 ~ gpio2b4 to input */
		mmio_clrbits_32(GPIO2_BASE + 0x04, 0x00001fff);
	}

	/* apio3 charge gpio2c0 ~ gpio2d4*/
	if (suspend_apio->apio3) {

		/* set gpio2a0 ~ gpio2b4 iomux to gpio */
		mmio_write_32(GRF_BASE + GRF_GPIO2C_IOMUX,
			      REG_SOC_WMSK | GRF_IOMUX_GPIO);
		mmio_write_32(GRF_BASE + GRF_GPIO2D_IOMUX,
			      REG_SOC_WMSK | GRF_IOMUX_GPIO);

		/* set gpio2c0 ~ gpio2d4 pull mode to pull none */
		mmio_write_32(GRF_BASE + GRF_GPIO2C_P, REG_SOC_WMSK | 0);
		mmio_write_32(GRF_BASE + GRF_GPIO2D_P, REG_SOC_WMSK | 0);

		/* set gpio2c0 ~ gpio2d4 to input */
		mmio_clrbits_32(GPIO2_BASE + 0x04, 0x1fff0000);
	}

	/* apio4 charge gpio4c0 ~ gpio4c7, gpio4d0 ~ gpio4d6 */
	if (suspend_apio->apio4) {

		/* set gpio4c0 ~ gpio4d6 iomux to gpio */
		mmio_write_32(GRF_BASE + GRF_GPIO4C_IOMUX,
			      REG_SOC_WMSK | GRF_IOMUX_GPIO);
		mmio_write_32(GRF_BASE + GRF_GPIO4D_IOMUX,
			      REG_SOC_WMSK | GRF_IOMUX_GPIO);

		/* set gpio4c0 ~ gpio4d6 pull mode to pull none */
		mmio_write_32(GRF_BASE + GRF_GPIO4C_P, REG_SOC_WMSK | 0);
		mmio_write_32(GRF_BASE + GRF_GPIO4D_P, REG_SOC_WMSK | 0);

		/* set gpio4c0 ~ gpio4d6 to input */
		mmio_clrbits_32(GPIO4_BASE + 0x04, 0x7fff0000);
	}

	/* apio5 charge gpio3d0 ~ gpio3d7, gpio4a0 ~ gpio4a7*/
	if (suspend_apio->apio5) {
		/* set gpio3d0 ~ gpio4a7 iomux to gpio */
		mmio_write_32(GRF_BASE + GRF_GPIO3D_IOMUX,
			      REG_SOC_WMSK | GRF_IOMUX_GPIO);
		mmio_write_32(GRF_BASE + GRF_GPIO4A_IOMUX,
			      REG_SOC_WMSK | GRF_IOMUX_GPIO);

		/* set gpio3d0 ~ gpio4a7 pull mode to pull none */
		mmio_write_32(GRF_BASE + GRF_GPIO3D_P, REG_SOC_WMSK | 0);
		mmio_write_32(GRF_BASE + GRF_GPIO4A_P, REG_SOC_WMSK | 0);

		/* set gpio4c0 ~ gpio4d6 to input */
		mmio_clrbits_32(GPIO3_BASE + 0x04, 0xff000000);
		mmio_clrbits_32(GPIO4_BASE + 0x04, 0x000000ff);
	}
}

static void resume_apio(void)
{
	struct apio_info *suspend_apio;
	int i;

	suspend_apio = plat_get_rockchip_suspend_apio();

	if (!suspend_apio)
		return;

	for (i = 0; i < 12; i++) {
		mmio_write_32(GRF_BASE + GRF_GPIO2A_P + i * 4,
			      REG_SOC_WMSK | pull_mode_status[i]);
		mmio_write_32(GRF_BASE + GRF_GPIO2A_IOMUX + i * 4,
			      REG_SOC_WMSK | iomux_status[i]);
	}

	/* set gpio2 ~ gpio4 direction back to store value */
	mmio_write_32(GPIO2_BASE + 0x04, gpio_direction[0]);
	mmio_write_32(GPIO3_BASE + 0x04, gpio_direction[1]);
	mmio_write_32(GPIO4_BASE + 0x04, gpio_direction[2]);

	/* set gpio2 ~ gpio4 clock gate back to store value */
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(31),
		      BITS_WITH_WMASK(gpio_2_4_clk_gate, 0x07,
				      PCLK_GPIO2_GATE_SHIFT));
}

static void suspend_gpio(void)
{
	struct gpio_info *suspend_gpio;
	uint32_t count;
	int i;

	suspend_gpio = plat_get_rockchip_suspend_gpio(&count);

	for (i = 0; i < count; i++) {
		gpio_set_value(suspend_gpio[i].index, suspend_gpio[i].polarity);
		gpio_set_direction(suspend_gpio[i].index, GPIO_DIR_OUT);
		udelay(1);
	}
}

static void resume_gpio(void)
{
	struct gpio_info *suspend_gpio;
	uint32_t count;
	int i;

	suspend_gpio = plat_get_rockchip_suspend_gpio(&count);

	for (i = count - 1; i >= 0; i--) {
		gpio_set_value(suspend_gpio[i].index,
			       !suspend_gpio[i].polarity);
		gpio_set_direction(suspend_gpio[i].index, GPIO_DIR_OUT);
		udelay(1);
	}
}

static void m0_clock_init(void)
{
	/* enable clocks for M0 */
	mmio_write_32(PMUCRU_BASE + PMUCRU_CLKGATE_CON2,
		      BITS_WITH_WMASK(0x0, 0x2f, 0));

	/* switch the parent to xin24M and div == 1 */
	mmio_write_32(PMUCRU_BASE + PMUCRU_CLKSEL_CON0,
		      BIT_WITH_WMSK(15) | BITS_WITH_WMASK(0x0, 0x1f, 8));

	/* start M0 */
	mmio_write_32(PMUCRU_BASE + PMUCRU_SOFTRST_CON0,
		      BITS_WITH_WMASK(0x0, 0x24, 0));

	/* gating disable for M0 */
	mmio_write_32(PMUCRU_BASE + PMUCRU_GATEDIS_CON0, BIT_WITH_WMSK(1));
}

static void m0_reset(void)
{
	/* stop M0 */
	mmio_write_32(PMUCRU_BASE + PMUCRU_SOFTRST_CON0,
		      BITS_WITH_WMASK(0x24, 0x24, 0));

	/* recover gating bit for M0 */
	mmio_write_32(PMUCRU_BASE + PMUCRU_GATEDIS_CON0, WMSK_BIT(1));

	/* disable clocks for M0 */
	mmio_write_32(PMUCRU_BASE + PMUCRU_CLKGATE_CON2,
		      BITS_WITH_WMASK(0x2f, 0x2f, 0));
}

static int sys_pwr_domain_suspend(void)
{
	uint32_t wait_cnt = 0;
	uint32_t status = 0;

	dmc_save();
	pmu_scu_b_pwrdn();

	pmu_power_domains_suspend();
	set_hw_idle(BIT(PMU_CLR_CENTER1) |
		    BIT(PMU_CLR_ALIVE) |
		    BIT(PMU_CLR_MSCH0) |
		    BIT(PMU_CLR_MSCH1) |
		    BIT(PMU_CLR_CCIM0) |
		    BIT(PMU_CLR_CCIM1) |
		    BIT(PMU_CLR_CENTER) |
		    BIT(PMU_CLR_GIC));

	sys_slp_config();

	m0_clock_init();

	pmu_sgrf_rst_hld();

	mmio_write_32(SGRF_BASE + SGRF_SOC_CON0_1(1),
		      (PMUSRAM_BASE >> CPU_BOOT_ADDR_ALIGN) |
		      CPU_BOOT_ADDR_WMASK);

	mmio_write_32(PMU_BASE + PMU_ADB400_CON,
		      BIT_WITH_WMSK(PMU_PWRDWN_REQ_CORE_B_2GIC_SW) |
		      BIT_WITH_WMSK(PMU_PWRDWN_REQ_CORE_B_SW) |
		      BIT_WITH_WMSK(PMU_PWRDWN_REQ_GIC2_CORE_B_SW));
	dsb();
	status = BIT(PMU_PWRDWN_REQ_CORE_B_2GIC_SW_ST) |
		BIT(PMU_PWRDWN_REQ_CORE_B_SW_ST) |
		BIT(PMU_PWRDWN_REQ_GIC2_CORE_B_SW_ST);
	while ((mmio_read_32(PMU_BASE +
	       PMU_ADB400_ST) & status) != status) {
		wait_cnt++;
		if (wait_cnt >= MAX_WAIT_COUNT) {
			ERROR("%s:wait cluster-b l2(%x)\n", __func__,
			      mmio_read_32(PMU_BASE + PMU_ADB400_ST));
			panic();
		}
	}
	mmio_setbits_32(PMU_BASE + PMU_PWRDN_CON, BIT(PMU_SCU_B_PWRDWN_EN));

	secure_watchdog_disable();

	/*
	 * Disabling PLLs/PWM/DVFS is approaching WFI which is
	 * the last steps in suspend.
	 */
	disable_dvfs_plls();
	disable_pwms();
	disable_nodvfs_plls();

	suspend_apio();
	suspend_gpio();

	return 0;
}

static int sys_pwr_domain_resume(void)
{
	uint32_t wait_cnt = 0;
	uint32_t status = 0;

	resume_apio();
	resume_gpio();
	enable_nodvfs_plls();
	enable_pwms();
	/* PWM regulators take time to come up; give 300us to be safe. */
	udelay(300);
	enable_dvfs_plls();

	secure_watchdog_restore();

	/* restore clk_ddrc_bpll_src_en gate */
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(3),
		      BITS_WITH_WMASK(clk_ddrc_save, 0xff, 0));

	/*
	 * The wakeup status is not cleared by itself, we need to clear it
	 * manually. Otherwise we will alway query some interrupt next time.
	 *
	 * NOTE: If the kernel needs to query this, we might want to stash it
	 * somewhere.
	 */
	mmio_write_32(PMU_BASE + PMU_WAKEUP_STATUS, 0xffffffff);
	mmio_write_32(PMU_BASE + PMU_WKUP_CFG4, 0x00);

	mmio_write_32(SGRF_BASE + SGRF_SOC_CON0_1(1),
		      (cpu_warm_boot_addr >> CPU_BOOT_ADDR_ALIGN) |
		      CPU_BOOT_ADDR_WMASK);

	mmio_write_32(PMU_BASE + PMU_CCI500_CON,
		      WMSK_BIT(PMU_CLR_PREQ_CCI500_HW) |
		      WMSK_BIT(PMU_CLR_QREQ_CCI500_HW) |
		      WMSK_BIT(PMU_QGATING_CCI500_CFG));
	dsb();
	mmio_clrbits_32(PMU_BASE + PMU_PWRDN_CON,
			BIT(PMU_SCU_B_PWRDWN_EN));

	mmio_write_32(PMU_BASE + PMU_ADB400_CON,
		      WMSK_BIT(PMU_PWRDWN_REQ_CORE_B_2GIC_SW) |
		      WMSK_BIT(PMU_PWRDWN_REQ_CORE_B_SW) |
		      WMSK_BIT(PMU_PWRDWN_REQ_GIC2_CORE_B_SW) |
		      WMSK_BIT(PMU_CLR_CORE_L_HW) |
		      WMSK_BIT(PMU_CLR_CORE_L_2GIC_HW) |
		      WMSK_BIT(PMU_CLR_GIC2_CORE_L_HW));

	status = BIT(PMU_PWRDWN_REQ_CORE_B_2GIC_SW_ST) |
		BIT(PMU_PWRDWN_REQ_CORE_B_SW_ST) |
		BIT(PMU_PWRDWN_REQ_GIC2_CORE_B_SW_ST);

	while ((mmio_read_32(PMU_BASE +
	   PMU_ADB400_ST) & status)) {
		wait_cnt++;
		if (wait_cnt >= MAX_WAIT_COUNT) {
			ERROR("%s:wait cluster-b l2(%x)\n", __func__,
			      mmio_read_32(PMU_BASE + PMU_ADB400_ST));
			panic();
		}
	}

	pmu_sgrf_rst_hld_release();
	pmu_scu_b_pwrup();
	pmu_power_domains_resume();

	restore_dpll();
	sram_func_set_ddrctl_pll(DPLL_ID);
	restore_abpll();

	clr_hw_idle(BIT(PMU_CLR_CENTER1) |
				BIT(PMU_CLR_ALIVE) |
				BIT(PMU_CLR_MSCH0) |
				BIT(PMU_CLR_MSCH1) |
				BIT(PMU_CLR_CCIM0) |
				BIT(PMU_CLR_CCIM1) |
				BIT(PMU_CLR_CENTER) |
				BIT(PMU_CLR_GIC));

	plat_rockchip_gic_cpuif_enable();

	m0_reset();

	return 0;
}

void __dead2 soc_soft_reset(void)
{
	struct gpio_info *rst_gpio;

	rst_gpio = plat_get_rockchip_gpio_reset();

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

	poweroff_gpio = plat_get_rockchip_gpio_poweroff();

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
	.hlvl_pwr_dm_suspend = hlvl_pwr_domain_suspend,
	.hlvl_pwr_dm_resume = hlvl_pwr_domain_resume,
	.hlvl_pwr_dm_off = hlvl_pwr_domain_off,
	.hlvl_pwr_dm_on_finish = hlvl_pwr_domain_on_finish,
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

	for (cpu = 0; cpu < PLATFORM_CLUSTER_COUNT; cpu++)
		clst_warmboot_data[cpu] = 0;

	psram_sleep_cfg->ddr_func = (uint64_t)dmc_restore;
	psram_sleep_cfg->ddr_data = (uint64_t)&sdram_config;
	psram_sleep_cfg->ddr_flag = 0x01;

	psram_sleep_cfg->boot_mpidr = read_mpidr_el1() & 0xffff;

	/* config cpu's warm boot address */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON0_1(1),
		      (cpu_warm_boot_addr >> CPU_BOOT_ADDR_ALIGN) |
		      CPU_BOOT_ADDR_WMASK);
	mmio_write_32(PMU_BASE + PMU_NOC_AUTO_ENA, NOC_AUTO_ENABLE);

	/*
	 * Enable Schmitt trigger for better 32 kHz input signal, which is
	 * important for suspend/resume reliability among other things.
	 */
	mmio_write_32(PMUGRF_BASE + PMUGRF_GPIO0A_SMT, GPIO0A0_SMT_ENABLE);

	init_pmu_counts();

	nonboot_cpus_off();

	INFO("%s(%d): pd status %x\n", __func__, __LINE__,
	     mmio_read_32(PMU_BASE + PMU_PWRDN_ST));
}
