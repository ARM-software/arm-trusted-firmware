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
#ifndef __PLAT_ARM_H__
#define __PLAT_ARM_H__

#include <bakery_lock.h>
#include <bl_common.h>
#include <cassert.h>
#include <cpu_data.h>
#include <stdint.h>
#include <xlat_tables.h>

/*
 * Extern declarations common to ARM standard platforms
 */
extern const mmap_region_t plat_arm_mmap[];

#define ARM_CASSERT_MMAP						\
	CASSERT((ARRAY_SIZE(plat_arm_mmap) + ARM_BL_REGIONS)		\
		<= MAX_MMAP_REGIONS,					\
		assert_max_mmap_regions);

/*
 * Utility functions common to ARM standard platforms
 */

void arm_configure_mmu_el1(unsigned long total_base,
			unsigned long total_size,
			unsigned long ro_start,
			unsigned long ro_limit
#if USE_COHERENT_MEM
			, unsigned long coh_start,
			unsigned long coh_limit
#endif
);
void arm_configure_mmu_el3(unsigned long total_base,
			unsigned long total_size,
			unsigned long ro_start,
			unsigned long ro_limit
#if USE_COHERENT_MEM
			, unsigned long coh_start,
			unsigned long coh_limit
#endif
);

#if IMAGE_BL31
/*
 * Use this macro to instantiate lock before it is used in below
 * arm_lock_xxx() macros
 */
#define ARM_INSTANTIATE_LOCK	DEFINE_BAKERY_LOCK(arm_lock);

/*
 * These are wrapper macros to the Coherent Memory Bakery Lock API.
 */
#define arm_lock_init()		bakery_lock_init(&arm_lock)
#define arm_lock_get()		bakery_lock_get(&arm_lock)
#define arm_lock_release()	bakery_lock_release(&arm_lock)

#else

/*
 * Empty macros for all other BL stages other than BL31
 */
#define ARM_INSTANTIATE_LOCK
#define arm_lock_init()
#define arm_lock_get()
#define arm_lock_release()

#endif /* IMAGE_BL31 */

#if ARM_RECOM_STATE_ID_ENC
/*
 * Macros used to parse state information from State-ID if it is using the
 * recommended encoding for State-ID.
 */
#define ARM_LOCAL_PSTATE_WIDTH		4
#define ARM_LOCAL_PSTATE_MASK		((1 << ARM_LOCAL_PSTATE_WIDTH) - 1)

/* Macros to construct the composite power state */

/* Make composite power state parameter till power level 0 */
#if PSCI_EXTENDED_STATE_ID

#define arm_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type) \
		(((lvl0_state) << PSTATE_ID_SHIFT) | ((type) << PSTATE_TYPE_SHIFT))
#else
#define arm_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type) \
		(((lvl0_state) << PSTATE_ID_SHIFT) | \
		((pwr_lvl) << PSTATE_PWR_LVL_SHIFT) | \
		((type) << PSTATE_TYPE_SHIFT))
#endif /* __PSCI_EXTENDED_STATE_ID__ */

/* Make composite power state parameter till power level 1 */
#define arm_make_pwrstate_lvl1(lvl1_state, lvl0_state, pwr_lvl, type) \
		(((lvl1_state) << ARM_LOCAL_PSTATE_WIDTH) | \
		arm_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type))

/* Make composite power state parameter till power level 2 */
#define arm_make_pwrstate_lvl2(lvl2_state, lvl1_state, lvl0_state, pwr_lvl, type) \
		(((lvl2_state) << (ARM_LOCAL_PSTATE_WIDTH * 2)) | \
		arm_make_pwrstate_lvl1(lvl1_state, lvl0_state, pwr_lvl, type))

#endif /* __ARM_RECOM_STATE_ID_ENC__ */


/* CCI utility functions */
void arm_cci_init(void);

/* IO storage utility functions */
void arm_io_setup(void);

/* Security utility functions */
void arm_tzc_setup(void);

/* Systimer utility function */
void arm_configure_sys_timer(void);

/* PM utility functions */
int arm_validate_power_state(unsigned int power_state,
			    psci_power_state_t *req_state);
int arm_validate_ns_entrypoint(uintptr_t entrypoint);
void arm_system_pwr_domain_resume(void);
void arm_program_trusted_mailbox(uintptr_t address);

/* Topology utility function */
int arm_check_mpidr(u_register_t mpidr);

/* BL1 utility functions */
void arm_bl1_early_platform_setup(void);
void arm_bl1_platform_setup(void);
void arm_bl1_plat_arch_setup(void);

/* BL2 utility functions */
void arm_bl2_early_platform_setup(meminfo_t *mem_layout);
void arm_bl2_platform_setup(void);
void arm_bl2_plat_arch_setup(void);
uint32_t arm_get_spsr_for_bl32_entry(void);
uint32_t arm_get_spsr_for_bl33_entry(void);

/* BL2U utility functions */
void arm_bl2u_early_platform_setup(struct meminfo *mem_layout,
				void *plat_info);
void arm_bl2u_platform_setup(void);
void arm_bl2u_plat_arch_setup(void);

/* BL31 utility functions */
void arm_bl31_early_platform_setup(bl31_params_t *from_bl2,
				void *plat_params_from_bl2);
void arm_bl31_platform_setup(void);
void arm_bl31_plat_runtime_setup(void);
void arm_bl31_plat_arch_setup(void);

/* TSP utility functions */
void arm_tsp_early_platform_setup(void);

/* FIP TOC validity check */
int arm_io_is_toc_valid(void);

/*
 * Mandatory functions required in ARM standard platforms
 */
void plat_arm_gic_driver_init(void);
void plat_arm_gic_init(void);
void plat_arm_gic_cpuif_enable(void);
void plat_arm_gic_cpuif_disable(void);
void plat_arm_gic_pcpu_init(void);
void plat_arm_security_setup(void);
void plat_arm_pwrc_setup(void);

/*
 * Optional functions required in ARM standard platforms
 */
void plat_arm_io_setup(void);
int plat_arm_get_alt_image_source(
	unsigned int image_id,
	uintptr_t *dev_handle,
	uintptr_t *image_spec);
unsigned int plat_arm_calc_core_pos(u_register_t mpidr);


#endif /* __PLAT_ARM_H__ */
