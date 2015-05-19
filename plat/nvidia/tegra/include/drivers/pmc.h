/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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

#ifndef __PMC_H__
#define __PMC_H__

#include <mmio.h>
#include <tegra_def.h>

#define PMC_CONFIG				0x0
#define PMC_PWRGATE_STATUS			0x38
#define PMC_PWRGATE_TOGGLE			0x30
#define  PMC_TOGGLE_START			0x100
#define PMC_SCRATCH39				0x138
#define PMC_SECURE_DISABLE2			0x2c4
#define  PMC_SECURE_DISABLE2_WRITE22_ON		(1 << 28)
#define PMC_SECURE_SCRATCH22			0x338
#define PMC_SECURE_DISABLE3			0x2d8
#define  PMC_SECURE_DISABLE3_WRITE34_ON		(1 << 20)
#define  PMC_SECURE_DISABLE3_WRITE35_ON		(1 << 22)
#define PMC_SECURE_SCRATCH34			0x368
#define PMC_SECURE_SCRATCH35			0x36c

static inline uint32_t tegra_pmc_read_32(uint32_t off)
{
	return mmio_read_32(TEGRA_PMC_BASE + off);
}

static inline void tegra_pmc_write_32(uint32_t off, uint32_t val)
{
	mmio_write_32(TEGRA_PMC_BASE + off, val);
}

void tegra_pmc_cpu_setup(uint64_t reset_addr);
void tegra_pmc_lock_cpu_vectors(void);
void tegra_pmc_cpu_on(int cpu);
__dead2 void tegra_pmc_system_reset(void);

#endif /* __PMC_H__ */
