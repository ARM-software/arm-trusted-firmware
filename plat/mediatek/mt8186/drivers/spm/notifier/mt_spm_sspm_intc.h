/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_SSPM_INTC_H
#define MT_SPM_SSPM_INTC_H

#include <mt_spm_reg.h>

#define MT_SPM_SSPM_INTC_SEL_0	(0x10)
#define MT_SPM_SSPM_INTC_SEL_1	(0x20)
#define MT_SPM_SSPM_INTC_SEL_2	(0x40)
#define MT_SPM_SSPM_INTC_SEL_3	(0x80)

#define MT_SPM_SSPM_INTC_TRIGGER(id, sg) \
	(((0x10 << id) | (sg << id)) & 0xff)

#define MT_SPM_SSPM_INTC0_HIGH	MT_SPM_SSPM_INTC_TRIGGER(0, 1)
#define MT_SPM_SSPM_INTC0_LOW	MT_SPM_SSPM_INTC_TRIGGER(0, 0)
#define MT_SPM_SSPM_INTC1_HIGH	MT_SPM_SSPM_INTC_TRIGGER(1, 1)
#define MT_SPM_SSPM_INTC1_LOW	MT_SPM_SSPM_INTC_TRIGGER(1, 0)
#define MT_SPM_SSPM_INTC2_HIGH	MT_SPM_SSPM_INTC_TRIGGER(2, 1)
#define MT_SPM_SSPM_INTC2_LOW	MT_SPM_SSPM_INTC_TRIGGER(2, 0)
#define MT_SPM_SSPM_INTC3_HIGH	MT_SPM_SSPM_INTC_TRIGGER(3, 1)
#define MT_SPM_SSPM_INTC3_LOW	MT_SPM_SSPM_INTC_TRIGGER(3, 0)

/*
 * mt8186 use cpc pbi as notify.
 * Therefore, it won't need be notified by spm driver.
 */
#define DO_SPM_SSPM_LP_SUSPEND()
#define DO_SPM_SSPM_LP_RESUME()

#endif /* MT_SPM_SSPM_INTC_H */
