/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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

#ifndef __CORTEX_A_COMMON_H__
#define __CORTEX_A_COMMON_H__

/* ACTLR definitions */
#define ACTLR_ENB_L2ACTLR_ACCESS	(1 << 6)
#define ACTLR_ENB_L2ECTLR_ACCESS	(1 << 5)
#define ACTLR_ENB_L2CTLR_ACCESS		(1 << 4)
#define ACTLR_ENB_CPUECTLR_ACCESS	(1 << 1)
#define ACTLR_ENB_CPUACTLR_ACCESS	(1 << 0)
#define ACTLR_ACCESS_CTRL		(ACTLR_ENB_L2ACTLR_ACCESS | \
					 ACTLR_ENB_L2ECTLR_ACCESS | \
					 ACTLR_ENB_L2CTLR_ACCESS | \
					 ACTLR_ENB_CPUECTLR_ACCESS | \
					 ACTLR_ENB_CPUACTLR_ACCESS)

#endif /* __CORTEX_A_COMMON_H__ */
