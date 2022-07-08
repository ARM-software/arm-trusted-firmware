/*
 * Copyright (c) 2019-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <lib/smccc.h>
#include <platform_def.h>
#include <services/std_svc.h>

#include <gpc.h>
#include <imx_sip_svc.h>

#define MIPI_PWR_REQ		BIT(0)
#define PCIE_PWR_REQ		BIT(1)
#define OTG1_PWR_REQ		BIT(2)
#define OTG2_PWR_REQ		BIT(3)
#define HSIOMIX_PWR_REQ		BIT(4)
#define GPU2D_PWR_REQ		BIT(6)
#define GPUMIX_PWR_REQ		BIT(7)
#define VPUMIX_PWR_REQ		BIT(8)
#define GPU3D_PWR_REQ		BIT(9)
#define DISPMIX_PWR_REQ		BIT(10)
#define VPU_G1_PWR_REQ		BIT(11)
#define VPU_G2_PWR_REQ		BIT(12)
#define VPU_H1_PWR_REQ		BIT(13)

#define HSIOMIX_ADB400_SYNC	(0x3 << 5)
#define DISPMIX_ADB400_SYNC	BIT(7)
#define VPUMIX_ADB400_SYNC	BIT(8)
#define GPU3D_ADB400_SYNC	BIT(9)
#define GPU2D_ADB400_SYNC	BIT(10)
#define GPUMIX_ADB400_SYNC	BIT(11)
#define HSIOMIX_ADB400_ACK	(0x3 << 23)
#define DISPMIX_ADB400_ACK	BIT(25)
#define VPUMIX_ADB400_ACK	BIT(26)
#define GPU3D_ADB400_ACK	BIT(27)
#define GPU2D_ADB400_ACK	BIT(28)
#define GPUMIX_ADB400_ACK	BIT(29)

#define MIPI_PGC		0xc00
#define PCIE_PGC		0xc40
#define OTG1_PGC		0xc80
#define OTG2_PGC		0xcc0
#define HSIOMIX_PGC	        0xd00
#define GPU2D_PGC		0xd80
#define GPUMIX_PGC		0xdc0
#define VPUMIX_PGC		0xe00
#define GPU3D_PGC		0xe40
#define DISPMIX_PGC		0xe80
#define VPU_G1_PGC		0xec0
#define VPU_G2_PGC		0xf00
#define VPU_H1_PGC		0xf40

enum pu_domain_id {
	HSIOMIX,
	PCIE,
	OTG1,
	OTG2,
	GPUMIX,
	VPUMIX,
	VPU_G1,
	VPU_G2,
	VPU_H1,
	DISPMIX,
	MIPI,
	/* below two domain only for ATF internal use */
	GPU2D,
	GPU3D,
	MAX_DOMAINS,
};

/* PU domain */
static struct imx_pwr_domain pu_domains[] = {
	IMX_MIX_DOMAIN(HSIOMIX, false),
	IMX_PD_DOMAIN(PCIE, false),
	IMX_PD_DOMAIN(OTG1, true),
	IMX_PD_DOMAIN(OTG2, true),
	IMX_MIX_DOMAIN(GPUMIX, false),
	IMX_MIX_DOMAIN(VPUMIX, false),
	IMX_PD_DOMAIN(VPU_G1, false),
	IMX_PD_DOMAIN(VPU_G2, false),
	IMX_PD_DOMAIN(VPU_H1, false),
	IMX_MIX_DOMAIN(DISPMIX, false),
	IMX_PD_DOMAIN(MIPI, false),
	/* below two domain only for ATF internal use */
	IMX_MIX_DOMAIN(GPU2D, false),
	IMX_MIX_DOMAIN(GPU3D, false),
};

static unsigned int pu_domain_status;

#define GPU_RCR		0x40
#define VPU_RCR		0x44

#define VPU_CTL_BASE		0x38330000
#define BLK_SFT_RSTN_CSR	0x0
#define H1_SFT_RSTN		BIT(2)
#define G1_SFT_RSTN		BIT(1)
#define G2_SFT_RSTN		BIT(0)

#define DISP_CTL_BASE		0x32e28000

void vpu_sft_reset_assert(uint32_t domain_id)
{
	uint32_t val;

	val = mmio_read_32(VPU_CTL_BASE + BLK_SFT_RSTN_CSR);

	switch (domain_id) {
	case VPU_G1:
		val &= ~G1_SFT_RSTN;
		mmio_write_32(VPU_CTL_BASE + BLK_SFT_RSTN_CSR, val);
		break;
	case VPU_G2:
		val &= ~G2_SFT_RSTN;
		mmio_write_32(VPU_CTL_BASE + BLK_SFT_RSTN_CSR, val);
		break;
	case VPU_H1:
		val &= ~H1_SFT_RSTN;
		mmio_write_32(VPU_CTL_BASE + BLK_SFT_RSTN_CSR, val);
		break;
	default:
		break;
	}
}

void vpu_sft_reset_deassert(uint32_t domain_id)
{
	uint32_t val;

	val = mmio_read_32(VPU_CTL_BASE + BLK_SFT_RSTN_CSR);

	switch (domain_id) {
	case VPU_G1:
		val |= G1_SFT_RSTN;
		mmio_write_32(VPU_CTL_BASE + BLK_SFT_RSTN_CSR, val);
		break;
	case VPU_G2:
		val |= G2_SFT_RSTN;
		mmio_write_32(VPU_CTL_BASE + BLK_SFT_RSTN_CSR, val);
		break;
	case VPU_H1:
		val |= H1_SFT_RSTN;
		mmio_write_32(VPU_CTL_BASE + BLK_SFT_RSTN_CSR, val);
		break;
	default:
		break;
	}
}

void imx_gpc_pm_domain_enable(uint32_t domain_id, bool on)
{
	if (domain_id >= MAX_DOMAINS) {
		return;
	}

	struct imx_pwr_domain *pwr_domain = &pu_domains[domain_id];

	if (on) {
		pu_domain_status |= (1 << domain_id);

		if (domain_id == VPU_G1 || domain_id == VPU_G2 ||
		    domain_id == VPU_H1) {
			vpu_sft_reset_assert(domain_id);
		}

		/* HSIOMIX has no PU bit, so skip for it */
		if (domain_id != HSIOMIX) {
			/* clear the PGC bit */
			mmio_clrbits_32(IMX_GPC_BASE + pwr_domain->pgc_offset, 0x1);

			/* power up the domain */
			mmio_setbits_32(IMX_GPC_BASE + PU_PGC_UP_TRG, pwr_domain->pwr_req);

			/* wait for power request done */
			while (mmio_read_32(IMX_GPC_BASE + PU_PGC_UP_TRG) & pwr_domain->pwr_req) {
				;
			}
		}

		if (domain_id == VPU_G1 || domain_id == VPU_G2 ||
		    domain_id == VPU_H1) {
			vpu_sft_reset_deassert(domain_id);
			/* dealy for a while to make sure reset done */
			udelay(100);
		}

		if (domain_id == GPUMIX) {
			/* assert reset */
			mmio_write_32(IMX_SRC_BASE + GPU_RCR, 0x1);

			/* power up GPU2D */
			mmio_clrbits_32(IMX_GPC_BASE + GPU2D_PGC, 0x1);

			mmio_setbits_32(IMX_GPC_BASE + PU_PGC_UP_TRG, GPU2D_PWR_REQ);

			/* wait for power request done */
			while (mmio_read_32(IMX_GPC_BASE + PU_PGC_UP_TRG) & GPU2D_PWR_REQ) {
				;
			}

			udelay(1);

			/* power up GPU3D */
			mmio_clrbits_32(IMX_GPC_BASE + GPU3D_PGC, 0x1);

			mmio_setbits_32(IMX_GPC_BASE + PU_PGC_UP_TRG, GPU3D_PWR_REQ);

			/* wait for power request done */
			while (mmio_read_32(IMX_GPC_BASE + PU_PGC_UP_TRG) & GPU3D_PWR_REQ) {
				;
			}

			udelay(10);
			/* release the gpumix reset */
			mmio_write_32(IMX_SRC_BASE + GPU_RCR, 0x0);
			udelay(10);
		}

		/* vpu sft clock enable */
		if (domain_id == VPUMIX) {
			mmio_write_32(IMX_SRC_BASE + VPU_RCR, 0x1);
			udelay(5);
			mmio_write_32(IMX_SRC_BASE + VPU_RCR, 0x0);
			udelay(5);

			/* enable all clock */
			mmio_write_32(VPU_CTL_BASE + 0x4, 0x7);
		}

		if (domain_id == DISPMIX) {
			/* special setting for DISPMIX */
			mmio_write_32(DISP_CTL_BASE + 0x4, 0x1fff);
			mmio_write_32(DISP_CTL_BASE, 0x7f);
			mmio_write_32(DISP_CTL_BASE + 0x8, 0x30000);
		}

		/* handle the ADB400 sync */
		if (pwr_domain->need_sync) {
			/* clear adb power down request */
			mmio_setbits_32(IMX_GPC_BASE + GPC_PU_PWRHSK, pwr_domain->adb400_sync);

			/* wait for adb power request ack */
			while (!(mmio_read_32(IMX_GPC_BASE + GPC_PU_PWRHSK) & pwr_domain->adb400_ack)) {
				;
			}
		}

		if (domain_id == GPUMIX) {
			/* power up GPU2D ADB */
			mmio_setbits_32(IMX_GPC_BASE + GPC_PU_PWRHSK, GPU2D_ADB400_SYNC);

			/* wait for adb power request ack */
			while (!(mmio_read_32(IMX_GPC_BASE + GPC_PU_PWRHSK) & GPU2D_ADB400_ACK)) {
				;
			}

			/* power up GPU3D ADB */
			mmio_setbits_32(IMX_GPC_BASE + GPC_PU_PWRHSK, GPU3D_ADB400_SYNC);

			/* wait for adb power request ack */
			while (!(mmio_read_32(IMX_GPC_BASE + GPC_PU_PWRHSK) & GPU3D_ADB400_ACK)) {
				;
			}
		}
	} else {
		pu_domain_status &= ~(1 << domain_id);

		if (domain_id == OTG1 || domain_id == OTG2) {
			return;
		}

		/* GPU2D & GPU3D ADB power down */
		if (domain_id == GPUMIX) {
			mmio_clrbits_32(IMX_GPC_BASE + GPC_PU_PWRHSK, GPU2D_ADB400_SYNC);

			/* wait for adb power request ack */
			while ((mmio_read_32(IMX_GPC_BASE + GPC_PU_PWRHSK) & GPU2D_ADB400_ACK)) {
				;
			}

			mmio_clrbits_32(IMX_GPC_BASE + GPC_PU_PWRHSK, GPU3D_ADB400_SYNC);

				/* wait for adb power request ack */
			while ((mmio_read_32(IMX_GPC_BASE + GPC_PU_PWRHSK) & GPU3D_ADB400_ACK)) {
				;
			}
		}

		/* handle the ADB400 sync */
		if (pwr_domain->need_sync) {
			/* set adb power down request */
			mmio_clrbits_32(IMX_GPC_BASE + GPC_PU_PWRHSK, pwr_domain->adb400_sync);

			/* wait for adb power request ack */
			while ((mmio_read_32(IMX_GPC_BASE + GPC_PU_PWRHSK) & pwr_domain->adb400_ack)) {
				;
			}
		}

		if (domain_id == GPUMIX) {
			/* power down GPU2D */
			mmio_setbits_32(IMX_GPC_BASE + GPU2D_PGC, 0x1);

			mmio_setbits_32(IMX_GPC_BASE + PU_PGC_DN_TRG, GPU2D_PWR_REQ);

			/* wait for power request done */
			while (mmio_read_32(IMX_GPC_BASE + PU_PGC_DN_TRG) & GPU2D_PWR_REQ) {
				;
			}

			/* power down GPU3D */
			mmio_setbits_32(IMX_GPC_BASE + GPU3D_PGC, 0x1);

			mmio_setbits_32(IMX_GPC_BASE + PU_PGC_DN_TRG, GPU3D_PWR_REQ);

			/* wait for power request done */
			while (mmio_read_32(IMX_GPC_BASE + PU_PGC_DN_TRG) & GPU3D_PWR_REQ) {
				;
			}
		}

		/* HSIOMIX has no PU bit, so skip for it */
		if (domain_id != HSIOMIX) {
			/* set the PGC bit */
			mmio_setbits_32(IMX_GPC_BASE + pwr_domain->pgc_offset, 0x1);

			/* power down the domain */
			mmio_setbits_32(IMX_GPC_BASE + PU_PGC_DN_TRG, pwr_domain->pwr_req);

			/* wait for power request done */
			while (mmio_read_32(IMX_GPC_BASE + PU_PGC_DN_TRG) & pwr_domain->pwr_req) {
				;
			}
		}
	}
}

void imx_gpc_init(void)
{
	unsigned int val;
	int i;

	/* mask all the wakeup irq by default */
	for (i = 0; i < 4; i++) {
		mmio_write_32(IMX_GPC_BASE + IMR1_CORE0_A53 + i * 4, ~0x0);
		mmio_write_32(IMX_GPC_BASE + IMR1_CORE1_A53 + i * 4, ~0x0);
		mmio_write_32(IMX_GPC_BASE + IMR1_CORE2_A53 + i * 4, ~0x0);
		mmio_write_32(IMX_GPC_BASE + IMR1_CORE3_A53 + i * 4, ~0x0);
		mmio_write_32(IMX_GPC_BASE + IMR1_CORE0_M4 + i * 4, ~0x0);
	}

	val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_BSC);
	/* use GIC wake_request to wakeup C0~C3 from LPM */
	val |= 0x30c00000;
	/* clear the MASTER0 LPM handshake */
	val &= ~(1 << 6);
	mmio_write_32(IMX_GPC_BASE + LPCR_A53_BSC, val);

	/* clear MASTER1 & MASTER2 mapping in CPU0(A53) */
	mmio_clrbits_32(IMX_GPC_BASE + MST_CPU_MAPPING, (MASTER1_MAPPING |
		MASTER2_MAPPING));

	/* set all mix/PU in A53 domain */
	mmio_write_32(IMX_GPC_BASE + PGC_CPU_0_1_MAPPING, 0xffff);

	/*
	 * Set the CORE & SCU power up timing:
	 * SW = 0x1, SW2ISO = 0x1;
	 * the CPU CORE and SCU power up timming counter
	 * is drived  by 32K OSC, each domain's power up
	 * latency is (SW + SW2ISO) / 32768
	 */
	mmio_write_32(IMX_GPC_BASE + COREx_PGC_PCR(0) + 0x4, 0x81);
	mmio_write_32(IMX_GPC_BASE + COREx_PGC_PCR(1) + 0x4, 0x81);
	mmio_write_32(IMX_GPC_BASE + COREx_PGC_PCR(2) + 0x4, 0x81);
	mmio_write_32(IMX_GPC_BASE + COREx_PGC_PCR(3) + 0x4, 0x81);
	mmio_write_32(IMX_GPC_BASE + PLAT_PGC_PCR + 0x4, 0x81);
	mmio_write_32(IMX_GPC_BASE + PGC_SCU_TIMING,
		      (0x59 << 10) | 0x5B | (0x2 << 20));

	/* set DUMMY PDN/PUP ACK by default for A53 domain */
	mmio_write_32(IMX_GPC_BASE + PGC_ACK_SEL_A53,
		      A53_DUMMY_PUP_ACK | A53_DUMMY_PDN_ACK);

	/* clear DSM by default */
	val = mmio_read_32(IMX_GPC_BASE + SLPCR);
	val &= ~SLPCR_EN_DSM;
	/* enable the fast wakeup wait mode */
	val |= SLPCR_A53_FASTWUP_WAIT_MODE;
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
}
