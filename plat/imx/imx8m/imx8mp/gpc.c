/*
 * Copyright 2019-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <lib/smccc.h>
#include <services/std_svc.h>

#include <gpc.h>
#include <imx_aipstz.h>
#include <imx_sip_svc.h>
#include <platform_def.h>

#define CCGR(x)		(0x4000 + (x) * 0x10)
#define IMR_NUM		U(5)

struct imx_noc_setting {
	uint32_t domain_id;
	uint32_t start;
	uint32_t end;
	uint32_t prioriy;
	uint32_t mode;
	uint32_t socket_qos_en;
};

enum clk_type {
	CCM_ROOT_SLICE,
	CCM_CCGR,
};

struct clk_setting {
	uint32_t offset;
	uint32_t val;
	enum clk_type type;
};

enum pu_domain_id {
	/* hsio ss */
	HSIOMIX,
	PCIE_PHY,
	USB1_PHY,
	USB2_PHY,
	MLMIX,
	AUDIOMIX,
	/* gpu ss */
	GPUMIX,
	GPU2D,
	GPU3D,
	/* vpu ss */
	VPUMIX,
	VPU_G1,
	VPU_G2,
	VPU_H1,
	/* media ss */
	MEDIAMIX,
	MEDIAMIX_ISPDWP,
	MIPI_PHY1,
	MIPI_PHY2,
	/* HDMI ss */
	HDMIMIX,
	HDMI_PHY,
	DDRMIX,
	MAX_DOMAINS,
};

/* PU domain, add some hole to minimize the uboot change */
static struct imx_pwr_domain pu_domains[MAX_DOMAINS] = {
	[MIPI_PHY1] = IMX_PD_DOMAIN(MIPI_PHY1, false),
	[PCIE_PHY] = IMX_PD_DOMAIN(PCIE_PHY, false),
	[USB1_PHY] = IMX_PD_DOMAIN(USB1_PHY, true),
	[USB2_PHY] = IMX_PD_DOMAIN(USB2_PHY, true),
	[MLMIX] = IMX_MIX_DOMAIN(MLMIX, false),
	[AUDIOMIX] = IMX_MIX_DOMAIN(AUDIOMIX, false),
	[GPU2D] = IMX_PD_DOMAIN(GPU2D, false),
	[GPUMIX] = IMX_MIX_DOMAIN(GPUMIX, false),
	[VPUMIX] = IMX_MIX_DOMAIN(VPUMIX, false),
	[GPU3D] = IMX_PD_DOMAIN(GPU3D, false),
	[MEDIAMIX] = IMX_MIX_DOMAIN(MEDIAMIX, false),
	[VPU_G1] = IMX_PD_DOMAIN(VPU_G1, false),
	[VPU_G2] = IMX_PD_DOMAIN(VPU_G2, false),
	[VPU_H1] = IMX_PD_DOMAIN(VPU_H1, false),
	[HDMIMIX] = IMX_MIX_DOMAIN(HDMIMIX, false),
	[HDMI_PHY] = IMX_PD_DOMAIN(HDMI_PHY, false),
	[MIPI_PHY2] = IMX_PD_DOMAIN(MIPI_PHY2, false),
	[HSIOMIX] = IMX_MIX_DOMAIN(HSIOMIX, false),
	[MEDIAMIX_ISPDWP] = IMX_PD_DOMAIN(MEDIAMIX_ISPDWP, false),
};

static struct imx_noc_setting noc_setting[] = {
	{MLMIX, 0x180, 0x180, 0x80000303, 0x0, 0x0},
	{AUDIOMIX, 0x200, 0x200, 0x80000303, 0x0, 0x0},
	{AUDIOMIX, 0x280, 0x480, 0x80000404, 0x0, 0x0},
	{GPUMIX, 0x500, 0x580, 0x80000303, 0x0, 0x0},
	{HDMIMIX, 0x600, 0x680, 0x80000202, 0x0, 0x1},
	{HDMIMIX, 0x700, 0x700, 0x80000505, 0x0, 0x0},
	{HSIOMIX, 0x780, 0x900, 0x80000303, 0x0, 0x0},
	{MEDIAMIX, 0x980, 0xb80, 0x80000202, 0x0, 0x1},
	{MEDIAMIX_ISPDWP, 0xc00, 0xd00, 0x80000505, 0x0, 0x0},
	{VPU_G1, 0xd80, 0xd80, 0x80000303, 0x0, 0x0},
	{VPU_G2, 0xe00, 0xe00, 0x80000303, 0x0, 0x0},
	{VPU_H1, 0xe80, 0xe80, 0x80000303, 0x0, 0x0}
};

static struct clk_setting hsiomix_clk[] = {
	{ 0x8380, 0x0, CCM_ROOT_SLICE },
	{ 0x44d0, 0x0, CCM_CCGR },
	{ 0x45c0, 0x0, CCM_CCGR },
};

static struct aipstz_cfg aipstz5[] = {
	{IMX_AIPSTZ5, 0x77777777, 0x77777777, .opacr = {0x0, 0x0, 0x0, 0x0, 0x0}, },
	{0},
};

static unsigned int pu_domain_status;

static void imx_noc_qos(unsigned int domain_id)
{
	unsigned int i;
	uint32_t hurry;

	if (domain_id == HDMIMIX) {
		mmio_write_32(IMX_HDMI_CTL_BASE + TX_CONTROL1, 0x22018);
		mmio_write_32(IMX_HDMI_CTL_BASE + TX_CONTROL1, 0x22010);

		/* set GPR to make lcdif read hurry level 0x7 */
		hurry = mmio_read_32(IMX_HDMI_CTL_BASE + TX_CONTROL0);
		hurry |= 0x00077000;
		mmio_write_32(IMX_HDMI_CTL_BASE + TX_CONTROL0, hurry);
	}

	if (domain_id == MEDIAMIX) {
		/* handle mediamix special */
		mmio_write_32(IMX_MEDIAMIX_CTL_BASE + RSTn_CSR, 0x1FFFFFF);
		mmio_write_32(IMX_MEDIAMIX_CTL_BASE + CLK_EN_CSR, 0x1FFFFFF);
		mmio_write_32(IMX_MEDIAMIX_CTL_BASE + RST_DIV, 0x40030000);

		/* set GPR to make lcdif read hurry level 0x7 */
		hurry = mmio_read_32(IMX_MEDIAMIX_CTL_BASE + LCDIF_ARCACHE_CTRL);
		hurry |= 0xfc00;
		mmio_write_32(IMX_MEDIAMIX_CTL_BASE + LCDIF_ARCACHE_CTRL, hurry);
		/* set GPR to make isi write hurry level 0x7 */
		hurry = mmio_read_32(IMX_MEDIAMIX_CTL_BASE + ISI_CACHE_CTRL);
		hurry |= 0x1ff00000;
		mmio_write_32(IMX_MEDIAMIX_CTL_BASE + ISI_CACHE_CTRL, hurry);
	}

	/* set MIX NoC */
	for (i = 0; i < ARRAY_SIZE(noc_setting); i++) {
		if (noc_setting[i].domain_id == domain_id) {
			udelay(50);
			uint32_t offset = noc_setting[i].start;

			while (offset <= noc_setting[i].end) {
				mmio_write_32(IMX_NOC_BASE + offset + 0x8, noc_setting[i].prioriy);
				mmio_write_32(IMX_NOC_BASE + offset + 0xc, noc_setting[i].mode);
				mmio_write_32(IMX_NOC_BASE + offset + 0x18, noc_setting[i].socket_qos_en);
				offset += 0x80;
			}
		}
	}
}

void imx_gpc_pm_domain_enable(uint32_t domain_id, bool on)
{
	struct imx_pwr_domain *pwr_domain = &pu_domains[domain_id];
	unsigned int i;

	/* validate the domain id */
	if (domain_id >= MAX_DOMAINS) {
		return;
	}

	if (domain_id == HSIOMIX) {
		for (i = 0; i < ARRAY_SIZE(hsiomix_clk); i++) {
			hsiomix_clk[i].val = mmio_read_32(IMX_CCM_BASE + hsiomix_clk[i].offset);
			mmio_setbits_32(IMX_CCM_BASE + hsiomix_clk[i].offset,
					hsiomix_clk[i].type == CCM_ROOT_SLICE ? BIT(28) : 0x3);
		}
	}

	if (on) {
		if (pwr_domain->need_sync) {
			pu_domain_status |= (1 << domain_id);
		}

		if (domain_id == HDMIMIX) {
			/* assert the reset */
			mmio_write_32(IMX_HDMI_CTL_BASE + RTX_RESET_CTL0, 0x0);
			/* enable all th function clock */
			mmio_write_32(IMX_HDMI_CTL_BASE + RTX_CLK_CTL0, 0xFFFFFFFF);
			mmio_write_32(IMX_HDMI_CTL_BASE + RTX_CLK_CTL1, 0x7ffff87e);
		}

		/* clear the PGC bit */
		mmio_clrbits_32(IMX_GPC_BASE + pwr_domain->pgc_offset, 0x1);

		/* power up the domain */
		mmio_setbits_32(IMX_GPC_BASE + PU_PGC_UP_TRG, pwr_domain->pwr_req);

		/* wait for power request done */
		while (mmio_read_32(IMX_GPC_BASE + PU_PGC_UP_TRG) & pwr_domain->pwr_req)
			;

		if (domain_id == HDMIMIX) {
			/* wait for memory repair done for HDMIMIX */
			while (!(mmio_read_32(IMX_SRC_BASE + 0x94) & BIT(8)))
				;
			/* disable all the function clock */
			mmio_write_32(IMX_HDMI_CTL_BASE + RTX_CLK_CTL0, 0x0);
			mmio_write_32(IMX_HDMI_CTL_BASE + RTX_CLK_CTL1, 0x0);
			/* deassert the reset */
			mmio_write_32(IMX_HDMI_CTL_BASE + RTX_RESET_CTL0, 0xffffffff);
			/* enable all the clock again */
			mmio_write_32(IMX_HDMI_CTL_BASE + RTX_CLK_CTL0, 0xFFFFFFFF);
			mmio_write_32(IMX_HDMI_CTL_BASE + RTX_CLK_CTL1, 0x7ffff87e);
		}

		if (domain_id == HSIOMIX) {
			/* enable HSIOMIX clock */
			mmio_write_32(IMX_HSIOMIX_CTL_BASE, 0x2);
		}

		/* handle the ADB400 sync */
		if (pwr_domain->need_sync) {
			/* clear adb power down request */
			mmio_setbits_32(IMX_GPC_BASE + GPC_PU_PWRHSK, pwr_domain->adb400_sync);

			/* wait for adb power request ack */
			while (!(mmio_read_32(IMX_GPC_BASE + GPC_PU_PWRHSK) & pwr_domain->adb400_ack))
				;
		}

		imx_noc_qos(domain_id);

		/* AIPS5 config is lost when audiomix is off, so need to re-init it */
		if (domain_id == AUDIOMIX) {
			imx_aipstz_init(aipstz5);
		}
	} else {
		if (pwr_domain->always_on) {
			return;
		}

		if (pwr_domain->need_sync) {
			pu_domain_status &= ~(1 << domain_id);
		}

		/* handle the ADB400 sync */
		if (pwr_domain->need_sync) {
			/* set adb power down request */
			mmio_clrbits_32(IMX_GPC_BASE + GPC_PU_PWRHSK, pwr_domain->adb400_sync);

			/* wait for adb power request ack */
			while ((mmio_read_32(IMX_GPC_BASE + GPC_PU_PWRHSK) & pwr_domain->adb400_ack))
				;
		}

		/* set the PGC bit */
		mmio_setbits_32(IMX_GPC_BASE + pwr_domain->pgc_offset, 0x1);

		/*
		 * leave the G1, G2, H1 power domain on until VPUMIX power off,
		 * otherwise system will hang due to VPUMIX ACK
		 */
		if (domain_id == VPU_H1 || domain_id == VPU_G1 || domain_id == VPU_G2) {
			return;
		}

		if (domain_id == VPUMIX) {
			mmio_write_32(IMX_GPC_BASE + PU_PGC_DN_TRG, VPU_G1_PWR_REQ |
				 VPU_G2_PWR_REQ | VPU_H1_PWR_REQ);

			while (mmio_read_32(IMX_GPC_BASE + PU_PGC_DN_TRG) & (VPU_G1_PWR_REQ |
					VPU_G2_PWR_REQ | VPU_H1_PWR_REQ))
				;
		}

		/* power down the domain */
		mmio_setbits_32(IMX_GPC_BASE + PU_PGC_DN_TRG, pwr_domain->pwr_req);

		/* wait for power request done */
		while (mmio_read_32(IMX_GPC_BASE + PU_PGC_DN_TRG) & pwr_domain->pwr_req)
			;

		if (domain_id == HDMIMIX) {
			/* disable all the clocks of HDMIMIX */
			mmio_write_32(IMX_HDMI_CTL_BASE + 0x40, 0x0);
			mmio_write_32(IMX_HDMI_CTL_BASE + 0x50, 0x0);
		}
	}

	if (domain_id == HSIOMIX) {
		for (i = 0; i < ARRAY_SIZE(hsiomix_clk); i++) {
			mmio_write_32(IMX_CCM_BASE + hsiomix_clk[i].offset, hsiomix_clk[i].val);
		}
	}
}

void imx_gpc_init(void)
{
	uint32_t val;
	unsigned int i;

	/* mask all the wakeup irq by default */
	for (i = 0; i < IMR_NUM; i++) {
		mmio_write_32(IMX_GPC_BASE + IMR1_CORE0_A53 + i * 4, ~0x0);
		mmio_write_32(IMX_GPC_BASE + IMR1_CORE1_A53 + i * 4, ~0x0);
		mmio_write_32(IMX_GPC_BASE + IMR1_CORE2_A53 + i * 4, ~0x0);
		mmio_write_32(IMX_GPC_BASE + IMR1_CORE3_A53 + i * 4, ~0x0);
		mmio_write_32(IMX_GPC_BASE + IMR1_CORE0_M4 + i * 4, ~0x0);
	}

	val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_BSC);
	/* use GIC wake_request to wakeup C0~C3 from LPM */
	val |= CORE_WKUP_FROM_GIC;
	/* clear the MASTER0 LPM handshake */
	val &= ~MASTER0_LPM_HSK;
	mmio_write_32(IMX_GPC_BASE + LPCR_A53_BSC, val);

	/* clear MASTER1 & MASTER2 mapping in CPU0(A53) */
	mmio_clrbits_32(IMX_GPC_BASE + MST_CPU_MAPPING, (MASTER1_MAPPING |
		MASTER2_MAPPING));

	/* set all mix/PU in A53 domain */
	mmio_write_32(IMX_GPC_BASE + PGC_CPU_0_1_MAPPING, 0x3fffff);

	/*
	 * Set the CORE & SCU power up timing:
	 * SW = 0x1, SW2ISO = 0x1;
	 * the CPU CORE and SCU power up timming counter
	 * is drived  by 32K OSC, each domain's power up
	 * latency is (SW + SW2ISO) / 32768
	 */
	mmio_write_32(IMX_GPC_BASE + COREx_PGC_PCR(0) + 0x4, 0x401);
	mmio_write_32(IMX_GPC_BASE + COREx_PGC_PCR(1) + 0x4, 0x401);
	mmio_write_32(IMX_GPC_BASE + COREx_PGC_PCR(2) + 0x4, 0x401);
	mmio_write_32(IMX_GPC_BASE + COREx_PGC_PCR(3) + 0x4, 0x401);
	mmio_write_32(IMX_GPC_BASE + PLAT_PGC_PCR + 0x4, 0x401);
	mmio_write_32(IMX_GPC_BASE + PGC_SCU_TIMING,
		      (0x59 << TMC_TMR_SHIFT) | 0x5B | (0x2 << TRC1_TMC_SHIFT));

	/* set DUMMY PDN/PUP ACK by default for A53 domain */
	mmio_write_32(IMX_GPC_BASE + PGC_ACK_SEL_A53,
		      A53_DUMMY_PUP_ACK | A53_DUMMY_PDN_ACK);

	/* clear DSM by default */
	val = mmio_read_32(IMX_GPC_BASE + SLPCR);
	val &= ~SLPCR_EN_DSM;
	/* enable the fast wakeup wait/stop mode */
	val |= SLPCR_A53_FASTWUP_WAIT_MODE;
	val |= SLPCR_A53_FASTWUP_STOP_MODE;
	/* clear the RBC */
	val &= ~(0x3f << SLPCR_RBC_COUNT_SHIFT);
	/* set the STBY_COUNT to 0x5, (128 * 30)us */
	val &= ~(0x7 << SLPCR_STBY_COUNT_SHFT);
	val |= (0x5 << SLPCR_STBY_COUNT_SHFT);
	mmio_write_32(IMX_GPC_BASE + SLPCR, val);

	/*
	 * USB PHY power up needs to make sure RESET bit in SRC is clear,
	 * otherwise, the PU power up bit in GPC will NOT self-cleared.
	 * only need to do it once.
	 */
	mmio_clrbits_32(IMX_SRC_BASE + SRC_OTG1PHY_SCR, 0x1);
	mmio_clrbits_32(IMX_SRC_BASE + SRC_OTG2PHY_SCR, 0x1);

	/* enable all the power domain by default */
	for (i = 0; i < 101; i++) {
		mmio_write_32(IMX_CCM_BASE + CCGR(i), 0x3);
	}

	for (i = 0; i < 20; i++) {
		imx_gpc_pm_domain_enable(i, true);
	}
}
