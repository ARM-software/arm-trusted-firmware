/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mt_ppu.h"

#define MTK_PPU_PWR_DYNAMIC_POLICY_SET(_ctrl, _policy) \
	mmio_clrsetbits_32(_ctrl->ppu_pwpr, \
			   PPU_PWPR_MASK, \
			   PPU_PWPR_DYNAMIC_MODE | ((_policy) & PPU_PWPR_MASK))

#define MTK_PPU_PWR_STATIC_POLICY_SET(_ctrl, _policy) \
	mmio_clrsetbits_32(_ctrl->ppu_pwpr, \
			   PPU_PWPR_MASK | PPU_PWPR_DYNAMIC_MODE, \
			   ((_policy) & PPU_PWPR_MASK))

void mt_smp_ppu_pwr_dynamic_set(struct ppu_pwr_ctrl *ctrl,
				unsigned int policy)
{
	CPU_PM_ASSERT(ctrl);
	MTK_PPU_PWR_DYNAMIC_POLICY_SET(ctrl, policy);
	dmbsy();
}

void mt_smp_ppu_pwr_static_set(struct ppu_pwr_ctrl *ctrl,
			       unsigned int policy)
{
	CPU_PM_ASSERT(ctrl);
	MTK_PPU_PWR_STATIC_POLICY_SET(ctrl, policy);
	dmbsy();
}

void mt_smp_ppu_pwr_set(struct ppu_pwr_ctrl *ctrl,
			unsigned int mode,
			unsigned int policy)
{
	CPU_PM_ASSERT(ctrl);
	if (mode & PPU_PWPR_DYNAMIC_MODE)
		MTK_PPU_PWR_DYNAMIC_POLICY_SET(ctrl, policy);
	else
		MTK_PPU_PWR_STATIC_POLICY_SET(ctrl, policy);
	mmio_write_32(ctrl->ppu_dcdr0, MT_PPU_DCDR0);
	mmio_write_32(ctrl->ppu_dcdr1, MT_PPU_DCDR1);
	dsbsy();
}

void mt_smp_ppu_op_set(struct ppu_pwr_ctrl *ctrl,
		       unsigned int mode,
		       unsigned int policy)
{
	unsigned int val;

	CPU_PM_ASSERT(ctrl);

	val = mmio_read_32(ctrl->ppu_pwpr);
	val &= ~(PPU_PWPR_OP_MASK | PPU_PWPR_OP_DYNAMIC_MODE);

	val |= PPU_PWPR_OP_MODE(policy);
	if (mode & PPU_PWPR_OP_DYNAMIC_MODE)
		val |= PPU_PWPR_OP_DYNAMIC_MODE;

	mmio_write_32(ctrl->ppu_pwpr, val);
	dsbsy();
}

void mt_smp_ppu_set(struct ppu_pwr_ctrl *ctrl,
		    unsigned int op_mode,
		    unsigned int policy,
		    unsigned int pwr_mode,
		    unsigned int pwr_policy)
{
	unsigned int val;

	CPU_PM_ASSERT(ctrl);
	val = mmio_read_32(ctrl->ppu_pwpr);

	if (op_mode & PPU_PWPR_OP_DYNAMIC_MODE)
		val |= (PPU_PWPR_OP_DYNAMIC_MODE |
		       PPU_PWPR_OP_MODE(policy));
	else
		val |= PPU_PWPR_OP_MODE(policy);

	if (pwr_mode & PPU_PWPR_DYNAMIC_MODE) {
		val &= ~(PPU_PWPR_MASK);
		val |= (PPU_PWPR_DYNAMIC_MODE | (pwr_policy & PPU_PWPR_MASK));
	} else {
		val &= ~(PPU_PWPR_MASK | PPU_PWPR_DYNAMIC_MODE);
		val |= (pwr_policy & PPU_PWPR_MASK);
	}
	mmio_write_32(ctrl->ppu_pwpr, val);
	dsbsy();
}
