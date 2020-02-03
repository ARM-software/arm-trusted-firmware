/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.h>
#include <lib/mmio.h>

#include "uniphier.h"

#define UNIPHIER_REVISION		0x5f800000UL
#define UNIPHIER_REVISION_NEW		0x1f800000UL

static unsigned int uniphier_get_revision_field(unsigned int mask,
						unsigned int shift)
{
	uintptr_t reg;

	if (BL_CODE_BASE >= 0x80000000UL)
		reg = UNIPHIER_REVISION;
	else
		reg = UNIPHIER_REVISION_NEW;

	return (mmio_read_32(reg) >> shift) & mask;
}

unsigned int uniphier_get_soc_type(void)
{
	return uniphier_get_revision_field(0xff, 16);
}

unsigned int uniphier_get_soc_model(void)
{
	return uniphier_get_revision_field(0x07, 8);
}

unsigned int uniphier_get_soc_revision(void)
{
	return uniphier_get_revision_field(0x1f, 0);
}

unsigned int uniphier_get_soc_id(void)
{
	uint32_t type = uniphier_get_soc_type();

	switch (type) {
	case 0x31:
		return UNIPHIER_SOC_LD11;
	case 0x32:
		return UNIPHIER_SOC_LD20;
	case 0x35:
		return UNIPHIER_SOC_PXS3;
	default:
		return UNIPHIER_SOC_UNKNOWN;
	}
}
