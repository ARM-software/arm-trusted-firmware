/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/xlat_tables/xlat_mmu_helpers.h>
#include <plat/common/platform.h>

/*
 * The following platform setup functions are weakly defined. They
 * provide typical implementations that may be re-used by multiple
 * platforms but may also be overridden by a platform if required.
 */
#pragma weak bl32_plat_enable_mmu


void bl32_plat_enable_mmu(uint32_t flags)
{
	enable_mmu_svc_mon(flags);
}
