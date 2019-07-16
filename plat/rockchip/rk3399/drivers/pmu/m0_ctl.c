/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <m0_ctl.h>
#include <plat_private.h>
#include <rk3399_def.h>
#include <secure.h>
#include <soc.h>

void m0_init(void)
{
	/* secure config for M0 */
	mmio_write_32(SGRF_BASE + SGRF_PMU_CON(0), WMSK_BIT(7));
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(6), WMSK_BIT(12));

	/* document is wrong, PMU_CRU_GATEDIS_CON0 do not need set MASK BIT */
	mmio_setbits_32(PMUCRU_BASE + PMUCRU_GATEDIS_CON0, 0x02);

	/*
	 * To switch the parent to xin24M and div == 1,
	 *
	 * We need to close most of the PLLs and clocks except the OSC 24MHz
	 * durning suspend, and this should be enough to supplies the ddrfreq,
	 * For the simple handle, we just keep the fixed 24MHz to supply the
	 * suspend and ddrfreq directly.
	 */
	mmio_write_32(PMUCRU_BASE + PMUCRU_CLKSEL_CON0,
		      BIT_WITH_WMSK(15) | BITS_WITH_WMASK(0x0, 0x1f, 8));

	mmio_write_32(PMUCRU_BASE + PMUCRU_CLKGATE_CON2, WMSK_BIT(5));
}

void m0_configure_execute_addr(uintptr_t addr)
{
	/* set the execute address for M0 */
	mmio_write_32(SGRF_BASE + SGRF_PMU_CON(3),
		      BITS_WITH_WMASK((addr >> 12) & 0xffff,
				      0xffffu, 0));
	mmio_write_32(SGRF_BASE + SGRF_PMU_CON(7),
		      BITS_WITH_WMASK((addr >> 28) & 0xf,
				      0xfu, 0));
}

void m0_start(void)
{
	/* enable clocks for M0 */
	mmio_write_32(PMUCRU_BASE + PMUCRU_CLKGATE_CON2,
		      BITS_WITH_WMASK(0x0, 0xf, 0));

	/* clean the PARAM_M0_DONE flag, mean that M0 will start working */
	mmio_write_32(M0_PARAM_ADDR + PARAM_M0_DONE, 0);
	dmbst();

	mmio_write_32(PMUCRU_BASE + PMUCRU_SOFTRST_CON0,
		      BITS_WITH_WMASK(0x0, 0x4, 0));

	udelay(5);
	/* start M0 */
	mmio_write_32(PMUCRU_BASE + PMUCRU_SOFTRST_CON0,
		      BITS_WITH_WMASK(0x0, 0x20, 0));
	dmbst();
}

void m0_stop(void)
{
	/* stop M0 */
	mmio_write_32(PMUCRU_BASE + PMUCRU_SOFTRST_CON0,
		      BITS_WITH_WMASK(0x24, 0x24, 0));

	/* disable clocks for M0 */
	mmio_write_32(PMUCRU_BASE + PMUCRU_CLKGATE_CON2,
		      BITS_WITH_WMASK(0xf, 0xf, 0));
}

void m0_wait_done(void)
{
	do {
		/*
		 * Don't starve the M0 for access to SRAM, so delay before
		 * reading the PARAM_M0_DONE value again.
		 */
		udelay(5);
		dsb();
	} while (mmio_read_32(M0_PARAM_ADDR + PARAM_M0_DONE) != M0_DONE_FLAG);

	/*
	 * Let the M0 settle into WFI before we leave. This is so we don't reset
	 * the M0 in a bad spot which can cause problems with the M0.
	 */
	udelay(10);
	dsb();
}
