/*
 * Copyright (c) 2024-2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <inttypes.h>

#include <include/drivers/spmi_api.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include "apusys_power.h"

static void apu_w_are(int entry, uint32_t reg, uint32_t data)
{
	uint32_t are_entry_addr;

	are_entry_addr = APUSYS_BASE + APU_ARE + ARE_REG_SIZE * ARE_ENTRY(entry);
	mmio_write_32(are_entry_addr, reg);
	mmio_write_32((are_entry_addr + ARE_REG_SIZE), data);
}

static void get_pll_pcw(uint32_t clk_rate, uint32_t *r1, uint32_t *r2)
{
	unsigned int fvco = clk_rate;
	unsigned int pcw_val;
	unsigned int postdiv_val = 1;
	unsigned int postdiv_reg = 0;

	while (fvco <= OUT_CLK_FREQ_MIN) {
		postdiv_val = postdiv_val << 1;
		postdiv_reg = postdiv_reg + 1;
		fvco = fvco << 1;
	}

	pcw_val = (fvco * (1 << DDS_SHIFT)) / BASIC_CLK_FREQ;

	if (postdiv_reg == 0) {
		pcw_val = pcw_val * 2;
		postdiv_val = postdiv_val << 1;
		postdiv_reg = postdiv_reg + 1;
	}

	*r1 = postdiv_reg;
	*r2 = pcw_val;
}

static void buck_off_by_pcu(uint32_t ofs, uint32_t shift, uint32_t slv_id)
{
	uint32_t pmif_id = 0x0;
	int retry = 10;

	mmio_setbits_32(APUSYS_PCU + APU_PCUTOP_CTRL_SET, PMIC_IRQ_EN);
	mmio_write_32(APUSYS_PCU + APU_PCU_PMIC_TAR_BUF1,
		      (ofs << PMIC_OFF_ADDR_OFF) | BIT(shift));
	mmio_write_32(APUSYS_PCU + APU_PCU_PMIC_TAR_BUF2,
		      (slv_id << PMIC_SLVID_OFF) | (pmif_id << PMIC_PMIFID_OFF) | PCU_BUCK_OFF_CMD);
	mmio_write_32(APUSYS_PCU + APU_PCU_PMIC_CMD, PMIC_CMD_EN);

	while ((mmio_read_32(APUSYS_PCU + APU_PCU_PMIC_IRQ) & PMIC_CMD_IRQ) == 0) {
		udelay(10);
		if (--retry < 0)
			ERROR("%s wait APU_PCU_PMIC_IRQ timeout !\n", __func__);
	}

	mmio_write_32(APUSYS_PCU + APU_PCU_PMIC_IRQ, PMIC_CMD_IRQ);
}

static void apu_buck_off_cfg(void)
{
	mmio_setbits_32(APUSYS_AO_CTL + APUSYS_AO_SRAM_SET, BIT(10));
	mmio_setbits_32(APUSYS_AO_CTL + APUSYS_AO_SRAM_CLR, BIT(9));
	mmio_setbits_32(APUSYS_AO_CTL + APUSYS_AO_SRAM_CLR, BIT(12));
	mmio_setbits_32(APUSYS_AO_CTL + APUSYS_AO_SRAM_CLR, BIT(14));

	mmio_clrbits_32(APUSYS_AO_CTL + APUSYS_AO_SRAM_SET, BIT(10));
	mmio_clrbits_32(APUSYS_AO_CTL + APUSYS_AO_SRAM_CLR, BIT(9));
	mmio_clrbits_32(APUSYS_AO_CTL + APUSYS_AO_SRAM_CLR, BIT(12));
	mmio_clrbits_32(APUSYS_AO_CTL + APUSYS_AO_SRAM_CLR, BIT(14));
	udelay(1);

	mmio_write_32(APUSYS_RPC + APU_RPC_HW_CON, BUCK_PROT_REQ_SET);
	udelay(1);

	mmio_write_32(APUSYS_RPC + APU_RPC_HW_CON, SRAM_AOC_LHENB_SET);
	udelay(1);

	mmio_write_32(APUSYS_RPC + APU_RPC_HW_CON, SRAM_AOC_ISO_SET);
	udelay(1);

	mmio_write_32(APUSYS_RPC + APU_RPC_HW_CON, PLL_AOC_ISO_EN_SET);
	udelay(1);

	mmio_write_32(APUSYS_RPC + APU_RPC_HW_CON, BUCK_ELS_EN_SET);
	udelay(1);

	mmio_write_32(APUSYS_RPC + APU_RPC_HW_CON, BUCK_AO_RST_B_CLR);
	udelay(1);

	buck_off_by_pcu(BUCK_VAPU_PMIC_REG_EN_CLR_ADDR, BUCK_VAPU_PMIC_REG_EN_SHIFT,
			BUCK_VAPU_PMIC_ID);

	mmio_setbits_32(APUSYS_AO_CTL + APUSYS_AO_SRAM_SET, BIT(6));
	udelay(1);
	mmio_setbits_32(APUSYS_AO_CTL + APUSYS_AO_SRAM_SET, BIT(7));
	udelay(1);
	mmio_clrbits_32(APUSYS_AO_CTL + APUSYS_AO_SRAM_SET, BIT(6));
	udelay(1);
	mmio_clrbits_32(APUSYS_AO_CTL + APUSYS_AO_SRAM_SET, BIT(7));
	udelay(1);
}

static void apu_acc_init(void)
{
	uint32_t top_acc_base_arr[] = {MNOC_ACC_BASE, UP_ACC_BASE};
	uint32_t eng_acc_base_arr[] = {MVPU_ACC_BASE, MDLA_ACC_BASE};
	int acc_idx;
	int are_idx = ACC_ENTRY_BEGIN;
	uint32_t base_reg;

	for (acc_idx = 0 ; acc_idx < ARRAY_SIZE(top_acc_base_arr) ; acc_idx++) {
		base_reg = APUSYS_ACC + top_acc_base_arr[acc_idx];
#if CFG_APU_ARDCM_ENABLE
		apu_w_are(are_idx++, base_reg + APU_ARDCM_CTRL1, APU_ARDCM_CTRL1_VAL_0);
		apu_w_are(are_idx++, base_reg + APU_ARDCM_CTRL0, APU_ARDCM_CTRL0_VAL_0);
		apu_w_are(are_idx++, base_reg + APU_ARDCM_CTRL1, APU_ARDCM_CTRL1_VAL_1);
		apu_w_are(are_idx++, base_reg + APU_ARDCM_CTRL0, APU_ARDCM_CTRL0_VAL_1);
#endif
		apu_w_are(are_idx++, base_reg + APU_ACC_CONFG_CLR0, CGEN_SOC);
		apu_w_are(are_idx++, base_reg + APU_ACC_CONFG_SET0, HW_CTRL_EN);
	}

	for (acc_idx = 0 ; acc_idx < ARRAY_SIZE(eng_acc_base_arr) ; acc_idx++) {
		base_reg = APUSYS_ACC + eng_acc_base_arr[acc_idx];
#if CFG_APU_ARDCM_ENABLE
		apu_w_are(are_idx++, base_reg + APU_ARDCM_CTRL1, APU_ARDCM_CTRL1_VAL_0);
		apu_w_are(are_idx++, base_reg + APU_ARDCM_CTRL0, APU_ARDCM_CTRL0_VAL_0);
		apu_w_are(are_idx++, base_reg + APU_ARDCM_CTRL1, APU_ARDCM_CTRL1_VAL_1);
		apu_w_are(are_idx++, base_reg + APU_ARDCM_CTRL0, APU_ARDCM_CTRL0_VAL_1);
#endif
		apu_w_are(are_idx++, base_reg + APU_ACC_CONFG_CLR0, CGEN_SOC);
		apu_w_are(are_idx++, base_reg + APU_ACC_CONFG_SET0, HW_CTRL_EN);
		apu_w_are(are_idx++, base_reg + APU_ACC_AUTO_CTRL_SET0, CLK_REQ_SW_EN);
	}
}

static void apu_pll_init(void)
{
	uint32_t pll_base_arr[] = {MNOC_PLL_BASE, UP_PLL_BASE, MVPU_PLL_BASE, MDLA_PLL_BASE};
	int32_t pll_freq_out[] = {
		APUPLL0_DEFAULT_FREQ,
		APUPLL1_DEFAULT_FREQ,
		APUPLL2_DEFAULT_FREQ,
		APUPLL3_DEFAULT_FREQ
	};
	uint32_t pcw_val, posdiv_val;
	int pll_idx, are_idx;
	uint32_t base_reg;

	mmio_setbits_32(APUSYS_BASE + APU_ARE, ARE_RCX_AO_EN);
	mmio_setbits_32(APUSYS_BASE + APU_ARE_REG, ARE_RCX_AO_EN);

	mmio_write_32(APUSYS_BASE + APU_ARE + ARE_RCX_AO_CONFIG, ARE_ENTRY(RCX_AO_BEGIN) |
		      (ARE_ENTRIES(RCX_AO_BEGIN, RCX_AO_END) << ARE_RCX_AO_CONFIG_HIGH_OFF));

	are_idx = PLL_ENTRY_BEGIN;
	for (pll_idx = 0 ; pll_idx < ARRAY_SIZE(pll_base_arr) ; pll_idx++) {
		base_reg = APUSYS_PLL + pll_base_arr[pll_idx];

		apu_w_are(are_idx++, base_reg + RG_PLLGP_LVR_REFSEL, RG_PLLGP_LVR_REFSEL_VAL);
		apu_w_are(are_idx++, base_reg + PLL1CPLL_FHCTL_HP_EN, FHCTL_CTRL);
		apu_w_are(are_idx++, base_reg + PLL1CPLL_FHCTL_RST_CON, FHCTL_NO_RESET);
		apu_w_are(are_idx++, base_reg + PLL1CPLL_FHCTL_CLK_CON, FHCTL_CLKEN);
		apu_w_are(are_idx++, base_reg + PLL1CPLL_FHCTL0_CFG,
			  FHCTL_HOPPING_EN | FHCTL_SFSTR0_EN);

		posdiv_val = 0;
		pcw_val = 0;
		get_pll_pcw(pll_freq_out[pll_idx], &posdiv_val, &pcw_val);

		apu_w_are(are_idx++, base_reg + PLL1C_PLL1_CON1,
			  ((0x1U << RG_PLL_SDM_PCW_CHG_OFF) |
			   (posdiv_val << RG_PLL_POSDIV_OFF) | pcw_val));

		apu_w_are(are_idx++, base_reg + PLL1CPLL_FHCTL0_DDS,
			  ((0x1U << FHCTL0_PLL_TGL_ORG) | pcw_val));
	}
}

static void apu_are_init(void)
{
	int entry = 0;

	mmio_clrbits_32(APUSYS_BASE + APU_ARE, 0xFFFU << ARE_VCORE_OFF);

	mmio_setbits_32(APUSYS_BASE + APU_ARE, ARE_VCORE_EN);
	mmio_setbits_32(APUSYS_BASE + APU_ARE_REG, ARE_VCORE_EN);

	for (entry = ARE_CONF_START; entry < ARE_CONF_END; entry += 4)
		mmio_write_32(APUSYS_BASE + APU_ARE + entry, 0);
}

static void apu_rpclite_init(void)
{
	uint32_t sleep_type_offset[] = {
		APU_RPC_SW_TYPE1_OFF,
		APU_RPC_SW_TYPE2_OFF,
		APU_RPC_SW_TYPE3_OFF,
		APU_RPC_SW_TYPE4_OFF
	};
	uint32_t rpc_lite_base[] = {
		APU_ACX0_RPC_LITE,
		APU_ACX1_RPC_LITE,
		APU_ACX2_RPC_LITE,
	};
	int ofs_idx, rpc_lite_idx;
	uint32_t base;

	for (rpc_lite_idx = 0; rpc_lite_idx < ARRAY_SIZE(rpc_lite_base); rpc_lite_idx++) {
		base = APUSYS_BASE + rpc_lite_base[rpc_lite_idx];
		for (ofs_idx = 0; ofs_idx < ARRAY_SIZE(sleep_type_offset); ofs_idx++)
			mmio_clrbits_32(base + sleep_type_offset[ofs_idx],
					SW_TYPE_MVPU_MDLA_RV);
		mmio_setbits_32(base + APU_RPC_TOP_SEL, TOP_SEL_VAL);
	}
}

static void apu_rpc_mdla_init(void)
{
	mmio_clrbits_32(APUSYS_BASE + APU_RPCTOP_MDLA + APU_RPC_SW_TYPE0_OFF, SW_TYPE_MVPU_MDLA_RV);
}

static void apu_rpc_init(void)
{
	mmio_write_32(APUSYS_RPC + APU_RPC_SW_TYPE0_OFF, RPC_TYPE_INIT_VAL);
	mmio_setbits_32(APUSYS_RPC + APU_RPC_TOP_SEL, RPC_TOP_SEL_VAL);

#if !CFG_CTL_RPC_BY_CE
	mmio_clrbits_32(APUSYS_RPC + APU_RPC_TOP_SEL, CE_ENABLE);
#endif

	mmio_setbits_32(APUSYS_RPC + APU_RPC_TOP_SEL_1, BUCK_PROT_SEL);
}

static int apu_pcu_init(void)
{
	uint32_t pmif_id = 0x0;
	uint32_t slave_id = BUCK_VAPU_PMIC_ID;
	uint32_t en_set_offset = BUCK_VAPU_PMIC_REG_EN_SET_ADDR;
	uint32_t en_clr_offset = BUCK_VAPU_PMIC_REG_EN_CLR_ADDR;
	uint32_t en_shift = BUCK_VAPU_PMIC_REG_EN_SHIFT;
	struct spmi_device *vsram_sdev;
	unsigned char vsram = 0;

	mmio_write_32(APUSYS_PCU + APU_PCUTOP_CTRL_SET, AUTO_BUCK_EN);

	mmio_write_32((APUSYS_PCU + APU_PCU_BUCK_STEP_SEL), BUCK_STEP_SEL_VAL);

	vsram_sdev = get_spmi_device(SPMI_MASTER_1, SPMI_SLAVE_4);
	if (!vsram_sdev) {
		ERROR("[APUPW] VSRAM BUCK4 get device fail\n");
		return -1;
	}

	if (spmi_ext_register_readl(vsram_sdev, MT6363_RG_BUCK_VBUCK4_VOSEL_ADDR, &vsram, 1)) {
		ERROR("[APUPW] VSRAM BUCK4 read fail\n");
		return -1;
	}

	mmio_write_32(APUSYS_PCU + APU_PCU_BUCK_ON_DAT0_L,
		      (BUCK_VAPU_PMIC_REG_VOSEL_ADDR << PMIC_OFF_ADDR_OFF) | vsram);

	mmio_write_32(APUSYS_PCU + APU_PCU_BUCK_ON_DAT0_H,
		      (slave_id << PMIC_SLVID_OFF) | (pmif_id << PMIC_PMIFID_OFF) | PCU_CMD_OP_W);

	mmio_write_32(APUSYS_PCU + APU_PCU_BUCK_ON_DAT1_L,
		      (en_set_offset << PMIC_OFF_ADDR_OFF) | (0x1U << en_shift));
	mmio_write_32(APUSYS_PCU + APU_PCU_BUCK_ON_DAT1_H,
		      (slave_id << PMIC_SLVID_OFF) | (pmif_id << PMIC_PMIFID_OFF) | PCU_CMD_OP_W);

	mmio_write_32(APUSYS_PCU + APU_PCU_BUCK_OFF_DAT0_L,
		      (en_clr_offset << PMIC_OFF_ADDR_OFF) | (0x1U << en_shift));
	mmio_write_32(APUSYS_PCU + APU_PCU_BUCK_OFF_DAT0_H,
		      (slave_id << PMIC_SLVID_OFF) | (pmif_id << PMIC_PMIFID_OFF) | PCU_CMD_OP_W);

	mmio_write_32(APUSYS_PCU + APU_PCU_BUCK_ON_SLE0, 0);
	mmio_write_32(APUSYS_PCU + APU_PCU_BUCK_ON_SLE1, VAPU_BUCK_ON_SETTLE_TIME);

	return 0;
}

static void apu_aoc_init(void)
{
	uint32_t reg;

	mmio_setbits_32(SPM_BASE + 0xF6C, BIT(4));
	mmio_clrbits_32(SPM_BASE + 0x414, BIT(1));

	mmio_write_32(APUSYS_AO_CTL + APUSYS_AO_SRAM_CONFIG, APUSYS_AO_SRAM_EN);
	udelay(1);

	reg = APUSYS_AO_CTL + APUSYS_AO_SRAM_SET;

#if !CFG_CTL_RPC_BY_CE
	mmio_setbits_32(reg, BIT(8));
	udelay(1);
	mmio_setbits_32(reg, BIT(11));
	udelay(1);
	mmio_setbits_32(reg, BIT(13));
	udelay(1);

	mmio_clrbits_32(reg, BIT(8));
	udelay(1);
	mmio_clrbits_32(reg, BIT(11));
	udelay(1);
	mmio_clrbits_32(reg, BIT(13));
#else
	mmio_setbits_32(reg, BIT(9));
	mmio_setbits_32(reg, BIT(12));
	mmio_setbits_32(reg, BIT(14));

	mmio_clrbits_32(reg, BIT(9));
	mmio_clrbits_32(reg, BIT(12));
	mmio_clrbits_32(reg, BIT(14));
	udelay(1);
#endif

	reg = APUSYS_RPC + APU_RPC_HW_CON;

	mmio_write_32(reg, BUCK_ELS_EN_CLR);
	udelay(1);

	mmio_write_32(reg, BUCK_AO_RST_B_SET);
	udelay(1);

	mmio_write_32(reg, BUCK_PROT_REQ_CLR);
	udelay(1);

	mmio_write_32(reg, SRAM_AOC_ISO_CLR);
	udelay(1);

	mmio_write_32(reg, PLL_AOC_ISO_EN_CLR);
	udelay(1);
}

static int init_hw_setting(void)
{
	int ret;

	apu_aoc_init();
	ret = apu_pcu_init();
	apu_rpc_init();
	apu_rpc_mdla_init();
	apu_rpclite_init();
	apu_are_init();
	apu_pll_init();
	apu_acc_init();
	apu_buck_off_cfg();

	return ret;
}

int apusys_power_init(void)
{
	int ret;

	ret = init_hw_setting();
	if (ret != 0)
		ERROR("%s init HW failed\n", __func__);
	else
		INFO("%s init HW done\n", __func__);

	mmio_write_32(APU_ACE_HW_FLAG_DIS, APU_ACE_DIS_FLAG_VAL);

	return ret;
}
