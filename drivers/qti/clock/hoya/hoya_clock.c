/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Secure GDSC and retention controls shared by the hoya SoCs. The caller
 * passes the register: a chipset calls them only for the blocks it has.
 */

#include <stddef.h>
#include <stdint.h>

#include "hoya_clock.h"
#include <lib/mmio.h>
#include <lib/utils_def.h>

/* Field layout of every GCC_APCS_TZ_*_SGDSCR. */
#define TZ_SGDSCR_SW_OVERRIDE		BIT_32(0)
#define TZ_SGDSCR_RETAIN_FF_ENABLE	BIT_32(1)

/* Field layout of the secure control of a GDSC outside GCC. */
#define GDSC_CTL_SW_OVERRIDE		BIT_32(2)
#define GDSC_CTL_RETAIN_FF_ENABLE	BIT_32(11)

#define RETENTION_VOTE			BIT_32(0)
#define RETENTION_HW_CONTROL_OVERRIDE	BIT_32(1)

/*
 * GCC secure GDSC control of a subsystem with an SMMU: SW override on and
 * GDSC retention on, so that the SMMU keeps its state across power collapse.
 */
void hoya_clock_tz_gdsc_init(uintptr_t sgdscr)
{
	mmio_setbits_32(sgdscr, TZ_SGDSCR_SW_OVERRIDE);
	mmio_setbits_32(sgdscr, TZ_SGDSCR_RETAIN_FF_ENABLE);
}

/* No SW override, so that TF-A's vote works; retention on. */
void hoya_clock_gdsc_retain(uintptr_t gdsc_ctl)
{
	mmio_clrbits_32(gdsc_ctl, GDSC_CTL_SW_OVERRIDE);
	mmio_setbits_32(gdsc_ctl, GDSC_CTL_RETAIN_FF_ENABLE);
}

/*
 * Secure retention votes of the domains of a subsystem, which otherwise lose
 * their state at the subsystem's first power collapse.
 */
void hoya_clock_retention_votes(const uintptr_t *votes, size_t num)
{
	size_t i;

	for (i = 0U; i < num; i++) {
		mmio_clrbits_32(votes[i], RETENTION_HW_CONTROL_OVERRIDE);
	}
	for (i = 0U; i < num; i++) {
		mmio_setbits_32(votes[i], RETENTION_VOTE);
	}
}
