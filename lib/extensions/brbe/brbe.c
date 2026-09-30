/*
 * Copyright (c) 2022-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cdefs.h>

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <lib/extensions/brbe.h>
#include <lib/per_cpu/per_cpu.h>

#define BRBE_MDCR_EL3_PMEE_MASK	MDCR_PMEE(UL(3))

PER_CPU_DEFINE(brbe_regs_t, brbe_ctx);
PER_CPU_DEFINE(brbe_dump_pmu_regs_t, brbe_dump_pmu_ctx);

static uint64_t brbe_buffer_dump_threshold(void)
{
	return EXTRACT(BRBIDR0_EL1_NUMREC, read_brbidr0_el1());
}

static uint64_t brbe_pmevcnt_threshold(void)
{
	return UINT64_MAX - (brbe_buffer_dump_threshold() - 1);
}

static u_register_t read_brbinf(unsigned int n)
{
	switch (n) {
	case 0: return read_brbinf0_el1();
	case 1: return read_brbinf1_el1();
	case 2: return read_brbinf2_el1();
	case 3: return read_brbinf3_el1();
	case 4: return read_brbinf4_el1();
	case 5: return read_brbinf5_el1();
	case 6: return read_brbinf6_el1();
	case 7: return read_brbinf7_el1();
	case 8: return read_brbinf8_el1();
	case 9: return read_brbinf9_el1();
	case 10: return read_brbinf10_el1();
	case 11: return read_brbinf11_el1();
	case 12: return read_brbinf12_el1();
	case 13: return read_brbinf13_el1();
	case 14: return read_brbinf14_el1();
	case 15: return read_brbinf15_el1();
	case 16: return read_brbinf16_el1();
	case 17: return read_brbinf17_el1();
	case 18: return read_brbinf18_el1();
	case 19: return read_brbinf19_el1();
	case 20: return read_brbinf20_el1();
	case 21: return read_brbinf21_el1();
	case 22: return read_brbinf22_el1();
	case 23: return read_brbinf23_el1();
	case 24: return read_brbinf24_el1();
	case 25: return read_brbinf25_el1();
	case 26: return read_brbinf26_el1();
	case 27: return read_brbinf27_el1();
	case 28: return read_brbinf28_el1();
	case 29: return read_brbinf29_el1();
	case 30: return read_brbinf30_el1();
	case 31: return read_brbinf31_el1();
	default: return 0;
	}
}

static u_register_t read_brbsrc(unsigned int n)
{
	switch (n) {
	case 0: return read_brbsrc0_el1();
	case 1: return read_brbsrc1_el1();
	case 2: return read_brbsrc2_el1();
	case 3: return read_brbsrc3_el1();
	case 4: return read_brbsrc4_el1();
	case 5: return read_brbsrc5_el1();
	case 6: return read_brbsrc6_el1();
	case 7: return read_brbsrc7_el1();
	case 8: return read_brbsrc8_el1();
	case 9: return read_brbsrc9_el1();
	case 10: return read_brbsrc10_el1();
	case 11: return read_brbsrc11_el1();
	case 12: return read_brbsrc12_el1();
	case 13: return read_brbsrc13_el1();
	case 14: return read_brbsrc14_el1();
	case 15: return read_brbsrc15_el1();
	case 16: return read_brbsrc16_el1();
	case 17: return read_brbsrc17_el1();
	case 18: return read_brbsrc18_el1();
	case 19: return read_brbsrc19_el1();
	case 20: return read_brbsrc20_el1();
	case 21: return read_brbsrc21_el1();
	case 22: return read_brbsrc22_el1();
	case 23: return read_brbsrc23_el1();
	case 24: return read_brbsrc24_el1();
	case 25: return read_brbsrc25_el1();
	case 26: return read_brbsrc26_el1();
	case 27: return read_brbsrc27_el1();
	case 28: return read_brbsrc28_el1();
	case 29: return read_brbsrc29_el1();
	case 30: return read_brbsrc30_el1();
	case 31: return read_brbsrc31_el1();
	default: return 0;
	}
}

static u_register_t read_brbtgt(unsigned int n)
{
	switch (n) {
	case 0: return read_brbtgt0_el1();
	case 1: return read_brbtgt1_el1();
	case 2: return read_brbtgt2_el1();
	case 3: return read_brbtgt3_el1();
	case 4: return read_brbtgt4_el1();
	case 5: return read_brbtgt5_el1();
	case 6: return read_brbtgt6_el1();
	case 7: return read_brbtgt7_el1();
	case 8: return read_brbtgt8_el1();
	case 9: return read_brbtgt9_el1();
	case 10: return read_brbtgt10_el1();
	case 11: return read_brbtgt11_el1();
	case 12: return read_brbtgt12_el1();
	case 13: return read_brbtgt13_el1();
	case 14: return read_brbtgt14_el1();
	case 15: return read_brbtgt15_el1();
	case 16: return read_brbtgt16_el1();
	case 17: return read_brbtgt17_el1();
	case 18: return read_brbtgt18_el1();
	case 19: return read_brbtgt19_el1();
	case 20: return read_brbtgt20_el1();
	case 21: return read_brbtgt21_el1();
	case 22: return read_brbtgt22_el1();
	case 23: return read_brbtgt23_el1();
	case 24: return read_brbtgt24_el1();
	case 25: return read_brbtgt25_el1();
	case 26: return read_brbtgt26_el1();
	case 27: return read_brbtgt27_el1();
	case 28: return read_brbtgt28_el1();
	case 29: return read_brbtgt29_el1();
	case 30: return read_brbtgt30_el1();
	case 31: return read_brbtgt31_el1();
	default: return 0;
	}
}

void brbe_enable(cpu_context_t *ctx)
{
	el3_state_t *state = get_el3state_ctx(ctx);
	u_register_t mdcr_el3_val = read_ctx_reg(state, CTX_MDCR_EL3);

	/*
	 * MDCR_EL3.SBRBE = 0b01
	 * Allows BRBE usage in non-secure world and prohibited in
	 * secure world. This is relied on by SMCCC_ARCH_FEATURE_AVAILABILITY.
	 *
	 * MDCR_EL3.{E3BREW, E3BREC} = 0b00
	 * Branch recording at EL3 is disabled
	 */
	mdcr_el3_val &= ~((MDCR_SBRBE(MDCR_SBRBE_ALL)) | MDCR_E3BREW_BIT | MDCR_E3BREC_BIT);
	mdcr_el3_val |= (MDCR_SBRBE(MDCR_SBRBE_NS));
	write_ctx_reg(state, CTX_MDCR_EL3, mdcr_el3_val);
}

void brbe_enable_el3(void)
{
	/* Synchronize any branches that might have been captured so far.
	 * Recording should be disabled by now  */
	isb();
	/* rule SYLMQQ says to do this. Is this in scope of the threat model? */
	brbiall();
}

static void select_bank(uint8_t bank)
{
	u_register_t brbfcr_val = read_brbfcr_el1();

	UPDATE_REG_FIELD(BRBFCR_EL1_BANK, brbfcr_val, bank);

	write_brbfcr_el1(brbfcr_val);
	isb();
}

static void brbe_context_save(void)
{
	u_register_t brbinf;
	u_register_t num_records = EXTRACT(BRBIDR0_EL1_NUMREC, read_brbidr0_el1());
	brbe_regs_t *ctx_brbe = PER_CPU_CUR(brbe_ctx);

	ctx_brbe->records = 0;
	ctx_brbe->brbfcr_el1 = read_brbfcr_el1();
	ctx_brbe->brbcr_el1 = read_brbcr_el1();
	ctx_brbe->brbcr_el2 = read_brbcr_el2();

	for (uint8_t record = 0; record < num_records; record++) {
		uint8_t reg_num = record % BRBE_RECORDS_PER_BANK;

		if (reg_num == 0) {
			select_bank(record / BRBE_RECORDS_PER_BANK);
		}

		brbinf = read_brbinf(reg_num);

		/* Rule RSQLCX, there won't be any more valid records */
		if (EXTRACT(BRBINF_VALID, brbinf) == BRBINF_INVALID) {
			break;
		}

		ctx_brbe->brbinf[ctx_brbe->records] = brbinf;
		ctx_brbe->brbsrc[ctx_brbe->records] = read_brbsrc(reg_num);
		ctx_brbe->brbtgt[ctx_brbe->records] = read_brbtgt(reg_num);
		ctx_brbe->records++;
	}

	brbe_dump_pmu_regs_t *ctx_brbe_dump = PER_CPU_CUR(brbe_dump_pmu_ctx);

	ctx_brbe_dump->pm = read_pm();
	ctx_brbe_dump->pmselr_el0 = read_pmselr_el0();
	ctx_brbe_dump->pmcr_el0 = read_pmcr_el0();
	ctx_brbe_dump->pmecr_el1 = read_pmecr_el1();
	ctx_brbe_dump->brbe_pmcnt_set =
		read_pmcntenset_el0() & DEFAULT_BRBE_SELECTED_PMU_COUNTER_BIT;
	ctx_brbe_dump->brbe_pmint_set =
		read_pmintenset_el1() & DEFAULT_BRBE_SELECTED_PMU_COUNTER_BIT;
	ctx_brbe_dump->brbe_pmov_set =
		read_pmovsclr_el0() & DEFAULT_BRBE_SELECTED_PMU_COUNTER_BIT;
	ctx_brbe_dump->mdcr_el3 = read_mdcr_el3();

	write_pmselr_el0(DEFAULT_BRBE_SELECTED_PMU_COUNTER);
	isb();
	ctx_brbe_dump->pmxevtyper_el0 = read_pmxevtyper_el0();
	ctx_brbe_dump->pmxevcntr_el0 = read_pmxevcntr_el0();
}

/*
 * Begin branch record capture at EL3. Will save the buffer and any registers
 * that get clobbered, though PMU counter 0 will be unavailable for the
 * duration of profiling.
 *
 * NOTE: must call brbe_stop_recording() before exiting EL3 to prevent BRBE
 * leakage
 */
void brbe_start_recording(void)
{
	u_register_t brbfcr_val = 0U;
	u_register_t saved_pmselr_el0 = read_pmselr_el0();

	brbe_context_save();

	/* Enable FZP and FZPSS, so BRB recording freezes on PMU overflow. */
	write_brbcr_el1(read_brbcr_el1() | BRBCR_ELx_FZP_EN |
			BRBCR_ELx_FZPSS_EN);
	write_brbcr_el2(read_brbcr_el2() | BRBCR_ELx_FZP_EN |
			BRBCR_ELx_FZPSS_EN);

	/*
	 * PM is set when an exception is taken, and this masks all profiling
	 * exceptions (needed for buffer dumping, as dumping is triggered on
	 * PMU overflows which are profiling exceptions with FEAT_EBEP).
	 *
	 * Clearing PM allows profiling to take place in exception handlers,
	 * which is most of the EL3 runtime. Outside of boot, the only way to
	 * enter EL3 is via an exception.
	 */
	write_pm(0);

	/*
	 * Invalidate everything captured up to the last ctx sync event. Branch
	 * recording at EL3 should be disabled so it's fine if the exception
	 * entry is the most recent.
	 */
	brbiall();

	/* Enable count and interrupts on this counter, and clear overflow */
	write_pmcntenset_el0(DEFAULT_BRBE_SELECTED_PMU_COUNTER_BIT);
	write_pmintenset_el1(DEFAULT_BRBE_SELECTED_PMU_COUNTER_BIT);
	write_pmovsclr_el0(DEFAULT_BRBE_SELECTED_PMU_COUNTER_BIT);

	/* Track <buffer size> BRB_FILTRATE events */
	write_pmxevtyper_el0(BRB_FILTRATE_EVENT);
	write_pmxevcntr_el0(brbe_pmevcnt_threshold());

	/* Enable PMU exceptions */
	write_pmcr_el0(read_pmcr_el0() | PMCR_EL0_E_BIT);
	write_pmecr_el1(read_pmecr_el1() | PMECR_EL1_KPME);

	/* Reset PMSELR */
	write_pmselr_el0(saved_pmselr_el0);

	/*
	 * Enable recording of direct and indirect branch with link
	 * instructions. This will produce a trace of function calls. More
	 * granular tracing must be configured manually.
	 */
	brbfcr_val |= BRBFCR_EL1_DIRCALL_BIT;
	brbfcr_val |= BRBFCR_EL1_INDCALL_BIT;
	write_brbfcr_el1(brbfcr_val);

	/*
	 * Enable branch recording at EL3. Use E3BREW so that recording stops
	 * as soon as the core encounters any reset.
	 */
	if (is_feat_brbev1p1_supported()) {
		u_register_t mdcr_el3_val = read_mdcr_el3();
		/* Allow PMU exceptions at EL3 */
		mdcr_el3_val |= MDCR_SPME_BIT | MDCR_PMEE(MDCR_PMEE_EXC_ONLY) |
				MDCR_E3BREW_BIT;
		mdcr_el3_val &= ~(MDCR_SDD_BIT | MDCR_MPMX_BIT);
		write_mdcr_el3(mdcr_el3_val);
	}
	isb();
}

static void brbe_context_restore(void)
{
	brbe_regs_t *ctx_brbe = PER_CPU_CUR(brbe_ctx);
	brbe_dump_pmu_regs_t *ctx_brbe_dump = PER_CPU_CUR(brbe_dump_pmu_ctx);
	u_register_t saved_pmselr_el0;

	/*
	 * Restore this counter to its original state, potentially disabling
	 * counting, interrupts, and clearing overflow.
	 */
	write_pmcntenclr_el0(~ctx_brbe_dump->brbe_pmcnt_set &
		DEFAULT_BRBE_SELECTED_PMU_COUNTER_BIT);
	write_pmintenclr_el1(~ctx_brbe_dump->brbe_pmint_set &
		DEFAULT_BRBE_SELECTED_PMU_COUNTER_BIT);
	write_pmovsclr_el0(~ctx_brbe_dump->brbe_pmov_set &
		DEFAULT_BRBE_SELECTED_PMU_COUNTER_BIT);

	/* Select the ascribed PMU counter */
	saved_pmselr_el0 = read_pmselr_el0();
	write_pmselr_el0(DEFAULT_BRBE_SELECTED_PMU_COUNTER);
	isb();

	write_pmxevtyper_el0(ctx_brbe_dump->pmxevtyper_el0);
	write_pmxevcntr_el0(ctx_brbe_dump->pmxevcntr_el0);

	write_pmcr_el0(ctx_brbe_dump->pmcr_el0);
	write_pmecr_el1(ctx_brbe_dump->pmecr_el1);

	/* Reset PMSELR, MDCR, and PM */
	write_pmselr_el0(saved_pmselr_el0);
	write_mdcr_el3(ctx_brbe_dump->mdcr_el3);
	write_pm(ctx_brbe_dump->pm);

	for (uint8_t record = 0; record < ctx_brbe->records; record++) {
		write_brbinfinj_el1(ctx_brbe->brbinf[record]);
		write_brbsrcinj_el1(ctx_brbe->brbsrc[record]);
		write_brbtgtinj_el1(ctx_brbe->brbtgt[record]);

		/* Rule RPWKFJ means isb not needed before or after */
		brbinj();
	}

	/* Unset FZP and FZPSS */
	write_brbcr_el1(ctx_brbe->brbcr_el1);
	write_brbcr_el2(ctx_brbe->brbcr_el2);

	write_brbfcr_el1(ctx_brbe->brbfcr_el1);
	isb();
}

/* Stop branch capture and put back BRBE context as it was */
void brbe_stop_recording(void)
{
	write_brbfcr_el1(read_brbfcr_el1() | BRBFCR_EL1_PAUSED_BIT);
	isb();
	brbe_reset_buffer();

	if (is_feat_brbev1p1_supported()) {
		write_mdcr_el3(read_mdcr_el3() & ~MDCR_E3BREW_BIT);
	}

	/* Needed for both the MDCR write and the invalidate. No need for isb
	 * after since recording will be disabled. */
	isb();
	brbiall();

	brbe_context_restore();
}

/*
 * Dump all BRBE records. Most useful when recording at EL3 (via
 * brbe_start_recording()). Note that MDCR_EL3 is context switched so branch
 * recording at EL3 will not work at the fringes of EL3 execution
 */
void _brbe_dump_branch_records(void)
{
	u_register_t brbinf;
	u_register_t num_records = EXTRACT(BRBIDR0_EL1_NUMREC, read_brbidr0_el1());

	/* As of BRBEv1p1 there are 2 banks */
	assert(num_records <= 2 * BRBE_RECORDS_PER_BANK);

	for (uint8_t record = 0; record < num_records; record++) {
		uint8_t reg_num = record % BRBE_RECORDS_PER_BANK;

		if (reg_num == 0) {
			select_bank(record / BRBE_RECORDS_PER_BANK);
		}

		brbinf = read_brbinf(reg_num);

		/* Rule RSQLCX, there won't be any more valid records */
		if (EXTRACT(BRBINF_VALID, brbinf) == BRBINF_INVALID) {
			break;
		}

		INFO("BRBINF[%02u] = 0x%016lx, "
		     "SRC: 0x%016lx, TGT: 0x%016lx\n",
		     record, brbinf, read_brbsrc(reg_num), read_brbtgt(reg_num));
	}
}

void brbe_reset_buffer(void)
{
	u_register_t saved_pmselr_el0;

	/* Dump the branch records & prepare to reset the counter */
	_brbe_dump_branch_records();
	saved_pmselr_el0 = read_pmselr_el0();
	write_pmselr_el0(DEFAULT_BRBE_SELECTED_PMU_COUNTER);
	isb();

	/* Clear overflow flag & reset counter */
	write_pmovsclr_el0(U(1) << DEFAULT_BRBE_SELECTED_PMU_COUNTER);
	write_pmxevcntr_el0(brbe_pmevcnt_threshold());
	write_pmselr_el0(saved_pmselr_el0);
	isb();

	/* Invalidate current BRBE */
	brbiall();
}
