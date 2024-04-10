/*
 * Copyright (c) 2016-2024, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/xlat_tables/xlat_mmu_helpers.h>
#include <plat/common/platform.h>

/* Pointer and function to register platform function to load alernate images */
const struct plat_try_images_ops *plat_try_img_ops;

void plat_setup_try_img_ops(const struct plat_try_images_ops *plat_try_ops)
{
	plat_try_img_ops = plat_try_ops;
}

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
