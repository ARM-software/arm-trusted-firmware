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

#include <debug.h>
#include <mmio.h>
#include <pmc.h>
#include <tegra_def.h>

#define SB_CSR				0x0
#define  SB_CSR_NS_RST_VEC_WR_DIS	(1 << 1)

/* CPU reset vector */
#define SB_AA64_RESET_LOW		0x30	/* width = 31:0 */
#define SB_AA64_RESET_HI		0x34	/* width = 11:0 */

extern void tegra_secure_entrypoint(void);

/*******************************************************************************
 * Setup secondary CPU vectors
 ******************************************************************************/
void plat_secondary_setup(void)
{
	uint32_t val;
	uint64_t reset_addr = (uint64_t)tegra_secure_entrypoint;

	INFO("Setting up secondary CPU boot\n");

	/* setup secondary CPU vector */
	mmio_write_32(TEGRA_SB_BASE + SB_AA64_RESET_LOW,
			(reset_addr & 0xFFFFFFFF) | 1);
	val = reset_addr >> 32;
	mmio_write_32(TEGRA_SB_BASE + SB_AA64_RESET_HI, val & 0x7FF);

	/* configure PMC */
	tegra_pmc_cpu_setup(reset_addr);
	tegra_pmc_lock_cpu_vectors();
}
