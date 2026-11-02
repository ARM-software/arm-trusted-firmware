/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdbool.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <ddrc.h>
#include <dram.h>
#include <platform_def.h>
#include <pwr_ctrl.h>

#define SRC_DDRC_SW_CTRL		U(0x44461020)
#define SRC_DDRPHY_SW_CTRL		U(0x44461420)
#define SRC_DDRPHY_SINGLE_RESET_SW_CTRL	U(0x44461424)

extern void ddr_disable_bypass(void);

static void dram_pll_init(void)
{
	unsigned int timeout = 10000U; /* ~10ms */

	/* Powerup and enable */
	mmio_write_32(DRAM_PLL_BASE + PLL_CTRL_SET, 0x1);

	/* Wait lock */
	do {
		if (mmio_read_32(DRAM_PLL_BASE + PLL_STATUS) & 0x1) {
			break;
		}
		udelay(1);
	} while (--timeout > 0U);
	if (timeout == 0U) {
		ERROR("DRAM PLL lock timeout in retention exit\n");
	}

	mmio_write_32(DRAM_PLL_BASE + PLL_CTRL_CLR, 0x4);
	mmio_write_32(DRAM_PLL_BASE + PLL_CTRL_SET, 0x2);
}


static void self_refresh_enter(void)
{
	mmio_setbits_32(REG_DDR_SDRAM_CFG_2, BIT(31));
}

static void ipg_stop_ack_wait(uint32_t expect_value)
{
	uint32_t read_data;
	unsigned int timeout = 10000U; /* ~10ms */

	do {
		read_data = mmio_read_32(REG_DDRC_STOP_CTRL);
		if (expect_value == 0x1) {
			if ((read_data & BIT(1)) != 0x0) {
				break;
			}
		} else if (expect_value == 0x0) {
			if ((read_data & BIT(1)) == 0x0) {
				break;
			}
		}
		udelay(1);
	} while (--timeout > 0U);
	if (timeout == 0U) {
		ERROR("ipg stop ack timeout expecting %u\n", expect_value);
	}
}

static void check_dfi_init_complete(void)
{
	uint32_t regval;
	unsigned int timeout = 10000U; /* ~10ms */

	do {
		regval = mmio_read_32(REG_DDRDSR_2);
		if (regval & 0x04) {
			break;
		}
		udelay(1);
	} while (--timeout > 0U);
	if (timeout == 0U) {
		ERROR("DFI init complete timeout\n");
	}

	mmio_setbits_32(REG_DDRDSR_2, BIT(2));
}

void dram_enter_retention(void)
{
	uint32_t val;
	bool eccen = false;
	uint32_t waitflag = 0U;

	eccen = (mmio_read_32(REG_ERR_EN) & ECC_EN) != 0U;

	if (eccen && ((mmio_read_32(REG_DDR_TX_CFG_1) & 0xF) != 0U)) {
		mmio_clrbits_32(REG_DDR_TX_CFG_1, 0xF);
	}

	if (eccen) {
		waitflag = ECC_EN | DDRC_IDLE;
	} else {
		waitflag = DDRC_IDLE;
	}

	/* 2. Polling for DDRDSR_2[IDLE] to be set */
	(void)check_ddrc_idle(0U, waitflag);

	/* HALT the ddrc axi port */
	mmio_setbits_32(REG_DDR_SDRAM_CFG, BIT(1));

	/* 3. Set DDR_SDRAM_CFG_3[SR_FAST_WK_EN] */
	mmio_setbits_32(REG_DDR_SDRAM_CFG_3, BIT(1));
	if ((mmio_read_32(REG_DDR_SDRAM_CFG_3) & 0x2) != 0U) {
		NOTICE("SR_FAST_WK_EN bit set success\n");
	} else {
		NOTICE("SR_FAST_WK_EN bit set failed\n");
	}

	/* 4. Program DDR_SDRAM_CFG_4[DFI_FREQ] to 2b'1111 */
	val = mmio_read_32(REG_DDR_SDRAM_CFG_4);
	mmio_write_32(REG_DDR_SDRAM_CFG_4, val | 0x1f000);
	if ((mmio_read_32(REG_DDR_SDRAM_CFG_4) & 0x1f000) == 0x1f000) {
		NOTICE("DFI_FREQ bits set success\n");
	} else {
		NOTICE("DFI_FREQ bit set failed\n");
	}

	/* 5. Clear DDR_ZQ_CNTL register */
	mmio_write_32(REG_DDR_ZQ_CNTL, 0x0);

	/* 7. Force the DDRC to enter self refresh */
	self_refresh_enter();
	/* Enable ipg_stop_reg while auto cg */
	mmio_setbits_32(REG_AUTO_CG_CTRL, BIT(17));
	mmio_setbits_32(REG_DDR_SDRAM_CFG, BIT(30)); /* Enable SR during sleep */
	mmio_setbits_32(REG_DDR_SDRAM_CFG_3, BIT(11)); /* Drain for self refresh */

	/* 8. Set stop and poll stop ack */
	ipg_stop_ack_wait(0x0);
	mmio_setbits_32(REG_DDRC_STOP_CTRL, BIT(0));
	ipg_stop_ack_wait(0x1);

	/* 9. Clear DDR_INTERVAL(this disables refreshes */
	mmio_write_32(REG_DDR_SDRAM_INTERVAL, 0x0);

	/* 10. Set DDR_SDRAM_MD_CNTL to force CKE low */
	mmio_write_32(REG_DDR_SDRAM_MD_CNTL, 0x00100000);

	/* 11. Remove Stop request via DDRMIX register */
	mmio_clrbits_32(REG_DDRC_STOP_CTRL, BIT(0));
	check_dfi_init_complete();

	/* 12. Clear DDR_SDRAM_CFG[SREN] */
	mmio_clrbits_32(REG_DDR_SDRAM_CFG, BIT(30));

	/* 13. Clear DDR_SDRAM_CFG_3[SR_FAST_WK_EN] */
	mmio_clrbits_32(REG_DDR_SDRAM_CFG_3, BIT(1));

	/* 14. Set stop request again via DDRMIX register */
	ipg_stop_ack_wait(0x0);
	mmio_setbits_32(REG_DDRC_STOP_CTRL, BIT(0));

	/* 15. Poll for STOP_ACK to be asserted in DDRMIX register */
	ipg_stop_ack_wait(0x1);

	/* 16. Set STOP request again via DDRMIX register */
	mmio_clrbits_32(REG_DDRC_STOP_CTRL, BIT(0));

	/* Should check PhyInLP3 pub reg */
	dwc_ddrphy_apb_wr(0xd0000, 0x0);
	if ((dwc_ddrphy_apb_rd(0x90028) & 0x1) == 0U) {
		INFO("PhyInLP3 = 1\n");
	}
	dwc_ddrphy_apb_wr(0xd0000, 0x1);

	/* 17. Clear PwrOkIn via DDRMIX register */
	mmio_setbits_32(SRC_DDRPHY_SINGLE_RESET_SW_CTRL, BIT(0));

	/* 18. Power off the DDRMIX */
	VERBOSE("Enable ddr power off\n");
	mmio_setbits_32(SRC_DDRC_SW_CTRL, BIT(31));
	VERBOSE("enter retention done\n");
}

/* Restore the dram PHY config */
static void dram_phy_init(struct dram_timing_info *timing)
{
	struct dram_cfg_param *cfg = timing->ddrphy_cfg;
	unsigned int i;

	/* Restore the PHY init config */
	cfg = timing->ddrphy_cfg;
	for (i = 0U; i < timing->ddrphy_cfg_num; i++) {
		dwc_ddrphy_apb_wr(cfg->reg, cfg->val);
		cfg++;
	}

	dwc_ddrphy_apb_wr(0xd0000, 0x0);
	dwc_ddrphy_apb_wr(0xc0080, 0x3);
	/* Restore the DDR PHY CSRs */
	cfg = timing->ddrphy_trained_csr;
	for (i = 0U; i < timing->ddrphy_trained_csr_num; i++) {
		dwc_ddrphy_apb_wr(cfg->reg, cfg->val);
		cfg++;
	}

	dwc_ddrphy_apb_wr(0xc0080, 0x2);
	dwc_ddrphy_apb_wr(0xd0000, 0x1);

	/* Load the PIE image */
	cfg = timing->ddrphy_pie;
	for (i = 0U; i < timing->ddrphy_pie_num; i++) {
		dwc_ddrphy_apb_wr(cfg->reg, cfg->val);
		cfg++;
	}
}

static void ddrc_init(struct dram_timing_info *timing)
{
	struct dram_cfg_param *ddrc_cfg = timing->ddrc_cfg;
	unsigned int i;
	uint32_t val;

	for (i = 0U; i < timing->ddrc_cfg_num; i++) {
		/* Skip the dram init as we resume from retention */
		if (ddrc_cfg->reg == REG_DDR_SDRAM_CFG_2) {
			mmio_write_32(ddrc_cfg->reg, ddrc_cfg->val & ~BIT(4));
		} else {
			mmio_write_32(ddrc_cfg->reg, ddrc_cfg->val);
		}
		ddrc_cfg++;
	}

	if (timing->fsp_cfg != NULL) {
		ddrc_cfg = timing->fsp_cfg[0].ddrc_cfg;
		while (ddrc_cfg->reg != 0U) {
			mmio_write_32(ddrc_cfg->reg, ddrc_cfg->val);
			ddrc_cfg++;
		}
	}

	check_dfi_init_complete();

	val = mmio_read_32(REG_DDR_SDRAM_CFG);
	mmio_write_32(REG_DDR_SDRAM_CFG, val | BIT(31));

	(void)check_ddrc_idle(0U, DDRC_IDLE);
}

static void ddrphy_coldreset(void)
{
	/*
	 * dramphy_apb_n default 1 , assert -> 0, de_assert -> 1
	 * dramphy_reset_n default 0 , assert -> 0, de_assert -> 1
	 * dramphy_PwrOKIn default 0 , assert -> 1, de_assert -> 0
	 */
	/* src_gen_dphy_apb_sw_rst_de_assert */
	mmio_clrbits_32(SRC_DDRPHY_SW_CTRL, BIT(0));
	/* src_gen_dphy_sw_rst_de_assert */
	mmio_clrbits_32(SRC_DDRPHY_SINGLE_RESET_SW_CTRL, BIT(2));
	/* src_gen_dphy_PwrOKIn_sw_rst_de_assert() */
	mmio_setbits_32(SRC_DDRPHY_SINGLE_RESET_SW_CTRL, BIT(0));
	udelay(10);

	/* src_gen_dphy_apb_sw_rst_assert */
	mmio_setbits_32(SRC_DDRPHY_SW_CTRL, BIT(0));
	/* src_gen_dphy_sw_rst_assert */
	mmio_setbits_32(SRC_DDRPHY_SINGLE_RESET_SW_CTRL, BIT(2));
	udelay(10);

	/* src_gen_dphy_PwrOKIn_sw_rst_assert */
	mmio_clrbits_32(SRC_DDRPHY_SINGLE_RESET_SW_CTRL, BIT(0));
	udelay(10);
	/* src_gen_dphy_apb_sw_rst_de_assert */
	mmio_clrbits_32(SRC_DDRPHY_SW_CTRL, BIT(0));
	/* src_gen_dphy_sw_rst_de_assert() */
	mmio_clrbits_32(SRC_DDRPHY_SINGLE_RESET_SW_CTRL, BIT(2));
}

void dram_exit_retention(void)
{
	/* 1. Power up the DDRMIX */
	mmio_clrbits_32(SRC_DDRC_SW_CTRL, BIT(31));

	/* Additional step to make sure DDR exit retenton works */
	mmio_setbits_32(SRC_DDRC_SW_CTRL, BIT(0));
	udelay(10);
	mmio_clrbits_32(SRC_DDRC_SW_CTRL, BIT(0));
	udelay(10);

	/* 2. Cold reset the DDRPHY */
	ddrphy_coldreset();

	/* 3. Config the DRAM PLL to FSP0 */
	dram_pll_init();
	ddr_disable_bypass();

	/* 4. Reload the DDRPHY config */
	dram_phy_init(timing_info);
	NOTICE("phy reload done\n");

	/* 5. Reload the ddrc config */
	ddrc_init(timing_info);

	/* Set SR_FAST_WK_EN to 1 by default */
	mmio_setbits_32(REG_DDR_SDRAM_CFG_3, BIT(1));

	NOTICE("exit retention done\n");
}
