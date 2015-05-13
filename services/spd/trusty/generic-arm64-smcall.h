/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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

#include "smcall.h"

#define	SMC_ENTITY_PLATFORM_MONITOR	61

/*
 * SMC calls implemented by EL3 monitor
 */

/*
 * Write character in r1 to debug console
 */
#define SMC_FC_DEBUG_PUTC	SMC_FASTCALL_NR(SMC_ENTITY_PLATFORM_MONITOR, 0x0)

/*
 * Get register base address
 * r1: SMC_GET_GIC_BASE_GICD or SMC_GET_GIC_BASE_GICC
 */
#define SMC_GET_GIC_BASE_GICD	0
#define SMC_GET_GIC_BASE_GICC	1
#define SMC_FC_GET_REG_BASE	SMC_FASTCALL_NR(SMC_ENTITY_PLATFORM_MONITOR, 0x1)
#define SMC_FC64_GET_REG_BASE	SMC_FASTCALL64_NR(SMC_ENTITY_PLATFORM_MONITOR, 0x1)
