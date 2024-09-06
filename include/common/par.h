/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PAR_H
#define PAR_H

#include<arch_features.h>
#include<lib/extensions/sysreg128.h>

static inline uint64_t get_par_el1_pa(sysreg_t par)
{
	uint64_t pa = par & UINT64_MAX;
	/* PA, bits [51:12] is Output address */
	uint64_t mask = PAR_ADDR_MASK;

#if ENABLE_FEAT_D128
	/* If D128 is in use, the PA is in the upper 64-bit word of PAR_EL1 */
	if (is_feat_d128_supported() && (par & PAR_EL1_D128)) {
		pa = (par >> 64) & UINT64_MAX;
		/* PA, bits [55:12] is Output address */
		mask = PAR_D128_ADDR_MASK;
	}
#endif
	return pa & mask;
}

#endif /* PAR_H */
