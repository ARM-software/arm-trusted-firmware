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

#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <denver.h>
#include <mmio.h>
#include <platform.h>
#include <psci.h>
#include <pmc.h>
#include <tegra_def.h>

#define SB_CSR				0x0
#define  SB_CSR_NS_RST_VEC_WR_DIS	(1 << 1)

/* AARCH64 CPU reset vector */
#define SB_AA64_RESET_LOW		0x30	/* width = 31:0 */
#define SB_AA64_RESET_HI		0x34	/* width = 11:0 */

/* AARCH32 CPU reset vector */
#define EVP_CPU_RESET_VECTOR		0x100

extern void tegra_secure_entrypoint(void);

/*
 * For T132, CPUs reset to AARCH32, so the reset vector is first
 * armv8_trampoline which does a warm reset to AARCH64 and starts
 * execution at the address in SB_AA64_RESET_LOW/SB_AA64_RESET_HI.
 */
uint32_t __aligned(8) armv8_trampoline[] = {
	0xE3A00003,		// mov	r0, #3
	0xEE0C0F50,		// mcr	p15, 0, r0, c12, c0, 2
	0xEAFFFFFE,		// b	.
};

/*******************************************************************************
 * Setup secondary CPU vectors
 ******************************************************************************/
void plat_secondary_setup(void)
{
	uint32_t val;
	uint64_t reset_addr = (uint64_t)tegra_secure_entrypoint;

	/*
	 * For T132, CPUs reset to AARCH32, so the reset vector is first
	 * armv8_trampoline, which does a warm reset to AARCH64 and starts
	 * execution at the address in SCRATCH34/SCRATCH35.
	 */
	INFO("Setting up T132 CPU boot\n");

	/* initial AARCH32 reset address */
	tegra_pmc_write_32(PMC_SECURE_SCRATCH22, (unsigned long)&armv8_trampoline);

	/* set AARCH32 exception vector (read to flush) */
	mmio_write_32(TEGRA_EVP_BASE + EVP_CPU_RESET_VECTOR,
		(unsigned long)&armv8_trampoline);
	val = mmio_read_32(TEGRA_EVP_BASE + EVP_CPU_RESET_VECTOR);

	/* setup secondary CPU vector */
	mmio_write_32(TEGRA_SB_BASE + SB_AA64_RESET_LOW,
			(reset_addr & 0xFFFFFFFF) | 1);
	val = reset_addr >> 32;
	mmio_write_32(TEGRA_SB_BASE + SB_AA64_RESET_HI, val & 0x7FF);

	/* configure PMC */
	tegra_pmc_cpu_setup(reset_addr);
	tegra_pmc_lock_cpu_vectors();
}
