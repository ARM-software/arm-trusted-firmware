/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#ifndef MTK_PLAT_CIRQ_UNSUPPORT
#include <mtk_cirq.h>
#endif
#include <drivers/spm/mt_spm_resource_req.h>
#include <lib/pm/mtk_pm.h>
#include <lpm/mt_lp_rm.h>
#include <mt_spm.h>
#include <mt_spm_cond.h>
#include <mt_spm_conservation.h>
#include <mt_spm_constraint.h>
#include <mt_spm_idle.h>
#include <mt_spm_internal.h>
#include <mt_spm_notifier.h>
#include "mt_spm_rc_api.h"
#include "mt_spm_rc_internal.h"
#include <mt_spm_reg.h>
#include <mt_spm_suspend.h>

#define CONSTRAINT_BUS26M_ALLOW (MT_RM_CONSTRAINT_ALLOW_CPU_BUCK_OFF | \
				 MT_RM_CONSTRAINT_ALLOW_DRAM_S0 | \
				 MT_RM_CONSTRAINT_ALLOW_DRAM_S1 | \
				 MT_RM_CONSTRAINT_ALLOW_VCORE_LP | \
				 MT_RM_CONSTRAINT_ALLOW_LVTS_STATE | \
				 MT_RM_CONSTRAINT_ALLOW_BUS26M_OFF)

#define CONSTRAINT_BUS26M_PCM_FLAG (SPM_FLAG_DISABLE_INFRA_PDN | \
				    SPM_FLAG_DISABLE_VCORE_DVS | \
				    SPM_FLAG_DISABLE_VCORE_DFS | \
				    SPM_FLAG_SRAM_SLEEP_CTRL | \
				    SPM_FLAG_ENABLE_LVTS_WORKAROUND | \
				    SPM_FLAG_KEEP_CSYSPWRACK_HIGH | \
				    SPM_FLAG_DISABLE_DRAMC_MCU_SRAM_SLEEP)

#define CONSTRAINT_BUS26M_PCM_FLAG1 (SPM_FLAG1_DISABLE_PWRAP_CLK_SWITCH)

/* If sspm sram won't enter sleep voltage then vcore couldn't enter low power mode */
#if defined(MTK_PLAT_SPM_SRAM_SLP_UNSUPPORT) && SPM_SRAM_SLEEP_RC_RES_RESTRICT
#define CONSTRAINT_BUS26M_RESOURCE_REQ	(MT_SPM_26M)
#else
#define CONSTRAINT_BUS26M_RESOURCE_REQ	(0)
#endif

static unsigned int bus26m_ext_opand;
static unsigned int bus26m_ext_opand2;

static struct mt_irqremain *refer2remain_irq;

static struct mt_spm_cond_tables cond_bus26m = {
	.name = "bus26m",
	.table_cg = {
		0xFF5DD002,	/* MTCMOS1 */
		0x0000003C,	/* MTCMOS2 */
		0x27AF8000,	/* INFRA0  */
		0x22010876,	/* INFRA1  */
		0x86000650,	/* INFRA2  */
		0x30008020,	/* INFRA3  */
		0x80000000,	/* INFRA4  */
		0x01002A3B,	/* PERI0   */
		0x00090000,	/* VPPSYS0_0  */
		0x38FF3E69,     /* VPPSYS0_1  */
		0xF0081450,	/* VPPSYS1_0  */
		0x00003000,     /* VPPSYS1_1  */
		0x00000000,	/* VDOSYS0_0  */
		0x00000000,     /* VDOSYS0_1  */
		0x000001FF,	/* VDOSYS1_0  */
		0x000001E0,     /* VDOSYS1_1  */
		0x00FB0007,	/* VDOSYS1_2  */
	},
	.table_pll = (PLL_BIT_UNIVPLL |
		      PLL_BIT_MFGPLL |
		      PLL_BIT_MSDCPLL |
		      PLL_BIT_TVDPLL1 |
		      PLL_BIT_TVDPLL2 |
		      PLL_BIT_MMPLL |
		      PLL_BIT_ETHPLL |
		      PLL_BIT_IMGPLL |
		      PLL_BIT_APLL1 |
		      PLL_BIT_APLL2 |
		      PLL_BIT_APLL3 |
		      PLL_BIT_APLL4 |
		      PLL_BIT_APLL5),
};

static struct mt_spm_cond_tables cond_bus26m_res = {
	.table_cg = { 0U },
	.table_pll = 0U,
};

static struct constraint_status status = {
	.id = MT_RM_CONSTRAINT_ID_BUS26M,
	.is_valid = (MT_SPM_RC_VALID_SW |
		     MT_SPM_RC_VALID_COND_CHECK |
		     MT_SPM_RC_VALID_COND_LATCH |
		     MT_SPM_RC_VALID_TRACE_TIME),
	.is_cond_block = 0U,
	.enter_cnt = 0U,
	.all_pll_dump = 0U,
	.cond_res = &cond_bus26m_res,
	.residency = 0ULL,
};

#ifdef MTK_PLAT_CIRQ_UNSUPPORT
#define do_irqs_delivery()
#else
static void mt_spm_irq_remain_dump(struct mt_irqremain *irqs,
				   unsigned int irq_index,
				   struct wake_status *wakeup)
{
	if ((irqs == NULL) || (wakeup == NULL)) {
		return;
	}

	INFO("[SPM] r12=0x%08x(0x%08x), flag=0x%08x 0x%08x 0x%08x, irq:%u(0x%08x) set pending\n",
	     wakeup->tr.comm.r12,
	     wakeup->md32pcm_wakeup_sta,
	     wakeup->tr.comm.debug_flag,
	     wakeup->tr.comm.b_sw_flag0,
	     wakeup->tr.comm.b_sw_flag1,
	     irqs->wakeupsrc[irq_index],
	     irqs->irqs[irq_index]);
}

static void do_irqs_delivery(void)
{
	unsigned int idx;
	struct wake_status *wakeup = NULL;
	struct mt_irqremain *irqs = refer2remain_irq;

	if (irqs == NULL) {
		return;
	}

	if (spm_conservation_get_result(&wakeup) == 0) {
		if (wakeup != NULL) {
			for (idx = 0; idx < irqs->count; idx++) {
				if (((wakeup->tr.comm.r12 & irqs->wakeupsrc[idx]) != 0U) ||
				    ((wakeup->tr.comm.raw_sta & irqs->wakeupsrc[idx]) != 0U)) {
					if ((irqs->wakeupsrc_cat[idx] &
					     MT_IRQ_REMAIN_CAT_LOG) != 0U) {
						mt_spm_irq_remain_dump(irqs, idx, wakeup);
					}
					mt_irq_set_pending(irqs->irqs[idx]);
				}
			}
		}
	}
}
#endif

int spm_bus26m_conduct(int state_id, struct spm_lp_scen *spm_lp, unsigned int *resource_req)
{
	unsigned int res_req = CONSTRAINT_BUS26M_RESOURCE_REQ;

	if ((spm_lp == NULL) || (resource_req == NULL)) {
		return -1;
	}

	spm_lp->pwrctrl->pcm_flags = (uint32_t)CONSTRAINT_BUS26M_PCM_FLAG;
	spm_lp->pwrctrl->pcm_flags1 = (uint32_t)CONSTRAINT_BUS26M_PCM_FLAG1;

	*resource_req |= res_req;
	return 0;
}

bool spm_is_valid_rc_bus26m(unsigned int cpu, int state_id)
{
	return (!(status.is_cond_block && (status.is_valid & MT_SPM_RC_VALID_COND_CHECK) > 0) &&
		IS_MT_RM_RC_READY(status.is_valid) &&
		(IS_PLAT_SUSPEND_ID(state_id) || (state_id == MT_PLAT_PWR_STATE_SYSTEM_BUS)));
}

static int update_rc_condition(int state_id, const void *val)
{
	const struct mt_spm_cond_tables *tlb = (const struct mt_spm_cond_tables *)val;
	const struct mt_spm_cond_tables *tlb_check =
		(const struct mt_spm_cond_tables *)&cond_bus26m;

	if (tlb == NULL) {
		return MT_RM_STATUS_BAD;
	}

	status.is_cond_block = mt_spm_cond_check(state_id, tlb, tlb_check,
						 (status.is_valid & MT_SPM_RC_VALID_COND_LATCH) ?
						 &cond_bus26m_res : NULL);
	status.all_pll_dump = mt_spm_dump_all_pll(tlb, tlb_check,
						  (status.is_valid & MT_SPM_RC_VALID_COND_LATCH) ?
						  &cond_bus26m_res : NULL);
	return MT_RM_STATUS_OK;
}

static void update_rc_remain_irqs(const void *val)
{
	refer2remain_irq = (struct mt_irqremain *)val;
}

static void update_rc_fmaudio_adsp(int type, const void *val)
{
	int *flag = (int *)val;
	unsigned int ext_op = (type == PLAT_RC_IS_ADSP) ?
			      (MT_SPM_EX_OP_SET_IS_ADSP | MT_SPM_EX_OP_SET_SUSPEND_MODE) :
			      MT_SPM_EX_OP_SET_SUSPEND_MODE;

	if (flag == NULL) {
		return;
	}

	if (*flag != 0) {
		SPM_RC_BITS_SET(bus26m_ext_opand, ext_op);
	} else {
		SPM_RC_BITS_CLR(bus26m_ext_opand, ext_op);
	}
}

static void update_rc_usb_peri(const void *val)
{
	int *flag = (int *)val;

	if (flag == NULL) {
		return;
	}

	if (*flag != 0) {
		SPM_RC_BITS_SET(bus26m_ext_opand2, MT_SPM_EX_OP_PERI_ON);
	} else {
		SPM_RC_BITS_CLR(bus26m_ext_opand2, MT_SPM_EX_OP_PERI_ON);
	}
}

static void update_rc_usb_infra(const void *val)
{
	int *flag = (int *)val;

	if (flag == NULL) {
		return;
	}

	if (*flag != 0) {
		SPM_RC_BITS_SET(bus26m_ext_opand2, MT_SPM_EX_OP_INFRA_ON);
	} else {
		SPM_RC_BITS_CLR(bus26m_ext_opand2, MT_SPM_EX_OP_INFRA_ON);
	}
}

static void update_rc_status(const void *val)
{
	const struct rc_common_state *st;

	st = (const struct rc_common_state *)val;

	if (st == NULL) {
		return;
	}

	if (st->type == CONSTRAINT_UPDATE_COND_CHECK) {
		struct mt_spm_cond_tables * const tlb = &cond_bus26m;

		spm_rc_condition_modifier(st->id, st->act, st->value,
					  MT_RM_CONSTRAINT_ID_BUS26M, tlb);
	} else if ((st->type == CONSTRAINT_UPDATE_VALID) ||
		   (st->type == CONSTRAINT_RESIDNECY)) {
		spm_rc_constraint_status_set(st->id, st->type, st->act,
					     MT_RM_CONSTRAINT_ID_BUS26M,
					     (struct constraint_status * const)st->value,
					     (struct constraint_status * const)&status);
	} else {
		INFO("[%s:%d] - Unknown type: 0x%x\n", __func__, __LINE__, st->type);
	}
}

int spm_update_rc_bus26m(int state_id, int type, const void *val)
{
	int res = MT_RM_STATUS_OK;

	switch (type) {
	case PLAT_RC_UPDATE_CONDITION:
		res = update_rc_condition(state_id, val);
		break;
	case PLAT_RC_UPDATE_REMAIN_IRQS:
		update_rc_remain_irqs(val);
		break;
	case PLAT_RC_IS_FMAUDIO:
	case PLAT_RC_IS_ADSP:
		update_rc_fmaudio_adsp(type, val);
		break;
	case PLAT_RC_IS_USB_PERI:
		update_rc_usb_peri(val);
		break;
	case PLAT_RC_IS_USB_INFRA:
		update_rc_usb_infra(val);
		break;
	case PLAT_RC_STATUS:
		update_rc_status(val);
		break;
	default:
		INFO("[%s:%d] - Do nothing for type: %d\n", __func__, __LINE__, type);
		break;
	}
	return res;
}

unsigned int spm_allow_rc_bus26m(int state_id)
{
	return CONSTRAINT_BUS26M_ALLOW;
}

int spm_run_rc_bus26m(unsigned int cpu, int state_id)
{
	unsigned int ext_op = MT_SPM_EX_OP_HW_S1_DETECT;

#ifndef MTK_PLAT_SPM_SSPM_NOTIFIER_UNSUPPORT
	mt_spm_sspm_notify_u32(MT_SPM_NOTIFY_LP_ENTER, CONSTRAINT_BUS26M_ALLOW |
			       (IS_PLAT_SUSPEND_ID(state_id) ?
				MT_RM_CONSTRAINT_ALLOW_AP_SUSPEND : 0));
#endif
	if (status.is_valid & MT_SPM_RC_VALID_TRACE_TIME) {
		ext_op |= MT_SPM_EX_OP_TRACE_TIMESTAMP_EN;
	}

	if (IS_PLAT_SUSPEND_ID(state_id)) {
		mt_spm_suspend_enter(state_id,
				     (MT_SPM_EX_OP_CLR_26M_RECORD |
				      MT_SPM_EX_OP_SET_WDT |
				      MT_SPM_EX_OP_HW_S1_DETECT |
				      bus26m_ext_opand |
				      bus26m_ext_opand2),
				     CONSTRAINT_BUS26M_RESOURCE_REQ);
	} else {
		mt_spm_idle_generic_enter(state_id, ext_op, spm_bus26m_conduct);
	}
	return MT_RM_STATUS_OK;
}

int spm_reset_rc_bus26m(unsigned int cpu, int state_id)
{
	unsigned int ext_op = MT_SPM_EX_OP_HW_S1_DETECT;

#ifndef MTK_PLAT_SPM_SSPM_NOTIFIER_UNSUPPORT
	mt_spm_sspm_notify_u32(MT_SPM_NOTIFY_LP_LEAVE, 0);
#endif
	if (status.is_valid & MT_SPM_RC_VALID_TRACE_TIME) {
		ext_op |= MT_SPM_EX_OP_TRACE_TIMESTAMP_EN;
	}

	if (IS_PLAT_SUSPEND_ID(state_id)) {
		mt_spm_suspend_resume(state_id,
				      (bus26m_ext_opand | bus26m_ext_opand2 |
				       MT_SPM_EX_OP_SET_WDT | ext_op),
				      NULL);
		bus26m_ext_opand = 0;
	} else {
		struct wake_status *waken = NULL;

		if (spm_unlikely(status.is_valid & MT_SPM_RC_VALID_TRACE_EVENT)) {
			ext_op |= MT_SPM_EX_OP_TRACE_LP;
		}

		mt_spm_idle_generic_resume(state_id, ext_op, &waken, NULL);
		status.enter_cnt++;

		if (spm_unlikely(status.is_valid & MT_SPM_RC_VALID_RESIDNECY)) {
			status.residency += (waken != NULL) ? waken->tr.comm.timer_out : 0;
		}
	}

	do_irqs_delivery();

	return MT_RM_STATUS_OK;
}

int spm_get_status_rc_bus26m(unsigned int type, void *priv)
{
	int ret = MT_RM_STATUS_OK;

	if (type == PLAT_RC_STATUS) {
		int res = 0;
		struct rc_common_state *st = (struct rc_common_state *)priv;

		if (st == NULL) {
			return MT_RM_STATUS_BAD;
		}

		res = spm_rc_constraint_status_get(st->id, st->type,
						   st->act, MT_RM_CONSTRAINT_ID_BUS26M,
						   (struct constraint_status * const)&status,
						   (struct constraint_status * const)st->value);
		if ((res == 0) && (st->id != MT_RM_CONSTRAINT_ID_ALL)) {
			ret = MT_RM_STATUS_STOP;
		}
	}
	return ret;
}
