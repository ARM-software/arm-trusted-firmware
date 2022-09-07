/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_LPM_SMC_H
#define MT_LPM_SMC_H

/*
 * MTK LPM smc user format:
 * bit[31:24]: magic number
 * bit[23:16]: user number
 * bit[15:00]: user id
 */

#define MT_LPM_SMC_MAGIC	(0xDA000000)
#define MT_LPM_SMC_MAGIC_MASK	(0xFF000000)
#define MT_LPM_SMC_USER_MASK	(0xFF)
#define MT_LPM_SMC_USER_SHIFT	(16)

#define MT_LPM_SMC_USER_ID_MASK	(0x0000FFFF)

/*
 * cpu_pm is used for MCDI to read/write CPC information
 * spm_dbg is used for spm related debug information
 * spm is used for spm related settings
 * cpu_pm_lp is used for MCDI setting irq_remain
 */
enum mt_lpm_smc_user_id {
	MT_LPM_SMC_USER_CPU_PM = 0,
	MT_LPM_SMC_USER_SPM_DBG,
	MT_LPM_SMC_USER_SPM,
	MT_LPM_SMC_USER_CPU_PM_LP,
	MT_LPM_SMC_USER_SECURE_CPU_PM,
	MT_LPM_SMC_USER_SECURE_SPM_DBG,
	MT_LPM_SMC_USER_SECURE_SPM,
	MT_LPM_SMC_USER_MAX,
};

#define IS_MT_LPM_SMC(smcid)	((smcid & MT_LPM_SMC_MAGIC_MASK) == MT_LPM_SMC_MAGIC)

/* get real user id */
#define MT_LPM_SMC_USER(id)	((id >> MT_LPM_SMC_USER_SHIFT) & MT_LPM_SMC_USER_MASK)
#define MT_LPM_SMC_USER_ID(uid)	(uid & MT_LPM_SMC_USER_ID_MASK)

/* sink user id to smc's user id */
#define MT_LPM_SMC_USER_SINK(user, uid)	(((uid & MT_LPM_SMC_USER_ID_MASK) |\
					((user & MT_LPM_SMC_USER_MASK) << MT_LPM_SMC_USER_SHIFT)) |\
					MT_LPM_SMC_MAGIC)

/* sink cpu pm's smc id */
#define MT_LPM_SMC_USER_ID_CPU_PM(uid)	MT_LPM_SMC_USER_SINK(MT_LPM_SMC_USER_CPU_PM, uid)
/* sink spm's smc id */
#define MT_LPM_SMC_USER_ID_SPM(uid)	MT_LPM_SMC_USER_SINK(MT_LPM_SMC_USER_SPM, uid)

/* sink cpu pm's user id */
#define MT_LPM_SMC_USER_CPU_PM(uid)	MT_LPM_SMC_USER_ID_CPU_PM(uid)

/* sink spm's user id */
#define MT_LPM_SMC_USER_SPM(uid)	MT_LPM_SMC_USER_ID_SPM(uid)

/* behavior */
#define MT_LPM_SMC_ACT_SET		BIT(0)
#define MT_LPM_SMC_ACT_CLR		BIT(1)
#define MT_LPM_SMC_ACT_GET		BIT(2)
#define MT_LPM_SMC_ACT_PUSH		BIT(3)
#define MT_LPM_SMC_ACT_POP		BIT(4)
#define MT_LPM_SMC_ACT_SUBMIT		BIT(5)

/* compatible action for legacy smc from lk */
#define MT_LPM_SMC_ACT_COMPAT		BIT(31)

enum mt_lpm_spmc_compat_id {
	MT_LPM_SPMC_COMPAT_LK_FW_INIT,
	MT_LPM_SPMC_COMPAT_LK_MCDI_WDT_DUMP,
};

#endif /* MT_LPM_SMC_H */
