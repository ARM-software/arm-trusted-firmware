/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>

#include <drivers/qti/pdc/pdc_seq.h>
#include "pdc_regs.h"

#include <platform_def.h>

extern struct pdc_seq *g_pdc_seqs;
extern uint32_t g_pdc_seq_count;

static enum pdc_seq_result pdc_seq_internal_cfg(struct pdc_seq *seq)
{
	uint32_t i;
	const struct pdc_seq_cfg *cfg = seq->cfg;

	for (i = 0U; i < cfg->br_count; i++) {
		PDC_SEQ_BR_ADDR_WRITE(seq->addr, i, cfg->br_addr[i]);
	}

	for (i = 0U; i < cfg->delay_count; i++) {
		PDC_SEQ_DELAY_WRITE(seq->addr, i, cfg->delay[i]);
	}

	if (PDC_PARAM_PROFILING_UNIT(seq->addr) != PDC_TS_COUNT) {
		return PDC_SEQ_INVALID_PARAM;
	}

	return PDC_SEQ_SUCCESS;
}

static uint16_t __pdc_seq_copy_cmd_seq(uint8_t *pdc_base, uint16_t start_addr,
				       uint8_t *cmds, uint16_t length)
{
	const uint16_t cmds_per_reg = 4U;
	uint32_t max_cmds;
	uint32_t mask = 0U;
	uint32_t value = 0U;
	uint32_t cmd_shift = 0U;
	uint16_t cmds_copied = 0U;
	uint16_t i;
	uint16_t addr = start_addr;

	max_cmds = PDC_PARAM_SEQ_CMD_WORDS(pdc_base) * cmds_per_reg;

	if ((start_addr >= max_cmds) ||
	    ((uint32_t)(start_addr + length) > max_cmds)) {
		return 0U;
	}

	for (i = 0U; i < length; i++, addr++) {
		switch (addr % cmds_per_reg) {
		case 0:
			if (cmds_copied != 0U) {
				PDC_SEQ_MEM_RMW((addr - 1U) / cmds_per_reg,
						mask, value);
				value = 0U;
			}
			cmd_shift = 0U;
			break;
		case 1:
			cmd_shift = 8U;
			break;
		case 2:
			cmd_shift = 16U;
			break;
		case 3:
			cmd_shift = 24U;
			break;
		default:
			break;
		}

		mask  |= (0xFFU << cmd_shift);
		value |= ((uint32_t)cmds[i] << cmd_shift);
		cmds_copied++;
	}

	PDC_SEQ_MEM_RMW((addr - 1U) / cmds_per_reg, mask, value);

	return cmds_copied;
}

static enum pdc_seq_result pdc_seq_copy_cmd_seq(struct pdc_seq *seq)
{
	uint32_t i;
	uint32_t j;
	uint32_t copied;
	enum pdc_seq_result result = PDC_SEQ_SUCCESS;
	struct pdc_seq_mode *curr;
	const struct pdc_seq_mode *ref = NULL;

	for (i = 0U; i < seq->mode_count; i++) {
		curr = &seq->modes[i];

		if (curr->length == 0U) {
			return PDC_SEQ_INVALID_PARAM;
		}

		for (j = 0U; j < i; j++) {
			ref = &seq->modes[j];
			if (ref->cmds == curr->cmds) {
				break;
			}
		}

		if (j == i) {
			copied = __pdc_seq_copy_cmd_seq(seq->addr,
							seq->copied_cmds,
							curr->cmds,
							curr->length);
			if (copied == 0U) {
				return PDC_SEQ_NO_MEM;
			}

			curr->start_addr = (int16_t)seq->copied_cmds;
			seq->copied_cmds += (uint16_t)copied;
		} else {
			curr->start_addr = ref->start_addr;
		}
	}

	return result;
}

static enum pdc_seq_result pdc_seq_init(struct pdc_seq *seq)
{
	enum pdc_seq_result result;

	if ((seq == NULL) || (seq->cfg == NULL) ||
	    (seq->modes == NULL) || (seq->offset == 0U)) {
		return PDC_SEQ_INVALID_HANDLE;
	}

	seq->addr = (uint8_t *)(QTI_AOSS_BASE + seq->offset);
	seq->rsc_addr = (uint8_t *)(QTI_APSS_HM_BASE + seq->rsc_offset);

	result = pdc_seq_internal_cfg(seq);
	if (result != PDC_SEQ_SUCCESS) {
		return result;
	}

	return pdc_seq_copy_cmd_seq(seq);
}

void pdc_seq_sys_init(void)
{
	uint32_t i;
	struct pdc_seq *seq;

	for (i = 0U; i < g_pdc_seq_count; i++) {
		seq = &g_pdc_seqs[i];

		if (pdc_seq_init(seq) != PDC_SEQ_SUCCESS) {
			ERROR("PDC sequence init failed");
			return;
		}

		PDC_ENABLE_PDC_RMW(seq->addr, 1U);
	}
}
