/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
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

#ifndef __PARAMETER_H__
#define __PARAMETER_H__

#include <arch_helpers.h>
#include <console.h>
#include <debug.h>
#include <delay_timer.h>
#include <mmio.h>
#include <platform_def.h>
#include <plat_private.h>
#include <soc.h>
#include <string.h>

#define DDR_REGION_NR_MAX		10
#define REGION_NR_OFFSET		0
#define REGION_ADDR_OFFSET		8
#define REGION_DATA_PER_BYTES		8
#define RG_SIZE_MB(byte)		((byte) >> 20)

/* unit: MB */
struct param_ddr_usage {
	uint64_t boundary;

	uint32_t ns_nr;
	uint64_t ns_base[DDR_REGION_NR_MAX];
	uint64_t ns_top[DDR_REGION_NR_MAX];

	uint32_t s_nr;
	uint64_t s_base[DDR_REGION_NR_MAX];
	uint64_t s_top[DDR_REGION_NR_MAX];
};

struct param_ddr_usage ddr_region_usage_parse(uint64_t addr, uint64_t max_mb);

#endif /* __PARAMETER_H__ */
