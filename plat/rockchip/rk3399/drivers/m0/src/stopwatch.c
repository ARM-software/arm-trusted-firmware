/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <m0_param.h>
#include "rk3399_mcu.h"

/* use 24MHz SysTick */
#define US_TO_CYCLE(US)	(US * 24)

#define SYST_CST	0xe000e010
/* enable counter */
#define ENABLE		(1 << 0)
/* count down to 0 does not cause SysTick exception to pend */
#define TICKINT		(1 << 1)
/* core clock used for SysTick */
#define CLKSOURCE	(1 << 2)

#define COUNTFLAG	(1 << 16)
#define SYST_RVR	0xe000e014
#define MAX_VALUE	0xffffff
#define MAX_USECS	(MAX_VALUE / US_TO_CYCLE(1))
#define SYST_CVR	0xe000e018
#define SYST_CALIB	0xe000e01c

unsigned int remaining_usecs;

static inline void stopwatch_set_usecs(void)
{
	unsigned int cycle;
	unsigned int usecs = MIN(MAX_USECS, remaining_usecs);

	remaining_usecs -= usecs;
	cycle = US_TO_CYCLE(usecs);
	mmio_write_32(SYST_RVR, cycle);
	mmio_write_32(SYST_CVR, 0);

	mmio_write_32(SYST_CST, ENABLE | TICKINT | CLKSOURCE);
}

void stopwatch_init_usecs_expire(unsigned int usecs)
{
	/*
	 * Enter an inifite loop if the stopwatch is in use. This will allow the
	 * state to be analyzed with a debugger.
	 */
	if (mmio_read_32(SYST_CST) & ENABLE)
		while (1)
			;

	remaining_usecs = usecs;
	stopwatch_set_usecs();
}

int stopwatch_expired(void)
{
	int val = mmio_read_32(SYST_CST);
	if ((val & COUNTFLAG) || !(val & ENABLE)) {
		if (!remaining_usecs)
			return 1;

		stopwatch_set_usecs();
	}

	return 0;
}

void stopwatch_reset(void)
{
	mmio_clrbits_32(SYST_CST, ENABLE);
	remaining_usecs = 0;
}
