/*
 * Copyright (c) 2024, Rockchip, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <common/debug.h>
#include <drivers/arm/gicv3.h>
#include <drivers/console.h>
#include <drivers/delay_timer.h>
#include <drivers/ti/uart/uart_16550.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <pmu.h>

#include <cpus_on_fixed_addr.h>
#include <plat_pm_helpers.h>
#include <plat_private.h>
#include <pm_pd_regs.h>
#include <rk3588_clk.h>
#include <rockchip_sip_svc.h>
#include <secure.h>
#include <soc.h>

#define PSRAM_SP_TOP	((PMUSRAM_BASE + PMUSRAM_RSIZE) & ~0xf)
#define NONBOOT_CPUS_OFF_LOOP (500000)

#define DSUGRF_REG_CNT			(0x78 / 4 + 1)
#define BCORE_GRF_REG_CNT		(0x30 / 4 + 1)
#define LCORE_GRF_REG_CNT		(0x30 / 4 + 1)

#define CENTER_GRF_REG_CNT		(0x20 / 4 + 1)

static struct psram_data_t *psram_sleep_cfg =
	(struct psram_data_t *)&sys_sleep_flag_sram;

static int8_t pd_repair_map[] = {
	[PD_GPU] = PD_RPR_GPU,
	[PD_NPU] = -1,
	[PD_VCODEC] = -1,
	[PD_NPUTOP] = PD_RPR_NPUTOP,
	[PD_NPU1] = PD_RPR_NPU1,
	[PD_NPU2] = PD_RPR_NPU2,
	[PD_VENC0] = PD_RPR_VENC0,
	[PD_VENC1] = PD_RPR_VENC1,
	[PD_RKVDEC0] = PD_RPR_RKVDEC0,
	[PD_RKVDEC1] = PD_RPR_RKVDEC1,
	[PD_VDPU] = PD_RPR_VDPU,
	[PD_RGA30] = PD_RPR_RGA30,
	[PD_AV1] = PD_RPR_AV1,
	[PD_VI] = PD_RPR_VI,
	[PD_FEC] = PD_RPR_FEC,
	[PD_ISP1] = PD_RPR_ISP1,
	[PD_RGA31] = PD_RPR_RGA31,
	[PD_VOP] = PD_RPR_VOP,
	[PD_VO0] = PD_RPR_VO0,
	[PD_VO1] = PD_RPR_VO1,
	[PD_AUDIO] = PD_RPR_AUDIO,
	[PD_PHP] = PD_RPR_PHP,
	[PD_GMAC] = PD_RPR_GMAC,
	[PD_PCIE] = PD_RPR_PCIE,
	[PD_NVM] = -1,
	[PD_NVM0] = PD_RPR_NVM0,
	[PD_SDIO] = PD_RPR_SDIO,
	[PD_USB] = PD_RPR_USB,
	[PD_SECURE] = -1,
	[PD_SDMMC] = PD_RPR_SDMMC,
	[PD_CRYPTO] = PD_RPR_CRYPTO,
	[PD_CENTER] = PD_RPR_CENTER,
	[PD_DDR01] = PD_RPR_DDR01,
	[PD_DDR23] = PD_RPR_DDR23,
};

struct rk3588_sleep_ddr_data {
	uint32_t gpio0a_iomux_l, gpio0a_iomux_h, gpio0b_iomux_l;
	uint32_t pmu_pd_st0, bus_idle_st0, qch_pwr_st;
	uint32_t pmu2_vol_gate_con[3], pmu2_submem_gate_sft_con0;
	uint32_t pmu2_bisr_con0;
	uint32_t cpll_con0;
	uint32_t cru_mode_con, busscru_mode_con;
	uint32_t bussgrf_soc_con7;
	uint32_t pmu0grf_soc_con0, pmu0grf_soc_con1, pmu0grf_soc_con3;
	uint32_t pmu1grf_soc_con2, pmu1grf_soc_con7, pmu1grf_soc_con8, pmu1grf_soc_con9;
	uint32_t pmu0sgrf_soc_con1;
	uint32_t pmu1sgrf_soc_con14;
	uint32_t ddrgrf_chn_con0[4], ddrgrf_chn_con1[4],
		ddrgrf_chn_con2[4], pmu1_ddr_pwr_sft_con[4];
	uint32_t pmu1cru_clksel_con1;
};

static struct rk3588_sleep_ddr_data ddr_data;

struct rk3588_sleep_pmusram_data {
	uint32_t dsusgrf_soc_con[DSUSGRF_SOC_CON_CNT],
		dsusgrf_ddr_hash_con[DSUSGRF_DDR_HASH_CON_CNT];
	uint32_t dsu_ddr_fw_rgn_reg[FIREWALL_DSU_RGN_CNT],
		dsu_ddr_fw_mst_reg[FIREWALL_DSU_MST_CNT],
		dsu_ddr_fw_con_reg[FIREWALL_DSU_CON_CNT];
	uint32_t busioc_gpio0b_iomux_h;
};

static __pmusramdata struct rk3588_sleep_pmusram_data pmusram_data;

static __pmusramfunc void dsu_restore_early(void)
{
	int i;

	/* dsusgrf */
	for (i = 0; i < DSUSGRF_SOC_CON_CNT; i++)
		mmio_write_32(DSUSGRF_BASE + DSUSGRF_SOC_CON(i),
			      WITH_16BITS_WMSK(pmusram_data.dsusgrf_soc_con[i]));

	for (i = 0; i < DSUSGRF_DDR_HASH_CON_CNT; i++)
		mmio_write_32(DSUSGRF_BASE + DSUSGRF_DDR_HASH_CON(i),
			      pmusram_data.dsusgrf_ddr_hash_con[i]);

	/* dsu ddr firewall */
	for (i = 0; i < FIREWALL_DSU_RGN_CNT; i++)
		mmio_write_32(FIREWALL_DSU_BASE + FIREWALL_DSU_RGN(i),
			      pmusram_data.dsu_ddr_fw_rgn_reg[i]);

	for (i = 0; i < FIREWALL_DSU_MST_CNT; i++)
		mmio_write_32(FIREWALL_DSU_BASE + FIREWALL_DSU_MST(i),
			      pmusram_data.dsu_ddr_fw_mst_reg[i]);

	for (i = 0; i < FIREWALL_DSU_CON_CNT; i++)
		mmio_write_32(FIREWALL_DSU_BASE + FIREWALL_DSU_CON(i),
			      pmusram_data.dsu_ddr_fw_con_reg[i]);
}

static __pmusramfunc void ddr_resume(void)
{
	/* check the crypto function had been enabled or not */
	if ((mmio_read_32(DSUSGRF_BASE + DSU_SGRF_SOC_CON(4)) & BIT(4)) != 0) {
		/* enable the crypto function */
		mmio_write_32(DSUSGRF_BASE + DSU_SGRF_SOC_CON(4), BITS_WITH_WMASK(0, 0x1, 4));
		dsb();
		isb();

		__asm__ volatile ("mov	x0, #3\n"
				  "dsb	sy\n"
				  "msr	rmr_el3, x0\n"
				  "1:\n"
				  "isb\n"
				  "wfi\n"
				  "b 1b\n");
	}

	dsu_restore_early();
}

static void dsu_core_save(void)
{
	int i;

	/* dsusgrf */
	for (i = 0; i < DSUSGRF_SOC_CON_CNT; i++)
		pmusram_data.dsusgrf_soc_con[i] =
			mmio_read_32(DSUSGRF_BASE + DSUSGRF_SOC_CON(i));

	for (i = 0; i < DSUSGRF_DDR_HASH_CON_CNT; i++)
		pmusram_data.dsusgrf_ddr_hash_con[i] =
			mmio_read_32(DSUSGRF_BASE + DSUSGRF_DDR_HASH_CON(i));

	/* dsu ddr firewall */
	for (i = 0; i < FIREWALL_DSU_RGN_CNT; i++)
		pmusram_data.dsu_ddr_fw_rgn_reg[i] =
			mmio_read_32(FIREWALL_DSU_BASE + FIREWALL_DSU_RGN(i));

	for (i = 0; i < FIREWALL_DSU_MST_CNT; i++)
		pmusram_data.dsu_ddr_fw_mst_reg[i] =
			mmio_read_32(FIREWALL_DSU_BASE + FIREWALL_DSU_MST(i));

	for (i = 0; i < FIREWALL_DSU_CON_CNT; i++)
		pmusram_data.dsu_ddr_fw_con_reg[i] =
			mmio_read_32(FIREWALL_DSU_BASE + FIREWALL_DSU_CON(i));

	pvtplls_suspend();
	pd_dsu_core_save();
}

static void dsu_core_restore(void)
{
	pd_dsu_core_restore();
	pvtplls_resume();
}

static uint32_t clk_save[CRU_CLKGATE_CON_CNT + PHPCRU_CLKGATE_CON_CNT +
			 SECURECRU_CLKGATE_CON_CNT + PMU1CRU_CLKGATE_CON_CNT];

void clk_gate_con_save(void)
{
	int i, j = 0;

	for (i = 0; i < CRU_CLKGATE_CON_CNT; i++, j++)
		clk_save[j] = mmio_read_32(CRU_BASE + CRU_CLKGATE_CON(i));

	clk_save[j] = mmio_read_32(PHP_CRU_BASE + PHPCRU_CLKGATE_CON);

	for (i = 0; i < SECURECRU_CLKGATE_CON_CNT; i++, j++)
		clk_save[j] = mmio_read_32(SCRU_BASE + SECURECRU_CLKGATE_CON(i));

	for (i = 0; i < PMU1CRU_CLKGATE_CON_CNT; i++, j++)
		clk_save[j] = mmio_read_32(PMU1CRU_BASE + CRU_CLKGATE_CON(i));
}

void clk_gate_con_disable(void)
{
	int i;

	for (i = 0; i < CRU_CLKGATE_CON_CNT; i++)
		mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(i), 0xffff0000);

	 mmio_write_32(PHP_CRU_BASE + PHPCRU_CLKGATE_CON, 0xffff0000);

	for (i = 0; i < SECURECRU_CLKGATE_CON_CNT; i++)
		mmio_write_32(SCRU_BASE + SECURECRU_CLKGATE_CON(i), 0xffff0000);

	for (i = 0; i < PMU1CRU_CLKGATE_CON_CNT; i++)
		mmio_write_32(PMU1CRU_BASE + CRU_CLKGATE_CON(i), 0xffff0000);
}

void clk_gate_con_restore(void)
{
	int i, j = 0;

	for (i = 0; i < CRU_CLKGATE_CON_CNT; i++, j++)
		mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(i),
			      WITH_16BITS_WMSK(clk_save[j]));

	mmio_write_32(PHP_CRU_BASE + PHPCRU_CLKGATE_CON,
		      WITH_16BITS_WMSK(clk_save[j]));

	for (i = 0; i < SECURECRU_CLKGATE_CON_CNT; i++, j++)
		mmio_write_32(SCRU_BASE + SECURECRU_CLKGATE_CON(i),
			      WITH_16BITS_WMSK(clk_save[j]));

	for (i = 0; i < PMU1CRU_CLKGATE_CON_CNT; i++, j++)
		mmio_write_32(PMU1CRU_BASE + CRU_CLKGATE_CON(i),
			      WITH_16BITS_WMSK(clk_save[j]));
}

static void pmu_bus_idle_req(uint32_t bus, uint32_t state)
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
		     mmio_read_32(PMU_BASE + PMU2_BUS_IDLE_ST(bus / 32)));
}

static void pmu_qch_pwr_ctlr(uint32_t msk, uint32_t state)
{
	uint32_t wait_cnt = 0;

	if (state != 0)
		state = msk;

	mmio_write_32(PMU_BASE + PMU2_QCHANNEL_PWR_SFTCON,
		      BITS_WITH_WMASK(state, msk, 0));

	while ((mmio_read_32(PMU_BASE + PMU2_QCHANNEL_STATUS) & msk) != state) {
		if (++wait_cnt > QCH_PWR_LOOP)
			break;
		udelay(1);
	}

	if (wait_cnt > BUS_IDLE_LOOP)
		WARN("%s: can't wait qch:0x%x to state:0x%x (0x%x)\n",
		     __func__, msk, state,
		     mmio_read_32(PMU_BASE + PMU2_QCHANNEL_STATUS));
}

static inline uint32_t pmu_power_domain_chain_st(uint32_t pd)
{
	return mmio_read_32(PMU_BASE + PMU2_PWR_CHAIN1_ST(pd / 32)) & BIT(pd % 32) ?
	       pmu_pd_on :
	       pmu_pd_off;
}

static inline uint32_t pmu_power_domain_mem_st(uint32_t pd)
{
	return mmio_read_32(PMU_BASE + PMU2_PWR_MEM_ST(pd / 32)) & BIT(pd % 32) ?
	       pmu_pd_off :
	       pmu_pd_on;
}

static inline uint32_t pmu_power_domain_st(uint32_t pd)
{
	int8_t pd_repair = pd_repair_map[pd];

	if (pd_repair >= 0)
		return mmio_read_32(PMU_BASE + PMU2_BISR_STATUS(4)) & BIT(pd_repair) ?
		       pmu_pd_on :
		       pmu_pd_off;
	else
		return mmio_read_32(PMU_BASE + PMU2_PWR_GATE_ST(pd / 32)) & BIT(pd % 32) ?
		       pmu_pd_off :
		       pmu_pd_on;
}

static int pmu_power_domain_pd_to_mem_st(uint32_t pd, uint32_t *pd_mem_st)
{
	uint32_t mem_st;

	switch (pd) {
	case PD_NPUTOP:
		mem_st = PD_NPU_TOP_MEM_ST;
		break;
	case PD_NPU1:
		mem_st = PD_NPU1_MEM_ST;
		break;
	case PD_NPU2:
		mem_st = PD_NPU2_MEM_ST;
		break;
	case PD_VENC0:
		mem_st = PD_VENC0_MEM_ST;
		break;
	case PD_VENC1:
		mem_st = PD_VENC1_MEM_ST;
		break;
	case PD_RKVDEC0:
		mem_st = PD_RKVDEC0_MEM_ST;
		break;
	case PD_RKVDEC1:
		mem_st = PD_RKVDEC1_MEM_ST;
		break;
	case PD_RGA30:
		mem_st = PD_RGA30_MEM_ST;
		break;
	case PD_AV1:
		mem_st = PD_AV1_MEM_ST;
		break;
	case PD_VI:
		mem_st = PD_VI_MEM_ST;
		break;
	case PD_FEC:
		mem_st = PD_FEC_MEM_ST;
		break;
	case PD_ISP1:
		mem_st = PD_ISP1_MEM_ST;
		break;
	case PD_RGA31:
		mem_st = PD_RGA31_MEM_ST;
		break;
	case PD_VOP:
		mem_st = PD_VOP_MEM_ST;
		break;
	case PD_VO0:
		mem_st = PD_VO0_MEM_ST;
		break;
	case PD_VO1:
		mem_st = PD_VO1_MEM_ST;
		break;
	case PD_AUDIO:
		mem_st = PD_AUDIO_MEM_ST;
		break;
	case PD_PHP:
		mem_st = PD_PHP_MEM_ST;
		break;
	case PD_GMAC:
		mem_st = PD_GMAC_MEM_ST;
		break;
	case PD_PCIE:
		mem_st = PD_PCIE_MEM_ST;
		break;
	case PD_NVM0:
		mem_st = PD_NVM0_MEM_ST;
		break;
	case PD_SDIO:
		mem_st = PD_SDIO_MEM_ST;
		break;
	case PD_USB:
		mem_st = PD_USB_MEM_ST;
		break;
	case PD_SDMMC:
		mem_st = PD_SDMMC_MEM_ST;
		break;
	default:
		return -EINVAL;
	}

	*pd_mem_st = mem_st;

	return 0;
}

static int pmu_power_domain_reset_mem(uint32_t pd, uint32_t pd_mem_st)
{
	uint32_t loop = 0;
	int ret = 0;

	while (pmu_power_domain_chain_st(pd_mem_st) != pmu_pd_on) {
		udelay(1);
		loop++;
		if (loop >= PD_CTR_LOOP) {
			WARN("%s: %d chain up time out\n", __func__, pd);
			ret = -EINVAL;
			goto error;
		}
	}

	udelay(60);

	mmio_write_32(PMU_BASE + PMU2_MEMPWR_GATE_SFTCON(pd / 16),
		      BITS_WITH_WMASK(pmu_pd_off, 0x1, pd % 16));
	dsb();

	loop = 0;
	while (pmu_power_domain_mem_st(pd_mem_st) != pmu_pd_off) {
		udelay(1);
		loop++;
		if (loop >= PD_CTR_LOOP) {
			WARN("%s: %d mem down time out\n", __func__, pd);
			ret = -EINVAL;
			goto error;
		}
	}

	mmio_write_32(PMU_BASE + PMU2_MEMPWR_GATE_SFTCON(pd / 16),
		      BITS_WITH_WMASK(pmu_pd_on, 0x1, pd % 16));
	dsb();

	loop = 0;
	while (pmu_power_domain_mem_st(pd_mem_st) != pmu_pd_on) {
		udelay(1);
		loop++;
		if (loop >= PD_CTR_LOOP) {
			WARN("%s: %d mem up time out\n", __func__, pd);
			ret = -EINVAL;
			goto error;
		}
	}

	return 0;

error:
	return ret;
}

static int pmu_power_domain_ctr(uint32_t pd, uint32_t pd_state)
{
	uint32_t loop = 0;
	uint32_t is_mem_on = pmu_pd_off;
	uint32_t pd_mem_st;
	int ret = 0;

	if (pd_state == pmu_pd_on) {
		ret = pmu_power_domain_pd_to_mem_st(pd, &pd_mem_st);
		if (ret == 0) {
			is_mem_on = pmu_power_domain_mem_st(pd_mem_st);
			if (is_mem_on == pmu_pd_on)
				WARN("%s: %d mem is up\n", __func__, pd);
		}
	}

	mmio_write_32(PMU_BASE + PMU2_PWR_GATE_SFTCON(pd / 16),
		      BITS_WITH_WMASK(pd_state, 0x1, pd % 16));
	dsb();

	if (is_mem_on == pmu_pd_on) {
		ret = pmu_power_domain_reset_mem(pd, pd_mem_st);
		if (ret != 0)
			goto out;
		WARN("%s: %d mem reset ok\n", __func__, pd);
	}

	while ((pmu_power_domain_st(pd) != pd_state) && (loop < PD_CTR_LOOP)) {
		udelay(1);
		loop++;
	}

	if (pmu_power_domain_st(pd) != pd_state) {
		WARN("%s: %d, %d, (0x%x, 0x%x) error!\n", __func__, pd, pd_state,
		     mmio_read_32(PMU_BASE + PMU2_PWR_GATE_ST(0)),
		     mmio_read_32(PMU_BASE + PMU2_BISR_STATUS(4)));
		ret = -EINVAL;
	}

out:
	return ret;
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
	case PD_NPUTOP:
		pmu_bus_idle_req(BUS_ID_NPUTOP, state);
		break;
	case PD_NPU1:
		pmu_bus_idle_req(BUS_ID_NPU1, state);
		break;
	case PD_NPU2:
		pmu_bus_idle_req(BUS_ID_NPU2, state);
		break;
	case PD_VENC0:
		pmu_bus_idle_req(BUS_ID_RKVENC0, state);
		break;
	case PD_VENC1:
		pmu_bus_idle_req(BUS_ID_RKVENC1, state);
		break;
	case PD_RKVDEC0:
		pmu_bus_idle_req(BUS_ID_RKVDEC0, state);
		break;
	case PD_RKVDEC1:
		pmu_bus_idle_req(BUS_ID_RKVDEC1, state);
		break;
	case PD_VDPU:
		pmu_bus_idle_req(BUS_ID_VDPU, state);
		break;
	case PD_AV1:
		pmu_bus_idle_req(BUS_ID_AV1, state);
		break;
	case PD_VI:
		pmu_bus_idle_req(BUS_ID_VI, state);
		break;
	case PD_ISP1:
		pmu_bus_idle_req(BUS_ID_ISP, state);
		break;
	case PD_RGA31:
		pmu_bus_idle_req(BUS_ID_RGA31, state);
		break;
	case PD_VOP:
		pmu_bus_idle_req(BUS_ID_VOP_CHANNEL, state);
		pmu_bus_idle_req(BUS_ID_VOP, state);
		break;
	case PD_VO0:
		pmu_bus_idle_req(BUS_ID_VO0, state);
		break;
	case PD_VO1:
		pmu_bus_idle_req(BUS_ID_VO1, state);
		break;
	case PD_AUDIO:
		pmu_bus_idle_req(BUS_ID_AUDIO, state);
		break;
	case PD_PHP:
		pmu_bus_idle_req(BUS_ID_PHP, state);
		break;
	case PD_NVM:
		pmu_bus_idle_req(BUS_ID_NVM, state);
		break;
	case PD_SDIO:
		pmu_bus_idle_req(BUS_ID_SDIO, state);
		break;
	case PD_USB:
		pmu_bus_idle_req(BUS_ID_USB, state);
		break;
	case PD_SECURE:
		pmu_bus_idle_req(BUS_ID_SECURE, state);
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
	ddr_data.qch_pwr_st =
		mmio_read_32(PMU_BASE + PMU2_QCHANNEL_STATUS) & PMU2_QCH_PWR_MSK;
	ddr_data.pmu_pd_st0 = mmio_read_32(PMU_BASE + PMU2_PWR_GATE_ST(0));
	ddr_data.bus_idle_st0 = mmio_read_32(PMU_BASE + PMU2_BUS_IDLE_ST(0));

	qos_save();

	if ((ddr_data.pmu_pd_st0 & BIT(PD_PHP)) == 0)
		pd_php_save();

	if ((ddr_data.pmu_pd_st0 & BIT(PD_CRYPTO)) == 0)
		pd_crypto_save();

	pmu_qch_pwr_ctlr(0x20, 1);
	pmu_qch_pwr_ctlr(0x40, 1);
	pmu_qch_pwr_ctlr(0x1, 1);
	pmu_qch_pwr_ctlr(0x2, 1);
	pmu_qch_pwr_ctlr(0x4, 1);
	pmu_qch_pwr_ctlr(0x8, 1);
	pmu_qch_pwr_ctlr(0x10, 1);

	pmu_bus_idle_req(BUS_ID_VO1USBTOP, bus_idle);
	pmu_bus_idle_req(BUS_ID_SECURE_VO1USB_CHANNEL, bus_idle);

	pmu_bus_idle_req(BUS_ID_USB, bus_idle);

	pmu_set_power_domain(PD_GPU, pmu_pd_off);

	pmu_set_power_domain(PD_NPU1, pmu_pd_off);
	pmu_set_power_domain(PD_NPU2, pmu_pd_off);
	pmu_set_power_domain(PD_NPUTOP, pmu_pd_off);
	pmu_set_power_domain(PD_NPU, pmu_pd_off);

	pmu_set_power_domain(PD_RKVDEC1, pmu_pd_off);
	pmu_set_power_domain(PD_RKVDEC0, pmu_pd_off);
	pmu_set_power_domain(PD_VENC1, pmu_pd_off);
	pmu_set_power_domain(PD_VENC0, pmu_pd_off);
	pmu_set_power_domain(PD_VCODEC, pmu_pd_off);

	pmu_set_power_domain(PD_RGA30, pmu_pd_off);
	pmu_set_power_domain(PD_AV1, pmu_pd_off);
	pmu_set_power_domain(PD_VDPU, pmu_pd_off);

	pmu_set_power_domain(PD_VO0, pmu_pd_off);
	pmu_set_power_domain(PD_VO1, pmu_pd_off);
	pmu_set_power_domain(PD_VOP, pmu_pd_off);

	pmu_set_power_domain(PD_FEC, pmu_pd_off);
	pmu_set_power_domain(PD_ISP1, pmu_pd_off);
	pmu_set_power_domain(PD_VI, pmu_pd_off);

	pmu_set_power_domain(PD_RGA31, pmu_pd_off);

	pmu_set_power_domain(PD_AUDIO, pmu_pd_off);

	pmu_set_power_domain(PD_GMAC, pmu_pd_off);
	pmu_set_power_domain(PD_PCIE, pmu_pd_off);
	pmu_set_power_domain(PD_PHP, pmu_pd_off);

	pmu_set_power_domain(PD_SDIO, pmu_pd_off);

	pmu_set_power_domain(PD_NVM0, pmu_pd_off);
	pmu_set_power_domain(PD_NVM, pmu_pd_off);

	pmu_set_power_domain(PD_SDMMC, pmu_pd_off);
	pmu_set_power_domain(PD_CRYPTO, pmu_pd_off);
}

static void pmu_power_domains_resume(void)
{
	int i;

	pmu_set_power_domain(PD_CRYPTO, !!(ddr_data.pmu_pd_st0 & BIT(PD_CRYPTO)));
	pmu_set_power_domain(PD_SDMMC, !!(ddr_data.pmu_pd_st0 & BIT(PD_SDMMC)));

	pmu_set_power_domain(PD_NVM, !!(ddr_data.pmu_pd_st0 & BIT(PD_NVM)));
	pmu_set_power_domain(PD_NVM0, !!(ddr_data.pmu_pd_st0 & BIT(PD_NVM0)));

	pmu_set_power_domain(PD_SDIO, !!(ddr_data.pmu_pd_st0 & BIT(PD_SDIO)));

	pmu_set_power_domain(PD_PHP, !!(ddr_data.pmu_pd_st0 & BIT(PD_PHP)));
	pmu_set_power_domain(PD_PCIE, !!(ddr_data.pmu_pd_st0 & BIT(PD_PCIE)));
	pmu_set_power_domain(PD_GMAC, !!(ddr_data.pmu_pd_st0 & BIT(PD_GMAC)));

	pmu_set_power_domain(PD_AUDIO, !!(ddr_data.pmu_pd_st0 & BIT(PD_AUDIO)));

	pmu_set_power_domain(PD_USB, !!(ddr_data.pmu_pd_st0 & BIT(PD_USB)));

	pmu_set_power_domain(PD_RGA31, !!(ddr_data.pmu_pd_st0 & BIT(PD_RGA31)));

	pmu_set_power_domain(PD_VI, !!(ddr_data.pmu_pd_st0 & BIT(PD_VI)));
	pmu_set_power_domain(PD_ISP1, !!(ddr_data.pmu_pd_st0 & BIT(PD_ISP1)));
	pmu_set_power_domain(PD_FEC, !!(ddr_data.pmu_pd_st0 & BIT(PD_FEC)));

	pmu_set_power_domain(PD_VOP, !!(ddr_data.pmu_pd_st0 & BIT(PD_VOP)));

	pmu_set_power_domain(PD_VO1, !!(ddr_data.pmu_pd_st0 & BIT(PD_VO1)));

	pmu_set_power_domain(PD_VO0, !!(ddr_data.pmu_pd_st0 & BIT(PD_VO0)));

	pmu_set_power_domain(PD_VDPU, !!(ddr_data.pmu_pd_st0 & BIT(PD_VDPU)));
	pmu_set_power_domain(PD_AV1, !!(ddr_data.pmu_pd_st0 & BIT(PD_AV1)));
	pmu_set_power_domain(PD_RGA30, !!(ddr_data.pmu_pd_st0 & BIT(PD_RGA30)));

	pmu_set_power_domain(PD_VCODEC, !!(ddr_data.pmu_pd_st0 & BIT(PD_VCODEC)));
	pmu_set_power_domain(PD_VENC0, !!(ddr_data.pmu_pd_st0 & BIT(PD_VENC0)));
	pmu_set_power_domain(PD_VENC1, !!(ddr_data.pmu_pd_st0 & BIT(PD_VENC1)));
	pmu_set_power_domain(PD_RKVDEC0, !!(ddr_data.pmu_pd_st0 & BIT(PD_RKVDEC0)));
	pmu_set_power_domain(PD_RKVDEC1, !!(ddr_data.pmu_pd_st0 & BIT(PD_RKVDEC1)));

	pmu_set_power_domain(PD_NPU, !!(ddr_data.pmu_pd_st0 & BIT(PD_NPU)));
	pmu_set_power_domain(PD_NPUTOP, !!(ddr_data.pmu_pd_st0 & BIT(PD_NPUTOP)));
	pmu_set_power_domain(PD_NPU2, !!(ddr_data.pmu_pd_st0 & BIT(PD_NPU2)));
	pmu_set_power_domain(PD_NPU1, !!(ddr_data.pmu_pd_st0 & BIT(PD_NPU1)));

	pmu_set_power_domain(PD_GPU, !!(ddr_data.pmu_pd_st0 & BIT(PD_GPU)));

	for (i = 0; i < 32; i++)
		pmu_bus_idle_req(i, !!(ddr_data.bus_idle_st0 & BIT(i)));

	pmu_qch_pwr_ctlr(0x10, !!(ddr_data.qch_pwr_st & 0x10));
	pmu_qch_pwr_ctlr(0x8, !!(ddr_data.qch_pwr_st & 0x8));
	pmu_qch_pwr_ctlr(0x4, !!(ddr_data.qch_pwr_st & 0x4));
	pmu_qch_pwr_ctlr(0x2, !!(ddr_data.qch_pwr_st & 0x2));
	pmu_qch_pwr_ctlr(0x1, !!(ddr_data.qch_pwr_st & 0x1));
	pmu_qch_pwr_ctlr(0x40, !!(ddr_data.qch_pwr_st & 0x40));
	pmu_qch_pwr_ctlr(0x20, !!(ddr_data.qch_pwr_st & 0x20));

	if ((ddr_data.pmu_pd_st0 & BIT(PD_CRYPTO)) == 0)
		pd_crypto_restore();

	if ((ddr_data.pmu_pd_st0 & BIT(PD_PHP)) == 0)
		pd_php_restore();

	qos_restore();
}

static int cpus_power_domain_on(uint32_t cpu_id)
{
	mmio_write_32(PMU_BASE + PMU2_CPU_AUTO_PWR_CON(cpu_id),
		      BITS_WITH_WMASK(0, 0x1, core_pm_en));
	mmio_write_32(PMU_BASE + PMU2_CPU_AUTO_PWR_CON(cpu_id),
		      BITS_WITH_WMASK(1, 0x1, core_pm_sft_wakeup_en));
	dsb();

	return 0;
}

static int cpus_power_domain_off(uint32_t cpu_id, uint32_t pd_cfg)
{
	uint32_t apm_value = BIT(core_pm_en);

	if (pd_cfg == core_pwr_wfi_int)
		apm_value |= BIT(core_pm_int_wakeup_en);

	mmio_write_32(PMU_BASE + PMU2_CPU_AUTO_PWR_CON(cpu_id),
		      BITS_WITH_WMASK(apm_value, 0x3, 0));
	dsb();

	return 0;
}

static inline void cpus_pd_req_enter_wfi(void)
{
	/* CORTEX_A55_CPUACTLR_EL1 */
	__asm__ volatile ("msr	DBGPRCR_EL1, xzr\n"
			  "mrs	x0, S3_0_C15_C2_7\n"
			  "orr	x0, x0, #0x1\n"
			  "msr	S3_0_C15_C2_7, x0\n"
			  "wfi_loop:\n"
			  "isb\n"
			  "wfi\n"
			  "b wfi_loop\n");
}

static void nonboot_cpus_off(void)
{
	uint32_t boot_cpu, cpu, tmp;
	uint32_t exp_st;
	uint32_t bcore0_rst_msk = 0, bcore1_rst_msk = 0;
	int wait_cnt;

	bcore0_rst_msk = CRU_BIGCPU02_RST_MSK | CRU_BIGCPU13_RST_MSK;
	bcore1_rst_msk = CRU_BIGCPU02_RST_MSK | CRU_BIGCPU13_RST_MSK;

	mmio_write_32(BIGCORE0CRU_BASE + 0xa00, BITS_WITH_WMASK(0, bcore0_rst_msk, 0));
	mmio_write_32(BIGCORE1CRU_BASE + 0xa00, BITS_WITH_WMASK(0, bcore1_rst_msk, 0));

	wait_cnt = NONBOOT_CPUS_OFF_LOOP;
	exp_st = SYS_GRF_BIG_CPUS_WFE;
	do {
		wait_cnt--;
		tmp = mmio_read_32(SYSGRF_BASE + SYS_GRF_SOC_STATUS(3));
		tmp &= SYS_GRF_BIG_CPUS_WFE;
	} while (tmp != exp_st && wait_cnt);

	boot_cpu = plat_my_core_pos();

	/* turn off noboot cpus */
	for (cpu = 0; cpu < PLATFORM_CORE_COUNT; cpu++) {
		if (cpu == boot_cpu)
			continue;
		cpus_power_domain_off(cpu, core_pwr_wfi);
	}

	mmio_write_32(SRAM_BASE + 0x08, (uintptr_t)&cpus_pd_req_enter_wfi);
	mmio_write_32(SRAM_BASE + 0x04, 0xdeadbeaf);

	dsb();
	isb();

	sev();

	wait_cnt = NONBOOT_CPUS_OFF_LOOP;
	do {
		wait_cnt--;
		tmp = mmio_read_32(PMU_BASE + PMU2_CLUSTER_ST);
		tmp &= CLUSTER_STS_NONBOOT_CPUS_DWN;
	} while (tmp != CLUSTER_STS_NONBOOT_CPUS_DWN && wait_cnt);

	if (tmp != CLUSTER_STS_NONBOOT_CPUS_DWN)
		ERROR("nonboot cpus status(%x) error!\n", tmp);
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

	flush_dcache_range((uintptr_t)cpuson_flags, sizeof(cpuson_flags));
	flush_dcache_range((uintptr_t)cpuson_entry_point,
			   sizeof(cpuson_entry_point));
	dsb();
	isb();

	cpus_power_domain_on(cpu_id);

	return PSCI_E_SUCCESS;
}

int rockchip_soc_cores_pwr_dm_on_finish(void)
{
	uint32_t cpu_id = plat_my_core_pos();

	mmio_write_32(PMU_BASE + PMU2_CPU_AUTO_PWR_CON(cpu_id),
		      BITS_WITH_WMASK(0, 0xf, 0));

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

	cpuson_flags[cpu_id] = PMU_CPU_AUTO_PWRDN;
	cpuson_entry_point[cpu_id] = plat_get_sec_entrypoint();
	dsb();
	flush_dcache_range((uintptr_t)cpuson_flags, sizeof(cpuson_flags));
	flush_dcache_range((uintptr_t)cpuson_entry_point,
			   sizeof(cpuson_entry_point));
	dsb();
	isb();

	cpus_power_domain_off(cpu_id, core_pwr_wfi_int);

	__asm__ volatile ("msr	DBGPRCR_EL1, xzr\n"
			  "mrs	x0, S3_0_C15_C2_7\n"
			  "orr	x0, x0, #0x1\n"
			  "msr	S3_0_C15_C2_7, x0\n");

	return PSCI_E_SUCCESS;
}

int rockchip_soc_cores_pwr_dm_resume(void)
{
	uint32_t cpu_id = plat_my_core_pos();

	mmio_write_32(PMU_BASE + PMU2_CPU_AUTO_PWR_CON(cpu_id),
		      BITS_WITH_WMASK(0, 0x3, 0));

	dsb();

	return PSCI_E_SUCCESS;
}

static void ddr_sleep_config(void)
{
	int i;

	if (pmu_power_domain_st(PD_DDR01) == 0) {
		ddr_data.ddrgrf_chn_con0[0] =
			mmio_read_32(DDR01GRF_BASE + DDRGRF_CHA_CON(0));
		ddr_data.ddrgrf_chn_con0[1] =
			mmio_read_32(DDR01GRF_BASE + DDRGRF_CHB_CON(0));
		ddr_data.ddrgrf_chn_con1[0] =
			mmio_read_32(DDR01GRF_BASE + DDRGRF_CHA_CON(1));
		ddr_data.ddrgrf_chn_con1[1] =
			mmio_read_32(DDR01GRF_BASE + DDRGRF_CHB_CON(1));
		ddr_data.ddrgrf_chn_con2[0] =
			mmio_read_32(DDR01GRF_BASE + DDRGRF_CHA_CON(2));
		ddr_data.ddrgrf_chn_con2[1] =
			mmio_read_32(DDR01GRF_BASE + DDRGRF_CHB_CON(2));

		mmio_write_32(DDR01GRF_BASE + DDRGRF_CHA_CON(2), 0x20002000);
		mmio_write_32(DDR01GRF_BASE + DDRGRF_CHB_CON(2), 0x20002000);
		mmio_write_32(DDR01GRF_BASE + DDRGRF_CHA_CON(2), 0x08000000);
		mmio_write_32(DDR01GRF_BASE + DDRGRF_CHB_CON(2), 0x08000000);
		mmio_write_32(DDR01GRF_BASE + DDRGRF_CHA_CON(0), 0x00200020);
		mmio_write_32(DDR01GRF_BASE + DDRGRF_CHB_CON(0), 0x00200020);
		mmio_write_32(DDR01GRF_BASE + DDRGRF_CHA_CON(1), 0x00400040);
		mmio_write_32(DDR01GRF_BASE + DDRGRF_CHB_CON(1), 0x00400040);
	}

	if (pmu_power_domain_st(PD_DDR23) == 0) {
		ddr_data.ddrgrf_chn_con0[2] =
			mmio_read_32(DDR23GRF_BASE + DDRGRF_CHA_CON(0));
		ddr_data.ddrgrf_chn_con0[3] =
			mmio_read_32(DDR23GRF_BASE + DDRGRF_CHB_CON(0));
		ddr_data.ddrgrf_chn_con1[2] =
			mmio_read_32(DDR23GRF_BASE + DDRGRF_CHA_CON(1));
		ddr_data.ddrgrf_chn_con1[3] =
			mmio_read_32(DDR23GRF_BASE + DDRGRF_CHB_CON(1));
		ddr_data.ddrgrf_chn_con2[2] =
			mmio_read_32(DDR23GRF_BASE + DDRGRF_CHA_CON(2));
		ddr_data.ddrgrf_chn_con2[3] =
			mmio_read_32(DDR23GRF_BASE + DDRGRF_CHB_CON(2));

		mmio_write_32(DDR23GRF_BASE + DDRGRF_CHA_CON(2), 0x20002000);
		mmio_write_32(DDR23GRF_BASE + DDRGRF_CHB_CON(2), 0x20002000);
		mmio_write_32(DDR23GRF_BASE + DDRGRF_CHA_CON(2), 0x08000000);
		mmio_write_32(DDR23GRF_BASE + DDRGRF_CHB_CON(2), 0x08000000);
		mmio_write_32(DDR23GRF_BASE + DDRGRF_CHA_CON(0), 0x00200020);
		mmio_write_32(DDR23GRF_BASE + DDRGRF_CHB_CON(0), 0x00200020);
		mmio_write_32(DDR23GRF_BASE + DDRGRF_CHA_CON(1), 0x00400040);
		mmio_write_32(DDR23GRF_BASE + DDRGRF_CHB_CON(1), 0x00400040);
	}

	for (i = 0; i < DDR_CHN_CNT; i++) {
		ddr_data.pmu1_ddr_pwr_sft_con[i] =
			mmio_read_32(PMU_BASE + PMU1_DDR_PWR_SFTCON(i));
		mmio_write_32(PMU_BASE + PMU1_DDR_PWR_SFTCON(i), 0x0fff0900);
	}
}

static void ddr_sleep_config_restore(void)
{
	int i;

	for (i = 0; i < DDR_CHN_CNT; i++) {
		mmio_write_32(PMU_BASE + PMU1_DDR_PWR_SFTCON(i),
			      0x0fff0000 | ddr_data.pmu1_ddr_pwr_sft_con[i]);
	}

	if (pmu_power_domain_st(PD_DDR01) == 0) {
		mmio_write_32(DDR01GRF_BASE + DDRGRF_CHA_CON(1),
			      0x00400000 | ddr_data.ddrgrf_chn_con1[0]);
		mmio_write_32(DDR01GRF_BASE + DDRGRF_CHB_CON(1),
			      0x00400000 | ddr_data.ddrgrf_chn_con1[1]);
		mmio_write_32(DDR01GRF_BASE + DDRGRF_CHA_CON(0),
			      0x00200000 | ddr_data.ddrgrf_chn_con0[0]);
		mmio_write_32(DDR01GRF_BASE + DDRGRF_CHB_CON(0),
			      0x00200000 | ddr_data.ddrgrf_chn_con0[1]);
		mmio_write_32(DDR01GRF_BASE + DDRGRF_CHA_CON(2),
			      0x28000000 | ddr_data.ddrgrf_chn_con2[0]);
		mmio_write_32(DDR01GRF_BASE + DDRGRF_CHB_CON(2),
			      0x28000000 | ddr_data.ddrgrf_chn_con2[1]);
	}

	if (pmu_power_domain_st(PD_DDR23) == 0) {
		mmio_write_32(DDR23GRF_BASE + DDRGRF_CHA_CON(1),
			      0x00400000 | ddr_data.ddrgrf_chn_con1[2]);
		mmio_write_32(DDR23GRF_BASE + DDRGRF_CHB_CON(1),
			      0x00400000 | ddr_data.ddrgrf_chn_con1[3]);
		mmio_write_32(DDR23GRF_BASE + DDRGRF_CHA_CON(0),
			      0x00200000 | ddr_data.ddrgrf_chn_con0[2]);
		mmio_write_32(DDR23GRF_BASE + DDRGRF_CHB_CON(0),
			      0x00200000 | ddr_data.ddrgrf_chn_con0[3]);
		mmio_write_32(DDR23GRF_BASE + DDRGRF_CHA_CON(2),
			      0x28000000 | ddr_data.ddrgrf_chn_con2[2]);
		mmio_write_32(DDR23GRF_BASE + DDRGRF_CHB_CON(2),
			      0x28000000 | ddr_data.ddrgrf_chn_con2[3]);
	}
}

static void pmu_sleep_config(void)
{
	uint32_t pmu1_pwr_con, pmu1_wkup_int_con, pmu1_cru_pwr_con;
	uint32_t pmu1_ddr_pwr_con, pmu1_pll_pd_con[2] = {0};
	uint32_t pmu2_dsu_pwr_con, pmu2_core_pwr_con, pmu2_clst_idle_con;
	uint32_t pmu2_bus_idle_con[3] = {0}, pmu2_pwr_gate_con[3] = {0};
	uint32_t pmu2_vol_gate_con[3] = {0}, pmu2_qch_pwr_con = 0;
	int i;

	ddr_data.pmu1grf_soc_con7 = mmio_read_32(PMU1GRF_BASE + PMU1_GRF_SOC_CON(7));
	ddr_data.pmu1grf_soc_con8 = mmio_read_32(PMU1GRF_BASE + PMU1_GRF_SOC_CON(8));
	ddr_data.pmu1grf_soc_con9 = mmio_read_32(PMU1GRF_BASE + PMU1_GRF_SOC_CON(9));
	ddr_data.pmu1sgrf_soc_con14 = mmio_read_32(PMU1SGRF_BASE + PMU1_SGRF_SOC_CON(14));
	ddr_data.pmu0sgrf_soc_con1 = mmio_read_32(PMU0SGRF_BASE + PMU0_SGRF_SOC_CON(1));
	ddr_data.pmu0grf_soc_con1 = mmio_read_32(PMU0GRF_BASE + PMU0_GRF_SOC_CON(1));

	ddr_data.pmu2_vol_gate_con[0] = mmio_read_32(PMU_BASE + PMU2_VOL_GATE_CON(0));
	ddr_data.pmu2_vol_gate_con[1] = mmio_read_32(PMU_BASE + PMU2_VOL_GATE_CON(1));
	ddr_data.pmu2_vol_gate_con[2] = mmio_read_32(PMU_BASE + PMU2_VOL_GATE_CON(2));

	ddr_data.pmu2_submem_gate_sft_con0 =
		mmio_read_32(PMU_BASE + PMU2_MEMPWR_MD_GATE_SFTCON(0));

	/* save pmic_sleep iomux gpio0_a4 */
	ddr_data.gpio0a_iomux_l = mmio_read_32(PMU0IOC_BASE + 0);
	ddr_data.gpio0a_iomux_h = mmio_read_32(PMU0IOC_BASE + 4);
	ddr_data.pmu0grf_soc_con3 = mmio_read_32(PMU0GRF_BASE + PMU0_GRF_SOC_CON(3));

	/* PMU1 repair disable */
	mmio_write_32(PMU0GRF_BASE + PMU0_GRF_SOC_CON(0), 0x00010000);

	/* set pmic_sleep iomux */
	mmio_write_32(PMU0IOC_BASE + 0,
		      BITS_WITH_WMASK(1, 0xf, 8) |
		      BITS_WITH_WMASK(1, 0xfu, 12));

	/* set tsadc_shut_m0 pin iomux to gpio */
	mmio_write_32(PMU0IOC_BASE + 0,
		      BITS_WITH_WMASK(0, 0xf, 4));

	/* set spi2_cs0/1 pin iomux to gpio */
	mmio_write_32(PMU0IOC_BASE + 8,
		      BITS_WITH_WMASK(0, 0xff, 0));

	/* sleep 1~2 src select */
	mmio_write_32(PMU0GRF_BASE + PMU0_GRF_SOC_CON(3),
		      BITS_WITH_WMASK(0x8, 0xf, 0) |
		      BITS_WITH_WMASK(0x8, 0xf, 4) |
		      BITS_WITH_WMASK(0x0, 0x3, 8));

	pmu1_wkup_int_con = BIT(WAKEUP_GPIO0_INT_EN) |
			    BIT(WAKEUP_CPU0_INT_EN);

	pmu1_pwr_con = BIT(powermode_en);

	pmu1_cru_pwr_con =
		BIT(alive_osc_mode_en) |
		BIT(power_off_en) |
		BIT(pd_clk_src_gate_en);

	pmu1_ddr_pwr_con = 0;

	pmu2_dsu_pwr_con =
		BIT(DSU_PWRDN_EN) |
		BIT(DSU_PWROFF_EN);

	pmu2_core_pwr_con = BIT(CORE_PWRDN_EN);

	pmu2_clst_idle_con =
		BIT(IDLE_REQ_BIGCORE0_EN) |
		BIT(IDLE_REQ_BIGCORE1_EN) |
		BIT(IDLE_REQ_DSU_EN) |
		BIT(IDLE_REQ_LITDSU_EN) |
		BIT(IDLE_REQ_ADB400_CORE_QCH_EN);

	pmu1_pll_pd_con[0] =
		BIT(B0PLL_PD_EN) |
		BIT(B1PLL_PD_EN) |
		BIT(LPLL_PD_EN) |
		BIT(V0PLL_PD_EN) |
		BIT(AUPLL_PD_EN) |
		BIT(GPLL_PD_EN) |
		BIT(CPLL_PD_EN) |
		BIT(NPLL_PD_EN);

	pmu1_pll_pd_con[1] =
		BIT(PPLL_PD_EN) |
		BIT(SPLL_PD_EN);

	pmu2_bus_idle_con[0] = 0;

	pmu2_bus_idle_con[1] =
		BIT(BUS_ID_SECURE - 16) |
		BIT(BUS_ID_SECURE_CENTER_CHANNEL - 16) |
		BIT(BUS_ID_CENTER_CHANNEL - 16);

	pmu2_bus_idle_con[2] =
		BIT(BUS_ID_MSCH - 32) |
		BIT(BUS_ID_BUS - 32) |
		BIT(BUS_ID_TOP - 32);

	pmu2_pwr_gate_con[0] = 0;
	pmu2_pwr_gate_con[1] = BIT(PD_SECURE - 16);
	pmu2_pwr_gate_con[2] = 0;

	pmu2_qch_pwr_con = 0;

	pmu2_vol_gate_con[0] = 0x7;
	pmu2_vol_gate_con[2] = 0;

	mmio_write_32(PMU_BASE + PMU2_CORE_AUTO_PWR_CON(0), 0x00030000);
	mmio_write_32(PMU_BASE + PMU2_CORE_AUTO_PWR_CON(1), 0x00030000);
	mmio_write_32(PMU_BASE + PMU2_CORE_PWR_CON(0),
		      WITH_16BITS_WMSK(pmu2_core_pwr_con));
	mmio_write_32(PMU_BASE + PMU2_CORE_PWR_CON(1),
		      WITH_16BITS_WMSK(pmu2_core_pwr_con));
	mmio_write_32(PMU_BASE + PMU2_CLUSTER_IDLE_CON,
		      WITH_16BITS_WMSK(pmu2_clst_idle_con));
	mmio_write_32(PMU_BASE + PMU2_DSU_AUTO_PWR_CON, 0x00030000);
	mmio_write_32(PMU_BASE + PMU2_DSU_PWR_CON,
		      WITH_16BITS_WMSK(pmu2_dsu_pwr_con));

	mmio_write_32(PMU_BASE + PMU1_OSC_STABLE_CNT_THRESH, 24000);
	mmio_write_32(PMU_BASE + PMU1_STABLE_CNT_THRESH, 24000);
	mmio_write_32(PMU_BASE + PMU1_WAKEUP_RST_CLR_CNT_THRESH, 24000);
	mmio_write_32(PMU_BASE + PMU1_PLL_LOCK_CNT_THRESH, 24000);
	mmio_write_32(PMU_BASE + PMU1_PWM_SWITCH_CNT_THRESH, 24000);
	mmio_write_32(PMU_BASE + PMU2_CORE0_STABLE_CNT_THRESH, 24000);
	mmio_write_32(PMU_BASE + PMU2_CORE0_PWRUP_CNT_THRESH, 24000);
	mmio_write_32(PMU_BASE + PMU2_CORE0_PWRDN_CNT_THRESH, 24000);
	mmio_write_32(PMU_BASE + PMU2_CORE1_STABLE_CNT_THRESH, 24000);
	mmio_write_32(PMU_BASE + PMU2_CORE1_PWRUP_CNT_THRESH, 24000);
	mmio_write_32(PMU_BASE + PMU2_CORE1_PWRDN_CNT_THRESH, 24000);
	mmio_write_32(PMU_BASE + PMU2_DSU_STABLE_CNT_THRESH, 24000);
	mmio_write_32(PMU_BASE + PMU2_DSU_PWRUP_CNT_THRESH, 24000);
	mmio_write_32(PMU_BASE + PMU2_DSU_PWRDN_CNT_THRESH, 24000);

	/* Config pmu power mode and pmu wakeup source */
	mmio_write_32(PMU_BASE + PMU1_INT_MASK_CON,
		      BITS_WITH_WMASK(1, 0x1, 0));

	/* pmu1_pwr_con */
	mmio_write_32(PMU_BASE + PMU1_PWR_CON,
		      WITH_16BITS_WMSK(pmu1_pwr_con));

	/* cru_pwr_con */
	mmio_write_32(PMU_BASE + PMU1_CRU_PWR_CON,
		      WITH_16BITS_WMSK(pmu1_cru_pwr_con));

	/* wakeup source */
	mmio_write_32(PMU_BASE + PMU1_WAKEUP_INT_CON, pmu1_wkup_int_con);

	/* ddr pwr con */
	for (i = 0; i < DDR_CHN_CNT; i++) {
		mmio_write_32(PMU_BASE + PMU1_DDR_PWR_CON(i),
			      WITH_16BITS_WMSK(pmu1_ddr_pwr_con));
		pmu2_bus_idle_con[1] |=
			BIT(BUS_ID_MSCH0 - 16 + i);
	}

	/* pll_pd */
	mmio_write_32(PMU_BASE + PMU1_PLLPD_CON(0),
		      WITH_16BITS_WMSK(pmu1_pll_pd_con[0]));
	mmio_write_32(PMU_BASE + PMU1_PLLPD_CON(1),
		      WITH_16BITS_WMSK(pmu1_pll_pd_con[1]));

	/* bypass cpu1~7*/
	mmio_write_32(PMU_BASE + PMU2_PWR_CON1, 0x00ff00fe);

	/* bus idle */
	mmio_write_32(PMU_BASE + PMU2_BUS_IDLE_CON(0),
		      WITH_16BITS_WMSK(pmu2_bus_idle_con[0]));
	mmio_write_32(PMU_BASE + PMU2_BUS_IDLE_CON(1),
		      WITH_16BITS_WMSK(pmu2_bus_idle_con[1]));
	mmio_write_32(PMU_BASE + PMU2_BUS_IDLE_CON(2),
		      WITH_16BITS_WMSK(pmu2_bus_idle_con[2]));
	mmio_write_32(PMU_BASE + PMU2_BUS_IDLE_CON(2),
		      0xf000f000);
	/* power gate */
	mmio_write_32(PMU_BASE + PMU2_PWR_GATE_CON(0),
		      WITH_16BITS_WMSK(pmu2_pwr_gate_con[0]));
	mmio_write_32(PMU_BASE + PMU2_PWR_GATE_CON(1),
		      WITH_16BITS_WMSK(pmu2_pwr_gate_con[1]));
	mmio_write_32(PMU_BASE + PMU2_PWR_GATE_CON(2),
		      WITH_16BITS_WMSK(pmu2_pwr_gate_con[2]));
	/* vol gate */
	mmio_write_32(PMU_BASE + PMU2_VOL_GATE_CON(0),
		      BITS_WITH_WMASK(pmu2_vol_gate_con[0], 0x7, 0));
	mmio_write_32(PMU_BASE + PMU2_VOL_GATE_CON(1), 0);
	mmio_write_32(PMU_BASE + PMU2_VOL_GATE_CON(2),
		      BITS_WITH_WMASK(pmu2_vol_gate_con[2], 0x3, 0));
	/* qch */
	mmio_write_32(PMU_BASE + PMU2_QCHANNEL_PWR_CON,
		      BITS_WITH_WMASK(pmu2_qch_pwr_con, 0x7f, 0));

	mmio_write_32(PMU_BASE + PMU2_MEMPWR_MD_GATE_SFTCON(0),
		      0x000f000f);
}

static void pmu_sleep_restore(void)
{
	mmio_write_32(PMU1GRF_BASE + PMU1_GRF_SOC_CON(7),
		      WITH_16BITS_WMSK(ddr_data.pmu1grf_soc_con7));
	mmio_write_32(PMU1GRF_BASE + PMU1_GRF_SOC_CON(8),
		      WITH_16BITS_WMSK(ddr_data.pmu1grf_soc_con8));
	mmio_write_32(PMU1GRF_BASE + PMU1_GRF_SOC_CON(9),
		      WITH_16BITS_WMSK(ddr_data.pmu1grf_soc_con9));
	mmio_write_32(PMU1SGRF_BASE + PMU1_SGRF_SOC_CON(14),
		      WITH_16BITS_WMSK(ddr_data.pmu1sgrf_soc_con14));

	mmio_write_32(PMU0SGRF_BASE + PMU0_SGRF_SOC_CON(1),
		      WITH_16BITS_WMSK(ddr_data.pmu0sgrf_soc_con1));
	mmio_write_32(PMU0GRF_BASE + PMU0_GRF_SOC_CON(1),
		      WITH_16BITS_WMSK(ddr_data.pmu0grf_soc_con1));

	mmio_write_32(PMU_BASE + PMU2_CORE_PWR_CON(0), 0xffff0000);
	mmio_write_32(PMU_BASE + PMU2_CORE_PWR_CON(1), 0xffff0000);
	mmio_write_32(PMU_BASE + PMU2_CLUSTER_IDLE_CON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU2_DSU_PWR_CON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU2_PWR_CON1, 0xffff0000);

	/* Must clear PMU1_WAKEUP_INT_CON because the wakeup source
	 * in PMU1_WAKEUP_INT_CON will wakeup cpus in cpu_auto_pd state.
	 */
	mmio_write_32(PMU_BASE + PMU1_WAKEUP_INT_CON, 0);
	mmio_write_32(PMU_BASE + PMU1_PWR_CON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU1_INT_MASK_CON, 0x00010000);
	mmio_write_32(PMU_BASE + PMU0_WAKEUP_INT_CON, 0x00010000);
	mmio_write_32(PMU_BASE + PMU0_PWR_CON, 0xffff0000);

	mmio_write_32(PMU_BASE + PMU2_VOL_GATE_CON(0),
		      WITH_16BITS_WMSK(ddr_data.pmu2_vol_gate_con[0]));
	mmio_write_32(PMU_BASE + PMU2_VOL_GATE_CON(1),
		      WITH_16BITS_WMSK(ddr_data.pmu2_vol_gate_con[1]));
	mmio_write_32(PMU_BASE + PMU2_VOL_GATE_CON(2),
		      WITH_16BITS_WMSK(ddr_data.pmu2_vol_gate_con[2]));

	mmio_write_32(PMU_BASE + PMU2_MEMPWR_MD_GATE_SFTCON(0),
		      WITH_16BITS_WMSK(ddr_data.pmu2_submem_gate_sft_con0));

	mmio_write_32(PMU0GRF_BASE + PMU0_GRF_SOC_CON(3),
		      WITH_16BITS_WMSK(ddr_data.pmu0grf_soc_con3));
	mmio_write_32(PMU1GRF_BASE + PMU1_GRF_SOC_CON(2),
		      WITH_16BITS_WMSK(ddr_data.pmu1grf_soc_con2));

	mmio_write_32(PMU0IOC_BASE + 0x4,
		      WITH_16BITS_WMSK(ddr_data.gpio0a_iomux_h));
	mmio_write_32(PMU0IOC_BASE + 0,
		      WITH_16BITS_WMSK(ddr_data.gpio0a_iomux_l));
}

static void soc_sleep_config(void)
{
	ddr_data.gpio0b_iomux_l = mmio_read_32(PMU0IOC_BASE + 0x8);

	pmu_sleep_config();
	ddr_sleep_config();
}

static void soc_sleep_restore(void)
{
	ddr_sleep_config_restore();
	pmu_sleep_restore();

	mmio_write_32(PMU0IOC_BASE + 0x8, WITH_16BITS_WMSK(ddr_data.gpio0b_iomux_l));
}

static void pm_pll_suspend(void)
{
	ddr_data.cru_mode_con = mmio_read_32(CRU_BASE + 0x280);
	ddr_data.busscru_mode_con = mmio_read_32(BUSSCRU_BASE + 0x280);
	ddr_data.pmu2_bisr_con0 = mmio_read_32(PMU_BASE + PMU2_BISR_CON(0));
	ddr_data.cpll_con0 = mmio_read_32(CRU_BASE + CRU_PLLS_CON(2, 0));
	ddr_data.pmu1cru_clksel_con1 = mmio_read_32(PMU1CRU_BASE + CRU_CLKSEL_CON(1));

	/* disable bisr_init */
	mmio_write_32(PMU_BASE + PMU2_BISR_CON(0), BITS_WITH_WMASK(0, 0x1, 0));
	/* cpll bypass */
	mmio_write_32(CRU_BASE + CRU_PLLS_CON(2, 0), BITS_WITH_WMASK(1u, 1u, 15));
}

static void pm_pll_restore(void)
{
	pm_pll_wait_lock(CRU_BASE + CRU_PLLS_CON(2, 0));

	mmio_write_32(CRU_BASE + 0x280, WITH_16BITS_WMSK(ddr_data.cru_mode_con));
	mmio_write_32(BUSSCRU_BASE + 0x280, WITH_16BITS_WMSK(ddr_data.busscru_mode_con));
	mmio_write_32(CRU_BASE + CRU_PLLS_CON(2, 0), WITH_16BITS_WMSK(ddr_data.cpll_con0));
	dsb();
	isb();
	mmio_write_32(PMU_BASE + PMU2_BISR_CON(0), WITH_16BITS_WMSK(ddr_data.pmu2_bisr_con0));
}

int rockchip_soc_sys_pwr_dm_suspend(void)
{
	clk_gate_con_save();
	clk_gate_con_disable();

	psram_sleep_cfg->pm_flag &= ~PM_WARM_BOOT_BIT;

	pmu_power_domains_suspend();
	soc_sleep_config();
	dsu_core_save();
	pm_pll_suspend();

	return 0;
}

int rockchip_soc_sys_pwr_dm_resume(void)
{
	pm_pll_restore();
	dsu_core_restore();
	soc_sleep_restore();
	pmu_power_domains_resume();
	plat_rockchip_gic_cpuif_enable();

	psram_sleep_cfg->pm_flag |= PM_WARM_BOOT_BIT;

	clk_gate_con_restore();

	return 0;
}

void __dead2 rockchip_soc_cores_pd_pwr_dn_wfi(const
					psci_power_state_t *target_state)
{
	psci_power_down_wfi();
}

void __dead2 rockchip_soc_sys_pd_pwr_dn_wfi(void)
{
	cpus_pd_req_enter_wfi();
	psci_power_down_wfi();
}

void __dead2 rockchip_soc_soft_reset(void)
{
	/* pll slow mode */
	mmio_write_32(CRU_BASE + 0x280, 0x03ff0000);
	mmio_write_32(BIGCORE0CRU_BASE + 0x280, 0x00030000);
	mmio_write_32(BIGCORE0CRU_BASE + 0x300, 0x60000000);
	mmio_write_32(BIGCORE0CRU_BASE + 0x304, 0x00600000);
	mmio_write_32(BIGCORE1CRU_BASE + 0x280, 0x00030000);
	mmio_write_32(BIGCORE1CRU_BASE + 0x300, 0x60000000);
	mmio_write_32(BIGCORE1CRU_BASE + 0x304, 0x00600000);
	mmio_write_32(DSUCRU_BASE + 0x280, 0x00030000);
	mmio_write_32(DSUCRU_BASE + 0x318, 0x30600000);
	mmio_write_32(DSUCRU_BASE + 0x31c, 0x30600000);
	mmio_write_32(DSUCRU_BASE + 0x304, 0x00010000);
	mmio_write_32(BUSSCRU_BASE + 0x280, 0x0003000);
	dsb();
	isb();

	mmio_write_32(CRU_BASE + CRU_GLB_SRST_FST, GLB_SRST_FST_CFG_VAL);

	/*
	 * Maybe the HW needs some times to reset the system,
	 * so we do not hope the core to execute valid codes.
	 */
	psci_power_down_wfi();
}

void __dead2 rockchip_soc_system_off(void)
{
	/* set pmic_sleep pin(gpio0_a2) to gpio mode */
	mmio_write_32(PMU0IOC_BASE + 0, BITS_WITH_WMASK(0, 0xf, 8));

	/* config output */
	mmio_write_32(GPIO0_BASE + GPIO_SWPORT_DDR_L,
		      BITS_WITH_WMASK(1, 0x1, 2));

	/* config output high level */
	mmio_write_32(GPIO0_BASE + GPIO_SWPORT_DR_L,
		      BITS_WITH_WMASK(1, 0x1, 2));
	dsb();

	/*
	 * Maybe the HW needs some times to reset the system,
	 * so we do not hope the core to execute valid codes.
	 */
	psci_power_down_wfi();
}

static void rockchip_pmu_pd_init(void)
{
	mmio_write_32(PMU_BASE + PMU2_BISR_CON(1), 0xffffffff);
	mmio_write_32(PMU_BASE + PMU2_BISR_CON(2), 0xffffffff);
	mmio_write_32(PMU_BASE + PMU2_BISR_CON(3), 0xffffffff);

	pmu_set_power_domain(PD_PHP, pmu_pd_on);
	pmu_set_power_domain(PD_PCIE, pmu_pd_on);
	pmu_set_power_domain(PD_GMAC, pmu_pd_on);
	pmu_set_power_domain(PD_SECURE, pmu_pd_on);
	pmu_set_power_domain(PD_VOP, pmu_pd_on);
	pmu_set_power_domain(PD_VO0, pmu_pd_on);
	pmu_set_power_domain(PD_VO1, pmu_pd_on);
}

#define PLL_LOCKED_TIMEOUT 600000U

void pm_pll_wait_lock(uint32_t pll_base)
{
	int delay = PLL_LOCKED_TIMEOUT;

	if ((mmio_read_32(pll_base + CRU_PLL_CON(1)) & CRU_PLLCON1_PWRDOWN) != 0)
		return;

	while (delay-- >= 0) {
		if (mmio_read_32(pll_base + CRU_PLL_CON(6)) &
		    CRU_PLLCON6_LOCK_STATUS)
			break;
		udelay(1);
	}

	if (delay <= 0)
		ERROR("Can't wait pll(0x%x) lock\n", pll_base);
}

void rockchip_plat_mmu_el3(void)
{
	/* Nothing todo */
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
	mmio_write_32(PMU_BASE + PMU2_BIU_AUTO_CON(0), 0xffffffff);
	mmio_write_32(PMU_BASE + PMU2_BIU_AUTO_CON(1), 0xffffffff);
	mmio_write_32(PMU_BASE + PMU2_BIU_AUTO_CON(2), 0x00070007);

	rockchip_pmu_pd_init();

	/* grf_con_pmic_sleep_sel
	 * pmic sleep function selection
	 * 1'b0: From reset pulse generator, can reset external PMIC
	 * 1'b1: From pmu block, only support sleep function for external PMIC
	 */
	mmio_write_32(PMU0GRF_BASE + PMU0_GRF_SOC_CON(3), 0x03ff0000);

	/* pmusram remap to 0xffff0000 */
	mmio_write_32(PMU0SGRF_BASE + PMU0_SGRF_SOC_CON(2), 0x00030001);

	pm_reg_rgns_init();
}

static uint64_t boot_cpu_save[4];
/* define in .data section */
static uint32_t need_en_crypto = 1;

void rockchip_cpu_reset_early(u_register_t arg0, u_register_t arg1,
			      u_register_t arg2, u_register_t arg3)
{
	if (need_en_crypto == 0)
		return;

	/* check the crypto function had been enabled or not */
	if ((mmio_read_32(DSUSGRF_BASE + DSU_SGRF_SOC_CON(4)) & BIT(4)) != 0) {
		/* save x0~x3 */
		boot_cpu_save[0] = arg0;
		boot_cpu_save[1] = arg1;
		boot_cpu_save[2] = arg2;
		boot_cpu_save[3] = arg3;

		/* enable the crypto function */
		mmio_write_32(DSUSGRF_BASE + DSU_SGRF_SOC_CON(4),
			      BITS_WITH_WMASK(0, 0x1, 4));

		/* remap pmusram to 0xffff0000 */
		mmio_write_32(PMU0SGRF_BASE + PMU0_SGRF_SOC_CON(2), 0x00030001);
		psram_sleep_cfg->pm_flag = PM_WARM_BOOT_BIT;
		cpuson_flags[0] = PMU_CPU_HOTPLUG;
		cpuson_entry_point[0] = (uintptr_t)BL31_BASE;
		dsb();

		/* Must reset core0 to enable the crypto function.
		 * Core0 will boot from pmu_sram and jump to BL31_BASE.
		 */
		__asm__ volatile ("mov	x0, #3\n"
				  "dsb	sy\n"
				  "msr	rmr_el3, x0\n"
				  "1:\n"
				  "isb\n"
				  "wfi\n"
				  "b	1b\n");
	} else {
		need_en_crypto = 0;

		/* remap bootrom to 0xffff0000 */
		mmio_write_32(PMU0SGRF_BASE + PMU0_SGRF_SOC_CON(2), 0x00030000);

		/*
		 * the crypto function has been enabled,
		 * restore the x0~x3.
		 */
		__asm__ volatile ("ldr	x20, [%0]\n"
				  "ldr	x21, [%0, 0x8]\n"
				  "ldr	x22, [%0, 0x10]\n"
				  "ldr	x23, [%0, 0x18]\n"
				  : : "r" (&boot_cpu_save[0]));
	}
}
