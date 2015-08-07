/*
 * Copyright (c) 2014-2015, ARM Limited and Contributors. All rights reserved.
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

#ifndef __CPU_DATA_H__
#define __CPU_DATA_H__

/* Offsets for the cpu_data structure */
#define CPU_DATA_CRASH_BUF_OFFSET	0x18
#if CRASH_REPORTING
#define CPU_DATA_LOG2SIZE		7
#else
#define CPU_DATA_LOG2SIZE		6
#endif
/* need enough space in crash buffer to save 8 registers */
#define CPU_DATA_CRASH_BUF_SIZE		64
#define CPU_DATA_CPU_OPS_PTR		0x10

#ifndef __ASSEMBLY__

#include <arch_helpers.h>
#include <cassert.h>
#include <platform_def.h>
#include <psci.h>
#include <stdint.h>

/* Offsets for the cpu_data structure */
#define CPU_DATA_PSCI_LOCK_OFFSET	__builtin_offsetof\
		(cpu_data_t, psci_svc_cpu_data.pcpu_bakery_info)

#if PLAT_PCPU_DATA_SIZE
#define CPU_DATA_PLAT_PCPU_OFFSET	__builtin_offsetof\
		(cpu_data_t, platform_cpu_data)
#endif

/*******************************************************************************
 * Function & variable prototypes
 ******************************************************************************/

/*******************************************************************************
 * Cache of frequently used per-cpu data:
 *   Pointers to non-secure and secure security state contexts
 *   Address of the crash stack
 * It is aligned to the cache line boundary to allow efficient concurrent
 * manipulation of these pointers on different cpus
 *
 * TODO: Add other commonly used variables to this (tf_issues#90)
 *
 * The data structure and the _cpu_data accessors should not be used directly
 * by components that have per-cpu members. The member access macros should be
 * used for this.
 ******************************************************************************/
typedef struct cpu_data {
	void *cpu_context[2];
	uint64_t cpu_ops_ptr;
#if CRASH_REPORTING
	uint64_t crash_buf[CPU_DATA_CRASH_BUF_SIZE >> 3];
#endif
	struct psci_cpu_data psci_svc_cpu_data;
#if PLAT_PCPU_DATA_SIZE
	uint8_t platform_cpu_data[PLAT_PCPU_DATA_SIZE];
#endif
} __aligned(CACHE_WRITEBACK_GRANULE) cpu_data_t;

#if CRASH_REPORTING
/* verify assembler offsets match data structures */
CASSERT(CPU_DATA_CRASH_BUF_OFFSET == __builtin_offsetof
	(cpu_data_t, crash_buf),
	assert_cpu_data_crash_stack_offset_mismatch);
#endif

CASSERT((1 << CPU_DATA_LOG2SIZE) == sizeof(cpu_data_t),
	assert_cpu_data_log2size_mismatch);

CASSERT(CPU_DATA_CPU_OPS_PTR == __builtin_offsetof
		(cpu_data_t, cpu_ops_ptr),
		assert_cpu_data_cpu_ops_ptr_offset_mismatch);

struct cpu_data *_cpu_data_by_index(uint32_t cpu_index);

/* Return the cpu_data structure for the current CPU. */
static inline struct cpu_data *_cpu_data(void)
{
	return (cpu_data_t *)read_tpidr_el3();
}


/**************************************************************************
 * APIs for initialising and accessing per-cpu data
 *************************************************************************/

void init_cpu_data_ptr(void);
void init_cpu_ops(void);

#define get_cpu_data(_m)		   _cpu_data()->_m
#define set_cpu_data(_m, _v)		   _cpu_data()->_m = _v
#define get_cpu_data_by_index(_ix, _m)	   _cpu_data_by_index(_ix)->_m
#define set_cpu_data_by_index(_ix, _m, _v) _cpu_data_by_index(_ix)->_m = _v

#define flush_cpu_data(_m)	   flush_dcache_range((uint64_t) 	  \
						      &(_cpu_data()->_m), \
						      sizeof(_cpu_data()->_m))
#define inv_cpu_data(_m)	   inv_dcache_range((uint64_t) 	  \
						      &(_cpu_data()->_m), \
						      sizeof(_cpu_data()->_m))
#define flush_cpu_data_by_index(_ix, _m)	\
				   flush_dcache_range((uint64_t)	  \
					 &(_cpu_data_by_index(_ix)->_m),  \
					 sizeof(_cpu_data_by_index(_ix)->_m))


#endif /* __ASSEMBLY__ */
#endif /* __CPU_DATA_H__ */
