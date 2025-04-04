/*
 * Copyright (c) 2024-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Runtime C routines for errata workarounds and common routines */

#include <assert.h>

#include <arch.h>
#include <arch_helpers.h>
#include <cortex_a75.h>
#include <cortex_a510.h>
#include <cortex_a520.h>
#include <cortex_a710.h>
#include <cortex_a715.h>
#include <cortex_a720.h>
#include <cortex_a720_ae.h>
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

struct erratum_entry *find_erratum_entry(uint32_t errata_id)
{
	struct cpu_ops *cpu_ops;
	struct erratum_entry *entry, *end;

	cpu_ops = get_cpu_ops_ptr();
	assert(cpu_ops != NULL);

	entry = cpu_ops->errata_list_start;
	assert(entry != NULL);

	end = cpu_ops->errata_list_end;
	assert(end != NULL);

	end--; /* point to the last erratum entry of the queried cpu */

	while ((entry <= end)) {
		if (entry->id == errata_id) {
			return entry;
		}
		entry += 1;
	}
	return NULL;
}

bool check_if_trbe_disable_affected_core(void)
{
	switch (EXTRACT_PARTNUM(read_midr())) {
#if ERRATA_A520_2938996
	case EXTRACT_PARTNUM(CORTEX_A520_MIDR):
		return check_erratum_cortex_a520_2938996(cpu_get_rev_var()) == ERRATA_APPLIES;
#endif
#if ERRATA_X4_2726228
	case EXTRACT_PARTNUM(CORTEX_X4_MIDR):
		return check_erratum_cortex_x4_2726228(cpu_get_rev_var()) == ERRATA_APPLIES;
#endif
#if ERRATA_A510_2971420
	case EXTRACT_PARTNUM(CORTEX_A510_MIDR):
		return check_erratum_cortex_a510_2971420(cpu_get_rev_var()) == ERRATA_APPLIES;
#endif
	default:
		break;
	}
	return false;
}

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

#if ERRATA_A720_AE_3699562
	case EXTRACT_PARTNUM(CORTEX_A720_AE_MIDR):
		if (check_erratum_cortex_a720_ae_3699562(cpu_get_rev_var()) == ERRATA_APPLIES)
			return true;
		break;
#endif /* ERRATA_A720_AE_3699562 */

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

int check_erratum_applies(uint32_t cve, int errata_id)
{
	struct erratum_entry *entry;
	long rev_var;

	rev_var = cpu_get_rev_var();

	entry = find_erratum_entry(errata_id);

	if (entry == NULL) {
		return ERRATA_NOT_APPLIES;
	}

	assert(entry->cve == cve);

	return entry->check_func(rev_var);
}
