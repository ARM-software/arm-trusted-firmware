/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MT_CPU_PM_H__
#define __MT_CPU_PM_H__

#define MCUSYS_STATUS_PDN		(1 << 0UL)
#define MCUSYS_STATUS_CPUSYS_PROTECT	(1 << 8UL)
#define MCUSYS_STATUS_MCUSYS_PROTECT	(1 << 9UL)

/* cpu_pm function ID*/
enum mt_cpu_pm_user_id {
	MCUSYS_STATUS,
	CPC_COMMAND,
	IRQ_REMAIN_LIST_ALLOC,
	IRQ_REMAIN_IRQ_ADD,
	IRQ_REMAIN_IRQ_SUBMIT,
	MBOX_INFO,
};

/* cpu_pm lp function ID */
enum mt_cpu_pm_lp_smc_id {
	LP_CPC_COMMAND,
	IRQS_REMAIN_ALLOC,
	IRQS_REMAIN_CTRL,
	IRQS_REMAIN_IRQ,
	IRQS_REMAIN_WAKEUP_CAT,
	IRQS_REMAIN_WAKEUP_SRC,
};

#endif
