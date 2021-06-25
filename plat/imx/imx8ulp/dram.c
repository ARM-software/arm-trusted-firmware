/*
 * Copyright 2021-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <lib/mmio.h>
#include <platform_def.h>

#include <upower_api.h>

#define PHY_FREQ_SEL_INDEX(x)		((x) << 16)
#define PHY_FREQ_MULTICAST_EN(x)	((x) << 8)
#define DENALI_PHY_1537			U(0x5804)

#define IMX_DDRC_BASE			U(0x2E060000)
#define SAVED_DRAM_DATA_BASE		U(0x20055000)
#define DENALI_CTL_144			0x240
#define LPI_WAKEUP_EN_SHIFT		U(8)
#define IMX_LPAV_SIM_BASE		0x2DA50000
#define LPDDR_CTRL			0x14
#define LPDDR_AUTO_LP_MODE_DISABLE	BIT(24)
#define SOC_LP_CMD_SHIFT		U(15)
#define LPDDR_CTRL2			0x18

#define DENALI_CTL_00			U(0x0)
#define DENALI_CTL_23			U(0x5c)
#define DFIBUS_FREQ_INIT_SHIFT		U(24)
#define TSREF2PHYMSTR_SHIFT		U(8)
#define TSREF2PHYMSTR_MASK		GENMASK(13, 8)

#define DENALI_CTL_24			U(0x60)
#define DENALI_CTL_25			U(0x64)

#define DENALI_CTL_93			U(0x174)
#define PWRUP_SREFRESH_EXIT		BIT(0)

#define DENALI_CTL_127				U(0x1fc)
#define PHYMSTR_TRAIN_AFTER_INIT_COMPLETE	BIT(16)

#define DENALI_CTL_147			U(0x24c)
#define DENALI_CTL_153			U(0x264)
#define PCPCS_PD_EN			BIT(8)

#define DENALI_CTL_249			U(0x3E4)
#define DENALI_CTL_266			U(0x428)

#define DENALI_PHY_1547			U(0x582c)
#define PHY_LP4_BOOT_DISABLE		BIT(8)

#define DENALI_PHY_1559			U(0x585c)
#define DENALI_PHY_1590			U(0x58D8)

#define DENALI_PI_00			U(0x2000)
#define DENALI_PI_04			U(0x2010)
#define DENALI_PI_52			U(0x20D0)
#define DENALI_PI_26			U(0x2068)
#define DENALI_PI_33			U(0x2084)
#define DENALI_PI_65			U(0x2104)
#define DENALI_PI_77			U(0x2134)
#define DENALI_PI_134			U(0x2218)
#define DENALI_PI_131			U(0x220C)
#define DENALI_PI_132			U(0x2210)
#define DENALI_PI_134			U(0x2218)
#define DENALI_PI_137			U(0x2224)
#define DENALI_PI_174			U(0x22B8)
#define DENALI_PI_175			U(0x22BC)
#define DENALI_PI_181			U(0x22D4)
#define DENALI_PI_182			U(0x22D8)
#define DENALI_PI_191			U(0x22FC)
#define DENALI_PI_192			U(0x2300)
#define DENALI_PI_212			U(0x2350)
#define DENALI_PI_214			U(0x2358)
#define DENALI_PI_217			U(0x2364)

#define LPDDR3_TYPE	U(0x7)
#define LPDDR4_TYPE	U(0xB)

extern void upower_wait_resp(void);

struct dram_cfg_param {
	uint32_t reg;
	uint32_t val;
};

struct dram_timing_info {
	/* ddr controller config */
	struct dram_cfg_param *ctl_cfg;
	unsigned int ctl_cfg_num;
	/* pi config */
	struct dram_cfg_param *pi_cfg;
	unsigned int pi_cfg_num;
	/* phy freq1 config */
	struct dram_cfg_param *phy_f1_cfg;
	unsigned int phy_f1_cfg_num;
	/* phy freq2 config */
	struct dram_cfg_param *phy_f2_cfg;
	unsigned int phy_f2_cfg_num;
	/* initialized drate table */
	unsigned int fsp_table[3];
};

#define CTL_NUM		U(680)
#define PI_NUM		U(298)
#define PHY_NUM		U(1654)
#define PHY_DIFF_NUM	U(49)
struct dram_cfg {
	uint32_t ctl_cfg[CTL_NUM];
	uint32_t pi_cfg[PI_NUM];
	uint32_t phy_full[PHY_NUM];
	uint32_t phy_diff[PHY_DIFF_NUM];
};

struct dram_timing_info *info;
struct dram_cfg *dram_timing_cfg;

/* mark if dram cfg is already saved */
static bool dram_cfg_saved;
static uint32_t dram_class;

/* PHY register index for frequency diff */
uint32_t freq_specific_reg_array[PHY_DIFF_NUM] = {
90, 92, 93, 96, 97, 100, 101, 102, 103, 104, 114,
346, 348, 349, 352, 353, 356, 357, 358, 359, 360,
370, 602, 604, 605, 608, 609, 612, 613, 614, 615,
616, 626, 858, 860, 861, 864, 865, 868, 869, 870,
871, 872, 882, 1063, 1319, 1566, 1624, 1625
};

static void ddr_init(void)
{
	unsigned int i;

	/* restore the ddr ctl config */
	for (i = 0U; i < CTL_NUM; i++) {
		mmio_write_32(IMX_DDRC_BASE + i * 4, dram_timing_cfg->ctl_cfg[i]);
	}

	/* load the PI registers */
	for (i = 0U; i < PI_NUM; i++) {
		mmio_write_32(IMX_DDRC_BASE + 0x2000 + i * 4, dram_timing_cfg->pi_cfg[i]);
	}


	 /* restore all PHY registers for all the fsp. */
	mmio_write_32(IMX_DDRC_BASE + DENALI_PHY_1537, 0x100);
	/* restore all the phy configs */
	for (i = 0U; i < PHY_NUM; i++) {
		/* skip the reserved registers space */
		if (i >= 121U && i <= 255U) {
			continue;
		}
		if (i >= 377U && i <= 511U) {
			continue;
		}
		if (i >= 633U && i <= 767U) {
			continue;
		}
		if (i >= 889U && i <= 1023U) {
			continue;
		}
		if (i >= 1065U && i <= 1279U) {
			continue;
		}
		if (i >= 1321U && i <= 1535U) {
			continue;
		}
		mmio_write_32(IMX_DDRC_BASE + 0x4000 + i * 4, dram_timing_cfg->phy_full[i]);
	}

	if (dram_class == LPDDR4_TYPE) {
		/* restore only the diff. */
		mmio_write_32(IMX_DDRC_BASE + DENALI_PHY_1537, 0x0);
		for (i = 0U; i < PHY_DIFF_NUM; i++) {
			mmio_write_32(IMX_DDRC_BASE + 0x4000 + freq_specific_reg_array[i] * 4,
				      dram_timing_cfg->phy_diff[i]);
		}
	}

	/* Re-enable MULTICAST mode */
	mmio_write_32(IMX_DDRC_BASE + DENALI_PHY_1537, PHY_FREQ_MULTICAST_EN(1));
}

void dram_enter_retention(void)
{
	unsigned int i;

	/* 1. config the PCC_LPDDR4[SSADO] to 2b'11 for ACK domain 0/1's STOP */
	mmio_setbits_32(IMX_PCC5_BASE + 0x108, 0x2 << 22);

	/*
	 * 2. Make sure the DENALI_CTL_144[LPI_WAKEUP_EN[5:0]] has the bit
	 * LPI_WAKEUP_EN[3] = 1b'1. This enables the option 'self-refresh
	 * long with mem and ctlr clk gating or self-refresh  power-down
	 * long with mem and ctlr clk gating'
	 */
	mmio_setbits_32(IMX_DDRC_BASE + DENALI_CTL_144, BIT(3) << LPI_WAKEUP_EN_SHIFT);

	/*
	 * 3a. Config SIM_LPAV LPDDR_CTRL[LPDDR_AUTO_LP_MODE_DISABLE] to 1b'0(enable
	 * the logic to automatic handles low power entry/exit. This is the recommended
	 * option over handling through software.
	 * 3b. Config the SIM_LPAV LPDDR_CTRL[SOC_LP_CMD] to 6b'101001(encoding for
	 * self_refresh with both DDR controller and DRAM clock gate. THis is mandatory
	 * since LPPDR logic will be power gated).
	 */
	mmio_clrbits_32(IMX_LPAV_SIM_BASE + LPDDR_CTRL, LPDDR_AUTO_LP_MODE_DISABLE);
	mmio_clrsetbits_32(IMX_LPAV_SIM_BASE + LPDDR_CTRL,
			   0x3f << SOC_LP_CMD_SHIFT, 0x29 << SOC_LP_CMD_SHIFT);

	/* Save DDR Controller & PHY config.
	 * Set PHY_FREQ_SEL_MULTICAST_EN=0 & PHY_FREQ_SEL_INDEX=1. Read and store all
	 * the PHY registers for F2 into phy_f1_cfg, then read/store the diff between
	 * F1 & F2 into phy_f2_cfg.
	 */
	if (!dram_cfg_saved) {
		info = (struct dram_timing_info *)SAVED_DRAM_DATA_BASE;
		dram_timing_cfg = (struct dram_cfg *)(SAVED_DRAM_DATA_BASE +
					sizeof(struct dram_timing_info));

		/* get the dram type */
		dram_class = mmio_read_32(IMX_DDRC_BASE + DENALI_CTL_00);
		dram_class = (dram_class >> 8) & 0xf;

		/* save the ctl registers */
		for (i = 0U; i < CTL_NUM; i++) {
			dram_timing_cfg->ctl_cfg[i] = mmio_read_32(IMX_DDRC_BASE + i * 4);
		}
		dram_timing_cfg->ctl_cfg[0] = dram_timing_cfg->ctl_cfg[0] & 0xFFFFFFFE;

		/* save the PI registers */
		for (i = 0U; i < PI_NUM; i++) {
			dram_timing_cfg->pi_cfg[i] = mmio_read_32(IMX_DDRC_BASE + 0x2000 + i * 4);
		}
		dram_timing_cfg->pi_cfg[0] = dram_timing_cfg->pi_cfg[0] & 0xFFFFFFFE;

		/*
		 * Read and store all PHY registers. full array is a full
		 * copy for all the setpoint
		 */
		if (dram_class == LPDDR4_TYPE) {
			mmio_write_32(IMX_DDRC_BASE + DENALI_PHY_1537, 0x10000);
			for (i = 0U; i < PHY_NUM; i++) {
				/* Make sure MULTICASE is enabled */
				if (i == 1537U) {
					dram_timing_cfg->phy_full[i] = 0x100;
				} else {
					dram_timing_cfg->phy_full[i] = mmio_read_32(IMX_DDRC_BASE + 0x4000 + i * 4);
				}
			}

			/*
			 * set PHY_FREQ_SEL_MULTICAST_EN=0 & PHY_FREQ_SEL_INDEX=0.
			 * Read and store only the diff.
			 */
			mmio_write_32(IMX_DDRC_BASE + DENALI_PHY_1537, 0x0);
			/* save only the frequency based diff config to save memory */
			for (i = 0U; i < PHY_DIFF_NUM; i++) {
				dram_timing_cfg->phy_diff[i] = mmio_read_32(IMX_DDRC_BASE + 0x4000 +
									    freq_specific_reg_array[i] * 4);
			}
		} else {
			/* LPDDR3, only f1 need to save */
			for (i = 0U; i < info->phy_f1_cfg_num; i++) {
				info->phy_f1_cfg[i].val = mmio_read_32(info->phy_f1_cfg[i].reg);
			}
		}

		dram_cfg_saved = true;
	}
}

void dram_exit_retention(void)
{
	uint32_t val;

	/* 1. Config the LPAV PLL4 and DDR clock for the desired LPDDR operating frequency. */
	mmio_setbits_32(IMX_PCC5_BASE + 0x108, BIT(30));

	/* 2. Write PCC5.PCC_LPDDR4[SWRST] to 1b'1 to release LPDDR from reset. */
	mmio_setbits_32(IMX_PCC5_BASE + 0x108, BIT(28));

	/* 3. Reload the LPDDR CTL/PI/PHY register */
	ddr_init();

	if (dram_class == LPDDR4_TYPE) {
		/* 4a. FIXME Set PHY_SET_DFI_INPUT_N parameters to 4'h1. LPDDR4 only */
		mmio_write_32(IMX_DDRC_BASE + DENALI_PHY_1559, 0x01010101);

		/*
		 * 4b. CTL PWRUP_SREFRESH_EXIT=1'b0 for disabling self refresh exit
		 * from controller.
		 */
		/*
		 * 4c. PI_PWRUP_SELF_REF_EXIT=1, PI_MC_PWRUP_SELF_REF_EXIT=0 for enabling
		 * self refresh exit from PI
		 */
		/* 4c. PI_INT_LVL_EN=0 to skip Initialization trainings. */
		/*
		 * 4d. PI_WRLVL_EN_F0/1/2= PI_CALVL_EN_F0/1/2= PI_RDLVL_EN_F0/1/2=
		 * PI_RDLVL_GATE_EN_F0/1/2= PI_WDQLVL_EN_F0/1/2=0x2.
		 * Enable non initialization trainings.
		 */
		/* 4e. PI_PWRUP_SREFRESH_EXIT_CS=0xF */
		/* 4f. PI_DLL_RESET=0x1 */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_137, 0x1);
		/* PI_PWRUP_SELF_REF_EXIT = 1 */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_132, 0x01000000);
		/* PI_MC_PWRUP_SELF_REF_EXIT = 0 */
		mmio_clrbits_32(IMX_DDRC_BASE + DENALI_PI_132, BIT(16));
		/* PI_INT_LVL_EN = 0 */
		mmio_clrbits_32(IMX_DDRC_BASE + DENALI_PI_04, BIT(0));
		/* PI_WRLVL_EN_F0 = 3, PI_WRLVL_EN_F1 = 3 */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_174, 0x03030000);
		/* PI_WRLVL_EN_F2 = 3 */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_175, 0x03);
		/* PI_CALVL_EN_F0 = 3, PI_CALVL_EN_F1 = 3 */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_191, 0x03030000);
		/* PI_CALVL_EN_F2 = 3 */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_192, 0x03);
		/* PI_WDQLVL_EN_F0 = 3 */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_212, 0x300);
		/* PI_WDQLVL_EN_F1 = 3 */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_214, 0x03000000);
		/* PI_WDQLVL_EN_F2 = 3 */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_217, 0x300);
		/* PI_EDLVL_EN_F0 = 3, PI_EDLVL_GATE_EN_F0 = 3 */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_181, 0x03030000);
		/*
		 * PI_RDLVL_EN_F1 = 3, PI_RDLVL_GATE_EN_F1 = 3,
		 * PI_RDLVL_EN_F2 = 3, PI_RDLVL_GATE_EN_F2 = 3
		 */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_182, 0x03030303);
		/* PI_PWRUP_SREFRESH_EXIT_CS = 0xF */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_134, 0x000F0000);
	} else {
		/* PI_DLL_RESET=1 */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_137, 0x1);
		/* PI_PWRUP_SELF_REF_EXIT=1 */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_132, 0x01000000);
		/* PI_MC_PWRUP_SELF_REF_EXIT=0 */
		mmio_clrbits_32(IMX_DDRC_BASE + DENALI_PI_132, BIT(16));
		/* PI_INT_LVL_EN=0 */
		mmio_clrbits_32(IMX_DDRC_BASE + DENALI_PI_04, BIT(0));
		/* PI_WRLVL_EN_F0=3 */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_174, 0x00030000);
		/* PI_CALVL_EN_F0=3 */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_191, 0x00030000);
		/* PI_RDLVL_EN_F0=3,PI_RDLVL_GATE_EN_F0=3 */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_181, 0x03030000);
		/* PI_PWRUP_SREFRESH_EXIT_CS=0xF */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_134, 0x000F0000);
	}

	mmio_write_32(IMX_DDRC_BASE + DENALI_CTL_144, 0x00002D00);

	/* Force in-order AXI read data */
	mmio_write_32(IMX_DDRC_BASE + DENALI_CTL_144, 0x1);

	/*
	 * Disable special R/W group switches so that R/W group placement
	 * is always at END of R/W group.
	 */
	mmio_write_32(IMX_DDRC_BASE + DENALI_CTL_249, 0x0);

	/* Reduce time for IO pad calibration */
	mmio_write_32(IMX_DDRC_BASE + DENALI_PHY_1590, 0x01000000);

	mmio_write_32(IMX_DDRC_BASE + DENALI_CTL_25, 0x00020100);

	/* PD disable */
	mmio_write_32(IMX_DDRC_BASE + DENALI_CTL_153, 0x04040000);
	/*
	 * 5. Disable automatic LP entry and PCPCS modes LP_AUTO_ENTRY_EN
	 * to 1b'0, PCPCS_PD_EN to 1b'0
	 */

	upwr_xcp_set_ddr_retention(APD_DOMAIN, 0, NULL);
	upower_wait_resp();

	if (dram_class == LPDDR4_TYPE) {
		/* 7. Write PI START parameter to 1'b1 */
		mmio_write_32(IMX_DDRC_BASE + DENALI_PI_00, 0x00000b01);

		/* 8. Write CTL START parameter to 1'b1 */
		mmio_write_32(IMX_DDRC_BASE + DENALI_CTL_00, 0x00000b01);
	} else {
		/* 7. Write PI START parameter to 1'b1 */
		mmio_write_32(IMX_DDRC_BASE + DENALI_PI_00, 0x00000701);

		/* 8. Write CTL START parameter to 1'b1 */
		mmio_write_32(IMX_DDRC_BASE + DENALI_CTL_00, 0x00000701);
	}

	/* 9. DENALI_CTL_266:  Wait for INT_STATUS_INIT=0x2 */
	do {
		val = (mmio_read_32(IMX_DDRC_BASE + DENALI_CTL_266) >> 8) & 0xFF;
	} while (val != 0x2);

	/*
	 * 10. Run SW trainings by setting PI_CALVL_REQ,PI_WRLVL_REQ,PI_RDLVL_GATE_REQ,
	 * PI_RDLVL_REQ,PI_WDQLVL_REQ(NA for LPDDR3) in same order.
	 */
	if (dram_class == LPDDR4_TYPE) {
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_52, 0x10000); /* CALVL */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_26, 0x100); /* WRLVL */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_33, 0x10000); /* RDGATE */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_33, 0x100); /* RDQLVL */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_65, 0x10000); /* WDQLVL */

		/* 11. Wait for trainings to get complete by polling PI_INT_STATUS */
		while ((mmio_read_32(IMX_DDRC_BASE + DENALI_PI_77) & 0x07E00000) != 0x07E00000) {
			;
		}
	} else {
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_52, 0x10000); /* CALVL */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_26, 0x100); /* WRLVL */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_33, 0x10000); /* RDGATE */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_PI_33, 0x100); /* RDQLVL */
		while ((mmio_read_32(IMX_DDRC_BASE + DENALI_PI_77) & 0x05E00000) != 0x05E00000) {
			;
		}
	}
}
