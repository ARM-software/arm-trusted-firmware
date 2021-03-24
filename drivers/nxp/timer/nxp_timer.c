/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <nxp_timer.h>
#include <plat/common/platform.h>

static uintptr_t g_nxp_timer_addr;
static timer_ops_t ops;

uint64_t get_timer_val(uint64_t start)
{
	uint64_t cntpct;

	isb();
	cntpct = read_cntpct_el0();
	return (cntpct * 1000ULL / read_cntfrq_el0() - start);
}

static uint32_t timer_get_value(void)
{
	uint64_t cntpct;

	isb();
	cntpct = read_cntpct_el0();
#ifdef ERRATA_SOC_A008585
	uint8_t	max_fetch_count = 10U;
	/* This erratum number needs to be confirmed to match ARM document */
	uint64_t temp;

	isb();
	temp = read_cntpct_el0();

	while (temp != cntpct && max_fetch_count) {
		isb();
		cntpct = read_cntpct_el0();
		isb();
		temp = read_cntpct_el0();
		max_fetch_count--;
	}
#endif

	/*
	 * Generic delay timer implementation expects the timer to be a down
	 * counter. We apply bitwise NOT operator to the tick values returned
	 * by read_cntpct_el0() to simulate the down counter. The value is
	 * clipped from 64 to 32 bits.
	 */
	return (uint32_t)(~cntpct);
}

static void delay_timer_init_args(uint32_t mult, uint32_t div)
{
	ops.get_timer_value	= timer_get_value,
	ops.clk_mult		= mult;
	ops.clk_div		= div;

	timer_init(&ops);

	VERBOSE("Generic delay timer configured with mult=%u and div=%u\n",
		mult, div);
}

/*
 * Initialise the nxp on-chip free rolling usec counter as the delay
 * timer.
 */
void delay_timer_init(uintptr_t nxp_timer_addr)
{
	/* Value in ticks */
	unsigned int mult = MHZ_TICKS_PER_SEC;

	unsigned int div;

	unsigned int counter_base_frequency = plat_get_syscnt_freq2();

	g_nxp_timer_addr = nxp_timer_addr;
	/* Rounding off the Counter Frequency to MHZ_TICKS_PER_SEC */
	if (counter_base_frequency > MHZ_TICKS_PER_SEC) {
		counter_base_frequency = (counter_base_frequency
					/ MHZ_TICKS_PER_SEC)
					* MHZ_TICKS_PER_SEC;
	} else {
		counter_base_frequency = (counter_base_frequency
					/ KHZ_TICKS_PER_SEC)
					* KHZ_TICKS_PER_SEC;
	}

	/* Value in ticks per second (Hz) */
	div = counter_base_frequency;

	/* Reduce multiplier and divider by dividing them repeatedly by 10 */
	while ((mult % 10U == 0U) && (div % 10U == 0U)) {
		mult /= 10U;
		div /= 10U;
	}

	/* Enable and initialize the System level generic timer */
	mmio_write_32(g_nxp_timer_addr + CNTCR_OFF,
			CNTCR_FCREQ(0) | CNTCR_EN);

	delay_timer_init_args(mult, div);
}


#ifdef IMAGE_BL31
/*******************************************************************************
 * TBD: Configures access to the system counter timer module.
 ******************************************************************************/
void ls_configure_sys_timer(uintptr_t ls_sys_timctl_base,
			    uint8_t ls_config_cntacr,
			    uint8_t plat_ls_ns_timer_frame_id)
{
	unsigned int reg_val;

	if (ls_config_cntacr == 1U) {
		reg_val = (1U << CNTACR_RPCT_SHIFT) | (1U << CNTACR_RVCT_SHIFT);
		reg_val |= (1U << CNTACR_RFRQ_SHIFT) | (1U << CNTACR_RVOFF_SHIFT);
		reg_val |= (1U << CNTACR_RWVT_SHIFT) | (1U << CNTACR_RWPT_SHIFT);
		mmio_write_32(ls_sys_timctl_base +
		      CNTACR_BASE(plat_ls_ns_timer_frame_id), reg_val);
		mmio_write_32(ls_sys_timctl_base, plat_get_syscnt_freq2());
	}

	reg_val = (1U << CNTNSAR_NS_SHIFT(plat_ls_ns_timer_frame_id));
	mmio_write_32(ls_sys_timctl_base + CNTNSAR, reg_val);
}

void enable_init_timer(void)
{
	/* Enable and initialize the System level generic timer */
	mmio_write_32(g_nxp_timer_addr + CNTCR_OFF,
			CNTCR_FCREQ(0) | CNTCR_EN);
}
#endif
