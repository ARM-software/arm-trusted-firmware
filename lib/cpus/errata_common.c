/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Runtime C routines for errata workarounds and common routines */

#include <arch.h>
#include <arch_helpers.h>
#include <cortex_a520.h>
#include <cortex_x4.h>
#include <cortex_a75.h>
#include <lib/cpus/cpu_ops.h>
#include <lib/cpus/errata.h>

#if ERRATA_A520_2938996 || ERRATA_X4_2726228
unsigned int check_if_affected_core(void)
{
	uint32_t midr_val = read_midr();
	long rev_var  = cpu_get_rev_var();

	if (EXTRACT_PARTNUM(midr_val) == EXTRACT_PARTNUM(CORTEX_A520_MIDR)) {
		return check_erratum_cortex_a520_2938996(rev_var);
	} else if (EXTRACT_PARTNUM(midr_val) == EXTRACT_PARTNUM(CORTEX_X4_MIDR)) {
		return check_erratum_cortex_x4_2726228(rev_var);
	}

	return ERRATA_NOT_APPLIES;
}
#endif

#if ERRATA_A75_764081
bool errata_a75_764081_applies(void)
{
	long rev_var = cpu_get_rev_var();
	if (check_erratum_cortex_a75_764081(rev_var) == ERRATA_APPLIES) {
		return true;
	}
	return false;
}
#endif /* ERRATA_A75_764081 */
