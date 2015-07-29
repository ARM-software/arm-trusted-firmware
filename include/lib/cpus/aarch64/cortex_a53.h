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

#ifndef __CORTEX_A53_H__
#define __CORTEX_A53_H__

/* Cortex-A53 midr for revision 0 */
#define CORTEX_A53_MIDR 0x410FD030

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define CPUECTLR_EL1			S3_1_C15_C2_1	/* Instruction def. */

#define CPUECTLR_SMP_BIT		(1 << 6)

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CPUACTLR_EL1			S3_1_C15_C2_0	/* Instruction def. */

#define CPUACTLR_DTAH			(1 << 24)

/*******************************************************************************
 * L2 Auxiliary Control register specific definitions.
 ******************************************************************************/
#define L2ACTLR_EL1			S3_1_C15_C0_0	/* Instruction def. */

#define L2ACTLR_ENABLE_UNIQUECLEAN	(1 << 14)
#define L2ACTLR_DISABLE_CLEAN_PUSH	(1 << 3)

#endif /* __CORTEX_A53_H__ */
