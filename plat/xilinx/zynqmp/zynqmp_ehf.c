/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) Siemens AG, 2020-2021
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <bl31/ehf.h>

/*
 * Enumeration of priority levels on ARM platforms.
 */
ehf_pri_desc_t zynqmp_exceptions[] = {
	/* Critical priority SDEI */
	EHF_PRI_DESC(PLAT_PRI_BITS, PLAT_SDEI_CRITICAL_PRI),

	/* Normal priority SDEI */
	EHF_PRI_DESC(PLAT_PRI_BITS, PLAT_SDEI_NORMAL_PRI),
};

/* Plug in ARM exceptions to Exception Handling Framework. */
EHF_REGISTER_PRIORITIES(zynqmp_exceptions, ARRAY_SIZE(zynqmp_exceptions), PLAT_PRI_BITS);
