/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SGI_DMC620_TZC_REGIONS_H
#define SGI_DMC620_TZC_REGIONS_H

#include <drivers/arm/tzc_dmc620.h>

#if SPM_MM
#define CSS_SGI_DMC620_TZC_REGIONS_DEF				\
	{							\
		.region_base = ARM_AP_TZC_DRAM1_BASE,		\
		.region_top = PLAT_SP_IMAGE_NS_BUF_BASE - 1,	\
		.sec_attr = TZC_DMC620_REGION_S_RDWR		\
	}, {							\
		.region_base = PLAT_SP_IMAGE_NS_BUF_BASE,	\
		.region_top = PLAT_ARM_SP_IMAGE_STACK_BASE - 1,	\
		.sec_attr = TZC_DMC620_REGION_S_NS_RDWR		\
	}, {							\
		.region_base = PLAT_ARM_SP_IMAGE_STACK_BASE,	\
		.region_top = ARM_AP_TZC_DRAM1_END,		\
		.sec_attr = TZC_DMC620_REGION_S_RDWR		\
	}
#else
#define CSS_SGI_DMC620_TZC_REGIONS_DEF				\
	{							\
		.region_base = ARM_AP_TZC_DRAM1_BASE,		\
		.region_top = ARM_AP_TZC_DRAM1_END,		\
		.sec_attr = TZC_DMC620_REGION_S_RDWR		\
	}
#endif /* SPM_MM */

#endif /* SGI_DMC620_TZC_REGIONS_H */
