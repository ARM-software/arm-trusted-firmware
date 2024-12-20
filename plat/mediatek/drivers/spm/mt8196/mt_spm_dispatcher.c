/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>
#include <platform_def.h>

#include <lpm_v2/mt_lpm_dispatch.h>
#include <lpm_v2/mt_lpm_smc.h>
#include <mt_spm_conservation.h>
#include <mt_spm_dispatcher.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_smc.h>
#include <mt_spm_suspend.h>
#include <pcm_def.h>

#define SPM_FW_BASE_SIZE	0x100000

static void mt_spm_pcm_wdt(int enable, uint64_t time)
{
	mmio_write_32(PCM_TIMER_VAL, time);
	__spm_set_pcm_wdt(enable);
}

static uint32_t mt_spm_phypll_mode_check(void)
{
	uint32_t val = mmio_read_32(SPM_POWER_ON_VAL0);

	return val;
}

static uint64_t mt_spm_compatible_smc_id(uint64_t lp_id)
{
	switch (lp_id) {
	case MT_LPM_SPMC_COMPAT_LK_FW_INIT:
		lp_id = MT_SPM_SMC_UID_FW_INIT;
		break;
	default:
		break;
	}
	return lp_id;
}

uint64_t mt_spm_dispatcher(u_register_t lp_id, u_register_t act,
			   u_register_t arg1, u_register_t arg2,
			   void *handle, struct smccc_res *smccc_ret)
{
	uint64_t ret = 0;

	if (act & MT_LPM_SMC_ACT_COMPAT) {
		lp_id = mt_spm_compatible_smc_id(lp_id);
		act &= ~(MT_LPM_SMC_ACT_COMPAT);
	}

	switch (lp_id) {
	case MT_SPM_SMC_UID_STATUS:
		if (!(arg2 & MT_SPM_STATUS_SUSPEND_SLEEP))
			break;
		if (act & MT_LPM_SMC_ACT_SET)
			/* Legacy audio check from kernel */
			mt_spm_suspend_mode_set(MT_SPM_SUSPEND_SLEEP, NULL);
		else if (act & MT_LPM_SMC_ACT_CLR)
			mt_spm_suspend_mode_set(MT_SPM_SUSPEND_SYSTEM_PDN,
						NULL);
		break;
	case MT_SPM_SMC_UID_PCM_WDT:
		if (act & MT_LPM_SMC_ACT_SET)
			mt_spm_pcm_wdt(1, arg2);
		else if (act & MT_LPM_SMC_ACT_CLR)
			mt_spm_pcm_wdt(0, arg2);
		break;
	case MT_SPM_SMC_UID_PHYPLL_MODE:
		if (act & MT_LPM_SMC_ACT_GET)
			ret = mt_spm_phypll_mode_check();
		break;
	case MT_SPM_SMC_UID_SET_PENDING_IRQ_INIT:
		spm_set_irq_num((uint32_t)arg1);
		break;
	default:
		break;
	}
	return ret;
}

int mt_spm_dispatcher_init(void)
{
	mt_lpm_dispatcher_registry(MT_LPM_SMC_USER_SPM,
				   mt_spm_dispatcher);
	return 0;
}
