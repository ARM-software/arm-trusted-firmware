/*
 * Copyright (C) 2021-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>

#include <ddrphy_phyinit.h>

#include <lib/mmio.h>

#include <platform_def.h>

/*
 * This function implements the register restore portion of S3/IO
 * retention sequence.
 *
 * \note This function requiers the runtime_config.reten=1 to enable PhyInit exit retention feature.
 * This variable can be set as in
 * \return 0 on completion of the sequence, EXIT_FAILURE on error.
 */
int ddrphy_phyinit_restore_sequence(void)
{
	int ret;

	/*
	 * Before you call this functions perform the following:
	 * --------------------------------------------------------------------------
	 * -# Bring up VDD, VDDQ should already be up
	 * -# Since the CKE* and MEMRESET pin state must be protected, special care
	 *    must be taken to ensure that the following signals
	 *    - atpg_mode = 1'b0
	 *    - PwrOkIn = 1'b0
	 *
	 * -# The {BypassModeEn*, WRSTN} signals may be defined at VDD power-on, but
	 *    must be driven to ZERO at least 10ns prior to the asserting edge of PwrOkIn.
	 *
	 * -# Start Clocks and Reset the PHY
	 *    This step is identical to ddrphy_phyinit_usercustom_b_startclockresetphy()
	 */

	/* Write the MicroContMuxSel CSR to 0x0 to allow access to the internal CSRs */
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TAPBONLY | CSR_MICROCONTMUXSEL_ADDR))),
		      0x0U);

	/*
	 * Write the UcclkHclkEnables CSR to 0x3 to enable all the clocks so the reads can
	 * complete.
	 */
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TDRTUB | CSR_UCCLKHCLKENABLES_ADDR))),
		      0x3U);

	/*
	 * Assert CalZap to force impedance calibration FSM to idle.
	 * De-asserted as part of dfi_init_start/complete handshake by the PIE when DfiClk is valid.
	 */
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_CALZAP_ADDR))), 0x1U);

	/* Issue register writes to restore registers values */
	ret = ddrphy_phyinit_reginterface(RESTOREREGS, 0U, 0U);
	if (ret != 0) {
		return ret;
	}

	/*
	 * Write the UcclkHclkEnables CSR to disable the appropriate clocks after all reads done.
	 * Disabling Ucclk (PMU) and Hclk (training hardware).
	 */
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TDRTUB | CSR_UCCLKHCLKENABLES_ADDR))),
		      0x0U);

	/* Write the MicroContMuxSel CSR to 0x1 to isolate the internal CSRs during mission mode */
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TAPBONLY | CSR_MICROCONTMUXSEL_ADDR))),
		      0x1U);

	return 0;
}
