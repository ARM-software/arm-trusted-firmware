/*
 * Copyright (c) 2014-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TSP_PRIVATE_H
#define TSP_PRIVATE_H

/*******************************************************************************
 * The TSP memory footprint starts at address BL32_BASE and ends with the
 * linker symbol __BL32_END__. Use these addresses to compute the TSP image
 * size.
 ******************************************************************************/
#define BL32_TOTAL_LIMIT BL32_END
#define BL32_TOTAL_SIZE (BL32_TOTAL_LIMIT - (unsigned long) BL32_BASE)

#ifndef __ASSEMBLER__

#include <stdint.h>

#include <bl32/tsp/tsp.h>
#include <lib/cassert.h>
#include <lib/spinlock.h>
#include <smccc_helpers.h>

typedef struct work_statistics {
	/* Number of s-el1 interrupts on this cpu */
	uint32_t sel1_intr_count;
	/* Number of non s-el1 interrupts on this cpu which preempted TSP */
	uint32_t preempt_intr_count;
	/* Number of sync s-el1 interrupts on this cpu */
	uint32_t sync_sel1_intr_count;
	/* Number of s-el1 interrupts returns on this cpu */
	uint32_t sync_sel1_intr_ret_count;
	uint32_t smc_count;		/* Number of returns on this cpu */
	uint32_t eret_count;		/* Number of entries on this cpu */
	uint32_t cpu_on_count;		/* Number of cpu on requests */
	uint32_t cpu_off_count;		/* Number of cpu off requests */
	uint32_t cpu_suspend_count;	/* Number of cpu suspend requests */
	uint32_t cpu_resume_count;	/* Number of cpu resume requests */
} __aligned(CACHE_WRITEBACK_GRANULE) work_statistics_t;

/* Macros to access members of the above structure using their offsets */
#define read_sp_arg(args, offset)	((args)->_regs[offset >> 3])
#define write_sp_arg(args, offset, val) (((args)->_regs[offset >> 3])	\
					 = val)

uint128_t tsp_get_magic(void);

smc_args_t *set_smc_args(uint64_t arg0,
			 uint64_t arg1,
			 uint64_t arg2,
			 uint64_t arg3,
			 uint64_t arg4,
			 uint64_t arg5,
			 uint64_t arg6,
			 uint64_t arg7);
smc_args_t *tsp_cpu_resume_main(uint64_t max_off_pwrlvl,
				uint64_t arg1,
				uint64_t arg2,
				uint64_t arg3,
				uint64_t arg4,
				uint64_t arg5,
				uint64_t arg6,
				uint64_t arg7);
smc_args_t *tsp_cpu_suspend_main(uint64_t arg0,
				 uint64_t arg1,
				 uint64_t arg2,
				 uint64_t arg3,
				 uint64_t arg4,
				 uint64_t arg5,
				 uint64_t arg6,
				 uint64_t arg7);
smc_args_t *tsp_cpu_on_main(void);
smc_args_t *tsp_cpu_off_main(uint64_t arg0,
			     uint64_t arg1,
			     uint64_t arg2,
			     uint64_t arg3,
			     uint64_t arg4,
			     uint64_t arg5,
			     uint64_t arg6,
			     uint64_t arg7);

/* Generic Timer functions */
void tsp_generic_timer_start(void);
void tsp_generic_timer_handler(void);
void tsp_generic_timer_stop(void);
void tsp_generic_timer_save(void);
void tsp_generic_timer_restore(void);

/* S-EL1 interrupt management functions */
void tsp_update_sync_sel1_intr_stats(uint32_t type, uint64_t elr_el3);


/* Data structure to keep track of TSP statistics */
extern work_statistics_t tsp_stats[PLATFORM_CORE_COUNT];

/* Vector table of jumps */
extern tsp_vectors_t tsp_vector_table;

/* functions */
int32_t tsp_common_int_handler(void);
int32_t tsp_handle_preemption(void);

smc_args_t *tsp_abort_smc_handler(uint64_t func,
				  uint64_t arg1,
				  uint64_t arg2,
				  uint64_t arg3,
				  uint64_t arg4,
				  uint64_t arg5,
				  uint64_t arg6,
				  uint64_t arg7);

smc_args_t *tsp_smc_handler(uint64_t func,
			    uint64_t arg1,
			    uint64_t arg2,
			    uint64_t arg3,
			    uint64_t arg4,
			    uint64_t arg5,
			    uint64_t arg6,
			    uint64_t arg7);

smc_args_t *tsp_system_reset_main(uint64_t arg0,
				  uint64_t arg1,
				  uint64_t arg2,
				  uint64_t arg3,
				  uint64_t arg4,
				  uint64_t arg5,
				  uint64_t arg6,
				  uint64_t arg7);

smc_args_t *tsp_system_off_main(uint64_t arg0,
				uint64_t arg1,
				uint64_t arg2,
				uint64_t arg3,
				uint64_t arg4,
				uint64_t arg5,
				uint64_t arg6,
				uint64_t arg7);

uint64_t tsp_main(void);
#endif /* __ASSEMBLER__ */

#endif /* TSP_PRIVATE_H */
