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

#ifndef __TSP_PRIVATE_H__
#define __TSP_PRIVATE_H__

/* Definitions to help the assembler access the SMC/ERET args structure */
#define TSP_ARGS_SIZE		0x40
#define TSP_ARG0		0x0
#define TSP_ARG1		0x8
#define TSP_ARG2		0x10
#define TSP_ARG3		0x18
#define TSP_ARG4		0x20
#define TSP_ARG5		0x28
#define TSP_ARG6		0x30
#define TSP_ARG7		0x38
#define TSP_ARGS_END		0x40


#ifndef __ASSEMBLY__

#include <cassert.h>
#include <platform_def.h> /* For CACHE_WRITEBACK_GRANULE */
#include <spinlock.h>
#include <stdint.h>
#include <tsp.h>


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

typedef struct tsp_args {
	uint64_t _regs[TSP_ARGS_END >> 3];
} __aligned(CACHE_WRITEBACK_GRANULE) tsp_args_t;

/* Macros to access members of the above structure using their offsets */
#define read_sp_arg(args, offset)	((args)->_regs[offset >> 3])
#define write_sp_arg(args, offset, val) (((args)->_regs[offset >> 3])	\
					 = val)
/*
 * Ensure that the assembler's view of the size of the tsp_args is the
 * same as the compilers
 */
CASSERT(TSP_ARGS_SIZE == sizeof(tsp_args_t), assert_sp_args_size_mismatch);

void tsp_get_magic(uint64_t args[4]);

tsp_args_t *tsp_cpu_resume_main(uint64_t arg0,
				uint64_t arg1,
				uint64_t arg2,
				uint64_t arg3,
				uint64_t arg4,
				uint64_t arg5,
				uint64_t arg6,
				uint64_t arg7);
tsp_args_t *tsp_cpu_suspend_main(uint64_t arg0,
				 uint64_t arg1,
				 uint64_t arg2,
				 uint64_t arg3,
				 uint64_t arg4,
				 uint64_t arg5,
				 uint64_t arg6,
				 uint64_t arg7);
tsp_args_t *tsp_cpu_on_main(void);
tsp_args_t *tsp_cpu_off_main(uint64_t arg0,
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
extern spinlock_t console_lock;
extern work_statistics_t tsp_stats[PLATFORM_CORE_COUNT];

/* Vector table of jumps */
extern tsp_vectors_t tsp_vector_table;


#endif /* __ASSEMBLY__ */

#endif /* __TSP_PRIVATE_H__ */

