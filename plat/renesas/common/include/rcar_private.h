/*
 * Copyright (c) 2015-2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RCAR_PRIVATE_H
#define RCAR_PRIVATE_H

#include <common/bl_common.h>
#include <lib/bakery_lock.h>
#include <lib/el3_runtime/cpu_data.h>

#include <platform_def.h>

typedef volatile struct mailbox {
	unsigned long value __aligned(CACHE_WRITEBACK_GRANULE);
} mailbox_t;

/*
 * This structure represents the superset of information that is passed to
 * BL31 e.g. while passing control to it from BL2 which is bl31_params
 * and bl31_plat_params and its elements
 */
typedef struct bl2_to_bl31_params_mem {
	image_info_t bl32_image_info;
	image_info_t bl33_image_info;
	entry_point_info_t bl33_ep_info;
	entry_point_info_t bl32_ep_info;
} bl2_to_bl31_params_mem_t;

#if USE_COHERENT_MEM
#define RCAR_INSTANTIATE_LOCK	DEFINE_BAKERY_LOCK(rcar_lock);
#define rcar_lock_init()	bakery_lock_init(&rcar_lock)
#define rcar_lock_get()		bakery_lock_get(&rcar_lock)
#define rcar_lock_release()	bakery_lock_release(&rcar_lock)
#else
/*
 * Constants to specify how many bakery locks this platform implements. These
 * are used if the platform chooses not to use coherent memory for bakery lock
 * data structures.
 */
#define RCAR_MAX_BAKERIES	2
#define RCAR_PWRC_BAKERY_ID	0

/*
 * Definition of structure which holds platform specific per-cpu data. Currently
 * it holds only the bakery lock information for each cpu. Constants to
 * specify how many bakeries this platform implements and bakery ids are
 * specified in rcar_def.h
 */
typedef struct rcar_cpu_data {
	bakery_info_t pcpu_bakery_info[RCAR_MAX_BAKERIES];
} rcar_cpu_data_t;

#define RCAR_CPU_DATA_LOCK_OFFSET	\
	__builtin_offsetof(rcar_cpu_data_t, pcpu_bakery_info)
/*
 * Helper macros for bakery lock api when using the above rcar_cpu_data_t for
 * bakery lock data structures. It assumes that the bakery_info is at the
 * beginning of the platform specific per-cpu data.
 */
#define rcar_lock_init(_lock_arg)

#define rcar_lock_get(_lock_arg)					\
	bakery_lock_get(_lock_arg,					\
		CPU_DATA_PLAT_PCPU_OFFSET + RCAR_CPU_DATA_LOCK_OFFSET)

#define rcar_lock_release(_lock_arg)					\
	bakery_lock_release(_lock_arg,					\
		CPU_DATA_PLAT_PCPU_OFFSET + RCAR_CPU_DATA_LOCK_OFFSET)
/*
 * Ensure that the size of the RCAR specific per-cpu data structure and the size
 * of the memory allocated in generic per-cpu data for the platform are the same
 */
CASSERT(sizeof(rcar_cpu_data_t) == PLAT_PCPU_DATA_SIZE,
	rcar_pcpu_data_size_mismatch);
#endif
/*
 * Function and variable prototypes
 */
void rcar_configure_mmu_el3(unsigned long total_base,
			    unsigned long total_size,
			    unsigned long ro_start, unsigned long ro_limit
#if USE_COHERENT_MEM
			    , unsigned long coh_start, unsigned long coh_limit
#endif
			    );

void rcar_setup_topology(void);
void rcar_cci_disable(void);
void rcar_cci_enable(void);
void rcar_cci_init(void);

void plat_invalidate_icache(void);
void plat_cci_disable(void);
void plat_cci_enable(void);
void plat_cci_init(void);

void mstpcr_write(uint32_t mstpcr, uint32_t mstpsr, uint32_t target_bit);
void cpg_write(uintptr_t regadr, uint32_t regval);

void rcar_console_boot_init(void);
void rcar_console_boot_end(void);
void rcar_console_runtime_init(void);
void rcar_console_runtime_end(void);

#endif /* RCAR_PRIVATE_H */
