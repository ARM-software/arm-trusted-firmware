/*
 * Copyright (c) 2024-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Runtime C routines for errata workarounds and common routines */

#include <arch.h>
#include <arch_helpers.h>
#include <cortex_a75.h>
#include <cortex_a520.h>
#include <cortex_a710.h>
#include <cortex_a715.h>
#include <cortex_a720.h>
#include <cortex_a725.h>
#include <cortex_x2.h>
#include <cortex_x3.h>
#include <cortex_x4.h>
#include <cortex_x925.h>
#include <lib/cpus/cpu_ops.h>
#include <lib/cpus/errata.h>
#include <neoverse_n2.h>
#include <neoverse_n3.h>
#include <neoverse_v3.h>

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

bool errata_ich_vmcr_el2_applies(void)
{
	switch (EXTRACT_PARTNUM(read_midr())) {
#if ERRATA_A710_3701772
	case EXTRACT_PARTNUM(CORTEX_A710_MIDR):
		if (check_erratum_cortex_a710_3701772(cpu_get_rev_var()) == ERRATA_APPLIES)
			return true;
		break;
#endif /* ERRATA_A710_3701772 */

#if ERRATA_A715_3699560
	case EXTRACT_PARTNUM(CORTEX_A715_MIDR):
		if (check_erratum_cortex_a715_3699560(cpu_get_rev_var()) == ERRATA_APPLIES)
			return true;
		break;
#endif /* ERRATA_A715_3699560 */

#if ERRATA_A720_3699561
	case EXTRACT_PARTNUM(CORTEX_A720_MIDR):
		if (check_erratum_cortex_a720_3699561(cpu_get_rev_var()) == ERRATA_APPLIES)
			return true;;
		break;
#endif /* ERRATA_A720_3699561 */

#if ERRATA_A725_3699564
	case EXTRACT_PARTNUM(CORTEX_A725_MIDR):
		if (check_erratum_cortex_a725_3699564(cpu_get_rev_var()) == ERRATA_APPLIES)
			return true;
		break;
#endif /* ERRATA_A725_3699564 */

#if ERRATA_X2_3701772
	case EXTRACT_PARTNUM(CORTEX_X2_MIDR):
		if (check_erratum_cortex_x2_3701772(cpu_get_rev_var()) == ERRATA_APPLIES)
			return true;
		break;
#endif /* ERRATA_X2_3701772 */

#if ERRATA_X3_3701769
	case EXTRACT_PARTNUM(CORTEX_X3_MIDR):
		if (check_erratum_cortex_x3_3701769(cpu_get_rev_var()) == ERRATA_APPLIES)
			return true;
		break;
#endif /* ERRATA_X3_3701769 */

#if ERRATA_X4_3701758
	case EXTRACT_PARTNUM(CORTEX_X4_MIDR):
		if (check_erratum_cortex_x4_3701758(cpu_get_rev_var()) == ERRATA_APPLIES)
			return true;
		break;
#endif /* ERRATA_X4_3701758 */

#if ERRATA_X925_3701747
	case EXTRACT_PARTNUM(CORTEX_X925_MIDR):
		if (check_erratum_cortex_x925_3701747(cpu_get_rev_var()) == ERRATA_APPLIES)
			return true;
		break;
#endif /* ERRATA_X925_3701747 */

#if ERRATA_N2_3701773
	case EXTRACT_PARTNUM(NEOVERSE_N2_MIDR):
		if (check_erratum_neoverse_n2_3701773(cpu_get_rev_var()) == ERRATA_APPLIES)
			return true;
		break;
#endif /* ERRATA_N2_3701773 */

#if ERRATA_N3_3699563
	case EXTRACT_PARTNUM(NEOVERSE_N3_MIDR):
		if (check_erratum_neoverse_n3_3699563(cpu_get_rev_var()) == ERRATA_APPLIES)
			return true;
		break;
#endif /* ERRATA_N3_3699563 */

#if ERRATA_V3_3701767
	case EXTRACT_PARTNUM(NEOVERSE_V3_MIDR):
		if (check_erratum_neoverse_v3_3701767(cpu_get_rev_var()) == ERRATA_APPLIES)
			return true;
		break;
#endif /* ERRATA_V3_3701767 */

	default:
		break;
	}

	return false;
}
