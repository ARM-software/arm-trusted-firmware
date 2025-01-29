/*
 * Copyright (c) 2014-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CPU_DATA_H
#define CPU_DATA_H

#include <platform_def.h>	/* CACHE_WRITEBACK_GRANULE required */

#include <bl31/ehf.h>
#include <context.h>
#include <lib/utils_def.h>
#include <lib/cpus/cpu_ops.h>

/* need enough space in crash buffer to save 8 registers */
#define CPU_DATA_CRASH_BUF_BYTES	64
#if ENABLE_RUNTIME_INSTRUMENTATION
/* Temporary space to store PMF timestamps from assembly code */
#define CPU_DATA_PMF_TS_COUNT		1
#define CPU_DATA_PMF_TS0_IDX		0
#endif

#ifdef __aarch64__
#define CPU_DATA_CPU_CONTEXT_SIZE	(CPU_CONTEXT_NUM * CPU_WORD_SIZE)
#else /* __aarch64__ */
#define CPU_DATA_CPU_CONTEXT_SIZE	0
#endif /* __aarch64__ */
#define CPU_DATA_WARMBOOT_EP_INFO_SIZE	CPU_WORD_SIZE
#define CPU_DATA_WARMBOOT_EP_INFO_ALIGN	CPU_WORD_SIZE
#define CPU_DATA_CPU_OPS_PTR_SIZE	CPU_WORD_SIZE
#define CPU_DATA_CPU_OPS_PTR_ALIGN	CPU_WORD_SIZE
#define CPU_DATA_PSCI_SVC_CPU_DATA_SIZE 12
#define CPU_DATA_PSCI_SVC_CPU_DATA_ALIGN CPU_WORD_SIZE
#if ENABLE_PAUTH
/* uint64_t apiakey[2] */
#define CPU_DATA_APIAKEY_SIZE		16
/* uint64_t alignement */
#define CPU_DATA_APIAKEY_ALIGN		8
#else /* ENABLE_PAUTH */
#define CPU_DATA_APIAKEY_SIZE		0
#define CPU_DATA_APIAKEY_ALIGN		1
#endif /* ENABLE_PAUTH */
#if CRASH_REPORTING
#define CPU_DATA_CRASH_BUF_SIZE		((CPU_DATA_CRASH_BUF_BYTES >> 3) * CPU_WORD_SIZE)
#define CPU_DATA_CRASH_BUF_ALIGN	CPU_WORD_SIZE
#else /* CRASH_REPORTING */
#define CPU_DATA_CRASH_BUF_SIZE		0
#define CPU_DATA_CRASH_BUF_ALIGN	1
#endif /* CRASH_REPORTING */
#if ENABLE_RUNTIME_INSTRUMENTATION
#define CPU_DATA_CPU_DATA_PMF_TS_SIZE	(CPU_DATA_PMF_TS_COUNT * 8)
/* uint64_t alignement */
#define CPU_DATA_CPU_DATA_PMF_TS_ALIGN	8
#else /* ENABLE_RUNTIME_INSTRUMENTATION */
#define CPU_DATA_CPU_DATA_PMF_TS_SIZE	0
#define CPU_DATA_CPU_DATA_PMF_TS_ALIGN	1
#endif /* ENABLE_RUNTIME_INSTRUMENTATION */
#ifdef PLAT_PCPU_DATA_SIZE
#define CPU_DATA_PLATFORM_CPU_DATA_SIZE	PLAT_PCPU_DATA_SIZE
#define CPU_DATA_PLATFORM_CPU_DATA_ALIGN 1
#else /* PLAT_PCPU_DATA_SIZE */
#define CPU_DATA_PLATFORM_CPU_DATA_SIZE	0
#define CPU_DATA_PLATFORM_CPU_DATA_ALIGN 1
#endif /* PLAT_PCPU_DATA_SIZE */
#if EL3_EXCEPTION_HANDLING
/* buffer space for EHF data is sizeof(pe_exc_data_t) */
#define CPU_DATA_EHF_DATA_SIZE		8
/* hardcoded to 64 bit alignment */
#define CPU_DATA_EHF_DATA_ALIGN		8
#else /* EL3_EXCEPTION_HANDLING */
#define CPU_DATA_EHF_DATA_SIZE		0
#define CPU_DATA_EHF_DATA_ALIGN		1
#endif
/* cpu_data size is the data size rounded up to the platform cache line size */
#define CPU_DATA_SIZE_ALIGN		CACHE_WRITEBACK_GRANULE

#define CPU_DATA_CPU_CONTEXT		0
#define CPU_DATA_WARMBOOT_EP_INFO	ROUND_UP_2EVAL((CPU_DATA_CPU_CONTEXT + CPU_DATA_CPU_CONTEXT_SIZE), CPU_DATA_CPU_OPS_PTR_ALIGN)
#define CPU_DATA_CPU_OPS_PTR		ROUND_UP_2EVAL((CPU_DATA_WARMBOOT_EP_INFO + CPU_DATA_WARMBOOT_EP_INFO_SIZE), CPU_DATA_CPU_OPS_PTR_ALIGN)
#define CPU_DATA_PSCI_SVC_CPU_DATA	ROUND_UP_2EVAL((CPU_DATA_CPU_OPS_PTR + CPU_DATA_CPU_OPS_PTR_SIZE), CPU_DATA_PSCI_SVC_CPU_DATA_ALIGN)
#define CPU_DATA_APIAKEY		ROUND_UP_2EVAL((CPU_DATA_PSCI_SVC_CPU_DATA + CPU_DATA_PSCI_SVC_CPU_DATA_SIZE), CPU_DATA_APIAKEY_ALIGN)
#define CPU_DATA_CRASH_BUF		ROUND_UP_2EVAL((CPU_DATA_APIAKEY + CPU_DATA_APIAKEY_SIZE), CPU_DATA_CRASH_BUF_ALIGN)
#define CPU_DATA_CPU_DATA_PMF_TS	ROUND_UP_2EVAL((CPU_DATA_CRASH_BUF + CPU_DATA_CRASH_BUF_SIZE), CPU_DATA_CPU_DATA_PMF_TS_ALIGN)
#define CPU_DATA_PLATFORM_CPU_DATA	ROUND_UP_2EVAL((CPU_DATA_CPU_DATA_PMF_TS + CPU_DATA_CPU_DATA_PMF_TS_SIZE), CPU_DATA_PLATFORM_CPU_DATA_ALIGN)
#define CPU_DATA_EHF_DATA		ROUND_UP_2EVAL((CPU_DATA_PLATFORM_CPU_DATA + CPU_DATA_PLATFORM_CPU_DATA_SIZE), CPU_DATA_EHF_DATA_ALIGN)
#define CPU_DATA_SIZE			ROUND_UP_2EVAL((CPU_DATA_EHF_DATA + CPU_DATA_EHF_DATA_SIZE), CPU_DATA_SIZE_ALIGN)

#ifndef __ASSEMBLER__

#include <assert.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <lib/cassert.h>
#include <lib/per_cpu/per_cpu.h>
#include <lib/psci/psci.h>

#include <platform_def.h>

/*******************************************************************************
 * Function & variable prototypes
 ******************************************************************************/

/*******************************************************************************
 * Cache of frequently used per-cpu data:
 *   Pointers to non-secure, realm, and secure security state contexts
 *   Address of the crash stack
 * It is aligned to the cache line boundary to allow efficient concurrent
 * manipulation of these pointers on different cpus
 *
 * The data structure and the _cpu_data accessors should not be used directly
 * by components that have per-cpu members. The member access macros should be
 * used for this.
 ******************************************************************************/
typedef struct cpu_data {
#ifdef __aarch64__
	void *cpu_context[CPU_CONTEXT_NUM];
#endif /* __aarch64__ */
	entry_point_info_t *warmboot_ep_info;
	struct cpu_ops *cpu_ops_ptr;
	struct psci_cpu_data psci_svc_cpu_data;
#if ENABLE_PAUTH
	uint64_t apiakey[2];
#endif
#if CRASH_REPORTING
	u_register_t crash_buf[CPU_DATA_CRASH_BUF_BYTES >> 3];
#endif
#if ENABLE_RUNTIME_INSTRUMENTATION
	uint64_t cpu_data_pmf_ts[CPU_DATA_PMF_TS_COUNT];
#endif
#if PLAT_PCPU_DATA_SIZE
	uint8_t platform_cpu_data[PLAT_PCPU_DATA_SIZE];
#endif
#if EL3_EXCEPTION_HANDLING
	pe_exc_data_t ehf_data;
#endif
} __aligned(CACHE_WRITEBACK_GRANULE) cpu_data_t;

PER_CPU_DECLARE(cpu_data_t, percpu_data);

#define CPU_DATA_ASSERT_OFFSET(left, right) \
	CASSERT(CPU_DATA_ ## left == __builtin_offsetof \
		(cpu_data_t, right), \
		assert_cpu_data_ ## right ## _mismatch)

/* verify assembler offsets match data structures */
CPU_DATA_ASSERT_OFFSET(WARMBOOT_EP_INFO, warmboot_ep_info);
CPU_DATA_ASSERT_OFFSET(CPU_OPS_PTR, cpu_ops_ptr);
CPU_DATA_ASSERT_OFFSET(PSCI_SVC_CPU_DATA, psci_svc_cpu_data);
#if ENABLE_PAUTH
CPU_DATA_ASSERT_OFFSET(APIAKEY, apiakey);
#endif
#if CRASH_REPORTING
CPU_DATA_ASSERT_OFFSET(CRASH_BUF, crash_buf);
#endif
#if ENABLE_RUNTIME_INSTRUMENTATION
CPU_DATA_ASSERT_OFFSET(CPU_DATA_PMF_TS, cpu_data_pmf_ts);
#endif
#if PLAT_PCPU_DATA_SIZE
CPU_DATA_ASSERT_OFFSET(PLATFORM_CPU_DATA, platform_cpu_data);
#endif
#if EL3_EXCEPTION_HANDLING
CPU_DATA_ASSERT_OFFSET(EHF_DATA, ehf_data);
#endif

CASSERT(CPU_DATA_SIZE == sizeof(cpu_data_t),
		assert_cpu_data_size_mismatch);

#ifndef __aarch64__
cpu_data_t *_cpu_data(void);
#endif

/**************************************************************************
 * APIs for initialising and accessing per-cpu data
 *************************************************************************/

void cpu_data_init_cpu_ops(void);

#define get_cpu_data(_m)			PER_CPU_CUR(percpu_data)->_m
#define set_cpu_data(_m, _v)			PER_CPU_CUR(percpu_data)->_m = (_v)
#define get_cpu_data_by_index(_ix, _m)		PER_CPU_BY_INDEX(percpu_data, _ix)->_m
#define set_cpu_data_by_index(_ix, _m, _v)	PER_CPU_BY_INDEX(percpu_data, _ix)->_m = (_v)
/* ((cpu_data_t *)0)->_m is a dummy to get the sizeof the struct member _m */
#define flush_cpu_data(_m)	   flush_dcache_range((uintptr_t)	  \
						&(PER_CPU_CUR(percpu_data)->_m), \
						sizeof(((cpu_data_t *)0)->_m))
#define inv_cpu_data(_m)	   inv_dcache_range((uintptr_t)	  	  \
						&(PER_CPU_CUR(percpu_data)->_m), \
						sizeof(((cpu_data_t *)0)->_m))
#define flush_cpu_data_by_index(_ix, _m)	\
				   flush_dcache_range((uintptr_t)	  \
					 &(PER_CPU_BY_INDEX(percpu_data, _ix)->_m),  \
						sizeof(((cpu_data_t *)0)->_m))


#endif /* __ASSEMBLER__ */
#endif /* CPU_DATA_H */
