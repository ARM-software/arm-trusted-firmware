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
#if USE_COHERENT_MEM

/*
 * Use this macro to instantiate lock before it is used in below
 * arm_lock_xxx() macros
 */
#define ARM_INSTANTIATE_LOCK	bakery_lock_t arm_lock	\
	__attribute__ ((section("tzfw_coherent_mem")));

/*
 * These are wrapper macros to the Coherent Memory Bakery Lock API.
 */
#define arm_lock_init()		bakery_lock_init(&arm_lock)
#define arm_lock_get()		bakery_lock_get(&arm_lock)
#define arm_lock_release()	bakery_lock_release(&arm_lock)

#else

/*******************************************************************************
 * Constants to specify how many bakery locks this platform implements. These
 * are used if the platform chooses not to use coherent memory for bakery lock
 * data structures.
 ******************************************************************************/
#define ARM_MAX_BAKERIES	1
#define ARM_PWRC_BAKERY_ID	0

/* Empty definition */
#define ARM_INSTANTIATE_LOCK

/*******************************************************************************
 * Definition of structure which holds platform specific per-cpu data. Currently
 * it holds only the bakery lock information for each cpu.
 ******************************************************************************/
typedef struct arm_cpu_data {
	bakery_info_t pcpu_bakery_info[ARM_MAX_BAKERIES];
} arm_cpu_data_t;

/* Macro to define the offset of bakery_info_t in arm_cpu_data_t */
#define ARM_CPU_DATA_LOCK_OFFSET	__builtin_offsetof\
					    (arm_cpu_data_t, pcpu_bakery_info)


/*******************************************************************************
 * Helper macros for bakery lock api when using the above arm_cpu_data_t for
 * bakery lock data structures. It assumes that the bakery_info is at the
 * beginning of the platform specific per-cpu data.
 ******************************************************************************/
#define arm_lock_init()		/* No init required */
#define arm_lock_get()		bakery_lock_get(ARM_PWRC_BAKERY_ID,	\
					CPU_DATA_PLAT_PCPU_OFFSET +	\
					ARM_CPU_DATA_LOCK_OFFSET)
#define arm_lock_release()	bakery_lock_release(ARM_PWRC_BAKERY_ID,	\
					CPU_DATA_PLAT_PCPU_OFFSET +	\
					ARM_CPU_DATA_LOCK_OFFSET)

/*
 * Ensure that the size of the platform specific per-cpu data structure and
 * the size of the memory allocated in generic per-cpu data for the platform
 * are the same.
 */
CASSERT(PLAT_PCPU_DATA_SIZE == sizeof(arm_cpu_data_t),
	arm_pcpu_data_size_mismatch);

#endif /* USE_COHERENT_MEM */

#else

/*
* Dummy macros for all other BL stages other than BL3-1
*/
#define ARM_INSTANTIATE_LOCK
#define arm_lock_init()
#define arm_lock_get()
#define arm_lock_release()

#endif /* IMAGE_BL31 */


/* CCI utility functions */
void arm_cci_init(void);

/* IO storage utility functions */
void arm_io_setup(void);

/* Security utility functions */
void arm_tzc_setup(void);

/* PM utility functions */
int32_t arm_do_affinst_actions(unsigned int afflvl, unsigned int state);
int arm_validate_power_state(unsigned int power_state);

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

/* BL3-1 utility functions */
void arm_bl31_early_platform_setup(bl31_params_t *from_bl2,
				void *plat_params_from_bl2);
void arm_bl31_platform_setup(void);
void arm_bl31_plat_arch_setup(void);

/* TSP utility functions */
void arm_tsp_early_platform_setup(void);


/*
 * Mandatory functions required in ARM standard platforms
 */
void plat_arm_gic_init(void);
void plat_arm_security_setup(void);
void plat_arm_pwrc_setup(void);

/*
 * Optional functions required in ARM standard platforms
 */
void plat_arm_io_setup(void);
int plat_arm_get_alt_image_source(
	const uintptr_t image_spec,
	uintptr_t *dev_handle);
void plat_arm_topology_setup(void);


#endif /* __PLAT_ARM_H__ */
