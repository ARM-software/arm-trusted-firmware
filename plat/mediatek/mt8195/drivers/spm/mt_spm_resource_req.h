/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_RESOURCE_REQ_H
#define MT_SPM_RESOURCE_REQ_H

/* SPM resource request internal bit */
#define MT_SPM_BIT_XO_FPM	0
#define MT_SPM_BIT_26M		1
#define MT_SPM_BIT_INFRA	2
#define MT_SPM_BIT_SYSPLL	3
#define MT_SPM_BIT_DRAM_S0	4
#define MT_SPM_BIT_DRAM_S1	5

/* SPM resource request internal bit_mask */
#define MT_SPM_XO_FPM	BIT(MT_SPM_BIT_XO_FPM)
#define MT_SPM_26M	BIT(MT_SPM_BIT_26M)
#define MT_SPM_INFRA	BIT(MT_SPM_BIT_INFRA)
#define MT_SPM_SYSPLL	BIT(MT_SPM_BIT_SYSPLL)
#define MT_SPM_DRAM_S0	BIT(MT_SPM_BIT_DRAM_S0)
#define MT_SPM_DRAM_S1	BIT(MT_SPM_BIT_DRAM_S1)
#endif /* MT_SPM_RESOURCE_REQ_H */
