/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <s32cc-mc-me.h>
#include <s32cc-mc-rgm.h>

#define MC_ME_MAX_PARTITIONS		(4U)

#define MC_ME_CTL_KEY(MC_ME)		((MC_ME) + 0x0UL)
#define MC_ME_CTL_KEY_KEY		(0x5AF0U)
#define MC_ME_CTL_KEY_INVERTEDKEY	(0xA50FU)

#define MC_ME_PRTN_N(MC_ME, PART)	((MC_ME) + 0x100UL + ((PART) * 0x200UL))
#define MC_ME_PRTN_N_PCONF(MC_ME, PART)	(MC_ME_PRTN_N(MC_ME, PART))
#define MC_ME_PRTN_N_PCE		BIT_32(0)
#define MC_ME_PRTN_N_OSSE		BIT_32(2)
#define MC_ME_PRTN_N_PUPD(MC_ME, PART)	(MC_ME_PRTN_N(MC_ME, PART) + 0x4UL)
#define MC_ME_PRTN_N_PCUD		BIT_32(0)
#define MC_ME_PRTN_N_OSSUD		BIT_32(2)
#define MC_ME_PRTN_N_STAT(MC_ME, PART)	(MC_ME_PRTN_N(MC_ME, PART) + 0x8UL)
#define MC_ME_PRTN_N_PCS		BIT_32(0)
#define MC_ME_PRTN_N_COFB0_STAT(MC_ME, PART) \
					(MC_ME_PRTN_N(MC_ME, PART) + 0x10UL)
#define MC_ME_PRTN_N_COFB0_CLKEN(MC_ME, PART) \
					(MC_ME_PRTN_N(MC_ME, PART) + 0x30UL)
#define MC_ME_PRTN_N_REQ(PART)		BIT_32(PART)

#define RDC_RD_CTRL(RDC, PART)		((RDC) + ((PART) * 0x4UL))
#define RDC_CTRL_UNLOCK			BIT_32(31)
#define RDC_RD_INTERCONNECT_DISABLE	BIT_32(3)

#define RDC_RD_N_STATUS(RDC, PART)	((RDC) + ((PART) * 0x4UL) + 0x80UL)
#define RDC_RD_INTERCONNECT_DISABLE_STAT \
					BIT_32(4)

static bool is_interconnect_disabled(uintptr_t rdc, uint32_t part)
{
	return ((mmio_read_32(RDC_RD_N_STATUS(rdc, part)) &
		  RDC_RD_INTERCONNECT_DISABLE_STAT) != 0U);
}

static void enable_interconnect(uintptr_t rdc, uint32_t part)
{
	/* Unlock RDC register write */
	mmio_setbits_32(RDC_RD_CTRL(rdc, part), RDC_CTRL_UNLOCK);

	/* Clear corresponding RDC_RD_INTERCONNECT bit */
	mmio_clrbits_32(RDC_RD_CTRL(rdc, part), RDC_RD_INTERCONNECT_DISABLE);

	/* Wait until the interface gets enabled */
	while (is_interconnect_disabled(rdc, part)) {
	}

	/* Lock RDC register write */
	mmio_clrbits_32(RDC_RD_CTRL(rdc, part), RDC_CTRL_UNLOCK);
}

static int mc_me_check_partition_nb_valid(uint32_t part)
{
	if (part >= MC_ME_MAX_PARTITIONS) {
		ERROR("Invalid partition %" PRIu32 "\n", part);
		return -EINVAL;
	}

	return 0;
}

static void part_pconf_write_pce(uintptr_t mc_me, uint32_t pce_bit,
				 uint32_t part)
{
	mmio_clrsetbits_32(MC_ME_PRTN_N_PCONF(mc_me, part), MC_ME_PRTN_N_PCE,
			   pce_bit & MC_ME_PRTN_N_PCE);
}

static void mc_me_apply_hw_changes(uintptr_t mc_me)
{
	mmio_write_32(MC_ME_CTL_KEY(mc_me), MC_ME_CTL_KEY_KEY);
	mmio_write_32(MC_ME_CTL_KEY(mc_me), MC_ME_CTL_KEY_INVERTEDKEY);
}

static void part_pupd_update_and_wait(uintptr_t mc_me, uint32_t part,
				      uint32_t mask)
{
	uint32_t pconf, stat;

	mmio_setbits_32(MC_ME_PRTN_N_PUPD(mc_me, part), mask);

	mc_me_apply_hw_changes(mc_me);

	/* wait for the updates to apply */
	pconf = mmio_read_32(MC_ME_PRTN_N_PCONF(mc_me, part));
	do {
		stat = mmio_read_32(MC_ME_PRTN_N_STAT(mc_me, part));
	} while ((stat & mask) != (pconf & mask));
}

static void part_pconf_write_osse(uintptr_t mc_me, uint32_t osse_bit,
				  uint32_t part)
{
	mmio_clrsetbits_32(MC_ME_PRTN_N_PCONF(mc_me, part), MC_ME_PRTN_N_OSSE,
			   (osse_bit & MC_ME_PRTN_N_OSSE));
}

int mc_me_enable_partition(uintptr_t mc_me, uintptr_t mc_rgm, uintptr_t rdc,
			   uint32_t part)
{
	uint32_t part_stat;
	int ret;

	/* Partition 0 is already enabled by BootROM */
	if (part == 0U) {
		return 0;
	}

	ret = mc_me_check_partition_nb_valid(part);
	if (ret != 0) {
		return ret;
	}

	/* Enable a partition only if it's disabled */
	part_stat = mmio_read_32(MC_ME_PRTN_N_STAT(mc_me, part));
	if ((MC_ME_PRTN_N_PCS & part_stat) != 0U) {
		return 0;
	}

	part_pconf_write_pce(mc_me, MC_ME_PRTN_N_PCE, part);
	part_pupd_update_and_wait(mc_me, part, MC_ME_PRTN_N_PCUD);

	enable_interconnect(rdc, part);

	/* Release partition reset */
	mc_rgm_release_part(mc_rgm, part);

	/* Clear OSSE bit */
	part_pconf_write_osse(mc_me, 0, part);

	part_pupd_update_and_wait(mc_me, part, MC_ME_PRTN_N_OSSUD);

	mc_rgm_wait_part_deassert(mc_rgm, part);

	return 0;
}

void mc_me_enable_part_cofb(uintptr_t mc_me, uint32_t partition_n, uint32_t block,
			    bool check_status)
{
	uint32_t block_mask = MC_ME_PRTN_N_REQ(block);
	uintptr_t cofb_stat_addr;

	mmio_setbits_32(MC_ME_PRTN_N_COFB0_CLKEN(mc_me, partition_n),
			block_mask);

	mmio_setbits_32(MC_ME_PRTN_N_PCONF(mc_me, partition_n),
			MC_ME_PRTN_N_PCE);

	part_pupd_update_and_wait(mc_me, partition_n, MC_ME_PRTN_N_PCUD);

	cofb_stat_addr = MC_ME_PRTN_N_COFB0_STAT(mc_me, partition_n);
	if (check_status) {
		while ((mmio_read_32(cofb_stat_addr) & block_mask) == 0U) {
		}
	}
}
