/*
 * Copyright 2021-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <arch_helpers.h>
#include <bl31/interrupt_mgmt.h>
#include <common/runtime_svc.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <plat/common/platform.h>

#include <platform_def.h>

#include <dram.h>
#include <upower_api.h>

#define PHY_FREQ_SEL_INDEX(x)		((x) << 16)
#define PHY_FREQ_MULTICAST_EN(x)	((x) << 8)
#define DENALI_PHY_1537			U(0x5804)

#define IMX_DDRC_BASE			U(0x2E060000)
#define SAVED_DRAM_DATA_BASE		U(0x20055000)
#define DENALI_CTL_143			U(0x23C)
#define DENALI_CTL_144			U(0x240)
#define DENALI_CTL_146			U(0x248)
#define LP_STATE_CS_IDLE		U(0x404000)
#define LP_STATE_CS_PD_CG		U(0x4F4F00)
#define LPI_WAKEUP_EN_SHIFT		U(8)
#define IMX_LPAV_SIM_BASE		0x2DA50000
#define LPDDR_CTRL			0x14
#define LPDDR_AUTO_LP_MODE_DISABLE	BIT(24)
#define SOC_LP_CMD_SHIFT		U(15)
#define LPDDR_CTRL2			0x18
#define LPDDR_EN_CLKGATE		(0x1<<17)
#define LPDDR_MAX_CLKDIV_EN		(0x1 << 16)
#define LP_AUTO_ENTRY_EN		0x4
#define LP_AUTO_EXIT_EN			0xF

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
	/* automatic low power config */
	struct dram_cfg_param *auto_lp_cfg;
	unsigned int auto_lp_cfg_num;
	/* initialized drate table */
	unsigned int fsp_table[3];
};

#define CTL_NUM		U(680)
#define PI_NUM		U(298)
#define PHY_NUM		U(1654)
#define PHY_DIFF_NUM	U(49)
#define AUTO_LP_NUM	U(3)
struct dram_cfg {
	uint32_t ctl_cfg[CTL_NUM];
	uint32_t pi_cfg[PI_NUM];
	uint32_t phy_full[PHY_NUM];
	uint32_t phy_diff[PHY_DIFF_NUM];
	uint32_t auto_lp_cfg[AUTO_LP_NUM];
};

struct dram_timing_info *info;
struct dram_cfg *dram_timing_cfg;

/* mark if dram cfg is already saved */
static bool dram_cfg_saved;
static bool dram_auto_lp_true;
static uint32_t dram_class, dram_ctl_143;

/* PHY register index for frequency diff */
uint32_t freq_specific_reg_array[PHY_DIFF_NUM] = {
90, 92, 93, 96, 97, 100, 101, 102, 103, 104, 114,
346, 348, 349, 352, 353, 356, 357, 358, 359, 360,
370, 602, 604, 605, 608, 609, 612, 613, 614, 615,
616, 626, 858, 860, 861, 864, 865, 868, 869, 870,
871, 872, 882, 1063, 1319, 1566, 1624, 1625
};

/* lock used for DDR DVFS */
spinlock_t dfs_lock;
static volatile uint32_t core_count;
static volatile bool in_progress;
static volatile bool sys_dvfs;
static int num_fsp;

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

void dram_lp_auto_disable(void)
{
	uint32_t lp_auto_en;

	dram_timing_cfg = (struct dram_cfg *)(SAVED_DRAM_DATA_BASE +
					      sizeof(struct dram_timing_info));
	lp_auto_en = (mmio_read_32(IMX_DDRC_BASE + DENALI_CTL_146) & (LP_AUTO_ENTRY_EN << 24));
	/* Save initial config */
	dram_ctl_143 = mmio_read_32(IMX_DDRC_BASE + DENALI_CTL_143);

	if (lp_auto_en && !dram_auto_lp_true) {
		/* 0.a Save DDRC auto low-power mode parameter */
		dram_timing_cfg->auto_lp_cfg[0] = mmio_read_32(IMX_DDRC_BASE + DENALI_CTL_144);
		dram_timing_cfg->auto_lp_cfg[1] = mmio_read_32(IMX_DDRC_BASE + DENALI_CTL_147);
		dram_timing_cfg->auto_lp_cfg[2] = mmio_read_32(IMX_DDRC_BASE + DENALI_CTL_146);
		/* Set LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F2 to Maximum */
		mmio_setbits_32(IMX_DDRC_BASE + DENALI_CTL_143, 0xF << 24);
		/* 0.b Disable DDRC auto low-power mode interface */
		mmio_clrbits_32(IMX_DDRC_BASE + DENALI_CTL_146, LP_AUTO_ENTRY_EN << 24);
		/* 0.c Read any location to get DRAM out of Self-refresh */
		mmio_read_32(DEVICE2_BASE);
		/* 0.d Confirm DRAM is out of Self-refresh */
		while ((mmio_read_32(IMX_DDRC_BASE + DENALI_CTL_146) &
			LP_STATE_CS_PD_CG) != LP_STATE_CS_IDLE) {
			;
		}
		/* 0.e Disable DDRC auto low-power exit */
		mmio_clrbits_32(IMX_DDRC_BASE + DENALI_CTL_147, LP_AUTO_EXIT_EN);
		/* dram low power mode flag */
		dram_auto_lp_true = true;
	}
}

void dram_lp_auto_enable(void)
{
	/* Switch back to Auto Low-power mode */
	if (dram_auto_lp_true) {
		/* 12.a Confirm DRAM is out of Self-refresh */
		while ((mmio_read_32(IMX_DDRC_BASE + DENALI_CTL_146) &
			LP_STATE_CS_PD_CG) != LP_STATE_CS_IDLE) {
			;
		}
		/* 12.b Enable DDRC auto low-power exit */
		/*
		 * 12.c TBC! : Set DENALI_CTL_144 [LPI_CTRL_REQ_EN[24]] and
		 * [DFI_LP_VERSION[16]] back to default settings = 1b'1.
		 */
		/*
		 * 12.d Reconfigure DENALI_CTL_144 [LPI_WAKEUP_EN[5:0]] bit
		 * LPI_WAKEUP_EN[3] = 1b'1.
		 */
		mmio_write_32(IMX_DDRC_BASE + DENALI_CTL_144, dram_timing_cfg->auto_lp_cfg[0]);
		mmio_write_32(IMX_DDRC_BASE + DENALI_CTL_147, dram_timing_cfg->auto_lp_cfg[1]);
		/* 12.e Re-enable DDRC auto low-power mode interface */
		mmio_write_32(IMX_DDRC_BASE + DENALI_CTL_146, dram_timing_cfg->auto_lp_cfg[2]);
		/* restore ctl config */
		mmio_write_32(IMX_DDRC_BASE + DENALI_CTL_143, dram_ctl_143);
		/* dram low power mode flag */
		dram_auto_lp_true = false;
	}
}

void dram_enter_self_refresh(void)
{
	/* disable auto low power interface */
	dram_lp_auto_disable();
	/* 1. config the PCC_LPDDR4[SSADO] to 2b'11 for ACK domain 0/1's STOP */
	mmio_setbits_32(IMX_PCC5_BASE + 0x108, 0x2 << 22);
	/* 1.a Clock gate PCC_LPDDR4[CGC] and no software reset PCC_LPDDR4[SWRST] */
	mmio_setbits_32(IMX_PCC5_BASE + 0x108, (BIT(30) | BIT(28)));

	/*
	 * 2. Make sure the DENALI_CTL_144[LPI_WAKEUP_EN[5:0]] has the bit
	 * LPI_WAKEUP_EN[3] = 1b'1. This enables the option 'self-refresh
	 * long with mem and ctlr clk gating or self-refresh power-down long
	 * with mem and ctlr clk gating'
	 */
	mmio_setbits_32(IMX_DDRC_BASE + DENALI_CTL_144, BIT(3) << LPI_WAKEUP_EN_SHIFT);
	/* TODO: Needed ? 2.a DENALI_CTL_144[LPI_TIMER_WAKEUP_F2] */
	//mmio_setbits_32(IMX_DDRC_BASE + DENALI_CTL_144, BIT(0));

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
	/* 3.c clock gate ddr controller */
	mmio_setbits_32(IMX_LPAV_SIM_BASE + LPDDR_CTRL2, LPDDR_EN_CLKGATE);
	/* 3.d lpddr max clk div en */
	mmio_clrbits_32(IMX_LPAV_SIM_BASE + LPDDR_CTRL2, LPDDR_MAX_CLKDIV_EN);
}

void dram_exit_self_refresh(void)
{
	dram_lp_auto_enable();
}

void dram_enter_retention(void)
{
	unsigned int i;

	dram_lp_auto_disable();

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

	dram_lp_auto_enable();
}

#define LPDDR_DONE       (0x1<<4)
#define SOC_FREQ_CHG_ACK (0x1<<6)
#define SOC_FREQ_CHG_REQ (0x1<<7)
#define LPI_WAKEUP_EN    (0x4<<8)
#define SOC_FREQ_REQ     (0x1<<11)

static void set_cgc2_ddrclk(uint8_t src, uint8_t div)
{

	/* Wait until the reg is unlocked for writing */
	while (mmio_read_32(IMX_CGC2_BASE + 0x40) & BIT(31))
		;

	mmio_write_32(IMX_CGC2_BASE + 0x40, (src << 28) | (div << 21));
	/* Wait for the clock switching done */
	while (!(mmio_read_32(IMX_CGC2_BASE + 0x40) & BIT(27)))
		;
}
static void set_ddr_clk(uint32_t ddr_freq)
{
	/* Disable DDR clock */
	mmio_clrbits_32(IMX_PCC5_BASE + 0x108, BIT(30));
	switch (ddr_freq) {
	/* boot frequency ? */
	case 48:
		set_cgc2_ddrclk(2, 0);
		break;
	/* default bypass frequency for fsp 1 */
	case 192:
		set_cgc2_ddrclk(0, 1);
		break;
	case 384:
		set_cgc2_ddrclk(0, 0);
		break;
	case 264:
		set_cgc2_ddrclk(4, 3);
		break;
	case 528:
		set_cgc2_ddrclk(4, 1);
		break;
	default:
		break;
	}
	/* Enable DDR clock */
	mmio_setbits_32(IMX_PCC5_BASE + 0x108, BIT(30));

	/* Wait until the reg is unlocked for writing */
	while (mmio_read_32(IMX_CGC2_BASE + 0x40) & BIT(31)) {
		;
	}
}

#define AVD_SIM_LPDDR_CTRL	(IMX_LPAV_SIM_BASE + 0x14)
#define AVD_SIM_LPDDR_CTRL2	(IMX_LPAV_SIM_BASE + 0x18)
#define MAX_FSP_NUM	U(3)
#define DDR_DFS_GET_FSP_COUNT	0x10
#define DDR_BYPASS_DRATE	U(400)

extern int upower_pmic_i2c_write(uint32_t reg_addr, uint32_t reg_val);

/* Normally, we only switch frequency between 1(bypass) and 2(highest) */
int lpddr4_dfs(uint32_t freq_index)
{
	uint32_t lpddr_ctrl, lpddr_ctrl2;
	uint32_t ddr_ctl_144;

	/*
	 * Valid index: 0 to 2
	 * index 0: boot frequency
	 * index 1: bypass frequency
	 * index 2: highest frequency
	 */
	if (freq_index > 2U) {
		return -1;
	}

	/*
	 * increase the voltage to 1.1V firstly before increase frequency
	 * and APD enter OD mode
	 */
	if (freq_index == 2U && sys_dvfs) {
		upower_pmic_i2c_write(0x22, 0x28);
	}

	/* Enable LPI_WAKEUP_EN */
	ddr_ctl_144 = mmio_read_32(IMX_DDRC_BASE + DENALI_CTL_144);
	mmio_setbits_32(IMX_DDRC_BASE + DENALI_CTL_144, LPI_WAKEUP_EN);

	/* put DRAM into long self-refresh & clock gating */
	lpddr_ctrl = mmio_read_32(AVD_SIM_LPDDR_CTRL);
	lpddr_ctrl = (lpddr_ctrl & ~((0x3f << 15) | (0x3 << 9))) | (0x28 << 15) | (freq_index << 9);
	mmio_write_32(AVD_SIM_LPDDR_CTRL, lpddr_ctrl);

	/* Gating the clock */
	lpddr_ctrl2 = mmio_read_32(AVD_SIM_LPDDR_CTRL2);
	mmio_setbits_32(AVD_SIM_LPDDR_CTRL2, LPDDR_EN_CLKGATE);

	/* Request frequency change */
	mmio_setbits_32(AVD_SIM_LPDDR_CTRL, SOC_FREQ_REQ);

	do {
		lpddr_ctrl = mmio_read_32(AVD_SIM_LPDDR_CTRL);
		if (lpddr_ctrl & SOC_FREQ_CHG_REQ) {
			/* Bypass mode */
			if (info->fsp_table[freq_index] < DDR_BYPASS_DRATE) {
				/* Change to PLL bypass mode */
				mmio_write_32(IMX_LPAV_SIM_BASE, 0x1);
				/* change the ddr clock source & frequency */
				set_ddr_clk(info->fsp_table[freq_index]);
			} else {
				/* Change to PLL unbypass mode */
				mmio_write_32(IMX_LPAV_SIM_BASE, 0x0);
				/* change the ddr clock source & frequency */
				set_ddr_clk(info->fsp_table[freq_index] >> 1);
			}

			mmio_clrsetbits_32(AVD_SIM_LPDDR_CTRL, SOC_FREQ_CHG_REQ, SOC_FREQ_CHG_ACK);
			continue;
		}
	} while ((lpddr_ctrl & LPDDR_DONE) != 0); /* several try? */

	/* restore the original setting */
	mmio_write_32(IMX_DDRC_BASE + DENALI_CTL_144, ddr_ctl_144);
	mmio_write_32(AVD_SIM_LPDDR_CTRL2, lpddr_ctrl2);

	/* Check the DFS result */
	lpddr_ctrl = mmio_read_32(AVD_SIM_LPDDR_CTRL) & 0xF;
	if (lpddr_ctrl != 0U) {
		/* Must be something wrong, return failure */
		return -1;
	}

	/* decrease the BUCK3 voltage after frequency changed to lower
	 * and APD in ND_MODE
	 */
	if (freq_index == 1U && sys_dvfs) {
		upower_pmic_i2c_write(0x22, 0x20);
	}

	/* DFS done successfully */
	return 0;
}

/* for the non-primary core, waiting for DFS done */
static uint64_t waiting_dvfs(uint32_t id, uint32_t flags,
		void *handle, void *cookie)
{
	uint32_t irq;

	irq = plat_ic_acknowledge_interrupt();
	if (irq < 1022U) {
		plat_ic_end_of_interrupt(irq);
	}

	/* set the WFE done status */
	spin_lock(&dfs_lock);
	core_count++;
	dsb();
	spin_unlock(&dfs_lock);

	while (in_progress) {
		wfe();
	}

	return 0;
}

int dram_dvfs_handler(uint32_t smc_fid, void *handle,
		u_register_t x1, u_register_t x2, u_register_t x3)
{
	unsigned int fsp_index = x1;
	uint32_t online_cpus = x2 - 1;
	uint64_t mpidr = read_mpidr_el1();
	unsigned int cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	/* Get the number of FSPs */
	if (x1 == DDR_DFS_GET_FSP_COUNT) {
		SMC_RET2(handle, num_fsp, info->fsp_table[1]);
	}

	/* start lpddr frequency scaling */
	in_progress = true;
	sys_dvfs = x3 ? true : false;
	dsb();

	/* notify other core wait for scaling done */
	for (unsigned int i = 0; i < PLATFORM_CORE_COUNT; i++)
		/* Skip raise SGI for current CPU */
		if (i != cpu_id) {
			plat_ic_raise_el3_sgi(0x8, i);
		}

	/* Make sure all the cpu in WFE */
	while (online_cpus != core_count) {
		;
	}

	/* Flush the L1/L2 cache */
	dcsw_op_all(DCCSW);

	lpddr4_dfs(fsp_index);

	in_progress = false;
	core_count = 0;
	dsb();
	sev();
	isb();

	SMC_RET1(handle, 0);
}

void dram_init(void)
{
	uint32_t flags = 0;
	uint32_t rc;
	unsigned int i;

	/* Register the EL3 handler for DDR DVFS */
	set_interrupt_rm_flag(flags, NON_SECURE);
	rc = register_interrupt_type_handler(INTR_TYPE_EL3, waiting_dvfs, flags);
	if (rc) {
		panic();
	}

	info = (struct dram_timing_info *)SAVED_DRAM_DATA_BASE;

	/* Get the num of the supported Fsp */
	for (i = 0; i < MAX_FSP_NUM; i++) {
		if (!info->fsp_table[i]) {
			break;
		}
	}

	num_fsp = (i > MAX_FSP_NUM) ? MAX_FSP_NUM : i;
}
