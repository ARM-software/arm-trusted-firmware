/*
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <bl31/ehf.h>

/*
 * Enumeration of priority levels on Tegra platforms.
 */
ehf_pri_desc_t tegra_exceptions[] = {
	/* Watchdog priority */
	EHF_PRI_DESC(PLAT_PRI_BITS, PLAT_TEGRA_WDT_PRIO),

#if SDEI_SUPPORT
	/* Critical priority SDEI */
	EHF_PRI_DESC(PLAT_PRI_BITS, PLAT_SDEI_CRITICAL_PRI),

	/* Normal priority SDEI */
	EHF_PRI_DESC(PLAT_PRI_BITS, PLAT_SDEI_NORMAL_PRI),
#endif
};

/* Plug in Tegra exceptions to Exception Handling Framework. */
EHF_REGISTER_PRIORITIES(tegra_exceptions, ARRAY_SIZE(tegra_exceptions), PLAT_PRI_BITS);
