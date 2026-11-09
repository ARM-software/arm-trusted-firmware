/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * LPASS side of the boot IMEM release on the hoya SoCs whose boot chain uses
 * the LPASS Q6 memory as boot IMEM, built against the chipset's
 * clock_hwio.h. The chipset ends the boot IMEM use between the two calls.
 */

#include <stdbool.h>
#include <stdint.h>

#include "clock_hwio.h"
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/qti/clock/clock_driver.h>
#include "hoya_clock.h"
#include <lib/mmio.h>
#include <lib/utils_def.h>

#define LPASS_ALT_RESET_DELAY_US	50U
#define LPASS_RESTART_DELAY_US		500U

static struct clock_desc lpass_imem_clks[] = {
	{ .cbcr_addr = GCC_CFG_NOC_LPASS_CBCR },
	{ .cbcr_addr = LPASS_LPASS_TOP_CC_LPI_Q6_AXIM_HS_CBCR },
};

static struct clock_desc lpass_axis2_clk = {
	.cbcr_addr = GCC_LPASS_AXIS2_CBCR,
};

/* Clock the path to the LPASS Q6 memory still in use as boot IMEM. */
void hoya_clock_lpass_imem_on(void)
{
	unsigned int i;

	for (i = 0U; i < ARRAY_SIZE(lpass_imem_clks); i++) {
		clock_hal_set_clock(&lpass_imem_clks[i], true);
		if (clock_hal_wait_for_clock_on(&lpass_imem_clks[i]) != 0) {
			WARN("Clock: LPASS IMEM branch 0x%lx stays off\n",
			     lpass_imem_clks[i].cbcr_addr);
		}
	}
}

/* Return the LPASS Q6 to reset once the boot IMEM use has ended. */
void hoya_clock_lpass_q6_reset(void)
{
	/*
	 * Restore the Q6 RCG XO input to XO and disable the AXIS2 clock, which
	 * is no longer required as IMEM.
	 */
	mmio_clrbits_32(GCC_LPASS_Q6SS_BOOT_GPLL0_MUXR,
			GCC_LPASS_Q6SS_BOOT_GPLL0_MUXR_MUX_SEL_BMSK);
	clock_hal_set_clock(&lpass_axis2_clk, false);

	mmio_write_32(LPASS_LPASS_ALT_RESET_Q6SS, 1U);
	udelay(LPASS_ALT_RESET_DELAY_US);

	/*
	 * Trigger the SS func_ares signal. Skipping this can result in rogue
	 * transactions from LPASS under certain conditions (e.g. when its CX
	 * collapses before its PIL has executed).
	 */
	mmio_setbits_32(AOSS_CC_LPASS_RESTART,
			AOSS_CC_LPASS_RESTART_SS_RESTART_BMSK);
	udelay(LPASS_RESTART_DELAY_US);
	mmio_clrbits_32(AOSS_CC_LPASS_RESTART,
			AOSS_CC_LPASS_RESTART_SS_RESTART_BMSK);
	udelay(LPASS_RESTART_DELAY_US);

	mmio_write_32(LPASS_LPASS_ALT_RESET_Q6SS, 0U);
}
