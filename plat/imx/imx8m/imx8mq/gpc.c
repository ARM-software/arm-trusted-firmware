/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <platform_def.h>
#include <services/std_svc.h>

#include <gpc.h>

/* use wfi power down the core */
void imx_set_cpu_pwr_off(unsigned int core_id)
{
	/* enable the wfi power down of the core */
	mmio_setbits_32(IMX_GPC_BASE + LPCR_A53_AD, COREx_WFI_PDN(core_id) |
			(1 << (core_id + 20)));
	/* assert the pcg pcr bit of the core */
	mmio_setbits_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), 0x1);
};

/* if out of lpm, we need to do reverse steps */
void imx_set_cpu_lpm(unsigned int core_id, bool pdn)
{
	if (pdn) {
		/* enable the core WFI PDN & IRQ PUP */
		mmio_setbits_32(IMX_GPC_BASE + LPCR_A53_AD, COREx_WFI_PDN(core_id) |
				(1 << (core_id + 20)) | COREx_IRQ_WUP(core_id));
		/* assert the pcg pcr bit of the core */
		mmio_setbits_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), 0x1);
	} else {
		/* disable CORE WFI PDN & IRQ PUP */
		mmio_clrbits_32(IMX_GPC_BASE + LPCR_A53_AD, COREx_WFI_PDN(core_id) |
				COREx_IRQ_WUP(core_id));
		/* deassert the pcg pcr bit of the core */
		mmio_setbits_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), 0x1);
	}
}

void imx_pup_pdn_slot_config(int last_core, bool pdn)
{
	if (pdn) {
		/* SLOT0 for A53 PLAT power down */
		mmio_setbits_32(IMX_GPC_BASE + SLTx_CFG(0), SLT_PLAT_PDN);
		/* SLOT1 for A53 PLAT power up */
		mmio_setbits_32(IMX_GPC_BASE + SLTx_CFG(1), SLT_PLAT_PUP);
		/* SLOT2 for A53 primary core power up */
		mmio_setbits_32(IMX_GPC_BASE + SLTx_CFG(2), SLT_COREx_PUP(last_core));
		/* ACK setting: PLAT ACK for PDN, CORE ACK for PUP */
		mmio_clrsetbits_32(IMX_GPC_BASE + PGC_ACK_SEL_A53, 0xFFFFFFFF,
			A53_PLAT_PDN_ACK | A53_PLAT_PUP_ACK);
	} else {
		mmio_clrbits_32(IMX_GPC_BASE + SLTx_CFG(0), 0xFFFFFFFF);
		mmio_clrbits_32(IMX_GPC_BASE + SLTx_CFG(1), 0xFFFFFFFF);
		mmio_clrbits_32(IMX_GPC_BASE + SLTx_CFG(2), 0xFFFFFFFF);
		mmio_clrsetbits_32(IMX_GPC_BASE + PGC_ACK_SEL_A53, 0xFFFFFFFF,
			A53_DUMMY_PDN_ACK | A53_DUMMY_PUP_ACK);
	}
}

void imx_set_cluster_powerdown(unsigned int last_core, uint8_t power_state)
{
	uint32_t val;

	if (is_local_state_off(power_state)) {
		val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_BSC);
		val |= A53_LPM_STOP; /* enable C0-C1's STOP mode */
		val &= ~CPU_CLOCK_ON_LPM; /* disable CPU clock in LPM mode */
		mmio_write_32(IMX_GPC_BASE + LPCR_A53_BSC, val);

		/* enable C2-3's STOP mode */
		mmio_setbits_32(IMX_GPC_BASE + LPCR_A53_BSC2, A53_LPM_STOP);

		/* enable PLAT/SCU power down */
		val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_AD);
		val &= ~EN_L2_WFI_PDN;
		val |= L2PGE | EN_PLAT_PDN;
		val &= ~COREx_IRQ_WUP(last_core); /* disable IRQ PUP for last core */
		val |= COREx_LPM_PUP(last_core); /* enable LPM PUP for last core */
		mmio_write_32(IMX_GPC_BASE + LPCR_A53_AD, val);

		imx_pup_pdn_slot_config(last_core, true);

		/* enable PLAT PGC */
		mmio_setbits_32(IMX_GPC_BASE + A53_PLAT_PGC, 0x1);
	} else {
		/* clear PLAT PGC */
		mmio_clrbits_32(IMX_GPC_BASE + A53_PLAT_PGC, 0x1);

		/* clear the slot and ack for cluster power down */
		imx_pup_pdn_slot_config(last_core, false);

		val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_BSC);
		val &= ~A53_LPM_MASK; /* clear the C0~1 LPM */
		val |= CPU_CLOCK_ON_LPM; /* disable cpu clock in LPM */
		mmio_write_32(IMX_GPC_BASE + LPCR_A53_BSC, val);

		/* set A53 LPM to RUN mode */
		mmio_clrbits_32(IMX_GPC_BASE + LPCR_A53_BSC2, A53_LPM_MASK);

		/* clear PLAT/SCU power down */
		val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_AD);
		val |= EN_L2_WFI_PDN;
		val &= ~(L2PGE | EN_PLAT_PDN);
		val &= ~COREx_LPM_PUP(last_core);  /* disable C0's LPM PUP */
		mmio_write_32(IMX_GPC_BASE + LPCR_A53_AD, val);
	}
}

void imx_gpc_init(void)
{
	uint32_t val;
	int i;
	/* mask all the interrupt by default */
	for (i = 0; i < 4; i++) {
		mmio_write_32(IMX_GPC_BASE + IMR1_CORE0_A53 + i * 4, ~0x0);
		mmio_write_32(IMX_GPC_BASE + IMR1_CORE1_A53 + i * 4, ~0x0);
		mmio_write_32(IMX_GPC_BASE + IMR1_CORE2_A53 + i * 4, ~0x0);
		mmio_write_32(IMX_GPC_BASE + IMR1_CORE3_A53 + i * 4, ~0x0);
		mmio_write_32(IMX_GPC_BASE + IMR1_CORE0_M4 + i * 4, ~0x0);
	}
	/* Due to the hardware design requirement, need to make
	 * sure GPR interrupt(#32) is unmasked during RUN mode to
	 * avoid entering DSM mode by mistake.
	 */
	mmio_write_32(IMX_GPC_BASE + IMR1_CORE0_A53, 0xFFFFFFFE);
	mmio_write_32(IMX_GPC_BASE + IMR1_CORE1_A53, 0xFFFFFFFE);
	mmio_write_32(IMX_GPC_BASE + IMR1_CORE2_A53, 0xFFFFFFFE);
	mmio_write_32(IMX_GPC_BASE + IMR1_CORE3_A53, 0xFFFFFFFE);

	/* use external IRQs to wakeup C0~C3 from LPM */
	val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_BSC);
	val |= IRQ_SRC_A53_WUP;
	/* clear the MASTER0 LPM handshake */
	val &= ~MASTER0_LPM_HSK;
	mmio_write_32(IMX_GPC_BASE + LPCR_A53_BSC, val);

	/* mask M4 DSM trigger if M4 is NOT enabled */
	mmio_setbits_32(IMX_GPC_BASE + LPCR_M4, DSM_MODE_MASK);

	/* set all mix/PU in A53 domain */
	mmio_write_32(IMX_GPC_BASE + PGC_CPU_0_1_MAPPING, 0xfffd);

	/* set SCU timming */
	mmio_write_32(IMX_GPC_BASE + PGC_SCU_TIMING,
		      (0x59 << 10) | 0x5B | (0x2 << 20));

	/* set DUMMY PDN/PUP ACK by default for A53 domain */
	mmio_write_32(IMX_GPC_BASE + PGC_ACK_SEL_A53, A53_DUMMY_PUP_ACK |
		A53_DUMMY_PDN_ACK);

	/* disable DSM mode by default */
	mmio_clrbits_32(IMX_GPC_BASE + SLPCR, DSM_MODE_MASK);

	/*
	 * USB PHY power up needs to make sure RESET bit in SRC is clear,
	 * otherwise, the PU power up bit in GPC will NOT self-cleared.
	 * only need to do it once.
	 */
	mmio_clrbits_32(IMX_SRC_BASE + SRC_OTG1PHY_SCR, 0x1);
	mmio_clrbits_32(IMX_SRC_BASE + SRC_OTG2PHY_SCR, 0x1);

	/* enable all the power domain by default */
	mmio_write_32(IMX_GPC_BASE + PU_PGC_UP_TRG, 0x3fcf);
}
