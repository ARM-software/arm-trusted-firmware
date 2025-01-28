/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <lib/spinlock.h>

#include "mt_cpu_pm.h"
#include "mt_cpu_pm_cpc.h"
#include "mt_cpu_pm_smc.h"
#include "mt_lp_irqremain.h"

/*
 * The locker must use the bakery locker when cache turn off.
 * Using spin_lock will has better performance.
 */
#ifdef MT_CPU_PM_USING_BAKERY_LOCK
DEFINE_BAKERY_LOCK(mt_cpu_pm_smc_lock);
#define plat_cpu_pm_smc_lock_init() bakery_lock_init(&mt_cpu_pm_smc_lock)
#define plat_cpu_pm_smc_lock() bakery_lock_get(&mt_cpu_pm_smc_lock)
#define plat_cpu_pm_smc_unlock() bakery_lock_release(&mt_cpu_pm_smc_lock)
#else
spinlock_t mt_cpu_pm_smc_lock;
#define plat_cpu_pm_smc_lock_init()
#define plat_cpu_pm_smc_lock() spin_lock(&mt_cpu_pm_smc_lock)
#define plat_cpu_pm_smc_unlock() spin_unlock(&mt_cpu_pm_smc_lock)
#endif /* MT_CPU_PM_USING_BAKERY_LOCK */

static uint64_t cpupm_dispatcher(u_register_t lp_id,
				 u_register_t act,
				 u_register_t arg1,
				 u_register_t arg2,
				 void *handle,
				 struct smccc_res *smccc_ret)
{
	uint64_t res = 0;

	switch (lp_id) {
	case CPC_COMMAND:
		res = mtk_cpc_handler(act, arg1, arg2);
		break;
	default:
		break;
	}

	return res;
}

static uint64_t cpupm_lp_dispatcher(u_register_t lp_id,
				    u_register_t act,
				    u_register_t arg1,
				    u_register_t arg2,
				    void *handle,
				    struct smccc_res *smccc_ret)
{
	uint64_t res = 0;
#ifdef CPU_PM_IRQ_REMAIN_ENABLE
	int ret;
#endif
	switch (lp_id) {
	case LP_CPC_COMMAND:
		res = mtk_cpc_handler(act, arg1, arg2);
		break;
#ifdef CPU_PM_IRQ_REMAIN_ENABLE
	case IRQS_REMAIN_ALLOC:
		if (act & MT_LPM_SMC_ACT_GET)
			res = (uint64_t)mt_lp_irqremain_count();
		break;
	case IRQS_REMAIN_CTRL:
		plat_cpu_pm_smc_lock();
		if (act & MT_LPM_SMC_ACT_SUBMIT)
			ret = mt_lp_irqremain_submit();
		else if (act & MT_LPM_SMC_ACT_PUSH) {
			ret = mt_lp_irqremain_push();
		if (ret)
			INFO("Irqs remain push fail\n");
		} else
			INFO("Irqs remain control not support! (0x%lx)\n", act);
		plat_cpu_pm_smc_unlock();
		break;
	case IRQS_REMAIN_IRQ:
	case IRQS_REMAIN_WAKEUP_CAT:
	case IRQS_REMAIN_WAKEUP_SRC:
		plat_cpu_pm_smc_lock();
		if (act & MT_LPM_SMC_ACT_SET) {
			const struct mt_lp_irqinfo info = {
				.val = (unsigned int)arg1,
			};

			ret = mt_lp_irqremain_set((unsigned int)lp_id, &info);
			if (ret)
				INFO("Irqs remain command: %lu, set fail\n",
				     lp_id);
		} else if (act & MT_LPM_SMC_ACT_GET) {
			struct mt_lp_irqinfo info;

			ret = mt_lp_irqremain_get((unsigned int)arg1,
						  (unsigned int)lp_id, &info);
			if (ret) {
				INFO("Irqs remain command: %lu, get fail\n",
				     lp_id);
				res = 0;
			} else
				res = (uint64_t)info.val;
		} else
			INFO("Irqs remain command not support! (0x%lx)\n", act);
		plat_cpu_pm_smc_unlock();
		break;
#ifdef CPU_PM_SUSPEND_NOTIFY
	case SUSPEND_SRC:
		ret = cpupm_set_suspend_state((unsigned int)act,
					      (unsigned int)arg1);
		if (ret)
			INFO("cpu_pm lp command: %lu, set fail\n", lp_id);
		break;
#endif
	case CPU_PM_COUNTER_CTRL:
		if (act & MT_LPM_SMC_ACT_SET)
			mtk_cpu_pm_counter_enable((bool)arg1);
		else if (act & MT_LPM_SMC_ACT_GET)
			res = (uint64_t)mtk_cpu_pm_counter_enabled();
		break;
	case CPU_PM_RECORD_CTRL:
		if (act & MT_LPM_SMC_ACT_GET) {
			if (arg1 == 0)
				res = mtk_mcusys_off_record_cnt_get();
			else if (arg1 == 1)
				res = mtk_mcusys_off_record_name_get();
		}
		break;
#if CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_CPU_SUSPEND_EN
	case SUSPEND_ABORT_REASON:
		if (act & MT_LPM_SMC_ACT_GET)
			res = mtk_suspend_abort_reason_get();
		break;
#endif
#ifdef CONFIG_MTK_CPU_ILDO
	case CPU_PM_RET_CTRL:
		if (act & MT_LPM_SMC_ACT_SET)
			res = cpupm_cpu_retention_control((unsigned int) arg1);
		else if (act & MT_LPM_SMC_ACT_GET)
			res = cpupu_get_cpu_retention_control();
		else if (act & MT_LPM_SMC_ACT_COMPAT)
			res = CPU_PM_CPU_RET_IS_ENABLED;
		break;
#endif
#endif
	default:
		break;
	}
	return res;
}

static uint64_t secure_cpupm_dispatcher(u_register_t lp_id,
					u_register_t act,
					u_register_t arg1,
					u_register_t arg2,
					void *handle,
					struct smccc_res *smccc_ret)
{
	uint64_t res = 0;

	switch (lp_id) {
	case CPC_COMMAND:
		res = mtk_cpc_trace_dump(act, arg1, arg2);
		break;
	default:
		break;
	}

	return res;
}

void cpupm_smc_init(void)
{
	plat_cpu_pm_smc_lock_init();
	mt_lpm_dispatcher_registry(MT_LPM_SMC_USER_CPU_PM,
				   cpupm_dispatcher);

	mt_lpm_dispatcher_registry(MT_LPM_SMC_USER_CPU_PM_LP,
				   cpupm_lp_dispatcher);

	mt_secure_lpm_dispatcher_registry(MT_LPM_SMC_USER_SECURE_CPU_PM,
					  secure_cpupm_dispatcher);
}
