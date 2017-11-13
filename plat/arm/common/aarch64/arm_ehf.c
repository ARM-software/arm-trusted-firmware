/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <ehf.h>
#include <platform_def.h>

/*
 * Enumeration of priority levels on ARM platforms.
 */
ehf_pri_desc_t arm_exceptions[] = {
#if SDEI_SUPPORT
	/* Critical priority SDEI */
	EHF_PRI_DESC(ARM_PRI_BITS, PLAT_SDEI_CRITICAL_PRI),

	/* Normal priority SDEI */
	EHF_PRI_DESC(ARM_PRI_BITS, PLAT_SDEI_NORMAL_PRI),
#endif
};

/* Plug in ARM exceptions to Exception Handling Framework. */
EHF_REGISTER_PRIORITIES(arm_exceptions, ARRAY_SIZE(arm_exceptions), ARM_PRI_BITS);
