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

void imx_set_cpu_secure_entry(unsigned int core_id, uintptr_t sec_entrypoint)
{
	uint64_t temp_base;

	temp_base = (uint64_t) sec_entrypoint;
	temp_base >>= 2;

	mmio_write_32(IMX_SRC_BASE + SRC_GPR1_OFFSET + (core_id << 3),
		((uint32_t)(temp_base >> 22) & 0xffff));
	mmio_write_32(IMX_SRC_BASE + SRC_GPR1_OFFSET + (core_id << 3) + 4,
		((uint32_t)temp_base & 0x003fffff));
}

/* use wfi power down the core */
void imx_set_cpu_pwr_off(unsigned int core_id)
{
	/* enable the wfi power down of the core */
	mmio_setbits_32(IMX_GPC_BASE + LPCR_A53_AD, COREx_WFI_PDN(core_id) |
			(1 << (core_id + 20)));
	/* assert the pcg pcr bit of the core */
	mmio_setbits_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), 0x1);
};

/* use the sw method to power up the core */
void imx_set_cpu_pwr_on(unsigned int core_id)
{
	/* clear the wfi power down bit of the core */
	mmio_clrbits_32(IMX_GPC_BASE + LPCR_A53_AD, COREx_WFI_PDN(core_id));
	/* assert the ncpuporeset */
	mmio_clrbits_32(IMX_SRC_BASE + SRC_A53RCR1, (1 << core_id));
	/* assert the pcg pcr bit of the core */
	mmio_setbits_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), 0x1);
	/* sw power up the core */
	mmio_setbits_32(IMX_GPC_BASE + CPU_PGC_UP_TRG, (1 << core_id));

	/* wait for the power up finished */
	while ((mmio_read_32(IMX_GPC_BASE + CPU_PGC_UP_TRG) & (1 << core_id)) != 0)
		;

	/* deassert the pcg pcr bit of the core */
	mmio_clrbits_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), 0x1);
	/* deassert the ncpuporeset */
	mmio_setbits_32(IMX_SRC_BASE + SRC_A53RCR1, (1 << core_id));
}

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

void imx_set_sys_wakeup(unsigned int last_core, bool pdn)
{
	/* TODO */
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

void imx_set_cluster_standby(bool retention)
{
	/*
	 * Enable BIT 6 of A53 AD register to make sure system
	 * don't enter LPM mode.
	 */
	if (retention)
		mmio_setbits_32(IMX_GPC_BASE + LPCR_A53_AD, (1 << 6));
	else
		mmio_clrbits_32(IMX_GPC_BASE + LPCR_A53_AD, (1 << 6));
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

/* config the system level power mode */
void imx_set_sys_lpm(bool retention)
{
	uint32_t val;

	/* set system DSM mode SLPCR(0x14) */
	val = mmio_read_32(IMX_GPC_BASE + SLPCR);
	val &= ~(SLPCR_EN_DSM | SLPCR_VSTBY | SLPCR_SBYOS |
		 SLPCR_BYPASS_PMIC_READY | SLPCR_RBC_EN);

	if (retention)
		val |= (SLPCR_EN_DSM | SLPCR_VSTBY | SLPCR_SBYOS |
			 SLPCR_BYPASS_PMIC_READY | SLPCR_RBC_EN |
			 SLPCR_A53_FASTWUP_STOP_MODE);

	mmio_write_32(IMX_GPC_BASE + SLPCR, val);
}

void imx_set_rbc_count(void)
{
	mmio_setbits_32(IMX_GPC_BASE + SLPCR, 0x3f << SLPCR_RBC_COUNT_SHIFT);
}

void imx_clear_rbc_count(void)
{
	mmio_clrbits_32(IMX_GPC_BASE + SLPCR, 0x3f << SLPCR_RBC_COUNT_SHIFT);
}

void imx_gpc_init(void)
{
	uint32_t val;
	int i;
	/* mask all the interrupt by default */
	/* Due to the hardware design requirement, need to make
	 * sure GPR interrupt(#32) is unmasked during RUN mode to
	 * avoid entering DSM mode by mistake.
	 */
	for (i = 0; i < 4; i++) {
		mmio_write_32(IMX_GPC_BASE + IMR1_CORE0_A53 + i * 4, 0xFFFFFFFE);
		mmio_write_32(IMX_GPC_BASE + IMR1_CORE1_A53 + i * 4, 0xFFFFFFFE);
		mmio_write_32(IMX_GPC_BASE + IMR1_CORE2_A53 + i * 4, 0xFFFFFFFE);
		mmio_write_32(IMX_GPC_BASE + IMR1_CORE3_A53 + i * 4, 0xFFFFFFFE);
		mmio_write_32(IMX_GPC_BASE + IMR1_CORE0_M4 + i * 4, ~0x0);
	}

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
}
