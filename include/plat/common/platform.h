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
struct el_change_info;
struct bl31_params;

/*******************************************************************************
 * Function declarations
 ******************************************************************************/
/*******************************************************************************
 * Mandatory common functions
 ******************************************************************************/
uint64_t plat_get_syscnt_freq(void);
int plat_get_image_source(const char *image_name,
			uintptr_t *dev_handle,
			uintptr_t *image_spec);
unsigned long plat_get_ns_image_entrypoint(void);

/*******************************************************************************
 * Mandatory interrupt management functions
 ******************************************************************************/
uint32_t ic_get_pending_interrupt_id(void);
uint32_t ic_get_pending_interrupt_type(void);
uint32_t ic_acknowledge_interrupt(void);
uint32_t ic_get_interrupt_type(uint32_t id);
void ic_end_of_interrupt(uint32_t id);
uint32_t plat_interrupt_type_to_line(uint32_t type,
				     uint32_t security_state);

/*******************************************************************************
 * Optional common functions (may be overridden)
 ******************************************************************************/
unsigned int platform_get_core_pos(unsigned long mpidr);
unsigned long platform_get_stack(unsigned long mpidr);
void plat_report_exception(unsigned long);

/*******************************************************************************
 * Mandatory BL1 functions
 ******************************************************************************/
void bl1_plat_arch_setup(void);
void bl1_platform_setup(void);
struct meminfo *bl1_plat_sec_mem_layout(void);
/*
 * Before calling this function BL2 is loaded in memory and its entrypoint
 * is set by load_image. This is a placeholder for the platform to change
 * the entrypoint of BL2 and set SPSR and security state.
 * On FVP we are only setting the security state, entrypoint
 */
void bl1_plat_bl2_loaded(struct image_info *image,
			 struct el_change_info *ep);

/*******************************************************************************
 * Optional BL1 functions (may be overridden)
 ******************************************************************************/
void init_bl2_mem_layout(struct meminfo *,
			struct meminfo *,
			unsigned int,
			unsigned long);

/*******************************************************************************
 * Mandatory BL2 functions
 ******************************************************************************/
void bl2_plat_arch_setup(void);
void bl2_platform_setup(void);
struct meminfo *bl2_plat_sec_mem_layout(void);
/*******************************************************************************
 * This function returns a pointer to the shared memory that the platform has
 * kept aside to pass trusted firmware related information that BL3-1
 * could need
 ******************************************************************************/
struct bl31_params *bl2_plat_get_bl31_params(void);
/*******************************************************************************
 * This function returns a pointer to the shared memory that the platform
 * has kept to point to entry point information of BL31 to BL2
 ******************************************************************************/
struct el_change_info *bl2_plat_get_bl31_ep(void);
/************************************************************************
 * This function flushes to main memory all the params that are
 * passed to BL3-1
 **************************************************************************/
void bl2_plat_flush_bl31_params(void);
/*
 * Before calling this function BL31 is loaded in memory and its entrypoint
 * is set by load_image. This is a placeholder for the platform to change
 * the entrypoint of BL31 and set SPSR and security state.
 * On FVP we are only setting the security state, entrypoint
 */
void bl2_plat_bl31_post_load_actions(struct image_info *image,
				     struct el_change_info *ep);
/*
 * Before calling this function BL32 is loaded in memory and its entrypoint
 * is set by load_image. This is a placeholder for the platform to change
 * the entrypoint of BL32 and set SPSR and security state.
 * On FVP we are only setting the security state, entrypoint
 */
void bl2_plat_bl32_post_load_actions(struct image_info *image,
				     struct el_change_info *ep);
/*
 * Before calling this function BL33 is loaded in memory and its entrypoint
 * is set by load_image. This is a placeholder for the platform to change
 * the entrypoint of BL33 and set SPSR and security state.
 * On FVP we are only setting the security state, entrypoint
 */
void bl2_plat_bl33_post_load_actions(struct image_info *image,
				     struct el_change_info *ep);
/* Gets the memory layout for BL32 */
void bl2_plat_get_bl32_meminfo(struct meminfo *mem_info);
/* Gets the memory layout for BL33 */
void bl2_plat_get_bl33_meminfo(struct meminfo *mem_info);

/*******************************************************************************
 * Optional BL2 functions (may be overridden)
 ******************************************************************************/

/*******************************************************************************
 * Mandatory BL3-1 functions
 ******************************************************************************/
void bl31_plat_arch_setup(void);
void bl31_platform_setup(void);
struct el_change_info *bl31_get_next_image_info(uint32_t type);

/*******************************************************************************
 * Mandatory PSCI functions (BL3-1)
 ******************************************************************************/
int platform_setup_pm(const struct plat_pm_ops **);
int plat_get_max_afflvl(void);
unsigned int plat_get_aff_count(unsigned int, unsigned long);
unsigned int plat_get_aff_state(unsigned int, unsigned long);

/*******************************************************************************
 * Optional BL3-1 functions (may be overridden)
 ******************************************************************************/
void bl31_plat_enable_mmu();

/*******************************************************************************
 * Mandatory BL3-2 functions (only if platform contains a BL3-2
 ******************************************************************************/
void bl32_platform_setup(void);
struct meminfo *bl32_plat_sec_mem_layout(void);

/*******************************************************************************
 * Optional BL3-2 functions (may be overridden)
 ******************************************************************************/
void bl32_plat_enable_mmu();


#endif /* __PLATFORM_H__ */
