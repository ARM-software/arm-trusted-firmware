/*
 * Copyright (c) 2024-2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Runtime C routines for errata workarounds and common routines */

#include <assert.h>

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <arm_impdef_regs.h>
#include <bl31/sync_handle.h>
#include <c1_pro.h>
#include <c1_ultra.h>
#include <cortex_a510.h>
#include <cortex_a520.h>
#include <cortex_a710.h>
#include <cortex_a715.h>
#include <cortex_a720.h>
#include <cortex_a720ae.h>
#include <cortex_a725.h>
#include <cortex_a75.h>
#include <cortex_x2.h>
#include <cortex_x3.h>
#include <cortex_x4.h>
#include <cortex_x925.h>
#include <lib/cpus/cpu_ops.h>
#include <lib/cpus/errata.h>
#include <neoverse_n2.h>
#include <neoverse_n3.h>
#include <neoverse_v2.h>
#include <neoverse_v3.h>
#include <neoverse_v3ae.h>

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
	case EXTRACT_PARTNUM(CORTEX_A720AE_MIDR):
		if (check_erratum_cortex_a720ae_3699562(cpu_get_rev_var()) == ERRATA_APPLIES)
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

#if ERRATA_V2_3701771
	case EXTRACT_PARTNUM(NEOVERSE_V2_MIDR):
		if (check_erratum_neoverse_v2_3701771(cpu_get_rev_var()) == ERRATA_APPLIES)
			return true;
		break;
#endif /* ERRATA_V2_3701771 */

#if ERRATA_V3_3701767
	case EXTRACT_PARTNUM(NEOVERSE_V3_MIDR):
		if (check_erratum_neoverse_v3_3701767(cpu_get_rev_var()) == ERRATA_APPLIES)
			return true;
		break;
	case EXTRACT_PARTNUM(NEOVERSE_V3_VNAE_MIDR):
		if (check_erratum_neoverse_v3ae_3701767(cpu_get_rev_var()) == ERRATA_APPLIES)
			return true;
		break;
#endif /* ERRATA_V3_3701767 */

#if ERRATA_C1ULTRA_3658374
	case EXTRACT_PARTNUM(C1_ULTRA_MIDR):
		if (check_erratum_c1_ultra_3658374(cpu_get_rev_var()) == ERRATA_APPLIES)
			return true;
		break;
#endif /* ERRATA_C1ULTRA_3658374 */

#if ERRATA_C1PRO_3300099 || ERRATA_C1PRO_3773617
	case EXTRACT_PARTNUM(C1_PRO_MIDR):
#if ERRATA_C1PRO_3300099
		if (check_erratum_c1_pro_3300099(cpu_get_rev_var()) == ERRATA_APPLIES)
			return true;
#endif /* ERRATA_C1PRO_3300099 */

#if ERRATA_C1PRO_3773617
		if (check_erratum_c1_pro_3773617(cpu_get_rev_var()) == ERRATA_APPLIES)
			return true;
#endif /* ERRATA_C1PRO_3773617 */
		break;
#endif /* ERRATA_C1PRO_3300099 || ERRATA_C1PRO_3773617 */

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

#if WORKAROUND_CVE_2025_10263
/*
 * Mitigation for CVE-2025-10263 requires inserting an extra TLBI and
 * DSB after a shareable TLBI-DSB sequence.
 */
void apply_cve_2025_10263_wa(void)
{
	tlbivale3is(0);
	dsbish();
}
#endif /* WORKAROUND_CVE_2025_10263 */

#if WORKAROUND_CVE_2025_0647
/*
 * Perform the workaround procedure for the CPP RCTX bug. The CPP RCTX
 * instruction usually takes an argument but that is unused by the workaround.
 */
int cve_2025_0647_cpp_wa(uint32_t iss)
{
	u_register_t scr_el3;
	u_register_t cpuactlr;
	u_register_t cpuectlr;

	(void) iss;

	psbcsync();
	tsbcsync();
	dsbosh();

	scr_el3 = read_scr_el3();

	/* Disable branch prediction. */
	cpuactlr = read_cpuactlr_el1();
	write_cpuactlr_el1(cpuactlr | BIT(0));

	/* Disable hardware prefetch. */
	cpuectlr = read_cpuectlr_el1();
	write_cpuectlr_el1(cpuectlr | BIT(15));
	isb();

	/*
	 * Execute CPP instruction for EL3 / root state
	 * EL3_rt: {GVMID,NSE,NS,EL,GASID} = {1,1,0,11,1}
	 */
	cpp_rctx(0x000100000b010000ULL);

	if (is_feat_rme_supported()) {
		/*
		 * Execute CPP instructions for realm state
		 * RL-EL2: {GVMID,NSE,NS,EL,GASID} = {1,1,1,10,1}
		 */
		cpp_rctx(0x000100000e010000ULL);
		/* RL-EL1: {GVMID,NSE,NS,EL,GASID} = {1,1,1,01,1} */
		cpp_rctx(0x000100000d010000ULL);
		/* RL-EL0: {GVMID,NSE,NS,EL,GASID} = {1,1,1,00,1} */
		cpp_rctx(0x000100000c010000ULL);
	}

	/*
	 * Execute CPP instructions for non-secure state
	 * EL2_ns: {GVMID,NSE,NS,EL,GASID} = {1,0,1,10,1}
	 */
	cpp_rctx(0x0001000006010000ULL);
	/* NS-EL1: {GVMID,NSE,NS,EL,GASID} = {1,0,1,01,1} */
	cpp_rctx(0x0001000005010000ULL);
	/* NS-EL0: {GVMID,NSE,NS,EL,GASID} = {1,0,1,00,1} */
	cpp_rctx(0x0001000004010000ULL);

	/*
	 * Execute CPP instructions for secure state
	 * EL1_s: {GVMID,NSE,NS,EL,GASID} = {1,0,0,01,1}
	 */
	cpp_rctx(0x0001000001010000ULL);
	/* S-EL0: {GVMID,NSE,NS,EL,GASID} = {1,0,0,00,1} */
	cpp_rctx(0x0001000000010000ULL);

	if ((scr_el3 & SCR_EEL2_BIT) != 0U) {
		/* S-EL2: {GVMID,NSE,NS,EL,GASID} = {1,0,0,10,1} */
		cpp_rctx(0x0001000002010000ULL);
	}

	dsbsy();

	/* EL3 / root state TLBI */
	tlbialle3();

	if (is_feat_rme_supported()) {
		/* Realm state TLBI {NSE,NS} = {1,1} */
		write_scr_el3(scr_el3 | SCR_NS_BIT | SCR_NSE_BIT);
		isb();
		tlbialle1();
		tlbialle2();
	}

	/* Non-secure state TLBI {NSE,NS} = {0,1} */
	write_scr_el3((scr_el3 | SCR_NS_BIT) & ~SCR_NSE_BIT);
	isb();
	tlbialle1();
	tlbialle2();

	/* Secure state TLBI {NSE,NS} = {0,0} */
	write_scr_el3(scr_el3 & ~(SCR_NS_BIT | SCR_NSE_BIT));
	isb();
	tlbialle1();

	if ((scr_el3 & SCR_EEL2_BIT) != 0U) {
		tlbialle2();
	}

	/* Clean up and restore register values. */
	dsbsy();
	write_scr_el3(scr_el3);
	write_cpuactlr_el1(cpuactlr);
	write_cpuectlr_el1(cpuectlr);

	isb();

	return TRAP_RET_CONTINUE;
}

/*
 * Performs the same workaround for the CPP RCTX bug except disables LS regional
 * clock gating on cores that require it for their workaround.
 */
int cve_2025_0647_cpp_no_gating_wa(uint32_t iss)
{
	u_register_t cpuactlr2;
	u_register_t ret;

	cpuactlr2 = read_cpuactlr2_el1();
	write_cpuactlr2_el1(cpuactlr2 | BIT(29));
	isb();

	ret = cve_2025_0647_cpp_wa(iss);

	write_cpuactlr2_el1(cpuactlr2);
	isb();

	return ret;
}
#endif /* WORKAROUND_CVE_2025_0647 */
