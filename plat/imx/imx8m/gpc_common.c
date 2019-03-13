/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>

#include <gpc.h>
#include <imx8m_psci.h>
#include <plat_imx8.h>

static uint32_t gpc_imr_offset[] = { 0x30, 0x40, 0x1c0, 0x1d0, };

#pragma weak imx_set_cpu_pwr_off
#pragma weak imx_set_cpu_pwr_on
#pragma weak imx_set_cpu_lpm
#pragma weak imx_set_cluster_powerdown

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

void imx_set_cpu_pwr_off(unsigned int core_id)
{
	/* enable the wfi power down of the core */
	mmio_setbits_32(IMX_GPC_BASE + LPCR_A53_AD, COREx_WFI_PDN(core_id));
	/* assert the pcg pcr bit of the core */
	mmio_setbits_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), 0x1);
}

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

void imx_set_cpu_lpm(unsigned int core_id, bool pdn)
{
	if (pdn) {
		/* enable the core WFI PDN & IRQ PUP */
		mmio_setbits_32(IMX_GPC_BASE + LPCR_A53_AD, COREx_WFI_PDN(core_id) |
				COREx_IRQ_WUP(core_id));
		/* assert the pcg pcr bit of the core */
		mmio_setbits_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), 0x1);
	} else {
		/* disbale CORE WFI PDN & IRQ PUP */
		mmio_clrbits_32(IMX_GPC_BASE + LPCR_A53_AD, COREx_WFI_PDN(core_id) |
				COREx_IRQ_WUP(core_id));
		/* deassert the pcg pcr bit of the core */
		mmio_clrbits_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), 0x1);
	}
}

/*
 * the plat and noc can only be power up & down by slot method,
 * slot0: plat power down; slot1: noc power down; slot2: noc power up;
 * slot3: plat power up. plat's pup&pdn ack is used by default. if
 * noc is config to power down, then noc's pdn ack should be used.
 */
static void imx_a53_plat_slot_config(bool pdn)
{
	if (pdn) {
		mmio_setbits_32(IMX_GPC_BASE + SLTx_CFG(0), PLAT_PDN_SLT_CTRL);
		mmio_setbits_32(IMX_GPC_BASE + SLTx_CFG(3), PLAT_PUP_SLT_CTRL);
		mmio_write_32(IMX_GPC_BASE + PGC_ACK_SEL_A53, A53_PLAT_PDN_ACK |
			A53_PLAT_PUP_ACK);
		mmio_setbits_32(IMX_GPC_BASE + PLAT_PGC_PCR, 0x1);
	} else {
		mmio_clrbits_32(IMX_GPC_BASE + SLTx_CFG(0), PLAT_PDN_SLT_CTRL);
		mmio_clrbits_32(IMX_GPC_BASE + SLTx_CFG(3), PLAT_PUP_SLT_CTRL);
		mmio_write_32(IMX_GPC_BASE + PGC_ACK_SEL_A53, A53_DUMMY_PUP_ACK |
			A53_DUMMY_PDN_ACK);
		mmio_clrbits_32(IMX_GPC_BASE + PLAT_PGC_PCR, 0x1);
	}
}

void imx_set_cluster_standby(bool enter)
{
	/*
	 * Enable BIT 6 of A53 AD register to make sure system
	 * don't enter LPM mode.
	 */
	if (enter)
		mmio_setbits_32(IMX_GPC_BASE + LPCR_A53_AD, (1 << 6));
	else
		mmio_clrbits_32(IMX_GPC_BASE + LPCR_A53_AD, (1 << 6));
}

/* i.mx8mq need to override it */
void imx_set_cluster_powerdown(unsigned int last_core, uint8_t power_state)
{
	uint32_t val;

	if (!is_local_state_run(power_state)) {
		/* config C0~1's LPM, enable a53 clock off in LPM */
		mmio_clrsetbits_32(IMX_GPC_BASE + LPCR_A53_BSC, A53_CLK_ON_LPM,
			LPM_MODE(power_state));
		/* config C2-3's LPM */
		mmio_setbits_32(IMX_GPC_BASE + LPCR_A53_BSC2, LPM_MODE(power_state));

		/* enable PLAT/SCU power down */
		val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_AD);
		val &= ~EN_L2_WFI_PDN;
		/* L2 cache memory is on in WAIT mode */
		if (is_local_state_off(power_state))
			val |= (L2PGE | EN_PLAT_PDN);
		else
			val |= EN_PLAT_PDN;

		mmio_write_32(IMX_GPC_BASE + LPCR_A53_AD, val);

		imx_a53_plat_slot_config(true);
	} else {
		/* clear the slot and ack for cluster power down */
		imx_a53_plat_slot_config(false);
		/* reverse the cluster level setting */
		mmio_clrsetbits_32(IMX_GPC_BASE + LPCR_A53_BSC, 0xf, A53_CLK_ON_LPM);
		mmio_clrbits_32(IMX_GPC_BASE + LPCR_A53_BSC2, 0xf);

		/* clear PLAT/SCU power down */
		mmio_clrsetbits_32(IMX_GPC_BASE + LPCR_A53_AD, (L2PGE | EN_PLAT_PDN),
			EN_L2_WFI_PDN);
	}
}

static unsigned int gicd_read_isenabler(uintptr_t base, unsigned int id)
{
	unsigned int n = id >> ISENABLER_SHIFT;

	return mmio_read_32(base + GICD_ISENABLER + (n << 2));
}

/*
 * gic's clock will be gated in system suspend, so gic has no ability to
 * to wakeup the system, we need to config the imr based on the irq
 * enable status in gic, then gpc will monitor the wakeup irq
 */
void imx_set_sys_wakeup(unsigned int last_core, bool pdn)
{
	uint32_t irq_mask;
	uintptr_t gicd_base = PLAT_GICD_BASE;

	if (pdn)
		mmio_clrsetbits_32(IMX_GPC_BASE + LPCR_A53_BSC, A53_CORE_WUP_SRC(last_core),
			IRQ_SRC_A53_WUP);
	else
		mmio_clrsetbits_32(IMX_GPC_BASE + LPCR_A53_BSC, IRQ_SRC_A53_WUP,
			A53_CORE_WUP_SRC(last_core));

	/* clear last core's IMR based on GIC's mask setting */
	for (int i = 0; i < IRQ_IMR_NUM; i++) {
		if (pdn)
			/* set the wakeup irq base GIC */
			irq_mask = ~gicd_read_isenabler(gicd_base, 32 * (i + 1));
		else
			irq_mask = IMR_MASK_ALL;

		mmio_write_32(IMX_GPC_BASE + gpc_imr_offset[last_core] + i * 4,
			      irq_mask);
	}
}

#pragma weak imx_noc_slot_config
/*
 * this function only need to be override by platform
 * that support noc power down, for example: imx8mm.
 *  otherwize, keep it empty.
 */
void imx_noc_slot_config(bool pdn)
{

}

/* this is common for all imx8m soc */
void imx_set_sys_lpm(unsigned int last_core, bool retention)
{
	uint32_t val;

	val = mmio_read_32(IMX_GPC_BASE + SLPCR);
	val &= ~(SLPCR_EN_DSM | SLPCR_VSTBY | SLPCR_SBYOS |
		 SLPCR_BYPASS_PMIC_READY | SLPCR_A53_FASTWUP_STOP_MODE);

	if (retention)
		val |= (SLPCR_EN_DSM | SLPCR_VSTBY | SLPCR_SBYOS |
			SLPCR_BYPASS_PMIC_READY | SLPCR_A53_FASTWUP_STOP_MODE);

	mmio_write_32(IMX_GPC_BASE + SLPCR, val);

	/* config the noc power down */
	imx_noc_slot_config(retention);

	/* config wakeup irqs' mask in gpc */
	imx_set_sys_wakeup(last_core, retention);
}

void imx_set_rbc_count(void)
{
	mmio_setbits_32(IMX_GPC_BASE + SLPCR, SLPCR_RBC_EN |
		(0x8 << SLPCR_RBC_COUNT_SHIFT));
}

void imx_clear_rbc_count(void)
{
	mmio_clrbits_32(IMX_GPC_BASE + SLPCR, SLPCR_RBC_EN |
		(0x3f << SLPCR_RBC_COUNT_SHIFT));
}
