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

#include <platform.h>
#include <cassert.h>

/*
 * SMC function IDs that TSP uses to signal various forms of completions
 * to the secure payload dispatcher.
 */
#define TSP_ENTRY_DONE		0xf2000000
#define TSP_ON_DONE		0xf2000001
#define TSP_OFF_DONE		0xf2000002
#define TSP_SUSPEND_DONE	0xf2000003
#define TSP_RESUME_DONE		0xf2000004
#define TSP_WORK_DONE		0xf2000005

/* SMC function ID that TSP uses to request service from secure montior */
#define TSP_GET_ARGS		0xf2001000

/* Function IDs for various TSP services */
#define TSP_FID_ADD		0xf2002000
#define TSP_FID_SUB		0xf2002001
#define TSP_FID_MUL		0xf2002002
#define TSP_FID_DIV		0xf2002003

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
#include <stdint.h>

typedef void (*tsp_generic_fptr)(uint64_t arg0,
				 uint64_t arg1,
				 uint64_t arg2,
				 uint64_t arg3,
				 uint64_t arg4,
				 uint64_t arg5,
				 uint64_t arg6,
				 uint64_t arg7);

typedef struct {
	tsp_generic_fptr fast_smc_entry;
	tsp_generic_fptr cpu_on_entry;
	tsp_generic_fptr cpu_off_entry;
	tsp_generic_fptr cpu_resume_entry;
	tsp_generic_fptr cpu_suspend_entry;
} entry_info;

typedef struct {
	uint32_t smc_count;		/* Number of returns on this cpu */
	uint32_t eret_count;		/* Number of entries on this cpu */
	uint32_t cpu_on_count;		/* Number of cpu on requests */
	uint32_t cpu_off_count;		/* Number of cpu off requests */
	uint32_t cpu_suspend_count;	/* Number of cpu suspend requests */
	uint32_t cpu_resume_count;	/* Number of cpu resume requests */
} __aligned(CACHE_WRITEBACK_GRANULE) work_statistics;

typedef struct {
	uint64_t _regs[TSP_ARGS_END >> 3];
} __aligned(CACHE_WRITEBACK_GRANULE) tsp_args;

/* Macros to access members of the above structure using their offsets */
#define read_sp_arg(args, offset)	((args)->_regs[offset >> 3])
#define write_sp_arg(args, offset, val)(((args)->_regs[offset >> 3])	\
					 = val)

/*
 * Ensure that the assembler's view of the size of the tsp_args is the
 * same as the compilers
 */
CASSERT(TSP_ARGS_SIZE == sizeof(tsp_args), assert_sp_args_size_mismatch);

extern void tsp_get_magic(uint64_t args[4]);

extern void tsp_fast_smc_entry(uint64_t arg0,
				uint64_t arg1,
				uint64_t arg2,
				uint64_t arg3,
				uint64_t arg4,
				uint64_t arg5,
				uint64_t arg6,
				uint64_t arg7);
extern void tsp_cpu_resume_entry(uint64_t arg0,
				 uint64_t arg1,
				 uint64_t arg2,
				 uint64_t arg3,
				 uint64_t arg4,
				 uint64_t arg5,
				 uint64_t arg6,
				 uint64_t arg7);
extern tsp_args *tsp_cpu_resume_main(uint64_t arg0,
				     uint64_t arg1,
				     uint64_t arg2,
				     uint64_t arg3,
				     uint64_t arg4,
				     uint64_t arg5,
				     uint64_t arg6,
				     uint64_t arg7);
extern void tsp_cpu_suspend_entry(uint64_t arg0,
				  uint64_t arg1,
				  uint64_t arg2,
				  uint64_t arg3,
				  uint64_t arg4,
				  uint64_t arg5,
				  uint64_t arg6,
				  uint64_t arg7);
extern tsp_args *tsp_cpu_suspend_main(uint64_t arg0,
				      uint64_t arg1,
				      uint64_t arg2,
				      uint64_t arg3,
				      uint64_t arg4,
				      uint64_t arg5,
				      uint64_t arg6,
				      uint64_t arg7);
extern void tsp_cpu_on_entry(uint64_t arg0,
			     uint64_t arg1,
			     uint64_t arg2,
			     uint64_t arg3,
			     uint64_t arg4,
			     uint64_t arg5,
			     uint64_t arg6,
			     uint64_t arg7);
extern tsp_args *tsp_cpu_on_main(void);
extern void tsp_cpu_off_entry(uint64_t arg0,
			      uint64_t arg1,
			      uint64_t arg2,
			      uint64_t arg3,
			      uint64_t arg4,
			      uint64_t arg5,
			      uint64_t arg6,
			      uint64_t arg7);
extern tsp_args *tsp_cpu_off_main(uint64_t arg0,
				  uint64_t arg1,
				  uint64_t arg2,
				  uint64_t arg3,
				  uint64_t arg4,
				  uint64_t arg5,
				  uint64_t arg6,
				  uint64_t arg7);
#endif /* __ASSEMBLY__ */

#endif /* __BL2_H__ */
