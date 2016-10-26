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

#ifndef __RK3399_MCU_H__
#define __RK3399_MCU_H__

typedef unsigned int uint32_t;

#define mmio_read_32(c)	({unsigned int __v = \
				(*(volatile unsigned int *)(c)); __v; })
#define mmio_write_32(c, v)	((*(volatile unsigned int *)(c)) = (v))

#define mmio_clrbits_32(addr, clear) \
		mmio_write_32(addr, (mmio_read_32(addr) & ~(clear)))
#define mmio_setbits_32(addr, set) \
		mmio_write_32(addr, (mmio_read_32(addr)) | (set))
#define mmio_clrsetbits_32(addr, clear, set) \
		mmio_write_32(addr, (mmio_read_32(addr) & ~(clear)) | (set))

#define MCU_BASE			0x40000000
#define PMU_BASE			(MCU_BASE + 0x07310000)
#define CRU_BASE_ADDR			0x47760000
#define GRF_BASE_ADDR			0x47770000
#define PMU_CRU_BASE_ADDR		0x47750000
#define VOP_LITE_BASE_ADDR		0x478F0000
#define VOP_BIG_BASE_ADDR		0x47900000
#define CIC_BASE_ADDR			0x47620000

void handle_suspend(void);
void handle_dram(void);

#endif /* __RK3399_MCU_H__ */
