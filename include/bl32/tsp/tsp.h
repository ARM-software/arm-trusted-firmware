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

#ifndef __TSP_H__
#define __TSP_H__

/*
 * SMC function IDs that TSP uses to signal various forms of completions
 * to the secure payload dispatcher.
 */
#define TSP_ENTRY_DONE		0xf2000000
#define TSP_ON_DONE		0xf2000001
#define TSP_OFF_DONE		0xf2000002
#define TSP_SUSPEND_DONE	0xf2000003
#define TSP_RESUME_DONE		0xf2000004
#define TSP_PREEMPTED		0xf2000005
#define TSP_SYSTEM_OFF_DONE	0xf2000008
#define TSP_SYSTEM_RESET_DONE	0xf2000009

/*
 * Function identifiers to handle FIQs through the synchronous handling model.
 * If the TSP was previously interrupted then control has to be returned to
 * the TSPD after handling the interrupt else execution can remain in the TSP.
 */
#define TSP_HANDLED_S_EL1_FIQ		0xf2000006
#define TSP_EL3_FIQ			0xf2000007

/* SMC function ID that TSP uses to request service from secure monitor */
#define TSP_GET_ARGS		0xf2001000

/*
 * Identifiers for various TSP services. Corresponding function IDs (whether
 * fast or standard) are generated by macros defined below
 */
#define TSP_ADD		0x2000
#define TSP_SUB		0x2001
#define TSP_MUL		0x2002
#define TSP_DIV		0x2003
#define TSP_HANDLE_FIQ_AND_RETURN	0x2004

/*
 * Generate function IDs for TSP services to be used in SMC calls, by
 * appropriately setting bit 31 to differentiate standard and fast SMC calls
 */
#define TSP_STD_FID(fid)	((fid) | 0x72000000 | (0 << 31))
#define TSP_FAST_FID(fid)	((fid) | 0x72000000 | (1 << 31))

/* SMC function ID to request a previously preempted std smc */
#define TSP_FID_RESUME		TSP_STD_FID(0x3000)

/*
 * Identify a TSP service from function ID filtering the last 16 bits from the
 * SMC function ID
 */
#define TSP_BARE_FID(fid)	((fid) & 0xffff)

/*
 * Total number of function IDs implemented for services offered to NS clients.
 * The function IDs are defined above
 */
#define TSP_NUM_FID		0x4

/* TSP implementation version numbers */
#define TSP_VERSION_MAJOR	0x0 /* Major version */
#define TSP_VERSION_MINOR	0x1 /* Minor version */

/*
 * Standard Trusted OS Function IDs that fall under Trusted OS call range
 * according to SMC calling convention
 */
#define TOS_CALL_COUNT		0xbf00ff00 /* Number of calls implemented */
#define TOS_UID			0xbf00ff01 /* Implementation UID */
/*				0xbf00ff02 is reserved */
#define TOS_CALL_VERSION	0xbf00ff03 /* Trusted OS Call Version */

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

typedef uint32_t tsp_vector_isn_t;

typedef struct tsp_vectors {
	tsp_vector_isn_t std_smc_entry;
	tsp_vector_isn_t fast_smc_entry;
	tsp_vector_isn_t cpu_on_entry;
	tsp_vector_isn_t cpu_off_entry;
	tsp_vector_isn_t cpu_resume_entry;
	tsp_vector_isn_t cpu_suspend_entry;
	tsp_vector_isn_t fiq_entry;
	tsp_vector_isn_t system_off_entry;
	tsp_vector_isn_t system_reset_entry;
} tsp_vectors_t;

typedef struct work_statistics {
	uint32_t fiq_count;		/* Number of FIQs on this cpu */
	uint32_t irq_count;		/* Number of IRQs on this cpu */
	uint32_t sync_fiq_count;	/* Number of sync. fiqs on this cpu */
	uint32_t sync_fiq_ret_count;	/* Number of fiq returns on this cpu */
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

/* FIQ management functions */
void tsp_update_sync_fiq_stats(uint32_t type, uint64_t elr_el3);

/* Data structure to keep track of TSP statistics */
extern spinlock_t console_lock;
extern work_statistics_t tsp_stats[PLATFORM_CORE_COUNT];

/* Vector table of jumps */
extern tsp_vectors_t tsp_vector_table;

#endif /* __ASSEMBLY__ */

#endif /* __BL2_H__ */
