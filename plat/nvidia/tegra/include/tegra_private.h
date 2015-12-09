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

#ifndef __TEGRA_PRIVATE_H__
#define __TEGRA_PRIVATE_H__

#include <arch.h>
#include <platform_def.h>
#include <psci.h>
#include <xlat_tables.h>

/*******************************************************************************
 * Tegra DRAM memory base address
 ******************************************************************************/
#define TEGRA_DRAM_BASE		0x80000000
#define TEGRA_DRAM_END		0x27FFFFFFF

typedef struct plat_params_from_bl2 {
	uint64_t tzdram_size;
} plat_params_from_bl2_t;

/* Declarations for plat_psci_handlers.c */
int32_t tegra_soc_validate_power_state(unsigned int power_state,
		psci_power_state_t *req_state);

/* Declarations for plat_setup.c */
const mmap_region_t *plat_get_mmio_map(void);
uint64_t plat_get_syscnt_freq(void);

/* Declarations for plat_secondary.c */
void plat_secondary_setup(void);
int plat_lock_cpu_vectors(void);

/* Declarations for tegra_gic.c */
void tegra_gic_setup(void);
void tegra_gic_cpuif_deactivate(void);

/* Declarations for tegra_security.c */
void tegra_security_setup(void);
void tegra_security_setup_videomem(uintptr_t base, uint64_t size);

/* Declarations for tegra_pm.c */
void tegra_pm_system_suspend_entry(void);
void tegra_pm_system_suspend_exit(void);
int tegra_system_suspended(void);

/* Declarations for tegraXXX_pm.c */
int tegra_prepare_cpu_suspend(unsigned int id, unsigned int afflvl);
int tegra_prepare_cpu_on_finish(unsigned long mpidr);

/* Declarations for tegra_bl31_setup.c */
plat_params_from_bl2_t *bl31_get_plat_params(void);
int bl31_check_ns_address(uint64_t base, uint64_t size_in_bytes);

/* Declarations for tegra_delay_timer.c */
void tegra_delay_timer_init(void);

#endif /* __TEGRA_PRIVATE_H__ */
