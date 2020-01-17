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
#include <imx_sip_svc.h>
#include <platform_def.h>

#define CCGR(x)		(0x4000 + (x) * 0x10)

#define MIPI_PWR_REQ		BIT(0)
#define OTG1_PWR_REQ		BIT(2)
#define HSIOMIX_PWR_REQ		BIT(4)
#define GPUMIX_PWR_REQ		BIT(7)
#define DISPMIX_PWR_REQ		BIT(10)

#define HSIOMIX_ADB400_SYNC	BIT(5)
#define DISPMIX_ADB400_SYNC	BIT(7)
#define GPUMIX_ADB400_SYNC	(0x5 << 9)
#define HSIOMIX_ADB400_ACK	BIT(23)
#define DISPMIX_ADB400_ACK	BIT(25)
#define GPUMIX_ADB400_ACK	(0x5 << 27)

#define MIPI_PGC		0xc00
#define OTG1_PGC		0xc80
#define HSIOMIX_PGC	        0xd00
#define GPUMIX_PGC		0xdc0
#define DISPMIX_PGC		0xe80

enum pu_domain_id {
	HSIOMIX,
	OTG1 = 2,
	GPUMIX = 4,
	DISPMIX = 9,
	MIPI,
};

/* PU domain, add some hole to minimize the uboot change */
static struct imx_pwr_domain pu_domains[11] = {
	[HSIOMIX] = IMX_MIX_DOMAIN(HSIOMIX, false),
	[OTG1] = IMX_PD_DOMAIN(OTG1, true),
	[GPUMIX] = IMX_MIX_DOMAIN(GPUMIX, false),
	[DISPMIX] = IMX_MIX_DOMAIN(DISPMIX, false),
	[MIPI] = IMX_PD_DOMAIN(MIPI, true),
};

static unsigned int pu_domain_status;

void imx_gpc_pm_domain_enable(uint32_t domain_id, bool on)
{
	if (domain_id > MIPI) {
		return;
	}

	struct imx_pwr_domain *pwr_domain = &pu_domains[domain_id];

	if (on) {
		if (pwr_domain->need_sync) {
			pu_domain_status |= (1 << domain_id);
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

		if (domain_id == DISPMIX) {
			/* de-reset bus_blk clk and
			 * enable bus_blk clk
			 */
			mmio_write_32(0x32e28000, 0x100);
			mmio_write_32(0x32e28004, 0x100);
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
	} else {
		pu_domain_status &= ~(1 << domain_id);

		if (domain_id == OTG1) {
			return;
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
	val |= CORE_WKUP_FROM_GIC;
	/* clear the MASTER0 LPM handshake */
	val &= ~MASTER0_LPM_HSK;
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
}
