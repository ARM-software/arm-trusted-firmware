/*
 * Copyright (c) 2018-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/common/platform.h>

#include <nrd_plat.h>
#include <nrd_variant.h>

unsigned int plat_arm_nrd_get_platform_id(void)
{
	return mmio_read_32(SSC_VERSION) & SSC_VERSION_PART_NUM_MASK;
}

unsigned int plat_arm_nrd_get_config_id(void)
{
	return (mmio_read_32(SSC_VERSION) >> SSC_VERSION_CONFIG_SHIFT)
			& SSC_VERSION_CONFIG_MASK;
}

unsigned int plat_arm_nrd_get_multi_chip_mode(void)
{
	return 0;
}

void bl31_platform_setup(void)
{
	nrd_bl31_common_platform_setup();
}
