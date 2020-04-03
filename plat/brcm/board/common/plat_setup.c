/*
 * Copyright (c) 2015 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/common/platform.h>

#include <platform_def.h>

/*
 * This function returns the fixed clock frequency at which private
 * timers run. This value will be programmed into CNTFRQ_EL0.
 */
unsigned int plat_get_syscnt_freq2(void)
{
	return SYSCNT_FREQ;
}

static const char * const plat_prefix_str[] = {
	"E: ", "N: ", "W: ", "I: ", "V: "
};

const char *plat_log_get_prefix(unsigned int log_level)
{
	return plat_prefix_str[log_level - 1U];
}
