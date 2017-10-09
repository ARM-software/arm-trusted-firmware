/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __XLAT_TABLES_ARCH_PRIVATE_H__
#define __XLAT_TABLES_ARCH_PRIVATE_H__

#include <assert.h>
#include <xlat_tables_defs.h>
#include <xlat_tables_v2.h>

/*
 * Return the execute-never mask that will prevent instruction fetch at all ELs
 * that are part of the given translation regime.
 */
static inline uint64_t xlat_arch_regime_get_xn_desc(xlat_regime_t regime)
{
	if (regime == EL1_EL0_REGIME) {
		return UPPER_ATTRS(UXN) | UPPER_ATTRS(PXN);
	} else {
		assert(regime == EL3_REGIME);
		return UPPER_ATTRS(XN);
	}
}

#endif /* __XLAT_TABLES_ARCH_PRIVATE_H__ */
