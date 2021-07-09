/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RMMD_INITIAL_CONTEXT_H
#define RMMD_INITIAL_CONTEXT_H

#include <arch.h>

/*
 * SPSR_EL2
 *   M=0x9 (0b1001 EL2h)
 *   M[4]=0
 *   DAIF=0xF Exceptions masked on entry.
 *   BTYPE=0  BTI not yet supported.
 *   SSBS=0   Not yet supported.
 *   IL=0     Not an illegal exception return.
 *   SS=0     Not single stepping.
 *   PAN=1    RMM shouldn't access realm memory.
 *   UAO=0
 *   DIT=0
 *   TCO=0
 *   NZCV=0
 */
#define REALM_SPSR_EL2		(					\
					SPSR_M_EL2H			| \
					(0xF << SPSR_DAIF_SHIFT)	| \
					SPSR_PAN_BIT			\
				)

#endif /* RMMD_INITIAL_CONTEXT_H */
