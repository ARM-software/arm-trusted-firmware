/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_features.h>
#include <arch_helpers.h>
#include <bl32/tsp/tsp_el1_context.h>
#include <common/debug.h>

#define DUMMY_CTX_VALUE		ULL(0xffffffff)
#define DUMMY_CTX_TCR_VALUE	ULL(0xffff0000)
#define DUMMY_CTX_TRF_VALUE	ULL(0xf)
#define DUMMY_CTX_GCS_VALUE	ULL(0xffff0000)
#define DEFAULT_CTX_VALUE	ULL(0x0)

/**
 * -------------------------------------------------------
 * Private Helper functions required to access and modify
 * EL1 context registers at S-EL1.
 * -------------------------------------------------------
 */
static void modify_el1_common_regs(uint64_t cm_value)
{
	/**
	 * NOTE: Few EL1 registers "SCTLR_EL1, SPSR_EL1, ELR_EL1" are
	 *       left out consciously as those are important registers for
	 *       execution in each world and overwriting them with dummy value
	 *       would cause unintended crash while executing the test.
	 */
	write_tcr_el1(cm_value);
	write_cpacr_el1(cm_value);
	write_csselr_el1(cm_value);
	write_esr_el1(cm_value);
	write_ttbr0_el1(cm_value);
	write_ttbr1_el1(cm_value);
	write_mair_el1(cm_value);
	write_amair_el1(cm_value);
	write_actlr_el1(cm_value);
	write_tpidr_el1(cm_value);
	write_tpidr_el0(cm_value);
	write_tpidrro_el0(cm_value);
	write_par_el1(cm_value);
	write_far_el1(cm_value);
	write_afsr0_el1(cm_value);
	write_afsr1_el1(cm_value);
	write_contextidr_el1(cm_value);
	write_vbar_el1(cm_value);
	write_mdccint_el1(cm_value);
	write_mdscr_el1(cm_value);
}

static void modify_el1_mte2_regs(uint64_t mte_value)
{
	if (is_feat_mte2_supported()) {
		write_tfsre0_el1(mte_value);
		write_tfsr_el1(mte_value);
		write_rgsr_el1(mte_value);
		write_gcr_el1(mte_value);
	}
}

static void modify_el1_ras_regs(uint64_t ras_value)
{
	if (is_feat_ras_supported()) {
		write_disr_el1(ras_value);
	}
}

static void modify_el1_s1pie_regs(uint64_t s1pie_value)
{
	if (is_feat_s1pie_supported()) {
		write_pire0_el1(s1pie_value);
		write_pir_el1(s1pie_value);
	}
}

static void modify_el1_s1poe_regs(uint64_t s1poe_value)
{
	if (is_feat_s1poe_supported()) {
		write_por_el1(s1poe_value);
	}
}

static void modify_el1_s2poe_regs(uint64_t s2poe_value)
{
	if (is_feat_s2poe_supported()) {
		write_s2por_el1(s2poe_value);
	}
}

static void modify_el1_tcr2_regs(uint64_t tcr_value)
{
	if (is_feat_tcr2_supported()) {
		write_tcr2_el1(tcr_value & DUMMY_CTX_TCR_VALUE);
	}
}

static void modify_el1_trf_regs(uint64_t trf_value)
{
	if (is_feat_trf_supported()) {
		write_trfcr_el1(trf_value & DUMMY_CTX_TRF_VALUE);
	}
}

static void modify_el1_gcs_regs(uint64_t gcs_value)
{
	if (is_feat_gcs_supported()) {
		write_gcscr_el1(gcs_value & DUMMY_CTX_GCS_VALUE);
		write_gcscre0_el1(gcs_value & DUMMY_CTX_GCS_VALUE);
		write_gcspr_el1(gcs_value & DUMMY_CTX_GCS_VALUE);
		write_gcspr_el0(gcs_value & DUMMY_CTX_GCS_VALUE);
	}
}

/**
 * -----------------------------------------------------
 * Public API, to modify/restore EL1 ctx registers:
 * -----------------------------------------------------
 */
void modify_el1_ctx_regs(const bool modify_option)
{
	uint64_t mask;

	if (modify_option == TSP_CORRUPT_EL1_REGS) {
		VERBOSE("TSP(S-EL1): Corrupt EL1 Registers with Dummy values\n");
		mask = DUMMY_CTX_VALUE;
	} else {
		VERBOSE("TSP(S-EL1): Restore EL1 Registers with Default values\n");
		mask = DEFAULT_CTX_VALUE;
	}

	modify_el1_common_regs(mask);
	modify_el1_mte2_regs(mask);
	modify_el1_ras_regs(mask);
	modify_el1_s1pie_regs(mask);
	modify_el1_s1poe_regs(mask);
	modify_el1_s2poe_regs(mask);
	modify_el1_tcr2_regs(mask);
	modify_el1_trf_regs(mask);
	modify_el1_gcs_regs(mask);
}
