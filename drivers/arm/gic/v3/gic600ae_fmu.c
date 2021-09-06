/*
 * Copyright (c) 2021, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Driver for GIC-600AE Fault Management Unit
 */

#include <assert.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gic600ae_fmu.h>
#include <drivers/arm/gicv3.h>

/* GIC-600 AE FMU specific register offsets */

/* GIC-600 AE FMU specific macros */
#define FMU_ERRIDR_NUM			U(44)
#define FMU_ERRIDR_NUM_MASK		U(0xFFFF)

/* Safety mechanisms for GICD block */
static char *gicd_sm_info[] = {
	"Reserved",
	"GICD dual lockstep error",
	"GICD AXI4 slave interface error",
	"GICD-PPI AXI4-Stream interface error",
	"GICD-ITS AXI4-Stream interface error",
	"GICD-SPI-Collator AXI4-Stream interface error",
	"GICD AXI4 master interface error",
	"SPI RAM DED error",
	"SGI RAM DED error",
	"Reserved",
	"LPI RAM DED error",
	"GICD-remote-GICD AXI4-Stream interface error",
	"GICD Q-Channel interface error",
	"GICD P-Channel interface error",
	"SPI RAM address decode error",
	"SGI RAM address decode error",
	"Reserved",
	"LPI RAM address decode error",
	"FMU dual lockstep error",
	"FMU ping ACK error",
	"FMU APB parity error",
	"GICD-Wake AXI4-Stream interface error",
	"GICD PageOffset or Chip ID error",
	"MBIST REQ error",
	"SPI RAM SEC error",
	"SGI RAM SEC error",
	"Reserved",
	"LPI RAM SEC error",
	"User custom SM0 error",
	"User custom SM1 error",
	"GICD-ITS Monolithic switch error",
	"GICD-ITS Q-Channel interface error",
	"GICD-ITS Monolithic interface error",
	"GICD FMU ClkGate override"
};

/* Safety mechanisms for PPI block */
static char *ppi_sm_info[] = {
	"Reserved",
	"PPI dual lockstep error",
	"PPI-GICD AXI4-Stream interface error",
	"PPI-CPU-IF AXI4-Stream interface error",
	"PPI Q-Channel interface error",
	"PPI RAM DED error",
	"PPI RAM address decode error",
	"PPI RAM SEC error",
	"PPI User0 SM",
	"PPI User1 SM",
	"MBIST REQ error",
	"PPI interrupt parity protection error",
	"PPI FMU ClkGate override"
};

/* Safety mechanisms for ITS block */
static char *its_sm_info[] = {
	"Reserved",
	"ITS dual lockstep error",
	"ITS-GICD AXI4-Stream interface error",
	"ITS AXI4 slave interface error",
	"ITS AXI4 master interface error",
	"ITS Q-Channel interface error",
	"ITS RAM DED error",
	"ITS RAM address decode error",
	"Bypass ACE switch error",
	"ITS RAM SEC error",
	"ITS User0 SM",
	"ITS User1 SM",
	"ITS-GICD Monolithic interface error",
	"MBIST REQ error",
	"ITS FMU ClkGate override"
};

/* Safety mechanisms for SPI Collator block */
static char *spicol_sm_info[] = {
	"Reserved",
	"SPI Collator dual lockstep error",
	"SPI-Collator-GICD AXI4-Stream interface error",
	"SPI Collator Q-Channel interface error",
	"SPI Collator Q-Channel clock error",
	"SPI interrupt parity error"
};

/* Safety mechanisms for Wake Request block */
static char *wkrqst_sm_info[] = {
	"Reserved",
	"Wake dual lockstep error",
	"Wake-GICD AXI4-Stream interface error"
};

/*
 * Initialization sequence for the FMU
 *
 * 1. enable error detection for error records that are passed in the blk_present_mask
 * 2. enable MBIST REQ and FMU Clk Gate override safety mechanisms for error records
 *    that are present on the platform
 *
 * The platforms are expected to pass `errctlr_ce_en` and `errctlr_ue_en`.
 */
void gic600_fmu_init(uint64_t base, uint64_t blk_present_mask,
		     bool errctlr_ce_en, bool errctlr_ue_en)
{
	unsigned int num_blk = gic_fmu_read_erridr(base) & FMU_ERRIDR_NUM_MASK;
	uint64_t errctlr;
	uint32_t smen;

	INFO("GIC600-AE FMU supports %d error records\n", num_blk);

	assert(num_blk == FMU_ERRIDR_NUM);

	/* sanitize block present mask */
	blk_present_mask &= FMU_BLK_PRESENT_MASK;

	/* Enable error detection for all error records */
	for (unsigned int i = 0U; i < num_blk; i++) {

		/* Skip next steps if the block is not present */
		if ((blk_present_mask & BIT(i)) == 0U) {
			continue;
		}

		/* Read the error record control register */
		errctlr = gic_fmu_read_errctlr(base, i);

		/* Enable error reporting and logging, if it is disabled */
		if ((errctlr & FMU_ERRCTLR_ED_BIT) == 0U) {
			errctlr |= FMU_ERRCTLR_ED_BIT;
		}

		/* Enable client provided ERRCTLR settings */
		errctlr |= (errctlr_ce_en ? (FMU_ERRCTLR_CI_BIT | FMU_ERRCTLR_CE_EN_BIT) : 0);
		errctlr |= (errctlr_ue_en ? FMU_ERRCTLR_UI_BIT : 0U);

		gic_fmu_write_errctlr(base, i, errctlr);
	}

	/*
	 * Enable MBIST REQ error and FMU CLK gate override safety mechanisms for
	 * all blocks
	 *
	 * GICD, SMID 23 and SMID 33
	 * PPI, SMID 10 and SMID 12
	 * ITS, SMID 13 and SMID 14
	 */
	if ((blk_present_mask & BIT(FMU_BLK_GICD)) != 0U) {
		smen = (GICD_MBIST_REQ_ERROR << FMU_SMEN_SMID_SHIFT) |
			(FMU_BLK_GICD << FMU_SMEN_BLK_SHIFT);
		gic_fmu_write_smen(base, smen);

		smen = (GICD_FMU_CLKGATE_ERROR << FMU_SMEN_SMID_SHIFT) |
			(FMU_BLK_GICD << FMU_SMEN_BLK_SHIFT);
		gic_fmu_write_smen(base, smen);
	}

	for (unsigned int i = FMU_BLK_PPI0; i < FMU_BLK_PPI31; i++) {
		if ((blk_present_mask & BIT(i)) != 0U) {
			smen = (PPI_MBIST_REQ_ERROR << FMU_SMEN_SMID_SHIFT) |
				(i << FMU_SMEN_BLK_SHIFT);
			gic_fmu_write_smen(base, smen);

			smen = (PPI_FMU_CLKGATE_ERROR << FMU_SMEN_SMID_SHIFT) |
				(i << FMU_SMEN_BLK_SHIFT);
			gic_fmu_write_smen(base, smen);
		}
	}

	for (unsigned int i = FMU_BLK_ITS0; i < FMU_BLK_ITS7; i++) {
		if ((blk_present_mask & BIT(i)) != 0U) {
			smen = (ITS_MBIST_REQ_ERROR << FMU_SMEN_SMID_SHIFT) |
				(i << FMU_SMEN_BLK_SHIFT);
			gic_fmu_write_smen(base, smen);

			smen = (ITS_FMU_CLKGATE_ERROR << FMU_SMEN_SMID_SHIFT) |
				(i << FMU_SMEN_BLK_SHIFT);
			gic_fmu_write_smen(base, smen);
		}
	}
}

/*
 * This function enable the GICD background ping engine. The GICD sends ping
 * messages to each remote GIC block, and expects a PING_ACK back within the
 * specified timeout. Pings need to be enabled after programming the timeout
 * value.
 */
void gic600_fmu_enable_ping(uint64_t base, uint64_t blk_present_mask,
		unsigned int timeout_val, unsigned int interval_diff)
{
	/*
	 * Populate the PING Mask to skip a specific block while generating
	 * background ping messages and enable the ping mechanism.
	 */
	gic_fmu_write_pingmask(base, ~blk_present_mask);
	gic_fmu_write_pingctlr(base, (interval_diff << FMU_PINGCTLR_INTDIFF_SHIFT) |
		(timeout_val << FMU_PINGCTLR_TIMEOUTVAL_SHIFT) | FMU_PINGCTLR_EN_BIT);
}

/* Print the safety mechanism description for a given block */
void gic600_fmu_print_sm_info(uint64_t base, unsigned int blk, unsigned int smid)
{
	if (blk == FMU_BLK_GICD && smid <= FMU_SMID_GICD_MAX) {
		INFO("GICD, SMID %d: %s\n", smid, gicd_sm_info[smid]);
	}

	if (blk == FMU_BLK_SPICOL && smid <= FMU_SMID_SPICOL_MAX) {
		INFO("SPI Collator, SMID %d: %s\n", smid, spicol_sm_info[smid]);
	}

	if (blk == FMU_BLK_WAKERQ && (smid <= FMU_SMID_WAKERQ_MAX)) {
		INFO("Wake Request, SMID %d: %s\n", smid, wkrqst_sm_info[smid]);
	}

	if (((blk >= FMU_BLK_ITS0) && (blk <= FMU_BLK_ITS7)) && (smid <= FMU_SMID_ITS_MAX)) {
		INFO("ITS, SMID %d: %s\n", smid, its_sm_info[smid]);
	}

	if (((blk >= FMU_BLK_PPI0) && (blk <= FMU_BLK_PPI31)) && (smid <= FMU_SMID_PPI_MAX)) {
		INFO("PPI, SMID %d: %s\n", smid, ppi_sm_info[smid]);
	}
}
