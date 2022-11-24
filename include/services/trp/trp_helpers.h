/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TRP_HELPERS_H
#define TRP_HELPERS_H

/* Definitions to help the assembler access the SMC/ERET args structure */
#define TRP_ARGS_SIZE		TRP_ARGS_END
#define TRP_ARG0		0x0
#define TRP_ARG1		0x8
#define TRP_ARG2		0x10
#define TRP_ARG3		0x18
#define TRP_ARG4		0x20
#define TRP_ARG5		0x28
#define TRP_ARG6		0x30
#define TRP_ARG7		0x38
#define TRP_ARGS_END		0x40

#ifndef __ASSEMBLER__

#include <platform_def.h>

/* Data structure to hold SMC arguments */
typedef struct trp_args {
	uint64_t regs[TRP_ARGS_END >> 3];
} __aligned(CACHE_WRITEBACK_GRANULE) trp_args_t;

trp_args_t *set_smc_args(uint64_t arg0,
			 uint64_t arg1,
			 uint64_t arg2,
			 uint64_t arg3,
			 uint64_t arg4,
			 uint64_t arg5,
			 uint64_t arg6,
			 uint64_t arg7);

__dead2 void trp_boot_abort(uint64_t err);

/* TRP SMC result registers X0-X4 */
#define TRP_SMC_RESULT_REGS	5

struct trp_smc_result {
	unsigned long long x[TRP_SMC_RESULT_REGS];
};

#endif /* __ASSEMBLER __ */
#endif /* TRP_HELPERS_H */
