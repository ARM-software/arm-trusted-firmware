/*
 * Copyright (c) since 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <platform_def.h>

#define INFRA_AO_RES_CTRL_MASK			(INFRACFG_AO_BASE + 0xB8)
#define INFRA_AO_RES_CTRL_MASK_EMI_IDLE		BIT(18)
#define INFRA_AO_RES_CTRL_MASK_MPU_IDLE		BIT(15)

void spm_extern_initialize(void)
{
	unsigned int val;

	val = mmio_read_32(INFRA_AO_RES_CTRL_MASK);

	val |= (INFRA_AO_RES_CTRL_MASK_EMI_IDLE | INFRA_AO_RES_CTRL_MASK_MPU_IDLE);
	mmio_write_32(INFRA_AO_RES_CTRL_MASK, val);
}
