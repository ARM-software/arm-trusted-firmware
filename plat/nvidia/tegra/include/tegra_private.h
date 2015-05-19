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

#include <platform_def.h>

/*******************************************************************************
 * Forward declarations
 ******************************************************************************/
struct meminfo;

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
int tegra_config_setup(void);

/* Declarations for plat_setup.c */
void plat_tegra_configure_mmu_el3(unsigned long total_base,
				  unsigned long total_size,
				  unsigned long ro_start,
				  unsigned long ro_size,
				  unsigned long coh_start,
				  unsigned long coh_size);
int plat_lock_cpu_vectors(void);

/* Declarations for plat_secondary.c */
void plat_secondary_setup(void);

/* Declarations for tegra_gic.c */
void gic_setup(void);

/* Declarations for tegra_security.c */
void tegra_security_setup(void);
void tegra_security_setup_videomem(uintptr_t base, uint64_t size);

/* Declarations for tegra_memory.c */
void tegra_memory_setup(void);

/* Declarations for tegra_pm.c */
void tegra_pm_system_suspend_entry(void);
void tegra_pm_system_suspend_exit(void);
int tegra_system_suspended(void);

/* Declarations for tegraXXX_pm.c */
int tegra_prepare_cpu_suspend(unsigned int id, unsigned int afflvl);
int tegra_prepare_cpu_on_finish(unsigned long mpidr);

#endif /* __TEGRA_PRIVATE_H__ */
