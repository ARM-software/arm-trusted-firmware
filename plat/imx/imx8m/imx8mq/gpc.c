/*
 * Copyright (c) 2018-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <lib/smccc.h>
#include <lib/spinlock.h>
#include <plat/common/platform.h>
#include <services/std_svc.h>

#include <gpc.h>
#include <platform_def.h>

#define FSL_SIP_CONFIG_GPC_MASK		U(0x00)
#define FSL_SIP_CONFIG_GPC_UNMASK	U(0x01)
#define FSL_SIP_CONFIG_GPC_SET_WAKE	U(0x02)
#define FSL_SIP_CONFIG_GPC_PM_DOMAIN	U(0x03)
#define FSL_SIP_CONFIG_GPC_SET_AFF	U(0x04)
#define FSL_SIP_CONFIG_GPC_CORE_WAKE	U(0x05)

#define MAX_HW_IRQ_NUM		U(128)
#define MAX_IMR_NUM		U(4)

static uint32_t gpc_saved_imrs[16];
static uint32_t gpc_wake_irqs[4];
static uint32_t gpc_imr_offset[] = {
	IMX_GPC_BASE + IMR1_CORE0_A53,
	IMX_GPC_BASE + IMR1_CORE1_A53,
	IMX_GPC_BASE + IMR1_CORE2_A53,
	IMX_GPC_BASE + IMR1_CORE3_A53,
	IMX_GPC_BASE + IMR1_CORE0_M4,
};

spinlock_t gpc_imr_lock[4];

static void gpc_imr_core_spin_lock(unsigned int core_id)
{
	spin_lock(&gpc_imr_lock[core_id]);
}

static void gpc_imr_core_spin_unlock(unsigned int core_id)
{
	spin_unlock(&gpc_imr_lock[core_id]);
}

static void gpc_save_imr_lpm(unsigned int core_id, unsigned int imr_idx)
{
	uint32_t reg = gpc_imr_offset[core_id] + imr_idx * 4;

	gpc_imr_core_spin_lock(core_id);

	gpc_saved_imrs[core_id + imr_idx * 4] = mmio_read_32(reg);
	mmio_write_32(reg, ~gpc_wake_irqs[imr_idx]);

	gpc_imr_core_spin_unlock(core_id);
}

static void gpc_restore_imr_lpm(unsigned int core_id, unsigned int imr_idx)
{
	uint32_t reg = gpc_imr_offset[core_id] + imr_idx * 4;
	uint32_t val = gpc_saved_imrs[core_id + imr_idx * 4];

	gpc_imr_core_spin_lock(core_id);

	mmio_write_32(reg, val);

	gpc_imr_core_spin_unlock(core_id);
}

/*
 * On i.MX8MQ, only in system suspend mode, the A53 cluster can
 * enter LPM mode and shutdown the A53 PLAT power domain. So LPM
 * wakeup only used for system suspend. when system enter suspend,
 * any A53 CORE can be the last core to suspend the system, But
 * the LPM wakeup can only use the C0's IMR to wakeup A53 cluster
 * from LPM, so save C0's IMRs before suspend, restore back after
 * resume.
 */
void imx_set_sys_wakeup(unsigned int last_core, bool pdn)
{
	unsigned int imr, core;

	if (pdn) {
		for (imr = 0U; imr < MAX_IMR_NUM; imr++) {
			for (core = 0U; core < PLATFORM_CORE_COUNT; core++) {
				gpc_save_imr_lpm(core, imr);
			}
		}
	} else {
		for (imr = 0U; imr < MAX_IMR_NUM; imr++) {
			for (core = 0U; core < PLATFORM_CORE_COUNT; core++) {
				gpc_restore_imr_lpm(core, imr);
			}
		}
	}
}

static void imx_gpc_hwirq_mask(unsigned int hwirq)
{
	uintptr_t reg;
	unsigned int val;

	if (hwirq >= MAX_HW_IRQ_NUM) {
		return;
	}

	gpc_imr_core_spin_lock(0);
	reg = gpc_imr_offset[0] + (hwirq / 32) * 4;
	val = mmio_read_32(reg);
	val |= 1 << hwirq % 32;
	mmio_write_32(reg, val);
	gpc_imr_core_spin_unlock(0);
}

static void imx_gpc_hwirq_unmask(unsigned int hwirq)
{
	uintptr_t reg;
	unsigned int val;

	if (hwirq >= MAX_HW_IRQ_NUM) {
		return;
	}

	gpc_imr_core_spin_lock(0);
	reg = gpc_imr_offset[0] + (hwirq / 32) * 4;
	val = mmio_read_32(reg);
	val &= ~(1 << hwirq % 32);
	mmio_write_32(reg, val);
	gpc_imr_core_spin_unlock(0);
}

static void imx_gpc_set_wake(uint32_t hwirq, bool on)
{
	uint32_t mask, idx;

	if (hwirq >= MAX_HW_IRQ_NUM) {
		return;
	}

	mask = 1 << hwirq % 32;
	idx = hwirq / 32;
	gpc_wake_irqs[idx] = on ? gpc_wake_irqs[idx] | mask :
				 gpc_wake_irqs[idx] & ~mask;
}

static void imx_gpc_mask_irq0(uint32_t core_id, uint32_t mask)
{
	gpc_imr_core_spin_lock(core_id);
	if (mask) {
		mmio_setbits_32(gpc_imr_offset[core_id], 1);
	} else {
		mmio_clrbits_32(gpc_imr_offset[core_id], 1);
	}

	dsb();
	gpc_imr_core_spin_unlock(core_id);
}

void imx_gpc_core_wake(uint32_t cpumask)
{
	for (int i = 0; i < PLATFORM_CORE_COUNT; i++) {
		if (cpumask & (1 << i)) {
			imx_gpc_mask_irq0(i, false);
		}
	}
}

void imx_gpc_set_a53_core_awake(uint32_t core_id)
{
	imx_gpc_mask_irq0(core_id, true);
}

static void imx_gpc_set_affinity(uint32_t hwirq, unsigned int cpu_idx)
{
	uintptr_t reg;
	unsigned int val;

	if (hwirq >= MAX_HW_IRQ_NUM || cpu_idx >= 4) {
		return;
	}

	/*
	 * using the mask/unmask bit as affinity function.unmask the
	 * IMR bit to enable IRQ wakeup for this core.
	 */
	gpc_imr_core_spin_lock(cpu_idx);
	reg = gpc_imr_offset[cpu_idx] + (hwirq / 32) * 4;
	val = mmio_read_32(reg);
	val &= ~(1 << hwirq % 32);
	mmio_write_32(reg, val);
	gpc_imr_core_spin_unlock(cpu_idx);

	/* clear affinity of other core */
	for (int i = 0; i < PLATFORM_CORE_COUNT; i++) {
		if (cpu_idx != i) {
			gpc_imr_core_spin_lock(i);
			reg = gpc_imr_offset[i] + (hwirq / 32) * 4;
			val = mmio_read_32(reg);
			val |= (1 << hwirq % 32);
			mmio_write_32(reg, val);
			gpc_imr_core_spin_unlock(i);
		}
	}
}

/* use wfi power down the core */
void imx_set_cpu_pwr_off(unsigned int core_id)
{
	bakery_lock_get(&gpc_lock);

	/* enable the wfi power down of the core */
	mmio_setbits_32(IMX_GPC_BASE + LPCR_A53_AD, COREx_WFI_PDN(core_id) |
			(1 << (core_id + 20)));

	bakery_lock_release(&gpc_lock);

	/* assert the pcg pcr bit of the core */
	mmio_setbits_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), 0x1);
};

/* if out of lpm, we need to do reverse steps */
void imx_set_cpu_lpm(unsigned int core_id, bool pdn)
{
	bakery_lock_get(&gpc_lock);

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

	bakery_lock_release(&gpc_lock);
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
			A53_PLAT_PDN_ACK | SLT_COREx_PUP_ACK(last_core));
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

#define MAX_PLL_NUM	U(12)

static const struct pll_override imx8mq_pll[MAX_PLL_NUM] = {
	{.reg = 0x0, .override_mask = 0x140000, },
	{.reg = 0x8, .override_mask = 0x140000, },
	{.reg = 0x10, .override_mask = 0x140000, },
	{.reg = 0x18, .override_mask = 0x140000, },
	{.reg = 0x20, .override_mask = 0x140000, },
	{.reg = 0x28, .override_mask = 0x140000, },
	{.reg = 0x30, .override_mask = 0x1555540, },
	{.reg = 0x3c, .override_mask = 0x1555540, },
	{.reg = 0x48, .override_mask = 0x140, },
	{.reg = 0x54, .override_mask = 0x140, },
	{.reg = 0x60, .override_mask = 0x140, },
	{.reg = 0x70, .override_mask = 0xa, },
};

void imx_anamix_override(bool enter)
{
	unsigned int i;

	/* enable the pll override bit before entering DSM mode */
	for (i = 0; i < MAX_PLL_NUM; i++) {
		if (enter) {
			mmio_setbits_32(IMX_ANAMIX_BASE + imx8mq_pll[i].reg,
				imx8mq_pll[i].override_mask);
		} else {
			mmio_clrbits_32(IMX_ANAMIX_BASE + imx8mq_pll[i].reg,
				imx8mq_pll[i].override_mask);
		}
	}
}

int imx_gpc_handler(uint32_t smc_fid,
			  u_register_t x1,
			  u_register_t x2,
			  u_register_t x3)
{
	switch (x1) {
	case FSL_SIP_CONFIG_GPC_CORE_WAKE:
		imx_gpc_core_wake(x2);
		break;
	case FSL_SIP_CONFIG_GPC_SET_WAKE:
		imx_gpc_set_wake(x2, x3);
		break;
	case FSL_SIP_CONFIG_GPC_MASK:
		imx_gpc_hwirq_mask(x2);
		break;
	case FSL_SIP_CONFIG_GPC_UNMASK:
		imx_gpc_hwirq_unmask(x2);
		break;
	case FSL_SIP_CONFIG_GPC_SET_AFF:
		imx_gpc_set_affinity(x2, x3);
		break;
	default:
		return SMC_UNK;
	}

	return 0;
}

void imx_gpc_init(void)
{
	uint32_t val;
	unsigned int i, j;

	/* mask all the interrupt by default */
	for (i = 0U; i < PLATFORM_CORE_COUNT; i++) {
		for (j = 0U; j < ARRAY_SIZE(gpc_imr_offset); j++) {
			mmio_write_32(gpc_imr_offset[j] + i * 4, ~0x0);
		}
	}

	/* Due to the hardware design requirement, need to make
	 * sure GPR interrupt(#32) is unmasked during RUN mode to
	 * avoid entering DSM mode by mistake.
	 */
	for (i = 0U; i < PLATFORM_CORE_COUNT; i++) {
		mmio_write_32(gpc_imr_offset[i], ~0x1);
	}

	/* leave the IOMUX_GPC bit 12 on for core wakeup */
	mmio_setbits_32(IMX_IOMUX_GPR_BASE + 0x4, 1 << 12);

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

	/* set SCU timing */
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

	/*
	 * for USB OTG, the limitation are:
	 * 1. before system clock config, the IPG clock run at 12.5MHz, delay time
	 *    should be longer than 82us.
	 * 2. after system clock config, ipg clock run at 66.5MHz, delay time
	 *    be longer that 15.3 us.
	 *    Add 100us to make sure the USB OTG SRC is clear safely.
	 */
	udelay(100);
}
