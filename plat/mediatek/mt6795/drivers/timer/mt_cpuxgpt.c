/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include <mt_cpuxgpt.h>

#define CPUXGPT_BASE	0x10200000
#define INDEX_BASE		(CPUXGPT_BASE+0x0674)
#define CTL_BASE		(CPUXGPT_BASE+0x0670)

uint64_t normal_time_base;
uint64_t atf_time_base;

void sched_clock_init(uint64_t normal_base, uint64_t atf_base)
{
	normal_time_base = normal_base;
	atf_time_base = atf_base;
}

uint64_t sched_clock(void)
{
	uint64_t cval;

	cval = (((read_cntpct_el0() - atf_time_base)*1000)/
		SYS_COUNTER_FREQ_IN_MHZ) + normal_time_base;
	return cval;
}

/*
  * Return: 0 - Trying to disable the CPUXGPT control bit,
  * and not allowed to disable it.
  * Return: 1 - reg_addr is not realted to disable the control bit.
  */
unsigned char check_cpuxgpt_write_permission(unsigned int reg_addr,
	unsigned int reg_value)
{
	unsigned int idx;
	unsigned int ctl_val;

	if (reg_addr == CTL_BASE) {
		idx = mmio_read_32(INDEX_BASE);

		/* idx 0: CPUXGPT system control */
		if (idx == 0) {
			ctl_val = mmio_read_32(CTL_BASE);
			if (ctl_val & 1) {
				/*
				 * if enable bit already set,
				 * then bit 0 is not allow to set as 0
				 */
				if (!(reg_value & 1))
					return 0;
			}
		}
	}
	return 1;
}

