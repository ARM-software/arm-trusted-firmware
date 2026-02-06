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

PER_CPU_DEFINE(brbe_regs_t, brbe_ctx);

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
	brbe_regs_t *ctx = PER_CPU_CUR(brbe_ctx);

	ctx->records = 0;
	ctx->brbfcr_el1 = read_brbfcr_el1();

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

		ctx->brbinf[ctx->records] = brbinf;
		ctx->brbsrc[ctx->records] = read_brbsrc(reg_num);
		ctx->brbtgt[ctx->records] = read_brbtgt(reg_num);
		ctx->records++;
	}
}

/*
 * Begin branch record capture at EL3. Will save the buffer and any registers
 * that get clobbered.
 *
 * NOTE: must call brbe_stop_recording() before exiting EL3 to prevent BRBE
 * leakage
 */
void brbe_start_recording(void)
{
	u_register_t brbfcr_val = 0U;

	brbe_context_save();

	/*
	 * Invalidate everything captured up to the last ctx sync event. Branch
	 * recording at EL3 should be disabled so it's fine if the exception
	 * entry is the most recent.
	 */
	brbiall();

	/*
	 * Enable recording of direct and indirect branch with link
	 * instructions. This will produce a trace of function calls. More
	 * granular tracing must be configured manually.
	 */
	brbfcr_val |= BRBFCR_EL1_DIRCALL_BIT;
	brbfcr_val |= BRBFCR_EL1_INDCALL_BIT;
	write_brbfcr_el1(brbfcr_val);

	/*
	 * Enable branch recording at EL3. Usee E3BREW so that recording stops
	 * as soon as the core encounters any reset.
	 */
	if (is_feat_brbev1p1_supported()) {
		write_mdcr_el3(read_mdcr_el3() | MDCR_E3BREW_BIT);
	}
	isb();
}

static void brbe_context_restore(void)
{
	brbe_regs_t *ctx = PER_CPU_CUR(brbe_ctx);

	for (uint8_t record = 0; record < ctx->records; record++) {
		write_brbinfinj_el1(ctx->brbinf[record]);
		write_brbsrcinj_el1(ctx->brbsrc[record]);
		write_brbtgtinj_el1(ctx->brbtgt[record]);

		/* Rule RPWKFJ means isb not needed before or after */
		brbinj();
	}

	write_brbfcr_el1(ctx->brbfcr_el1);
	isb();
}

/* Stop branch capture and put back BRBE context as it was */
void brbe_stop_recording(void)
{
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
