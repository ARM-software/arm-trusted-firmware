/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef IMX_CSU_H
#define IMX_CSU_H

#include <arch.h>

/*
 * Security Reference Manual for i.MX 7Dual and 7Solo Applications Processors,
 * Rev. 0, 03/2017 Section 3.3.1
 *
 * Config secure level register (CSU_CSLn)
 */
#define CSU_CSL_LOCK_S1		BIT(24)
#define CSU_CSL_NSW_S1		BIT(23)
#define CSU_CSL_NUW_S1		BIT(22)
#define CSU_CSL_SSW_S1		BIT(21)
#define CSU_CSL_SUW_S1		BIT(20)
#define CSU_CSL_NSR_S1		BIT(19)
#define CSU_CSL_NUR_S1		BIT(18)
#define CSU_CSL_SSR_S1		BIT(17)
#define CSU_CSL_SUR_S1		BIT(16)
#define CSU_CSL_LOCK_S2		BIT(8)
#define CSU_CSL_NSW_S2		BIT(7)
#define CSU_CSL_NUW_S2		BIT(6)
#define CSU_CSL_SSW_S2		BIT(5)
#define CSU_CSL_SUW_S2		BIT(4)
#define CSU_CSL_NSR_S2		BIT(3)
#define CSU_CSL_NUR_S2		BIT(2)
#define CSU_CSL_SSR_S2		BIT(1)
#define CSU_CSL_SUR_S2		BIT(0)

#define CSU_CSL_OPEN_ACCESS  (CSU_CSL_NSW_S1 | CSU_CSL_NUW_S1 | CSU_CSL_SSW_S1 |\
			      CSU_CSL_SUW_S1 | CSU_CSL_NSR_S1 | CSU_CSL_NUR_S1 |\
			      CSU_CSL_SSR_S1 | CSU_CSL_SUR_S1 | CSU_CSL_NSW_S2 |\
			      CSU_CSL_NUW_S2 | CSU_CSL_SSW_S2 | CSU_CSL_SUW_S2 |\
			      CSU_CSL_NSR_S2 | CSU_CSL_NUR_S2 | CSU_CSL_SSR_S2 |\
			      CSU_CSL_SUR_S2)
void imx_csu_init(void);

#endif /* IMX_CSU_H */
