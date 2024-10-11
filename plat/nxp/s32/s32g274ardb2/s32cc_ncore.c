/*
 * Copyright 2019-2021, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <lib/mmio.h>
#include <platform_def.h>

#include <s32cc-ncore.h>

static void ncore_diru_online(uint32_t diru)
{
	uint32_t numsfs, sf;

	numsfs = NCORE_CSID_NUMSFS(mmio_read_32(NCORE_CSID)) + 1U;

	/* Initialize all entries maintenance operation for each snoop filter */
	for (sf = 0U; sf < numsfs; sf++) {
		mmio_write_32(NCORE_DIRUSFMC(diru), NCORE_DIRUSFMC_SFID(sf) |
			      NCORE_DIRUSFMC_SFMNTOP_ALL);

		while ((mmio_read_32(NCORE_DIRUSFMA(diru)) & NCORE_DIRUSFMA_MNTOPACTV) != 0U) {
		}

		mmio_setbits_32(NCORE_DIRUSFE(diru), NCORE_DIRUSFE_SFEN(sf));
	}
}

void ncore_disable_caiu_isolation(uint32_t caiu)
{
	/* Exit from low-power state */
	mmio_clrbits_32(NCORE_CAIUTC(caiu), NCORE_CAIUTC_ISOLEN_MASK);
}

static void set_caiu(uint32_t caiu, bool on)
{
	uint32_t dirucase, csadser, caiuidr;
	uint32_t numdirus, diru;

	/* Enable or disable snoop messages to the CAI for each DIRU */
	numdirus = NCORE_CSUID_NUMDIRUS(mmio_read_32(NCORE_CSUID));
	for (diru = 0; diru < numdirus; diru++) {
		dirucase = mmio_read_32(NCORE_DIRUCASE(diru));

		if (on) {
			dirucase |= NCORE_DIRUCASE_CASNPEN(caiu);
		} else {
			dirucase &= ~NCORE_DIRUCASE_CASNPEN(caiu);
		}

		mmio_write_32(NCORE_DIRUCASE(diru), dirucase);
	}

	/* Enable or disable DVM messages to the CAI */
	caiuidr = mmio_read_32(NCORE_CAIUID(caiu));
	if ((caiuidr & NCORE_CAIUID_TYPE) == NCORE_CAIUID_TYPE_ACE_DVM) {
		csadser = mmio_read_32(NCORE_CSADSE);

		if (on) {
			csadser |= NCORE_CSADSE_DVMSNPEN(caiu);
		} else {
			csadser &= ~NCORE_CSADSE_DVMSNPEN(caiu);
		}

		mmio_write_32(NCORE_CSADSE, csadser);
	}
}

void ncore_caiu_online(uint32_t caiu)
{
	set_caiu(caiu, true);
}

void ncore_caiu_offline(uint32_t caiu)
{
	set_caiu(caiu, false);
}

bool ncore_is_caiu_online(uint32_t caiu)
{
	uint32_t stat = mmio_read_32(NCORE_CSADSE);

	return ((stat & NCORE_CSADSE_DVMSNPEN(caiu)) != 0U);
}

void ncore_init(void)
{
	uint32_t csuidr = mmio_read_32(NCORE_CSUID);
	uint32_t numdirus, diru;

	numdirus = NCORE_CSUID_NUMDIRUS(csuidr);
	for (diru = 0U; diru < numdirus; diru++) {
		/**
		 * Transition the directory to an online state by ensuring that
		 * all DIRUs within the interface are operational.
		 */
		ncore_diru_online(diru);
	}
}
