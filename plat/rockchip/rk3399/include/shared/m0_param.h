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

#ifndef __M0_PARAM_H__
#define __M0_PARAM_H__

#ifndef __LINKER__
enum {
	M0_FUNC_SUSPEND = 0,
	M0_FUNC_DRAM	= 1,
};
#endif /* __LINKER__ */

#define PARAM_ADDR		0xc0

#define PARAM_M0_FUNC		0x00
#define PARAM_DRAM_FREQ		0x04
#define PARAM_DPLL_CON0		0x08
#define PARAM_DPLL_CON1		0x0c
#define PARAM_DPLL_CON2		0x10
#define PARAM_DPLL_CON3		0x14
#define PARAM_DPLL_CON4		0x18
#define PARAM_DPLL_CON5		0x1c
#define PARAM_FREQ_SELECT	0x20
#define PARAM_M0_DONE		0x24
#define PARAM_M0_SIZE		0x28
#define M0_DONE_FLAG		0xf59ec39a

#endif /*__M0_PARAM_H__*/
