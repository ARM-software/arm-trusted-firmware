/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_PM_H
#define PLAT_PM_H

#include <lib/utils_def.h>

#define MT_PLAT_PWR_STATE_CPU			U(1)
#define MT_PLAT_PWR_STATE_CLUSTER		U(2)
#define MT_PLAT_PWR_STATE_MCUSYS		U(3)
#define MT_PLAT_PWR_STATE_SUSPEND2IDLE		U(8)
#define MT_PLAT_PWR_STATE_SYSTEM_SUSPEND	U(9)

#define MTK_LOCAL_STATE_RUN			U(0)
#define MTK_LOCAL_STATE_RET			U(1)
#define MTK_LOCAL_STATE_OFF			U(2)

#define MTK_AFFLVL_CPU				U(0)
#define MTK_AFFLVL_CLUSTER			U(1)
#define MTK_AFFLVL_MCUSYS			U(2)
#define MTK_AFFLVL_SYSTEM			U(3)

#define IS_CLUSTER_OFF_STATE(s)		\
		is_local_state_off(s->pwr_domain_state[MTK_AFFLVL_CLUSTER])
#define IS_MCUSYS_OFF_STATE(s)		\
		is_local_state_off(s->pwr_domain_state[MTK_AFFLVL_MCUSYS])
#define IS_SYSTEM_SUSPEND_STATE(s)	\
		is_local_state_off(s->pwr_domain_state[MTK_AFFLVL_SYSTEM])

#define IS_PLAT_SUSPEND_ID(stateid)\
		((stateid == MT_PLAT_PWR_STATE_SUSPEND2IDLE)	\
		|| (stateid == MT_PLAT_PWR_STATE_SYSTEM_SUSPEND))

#endif /* PLAT_PM_H */
