/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

#include <bakery_lock.h>
#include <mmio.h>
#include "../../fvp_def.h"
#include "fvp_pwrc.h"

/*
 * TODO: Someday there will be a generic power controller api. At the moment
 * each platform has its own pwrc so just exporting functions is fine.
 */
static bakery_lock_t pwrc_lock __attribute__ ((section("tzfw_coherent_mem")));

unsigned int fvp_pwrc_get_cpu_wkr(unsigned long mpidr)
{
	unsigned int rc = 0;
	bakery_lock_get(mpidr, &pwrc_lock);
	mmio_write_32(PWRC_BASE + PSYSR_OFF, (unsigned int) mpidr);
	rc = PSYSR_WK(mmio_read_32(PWRC_BASE + PSYSR_OFF));
	bakery_lock_release(mpidr, &pwrc_lock);
	return rc;
}

unsigned int fvp_pwrc_read_psysr(unsigned long mpidr)
{
	unsigned int rc = 0;
	bakery_lock_get(mpidr, &pwrc_lock);
	mmio_write_32(PWRC_BASE + PSYSR_OFF, (unsigned int) mpidr);
	rc = mmio_read_32(PWRC_BASE + PSYSR_OFF);
	bakery_lock_release(mpidr, &pwrc_lock);
	return rc;
}

void fvp_pwrc_write_pponr(unsigned long mpidr)
{
	bakery_lock_get(mpidr, &pwrc_lock);
	mmio_write_32(PWRC_BASE + PPONR_OFF, (unsigned int) mpidr);
	bakery_lock_release(mpidr, &pwrc_lock);
}

void fvp_pwrc_write_ppoffr(unsigned long mpidr)
{
	bakery_lock_get(mpidr, &pwrc_lock);
	mmio_write_32(PWRC_BASE + PPOFFR_OFF, (unsigned int) mpidr);
	bakery_lock_release(mpidr, &pwrc_lock);
}

void fvp_pwrc_set_wen(unsigned long mpidr)
{
	bakery_lock_get(mpidr, &pwrc_lock);
	mmio_write_32(PWRC_BASE + PWKUPR_OFF,
		      (unsigned int) (PWKUPR_WEN | mpidr));
	bakery_lock_release(mpidr, &pwrc_lock);
}

void fvp_pwrc_clr_wen(unsigned long mpidr)
{
	bakery_lock_get(mpidr, &pwrc_lock);
	mmio_write_32(PWRC_BASE + PWKUPR_OFF,
		      (unsigned int) mpidr);
	bakery_lock_release(mpidr, &pwrc_lock);
}

void fvp_pwrc_write_pcoffr(unsigned long mpidr)
{
	bakery_lock_get(mpidr, &pwrc_lock);
	mmio_write_32(PWRC_BASE + PCOFFR_OFF, (unsigned int) mpidr);
	bakery_lock_release(mpidr, &pwrc_lock);
}

/* Nothing else to do here apart from initializing the lock */
int fvp_pwrc_setup(void)
{
	bakery_lock_init(&pwrc_lock);
	return 0;
}



