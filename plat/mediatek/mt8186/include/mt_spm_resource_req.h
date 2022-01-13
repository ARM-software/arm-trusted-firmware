/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_RESOURCE_REQ_H
#define MT_SPM_RESOURCE_REQ_H

/* SPM resource request internal bit */
#define MT_SPM_BIT_XO_FPM	(0U)
#define MT_SPM_BIT_26M		(1U)
#define MT_SPM_BIT_INFRA	(2U)
#define MT_SPM_BIT_SYSPLL	(3U)
#define MT_SPM_BIT_DRAM_S0	(4U)
#define MT_SPM_BIT_DRAM_S1	(5U)

/* SPM resource request internal bit_mask */
#define MT_SPM_XO_FPM	BIT(MT_SPM_BIT_XO_FPM)
#define MT_SPM_26M	BIT(MT_SPM_BIT_26M)
#define MT_SPM_INFRA	BIT(MT_SPM_BIT_INFRA)
#define MT_SPM_SYSPLL	BIT(MT_SPM_BIT_SYSPLL)
#define MT_SPM_DRAM_S0	BIT(MT_SPM_BIT_DRAM_S0)
#define MT_SPM_DRAM_S1	BIT(MT_SPM_BIT_DRAM_S1)

char spm_resource_req(unsigned int user, unsigned int req_mask);

#define IS_PLAT_SUSPEND_ID(stateid)\
	((stateid == MT_PLAT_PWR_STATE_SUSPEND2IDLE)\
	 || (stateid == MT_PLAT_PWR_STATE_SYSTEM_SUSPEND))

#endif /* MT_SPM_RESOURCE_REQ_H */
