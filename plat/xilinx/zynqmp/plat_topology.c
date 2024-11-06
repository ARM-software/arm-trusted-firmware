/*
 * Copyright (c) 2013-2016, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdint.h>

#include <plat/common/platform.h>

static const uint8_t plat_power_domain_tree_desc[] = {1, 4};

const uint8_t *plat_get_power_domain_tree_desc(void)
{
	return plat_power_domain_tree_desc;
}
