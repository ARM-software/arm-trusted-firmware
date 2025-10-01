/*
 * Copyright 2023-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <lib/mmio.h>
#include <lib/mmio_poll.h>
#include <lib/utils_def.h>
#include <s32cc-mc-rgm.h>
#include <s32cc-clk-regs.h>

#define MC_RGM_PRST(RGM, PER)		((RGM) + 0x40UL + ((PER) * 0x8UL))
#define MC_RGM_PRST_PERIPH_N_RST(PER)	BIT_32(PER)
#define MC_RGM_PSTAT(RGM, PER)		((RGM) + 0x140UL + ((PER) * 0x8UL))
#define MC_RGM_PSTAT_PERIPH(PER)	BIT_32(PER)

/*  ERR051700
 *  Releasing more than one Software Resettable Domain (SRD)
 *  from reset simultaneously, by clearing the corresponding
 *  peripheral MC_RGM_PRSTn[PERIPH_x_RST] reset control may
 *  cause a false setting of the Fault Collection and
 *  Control Unit (FCCU) Non-Critical Fault (NCF) flag
 *  corresponding to a Memory-Test-Repair (MTR) Error
 */
#if (ERRATA_S32_051700 == 1)
void mc_rgm_periph_reset(uintptr_t rgm, uint32_t part, uint32_t value)
{
	uint32_t current_bit_checked, i;
	uint32_t current_regs, mask;
	int bit_index;

	current_regs = mmio_read_32(MC_RGM_PRST(rgm, part));
	/* Create a mask with all changed bits */
	mask = current_regs ^ value;

	while (mask != 0U) {
		bit_index = __builtin_ffs(mask);
		if (bit_index < 1) {
			break;
		}

		i = (uint32_t)bit_index - 1U;
		current_bit_checked = BIT_32(i);

		/* Check if we assert or de-assert.
		 * Also wait for completion.
		 */
		if ((value & current_bit_checked) != 0U) {
			mmio_setbits_32(MC_RGM_PRST(rgm, part),
					current_bit_checked);
			while ((mmio_read_32(MC_RGM_PRST(rgm, part)) &
				 current_bit_checked) == 0U)
				;
		} else {
			mmio_clrbits_32(MC_RGM_PRST(rgm, part),
					current_bit_checked);
			while ((mmio_read_32(MC_RGM_PRST(rgm, part)) &
					    current_bit_checked) != 0U)
				;
		}

		mask &= ~current_bit_checked;
	}
}
#else /* ERRATA_S32_051700 */
void mc_rgm_periph_reset(uintptr_t rgm, uint32_t part, uint32_t value)
{
	mmio_write_32(MC_RGM_PRST(rgm, part), value);
}
#endif /* ERRATA_S32_051700 */

void mc_rgm_release_periph(uintptr_t rgm, uint32_t part, uint32_t periph)
{
	uint32_t reg;

	reg = mmio_read_32(MC_RGM_PRST(rgm, part));
	reg &= ~MC_RGM_PRST_PERIPH_N_RST(periph);
	mc_rgm_periph_reset(rgm, part, reg);
}

void mc_rgm_release_part(uintptr_t rgm, uint32_t part)
{
	mc_rgm_release_periph(rgm, part, 0);
}

void mc_rgm_wait_part_deassert(uintptr_t rgm, uint32_t part)
{
	while ((mmio_read_32(MC_RGM_PSTAT(rgm, part)) &
		MC_RGM_PSTAT_PERIPH(0)) != 0U) {
	}
}

int mc_rgm_ddr_reset(uintptr_t rgm, uint32_t timeout)
{
	uint32_t pstat, prst;
	int err;

	prst = mmio_read_32(MC_RGM_PRST(MC_RGM_BASE_ADDR, 0U));
	if ((prst & MC_RGM_PRST_PERIPH_N_RST(3)) == 0U) {
		return -EINVAL;
	}

	mc_rgm_release_periph(MC_RGM_BASE_ADDR, 0, 3);

	err = mmio_read_32_poll_timeout(MC_RGM_PSTAT(MC_RGM_BASE_ADDR, 0U), pstat,
					((pstat & MC_RGM_PSTAT_PERIPH(3)) == 0U),
					timeout);
	return err;
}
