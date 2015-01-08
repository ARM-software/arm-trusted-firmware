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

#ifndef __FVP_PRIVATE_H__
#define __FVP_PRIVATE_H__

#include <bakery_lock.h>
#include <bl_common.h>
#include <cpu_data.h>
#include <platform_def.h>


typedef volatile struct mailbox {
	unsigned long value
	__attribute__((__aligned__(CACHE_WRITEBACK_GRANULE)));
} mailbox_t;

/*******************************************************************************
 * This structure represents the superset of information that is passed to
 * BL31 e.g. while passing control to it from BL2 which is bl31_params
 * and bl31_plat_params and its elements
 ******************************************************************************/
typedef struct bl2_to_bl31_params_mem {
	bl31_params_t bl31_params;
	image_info_t bl31_image_info;
	image_info_t bl32_image_info;
	image_info_t bl33_image_info;
	entry_point_info_t bl33_ep_info;
	entry_point_info_t bl32_ep_info;
	entry_point_info_t bl31_ep_info;
} bl2_to_bl31_params_mem_t;

#if USE_COHERENT_MEM
/*
 * These are wrapper macros to the Coherent Memory Bakery Lock API.
 */
#define fvp_lock_init(_lock_arg)	bakery_lock_init(_lock_arg)
#define fvp_lock_get(_lock_arg)		bakery_lock_get(_lock_arg)
#define fvp_lock_release(_lock_arg)	bakery_lock_release(_lock_arg)

#else

/*******************************************************************************
 * Constants to specify how many bakery locks this platform implements. These
 * are used if the platform chooses not to use coherent memory for bakery lock
 * data structures.
 ******************************************************************************/
#define FVP_MAX_BAKERIES	1
#define FVP_PWRC_BAKERY_ID	0

/*******************************************************************************
 * Definition of structure which holds platform specific per-cpu data. Currently
 * it holds only the bakery lock information for each cpu. Constants to
 * specify how many bakeries this platform implements and bakery ids are
 * specified in fvp_def.h
 ******************************************************************************/
typedef struct fvp_cpu_data {
	bakery_info_t pcpu_bakery_info[FVP_MAX_BAKERIES];
} fvp_cpu_data_t;

/* Macro to define the offset of bakery_info_t in fvp_cpu_data_t */
#define FVP_CPU_DATA_LOCK_OFFSET	__builtin_offsetof\
					    (fvp_cpu_data_t, pcpu_bakery_info)


/*******************************************************************************
 * Helper macros for bakery lock api when using the above fvp_cpu_data_t for
 * bakery lock data structures. It assumes that the bakery_info is at the
 * beginning of the platform specific per-cpu data.
 ******************************************************************************/
#define fvp_lock_init(_lock_arg)	/* No init required */
#define fvp_lock_get(_lock_arg)		bakery_lock_get(_lock_arg,  	    \
						CPU_DATA_PLAT_PCPU_OFFSET + \
						FVP_CPU_DATA_LOCK_OFFSET)
#define fvp_lock_release(_lock_arg)	bakery_lock_release(_lock_arg,	    \
						CPU_DATA_PLAT_PCPU_OFFSET + \
						FVP_CPU_DATA_LOCK_OFFSET)

/*
 * Ensure that the size of the FVP specific per-cpu data structure and the size
 * of the memory allocated in generic per-cpu data for the platform are the same.
 */
CASSERT(PLAT_PCPU_DATA_SIZE == sizeof(fvp_cpu_data_t),	\
	fvp_pcpu_data_size_mismatch);

#endif /* __USE_COHERENT_MEM__ */

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
void fvp_configure_mmu_el1(unsigned long total_base,
			   unsigned long total_size,
			   unsigned long,
			   unsigned long
#if USE_COHERENT_MEM
			   , unsigned long,
			   unsigned long
#endif
			   );
void fvp_configure_mmu_el3(unsigned long total_base,
			   unsigned long total_size,
			   unsigned long,
			   unsigned long
#if USE_COHERENT_MEM
			   , unsigned long,
			   unsigned long
#endif
			   );

int fvp_config_setup(void);

void fvp_cci_init(void);
void fvp_cci_enable(void);

void fvp_gic_init(void);

/* Declarations for fvp_topology.c */
int fvp_setup_topology(void);

/* Declarations for fvp_io_storage.c */
void fvp_io_setup(void);

/* Declarations for fvp_security.c */
void fvp_security_setup(void);

/* Gets the SPR for BL32 entry */
uint32_t fvp_get_spsr_for_bl32_entry(void);

/* Gets the SPSR for BL33 entry */
uint32_t fvp_get_spsr_for_bl33_entry(void);


#endif /* __FVP_PRIVATE_H__ */
