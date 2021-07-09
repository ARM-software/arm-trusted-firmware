/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TRP_PRIVATE_H
#define TRP_PRIVATE_H

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

#include <stdint.h>

/* Data structure to hold SMC arguments */
typedef struct trp_args {
	uint64_t regs[TRP_ARGS_END >> 3];
} __aligned(CACHE_WRITEBACK_GRANULE) trp_args_t;

#define write_trp_arg(args, offset, val) (((args)->regs[offset >> 3])	\
					 = val)

/* Definitions for RMI VERSION */
#define RMI_ABI_VERSION_MAJOR		U(0x0)
#define RMI_ABI_VERSION_MINOR		U(0x0)
#define RMI_ABI_VERSION			((RMI_ABI_VERSION_MAJOR << 16) | \
					RMI_ABI_VERSION_MINOR)

/* Helper to issue SMC calls to BL31 */
uint64_t trp_smc(trp_args_t *);

/* The main function to executed only by Primary CPU */
void trp_main(void);

/* Setup TRP. Executed only by Primary CPU */
void trp_setup(void);

#endif /* __ASSEMBLER__ */
#endif /* TRP_PRIVATE_H */
