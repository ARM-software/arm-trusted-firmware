/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
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

#ifndef __JUNO_PRIVATE_H__
#define __JUNO_PRIVATE_H__

#include <bakery_lock.h>
#include <bl_common.h>
#include <cpu_data.h>
#include <platform_def.h>
#include <stdint.h>

/*******************************************************************************
 * Forward declarations
 ******************************************************************************/
struct plat_pm_ops;
struct meminfo;
struct bl31_params;
struct image_info;
struct entry_point_info;

/*******************************************************************************
 * This structure represents the superset of information that is passed to
 * BL3-1 e.g. while passing control to it from BL2 which is bl31_params
 * and other platform specific params
 ******************************************************************************/
typedef struct bl2_to_bl31_params_mem {
	struct bl31_params bl31_params;
	struct image_info bl31_image_info;
	struct image_info bl32_image_info;
	struct image_info bl33_image_info;
	struct entry_point_info bl33_ep_info;
	struct entry_point_info bl32_ep_info;
	struct entry_point_info bl31_ep_info;
} bl2_to_bl31_params_mem_t;

#if IMAGE_BL31
#if USE_COHERENT_MEM
/*
 * These are wrapper macros to the Coherent Memory Bakery Lock API.
 */
#define juno_lock_init(_lock_arg)		bakery_lock_init(_lock_arg)
#define juno_lock_get(_lock_arg)		bakery_lock_get(_lock_arg)
#define juno_lock_release(_lock_arg)		bakery_lock_release(_lock_arg)

#else

/*******************************************************************************
 * Constants that specify how many bakeries this platform implements and bakery
 * ids.
 ******************************************************************************/
#define JUNO_MAX_BAKERIES	1
#define JUNO_MHU_BAKERY_ID	0

/*******************************************************************************
 * Definition of structure which holds platform specific per-cpu data. Currently
 * it holds only the bakery lock information for each cpu. Constants to specify
 * how many bakeries this platform implements and bakery ids are specified in
 * juno_def.h
 ******************************************************************************/
typedef struct juno_cpu_data {
	bakery_info_t pcpu_bakery_info[JUNO_MAX_BAKERIES];
} juno_cpu_data_t;

/* Macro to define the offset of bakery_info_t in juno_cpu_data_t */
#define JUNO_CPU_DATA_LOCK_OFFSET	__builtin_offsetof\
					    (juno_cpu_data_t, pcpu_bakery_info)

/*******************************************************************************
 * Helper macros for bakery lock api when using the above juno_cpu_data_t for
 * bakery lock data structures. It assumes that the bakery_info is at the
 * beginning of the platform specific per-cpu data.
 ******************************************************************************/
#define juno_lock_init(_lock_arg)		/* No init required */
#define juno_lock_get(_lock_arg)		bakery_lock_get(_lock_arg,	\
						    CPU_DATA_PLAT_PCPU_OFFSET + \
						    JUNO_CPU_DATA_LOCK_OFFSET)
#define juno_lock_release(_lock_arg)		bakery_lock_release(_lock_arg,	\
						    CPU_DATA_PLAT_PCPU_OFFSET + \
						    JUNO_CPU_DATA_LOCK_OFFSET)

/*
 * Ensure that the size of the Juno specific per-cpu data structure and the size
 * of the memory allocated in generic per-cpu data for the platform are the same.
 */
CASSERT(PLAT_PCPU_DATA_SIZE == sizeof(juno_cpu_data_t),	\
	juno_pcpu_data_size_mismatch);
#endif /* __USE_COHERENT_MEM__ */
#else
/*
 * Dummy wrapper macros for all other BL stages other than BL3-1
 */
#define juno_lock_init(_lock_arg)
#define juno_lock_get(_lock_arg)
#define juno_lock_release(_lock_arg)

#endif /* __IMAGE_BL31__ */

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
void bl1_plat_arch_setup(void);
void bl2_plat_arch_setup(void);
void bl31_plat_arch_setup(void);
int platform_setup_pm(const struct plat_pm_ops **plat_ops);
unsigned int platform_get_core_pos(unsigned long mpidr);
void configure_mmu_el1(unsigned long total_base,
		       unsigned long total_size,
		       unsigned long ro_start,
		       unsigned long ro_limit
#if USE_COHERENT_MEM
		       , unsigned long coh_start,
		       unsigned long coh_limit
#endif
		       );
void configure_mmu_el3(unsigned long total_base,
		       unsigned long total_size,
		       unsigned long ro_start,
		       unsigned long ro_limit
#if USE_COHERENT_MEM
		       , unsigned long coh_start,
		       unsigned long coh_limit
#endif
		       );
void plat_report_exception(unsigned long type);
unsigned long plat_get_ns_image_entrypoint(void);
unsigned long platform_get_stack(unsigned long mpidr);
uint64_t plat_get_syscnt_freq(void);
void plat_gic_init(void);

/* Declarations for plat_topology.c */
int plat_setup_topology(void);
int plat_get_max_afflvl(void);
unsigned int plat_get_aff_count(unsigned int aff_lvl, unsigned long mpidr);
unsigned int plat_get_aff_state(unsigned int aff_lvl, unsigned long mpidr);

/* Declarations for plat_io_storage.c */
void io_setup(void);
int plat_get_image_source(const char *image_name,
			  uintptr_t *dev_handle,
			  uintptr_t *image_spec);

/* Declarations for security.c */
void plat_security_setup(void);

/*
 * Before calling this function BL2 is loaded in memory and its entrypoint
 * is set by load_image. This is a placeholder for the platform to change
 * the entrypoint of BL2 and set SPSR and security state.
 * On Juno we are only setting the security state, entrypoint
 */
void bl1_plat_set_bl2_ep_info(struct image_info *image,
			      struct entry_point_info *ep);

/*
 * Before calling this function BL3-1 is loaded in memory and its entrypoint
 * is set by load_image. This is a placeholder for the platform to change
 * the entrypoint of BL3-1 and set SPSR and security state.
 * On Juno we are only setting the security state, entrypoint
 */
void bl2_plat_set_bl31_ep_info(struct image_info *image,
			       struct entry_point_info *ep);

/*
 * Before calling this function BL3-2 is loaded in memory and its entrypoint
 * is set by load_image. This is a placeholder for the platform to change
 * the entrypoint of BL3-2 and set SPSR and security state.
 * On Juno we are only setting the security state, entrypoint
 */
void bl2_plat_set_bl32_ep_info(struct image_info *image,
			       struct entry_point_info *ep);

/*
 * Before calling this function BL3-3 is loaded in memory and its entrypoint
 * is set by load_image. This is a placeholder for the platform to change
 * the entrypoint of BL3-3 and set SPSR and security state.
 * On Juno we are only setting the security state, entrypoint
 */
void bl2_plat_set_bl33_ep_info(struct image_info *image,
			       struct entry_point_info *ep);

/* Gets the memory layout for BL3-2 */
void bl2_plat_get_bl32_meminfo(struct meminfo *mem_info);

/* Gets the memory layout for BL3-3 */
void bl2_plat_get_bl33_meminfo(struct meminfo *mem_info);

#endif /* __JUNO_PRIVATE_H__ */
