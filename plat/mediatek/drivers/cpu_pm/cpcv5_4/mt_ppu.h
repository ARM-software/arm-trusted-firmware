/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_PPU_H
#define MT_PPU_H

#include <lib/mmio.h>
#include "mt_cpu_pm.h"

/* PPU PWPR definition */
#define PPU_PWPR_MASK		0xF
#define PPU_PWPR_MODE_MASK	0x1
#define PPU_PWPR_OFF		0
#define PPU_PWPR_MEM_RET	2
#define PPU_PWPR_FULL_RET	5
#define PPU_PWPR_MEM_OFF	6
#define PPU_PWPR_FUN_RET	7
#define PPU_PWPR_ON		8
#define PPU_PWPR_WARM_RESET	10
#define PPU_PWPR_DYNAMIC_MODE	BIT(8)

#define PPU_PWPR_OP_MASK			0xF0000
#define PPU_PWPR_OP_DYNAMIC_MODE		BIT(24)
#define PPU_PWPR_OP_MODE(_policy)		(((_policy) << 16) & PPU_PWPR_OP_MASK)
#define PPU_PWPR_OP_ONE_SLICE_SF_ONLY		0
#define PPU_PWPR_OP_ONE_SLICE_HALF_DRAM		1
#define PPU_PWPR_OP_ONE_SLICE_FULL_DRAM		3
#define PPU_PWPR_OP_ALL_SLICE_SF_ONLY		4
#define PPU_PWPR_OP_ALL_SLICE_HALF_DRAM		5
#define PPU_PWPR_OP_ALL_SLICE_FULL_DRAM		7

#define DSU_PPU_PWPR_OP_MODE_DEF (PPU_PWPR_OP_ONE_SLICE_HALF_DRAM)

/* PPU PWSR definition */
#define PPU_PWSR_STATE_ON	BIT(3)

#ifdef CPU_PM_ACP_FSM
#define PPU_PWSR_OP_STATUS	0x30000
#define PPU_OP_ST_SF_ONLY	0x0
#endif /* CPU_PM_ACP_FSM */

#define MT_PPU_DCDR0			0x00606060
#define MT_PPU_DCDR1			0x00006060

void mt_smp_ppu_pwr_set(struct ppu_pwr_ctrl *ctrl,
			unsigned int mode,
			unsigned int policy);

void mt_smp_ppu_op_set(struct ppu_pwr_ctrl *ctrl,
		       unsigned int mode,
		       unsigned int policy);

void mt_smp_ppu_pwr_dynamic_set(struct ppu_pwr_ctrl *ctrl,
				unsigned int policy);

void mt_smp_ppu_pwr_static_set(struct ppu_pwr_ctrl *ctrl,
			       unsigned int policy);

void mt_smp_ppu_set(struct ppu_pwr_ctrl *ctrl,
		    unsigned int op_mode,
		    unsigned int policy,
		    unsigned int pwr_mode,
		    unsigned int pwr_policy);

#endif /* MT_PPU_H */
