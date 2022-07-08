/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>

#ifndef ATF_PLAT_CIRQ_UNSUPPORT
#include <mt_gic_v3.h>
#endif

#include <mt_lp_rm.h>
#include <mt_spm.h>
#include <mt_spm_cond.h>
#include <mt_spm_conservation.h>
#include <mt_spm_constraint.h>
#include <mt_spm_idle.h>
#include <mt_spm_internal.h>
#include <mt_spm_notifier.h>
#include <mt_spm_pmic_wrap.h>
#include <mt_spm_rc_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_resource_req.h>
#include <mt_spm_suspend.h>

#ifndef ATF_PLAT_CIRQ_UNSUPPORT
#include <mt_cirq.h>
#endif

#include <plat_mtk_lpm.h>
#include <plat_pm.h>

#define CONSTRAINT_BUS26M_ALLOW			\
	(MT_RM_CONSTRAINT_ALLOW_CPU_BUCK_OFF |	\
	 MT_RM_CONSTRAINT_ALLOW_DRAM_S0 |	\
	 MT_RM_CONSTRAINT_ALLOW_DRAM_S1 |	\
	 MT_RM_CONSTRAINT_ALLOW_VCORE_LP |	\
	 MT_RM_CONSTRAINT_ALLOW_LVTS_STATE |	\
	 MT_RM_CONSTRAINT_ALLOW_BUS26M_OFF)

#define CONSTRAINT_BUS26M_PCM_FLAG		\
	(SPM_FLAG_DISABLE_INFRA_PDN |		\
	 SPM_FLAG_DISABLE_VCORE_DVS |		\
	 SPM_FLAG_DISABLE_VCORE_DFS |		\
	 SPM_FLAG_SRAM_SLEEP_CTRL |		\
	 SPM_FLAG_ENABLE_TIA_WORKAROUND |	\
	 SPM_FLAG_ENABLE_LVTS_WORKAROUND |	\
	 SPM_FLAG_KEEP_CSYSPWRACK_HIGH)

#define CONSTRAINT_BUS26M_PCM_FLAG1	(0U)
#define CONSTRAINT_BUS26M_RESOURCE_REQ	(0U)

static unsigned int bus26m_ext_opand;
static struct mt_irqremain *refer2remain_irq;
static struct mt_spm_cond_tables cond_bus26m = {
	.name = "bus26m",
	.table_cg = {
		0x0385E03C, /* MTCMOS1 */
		0x003F0100, /* INFRA0  */
		0x0A040802, /* INFRA1  */
		0x06017E51, /* INFRA2  */
		0x08000000, /* INFRA3  */
		0x00000000, /* INFRA4  */
		0x00000000, /* INFRA5  */
		0x03720820, /* MMSYS0  */
		0x00000000, /* MMSYS1  */
		0x00000000, /* MMSYS2  */
		0x00015151, /* MMSYS3  */
	},
	.table_pll = (PLL_BIT_UNIVPLL | PLL_BIT_MFGPLL |
		      PLL_BIT_MSDCPLL | PLL_BIT_TVDPLL |
		      PLL_BIT_MMPLL),
};

static struct mt_spm_cond_tables cond_bus26m_res = {
	.table_cg = {0U},
	.table_pll = 0U,
};

static struct constraint_status status = {
	.id = MT_RM_CONSTRAINT_ID_BUS26M,
	.valid = (MT_SPM_RC_VALID_SW | MT_SPM_RC_VALID_COND_LATCH),
	.cond_block = 0U,
	.enter_cnt = 0U,
	.cond_res = &cond_bus26m_res,
};

/*
 * Cirq will take the place of gic when gic is off.
 * However, cirq cannot work if 26m clk is turned off when system idle/suspend.
 * Therefore, we need to set irq pending for specific wakeup source.
 */
#ifdef ATF_PLAT_CIRQ_UNSUPPORT
#define do_irqs_delivery()
#else
static void mt_spm_irq_remain_dump(struct mt_irqremain *irqs,
				   unsigned int irq_index,
				   struct wake_status *wakeup)
{
	INFO("[SPM] r12 = 0x%08x(0x%08x), flag = 0x%08x 0x%08x 0x%08x\n",
	     wakeup->tr.comm.r12, wakeup->md32pcm_wakeup_sta,
	     wakeup->tr.comm.debug_flag, wakeup->tr.comm.b_sw_flag0,
	     wakeup->tr.comm.b_sw_flag1);

	INFO("irq:%u(0x%08x) set pending\n",
	     irqs->wakeupsrc[irq_index], irqs->irqs[irq_index]);
}

static void do_irqs_delivery(void)
{
	unsigned int idx;
	int res = 0;
	struct wake_status *wakeup = NULL;
	struct mt_irqremain *irqs = refer2remain_irq;

	res = spm_conservation_get_result(&wakeup);

	if ((res != 0) && (irqs == NULL)) {
		return;
	}

	for (idx = 0; idx < irqs->count; ++idx) {
		if (((wakeup->tr.comm.r12 & irqs->wakeupsrc[idx]) != 0U) ||
		    ((wakeup->raw_sta & irqs->wakeupsrc[idx]) != 0U)) {
			if ((irqs->wakeupsrc_cat[idx] & MT_IRQ_REMAIN_CAT_LOG) != 0U) {
				mt_spm_irq_remain_dump(irqs, idx, wakeup);
			}

			mt_irq_set_pending(irqs->irqs[idx]);
		}
	}
}
#endif

static void spm_bus26m_conduct(struct spm_lp_scen *spm_lp, unsigned int *resource_req)
{
	spm_lp->pwrctrl->pcm_flags = (uint32_t)CONSTRAINT_BUS26M_PCM_FLAG;
	spm_lp->pwrctrl->pcm_flags1 = (uint32_t)CONSTRAINT_BUS26M_PCM_FLAG1;
	*resource_req |= CONSTRAINT_BUS26M_RESOURCE_REQ;
}

bool spm_is_valid_rc_bus26m(unsigned int cpu, int state_id)
{
	(void)cpu;
	(void)state_id;

	return ((status.cond_block == 0U) && IS_MT_RM_RC_READY(status.valid));
}

int spm_update_rc_bus26m(int state_id, int type, const void *val)
{
	const struct mt_spm_cond_tables *tlb;
	const struct mt_spm_cond_tables *tlb_check;
	int res = MT_RM_STATUS_OK;

	if (val == NULL) {
		res = MT_RM_STATUS_BAD;
	} else {
		if (type == PLAT_RC_UPDATE_CONDITION) {
			tlb = (const struct mt_spm_cond_tables *)val;
			tlb_check = (const struct mt_spm_cond_tables *)&cond_bus26m;

			status.cond_block =
				mt_spm_cond_check(state_id, tlb, tlb_check,
						  ((status.valid &
						   MT_SPM_RC_VALID_COND_LATCH) != 0U) ?
						  (&cond_bus26m_res) : (NULL));
		} else if (type == PLAT_RC_UPDATE_REMAIN_IRQS) {
			refer2remain_irq = (struct mt_irqremain *)val;
		} else {
			res = MT_RM_STATUS_BAD;
		}
	}

	return res;
}

unsigned int spm_allow_rc_bus26m(int state_id)
{
	(void)state_id;

	return CONSTRAINT_BUS26M_ALLOW;
}

int spm_run_rc_bus26m(unsigned int cpu, int state_id)
{
	(void)cpu;

	unsigned int ext_op = MT_SPM_EX_OP_HW_S1_DETECT;

#ifndef ATF_PLAT_SPM_SSPM_NOTIFIER_UNSUPPORT

#ifdef ATF_VOLTAGE_BIN_VCORE_SUPPORT
#define SUSPEND_VB_MAGIC    (0x5642)
	if (IS_PLAT_SUSPEND_ID(state_id)) {
		mt_spm_sspm_notify_u32(MT_SPM_NOTIFY_SUSPEND_VCORE_VOLTAGE,
				       ((SUSPEND_VB_MAGIC << 16) |
					spm_get_suspend_vcore_voltage_idx()));
	}
#endif
	mt_spm_sspm_notify_u32(MT_SPM_NOTIFY_LP_ENTER, CONSTRAINT_BUS26M_ALLOW |
			       (IS_PLAT_SUSPEND_ID(state_id) ?
			       (MT_RM_CONSTRAINT_ALLOW_AP_SUSPEND) : (0U)));
#endif
	if (IS_PLAT_SUSPEND_ID(state_id)) {
		mt_spm_suspend_enter(state_id,
				     (MT_SPM_EX_OP_CLR_26M_RECORD |
				      MT_SPM_EX_OP_SET_WDT |
				      MT_SPM_EX_OP_HW_S1_DETECT |
				      bus26m_ext_opand),
				     CONSTRAINT_BUS26M_RESOURCE_REQ);
	} else {
		mt_spm_idle_generic_enter(state_id, ext_op, spm_bus26m_conduct);
	}

	return 0;
}

int spm_reset_rc_bus26m(unsigned int cpu, int state_id)
{
	unsigned int ext_op = MT_SPM_EX_OP_HW_S1_DETECT;

	(void)cpu;

#ifndef ATF_PLAT_SPM_SSPM_NOTIFIER_UNSUPPORT
	mt_spm_sspm_notify_u32(MT_SPM_NOTIFY_LP_LEAVE, 0U);
#endif
	if (IS_PLAT_SUSPEND_ID(state_id)) {
		ext_op |= (bus26m_ext_opand | MT_SPM_EX_OP_SET_WDT);
		mt_spm_suspend_resume(state_id, ext_op, NULL);
		bus26m_ext_opand = 0U;
	} else {
		mt_spm_idle_generic_resume(state_id, ext_op, NULL, NULL);
		status.enter_cnt++;
	}

	do_irqs_delivery();
	return 0;
}
