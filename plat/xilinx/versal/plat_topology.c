/*
 * Copyright (c) 2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/common/platform.h>
#include <platform_def.h>

static const uint8_t plat_power_domain_tree_desc[] = {1, PLATFORM_CORE_COUNT};

const uint8_t *plat_get_power_domain_tree_desc(void)
{
	return plat_power_domain_tree_desc;
}
