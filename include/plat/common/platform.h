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

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <stdint.h>


/*******************************************************************************
 * Forward declarations
 ******************************************************************************/
struct plat_pm_ops;
struct meminfo;
struct image_info;
struct entry_point_info;

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
void bl1_plat_arch_setup(void);
void bl2_plat_arch_setup(void);
void bl31_plat_arch_setup(void);
int platform_setup_pm(const struct plat_pm_ops **);
unsigned int platform_get_core_pos(unsigned long mpidr);
void enable_mmu_el1(void);
void enable_mmu_el3(void);
void configure_mmu_el1(unsigned long total_base,
		       unsigned long total_size,
		       unsigned long,
		       unsigned long,
		       unsigned long,
		       unsigned long);
void configure_mmu_el3(unsigned long total_base,
		       unsigned long total_size,
		       unsigned long,
		       unsigned long,
		       unsigned long,
		       unsigned long);
void plat_report_exception(unsigned long);
unsigned long plat_get_ns_image_entrypoint(void);
unsigned long platform_get_stack(unsigned long mpidr);
uint64_t plat_get_syscnt_freq(void);
uint32_t ic_get_pending_interrupt_id(void);
uint32_t ic_get_pending_interrupt_type(void);
uint32_t ic_acknowledge_interrupt(void);
uint32_t ic_get_interrupt_type(uint32_t id);
void ic_end_of_interrupt(uint32_t id);
uint32_t plat_interrupt_type_to_line(uint32_t type,
				     uint32_t security_state);

int plat_get_max_afflvl(void);
unsigned int plat_get_aff_count(unsigned int, unsigned long);
unsigned int plat_get_aff_state(unsigned int, unsigned long);
int plat_get_image_source(const char *image_name,
			uintptr_t *dev_handle,
			uintptr_t *image_spec);

/*
 * Before calling this function BL2 is loaded in memory and its entrypoint
 * is set by load_image. This is a placeholder for the platform to change
 * the entrypoint of BL2 and set SPSR and security state.
 * On FVP we are only setting the security state, entrypoint
 */
void bl1_plat_set_bl2_ep_info(struct image_info *image,
			      struct entry_point_info *ep);

/*
 * Before calling this function BL31 is loaded in memory and its entrypoint
 * is set by load_image. This is a placeholder for the platform to change
 * the entrypoint of BL31 and set SPSR and security state.
 * On FVP we are only setting the security state, entrypoint
 */
void bl2_plat_set_bl31_ep_info(struct image_info *image,
			       struct entry_point_info *ep);

/*
 * Before calling this function BL32 is loaded in memory and its entrypoint
 * is set by load_image. This is a placeholder for the platform to change
 * the entrypoint of BL32 and set SPSR and security state.
 * On FVP we are only setting the security state, entrypoint
 */
void bl2_plat_set_bl32_ep_info(struct image_info *image,
			       struct entry_point_info *ep);

/*
 * Before calling this function BL33 is loaded in memory and its entrypoint
 * is set by load_image. This is a placeholder for the platform to change
 * the entrypoint of BL33 and set SPSR and security state.
 * On FVP we are only setting the security state, entrypoint
 */
void bl2_plat_set_bl33_ep_info(struct image_info *image,
			       struct entry_point_info *ep);

/* Gets the memory layout for BL32 */
void bl2_plat_get_bl32_meminfo(struct meminfo *mem_info);

/* Gets the memory layout for BL33 */
void bl2_plat_get_bl33_meminfo(struct meminfo *mem_info);


#endif /* __PLATFORM_H__ */
