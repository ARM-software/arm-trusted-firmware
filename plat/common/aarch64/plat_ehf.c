/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, Broadcom
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl31/ehf.h>

#include <platform_def.h>

/*
 * Enumeration of priority levels on ARM platforms.
 */
ehf_pri_desc_t plat_exceptions[] = {
#if RAS_EXTENSION
	/* RAS Priority */
	EHF_PRI_DESC(PLAT_PRI_BITS, PLAT_RAS_PRI),
#endif

#if SDEI_SUPPORT
	/* Critical priority SDEI */
	EHF_PRI_DESC(PLAT_PRI_BITS, PLAT_SDEI_CRITICAL_PRI),

	/* Normal priority SDEI */
	EHF_PRI_DESC(PLAT_PRI_BITS, PLAT_SDEI_NORMAL_PRI),
#endif
#if SPM_MM
	EHF_PRI_DESC(PLAT_PRI_BITS, PLAT_SP_PRI),
#endif
	/* Plaform specific exceptions description */
#ifdef PLAT_EHF_DESC
	PLAT_EHF_DESC,
#endif
};

/* Plug in ARM exceptions to Exception Handling Framework. */
EHF_REGISTER_PRIORITIES(plat_exceptions, ARRAY_SIZE(plat_exceptions), PLAT_PRI_BITS);
