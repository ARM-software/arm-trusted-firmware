/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <ehf.h>
#include "../hikey960_def.h"

/*
 * Enumeration of priority levels on ARM platforms.
 */
ehf_pri_desc_t hikey960_exceptions[] = {
	/* Critical priority */
	EHF_PRI_DESC(PLAT_GIC_PRI_BITS, PLAT_GIC_CRITICAL_PRI),

	/* Normal priority */
	EHF_PRI_DESC(PLAT_GIC_PRI_BITS, PLAT_GIC_NORMAL_PRI),
};

/* Plug in exceptions to Exception Handling Framework. */
EHF_REGISTER_PRIORITIES(hikey960_exceptions, ARRAY_SIZE(hikey960_exceptions),
			PLAT_GIC_PRI_BITS);
