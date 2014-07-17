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

#include <bl_common.h>
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
		       unsigned long ro_limit,
		       unsigned long coh_start,
		       unsigned long coh_limit);
void configure_mmu_el3(unsigned long total_base,
		       unsigned long total_size,
		       unsigned long ro_start,
		       unsigned long ro_limit,
		       unsigned long coh_start,
		       unsigned long coh_limit);
void plat_report_exception(unsigned long type);
unsigned long plat_get_ns_image_entrypoint(void);
unsigned long platform_get_stack(unsigned long mpidr);
uint64_t plat_get_syscnt_freq(void);

/* Declarations for plat_gic.c */
uint32_t ic_get_pending_interrupt_id(void);
uint32_t ic_get_pending_interrupt_type(void);
uint32_t ic_acknowledge_interrupt(void);
uint32_t ic_get_interrupt_type(uint32_t id);
void ic_end_of_interrupt(uint32_t id);
void gic_cpuif_deactivate(unsigned int gicc_base);
void gic_cpuif_setup(unsigned int gicc_base);
void gic_pcpu_distif_setup(unsigned int gicd_base);
void gic_setup(void);
uint32_t plat_interrupt_type_to_line(uint32_t type, uint32_t security_state);

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
