/*
 * Copyright (c) 2014-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CPU_DATA_H
#define CPU_DATA_H

#include <platform_def.h>	/* CACHE_WRITEBACK_GRANULE required */

#include <bl31/ehf.h>

/* Size of psci_cpu_data structure */
#define PSCI_CPU_DATA_SIZE		12

#ifdef __aarch64__

/* 8-bytes aligned size of psci_cpu_data structure */
#define PSCI_CPU_DATA_SIZE_ALIGNED	((PSCI_CPU_DATA_SIZE + 7) & ~7)

#if ENABLE_RME
/* Size of cpu_context array */
#define CPU_DATA_CONTEXT_NUM		3
/* Offset of cpu_ops_ptr, size 8 bytes */
#define CPU_DATA_CPU_OPS_PTR		0x18
#else /* ENABLE_RME */
#define CPU_DATA_CONTEXT_NUM		2
#define CPU_DATA_CPU_OPS_PTR		0x10
#endif /* ENABLE_RME */

#if ENABLE_PAUTH
/* 8-bytes aligned offset of apiakey[2], size 16 bytes */
#define	CPU_DATA_APIAKEY_OFFSET		(0x8 + PSCI_CPU_DATA_SIZE_ALIGNED \
					     + CPU_DATA_CPU_OPS_PTR)
#define CPU_DATA_CRASH_BUF_OFFSET	(0x10 + CPU_DATA_APIAKEY_OFFSET)
#else /* ENABLE_PAUTH */
#define CPU_DATA_CRASH_BUF_OFFSET	(0x8 + PSCI_CPU_DATA_SIZE_ALIGNED \
					     + CPU_DATA_CPU_OPS_PTR)
#endif /* ENABLE_PAUTH */

/* need enough space in crash buffer to save 8 registers */
#define CPU_DATA_CRASH_BUF_SIZE		64

#else	/* !__aarch64__ */

#if CRASH_REPORTING
#error "Crash reporting is not supported in AArch32"
#endif
#define CPU_DATA_CPU_OPS_PTR		0x0
#define CPU_DATA_CRASH_BUF_OFFSET	(0x4 + PSCI_CPU_DATA_SIZE)

#endif	/* __aarch64__ */

#if CRASH_REPORTING
#define CPU_DATA_CRASH_BUF_END		(CPU_DATA_CRASH_BUF_OFFSET + \
						CPU_DATA_CRASH_BUF_SIZE)
#else
#define CPU_DATA_CRASH_BUF_END		CPU_DATA_CRASH_BUF_OFFSET
#endif

/* buffer space for EHF data is sizeof(pe_exc_data_t) */
#define CPU_DATA_EHF_DATA_SIZE		8
#define CPU_DATA_EHF_DATA_BUF_OFFSET	CPU_DATA_CRASH_BUF_END

#if defined(IMAGE_BL31) && EL3_EXCEPTION_HANDLING
#define CPU_DATA_EHF_DATA_BUF_END	(CPU_DATA_EHF_DATA_BUF_OFFSET + \
						CPU_DATA_EHF_DATA_SIZE)
#else
#define CPU_DATA_EHF_DATA_BUF_END	CPU_DATA_EHF_DATA_BUF_OFFSET
#endif	/* EL3_EXCEPTION_HANDLING */

/* cpu_data size is the data size rounded up to the platform cache line size */
#define CPU_DATA_SIZE			(((CPU_DATA_EHF_DATA_BUF_END + \
					CACHE_WRITEBACK_GRANULE - 1) / \
						CACHE_WRITEBACK_GRANULE) * \
							CACHE_WRITEBACK_GRANULE)

#if ENABLE_RUNTIME_INSTRUMENTATION
/* Temporary space to store PMF timestamps from assembly code */
#define CPU_DATA_PMF_TS_COUNT		1
#define CPU_DATA_PMF_TS0_OFFSET		CPU_DATA_EHF_DATA_BUF_END
#define CPU_DATA_PMF_TS0_IDX		0
#endif

#ifndef __ASSEMBLER__

#include <assert.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <lib/cassert.h>
#include <lib/psci/psci.h>

#include <platform_def.h>

/* Offsets for the cpu_data structure */
#define CPU_DATA_PSCI_LOCK_OFFSET	__builtin_offsetof\
		(cpu_data_t, psci_svc_cpu_data.pcpu_bakery_info)

#if PLAT_PCPU_DATA_SIZE
#define CPU_DATA_PLAT_PCPU_OFFSET	__builtin_offsetof\
		(cpu_data_t, platform_cpu_data)
#endif

typedef enum context_pas {
	CPU_CONTEXT_SECURE = 0,
	CPU_CONTEXT_NS,
#if ENABLE_RME
	CPU_CONTEXT_REALM,
#endif
	CPU_CONTEXT_NUM
} context_pas_t;

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
	void *cpu_context[CPU_DATA_CONTEXT_NUM];
#endif /* __aarch64__ */
	uintptr_t cpu_ops_ptr;
	struct psci_cpu_data psci_svc_cpu_data;
#if ENABLE_PAUTH
	uint64_t apiakey[2];
#endif
#if CRASH_REPORTING
	u_register_t crash_buf[CPU_DATA_CRASH_BUF_SIZE >> 3];
#endif
#if ENABLE_RUNTIME_INSTRUMENTATION
	uint64_t cpu_data_pmf_ts[CPU_DATA_PMF_TS_COUNT];
#endif
#if PLAT_PCPU_DATA_SIZE
	uint8_t platform_cpu_data[PLAT_PCPU_DATA_SIZE];
#endif
#if defined(IMAGE_BL31) && EL3_EXCEPTION_HANDLING
	pe_exc_data_t ehf_data;
#endif
} __aligned(CACHE_WRITEBACK_GRANULE) cpu_data_t;

extern cpu_data_t percpu_data[PLATFORM_CORE_COUNT];

#ifdef __aarch64__
CASSERT(CPU_DATA_CONTEXT_NUM == CPU_CONTEXT_NUM,
		assert_cpu_data_context_num_mismatch);
#endif

#if ENABLE_PAUTH
CASSERT(CPU_DATA_APIAKEY_OFFSET == __builtin_offsetof
	(cpu_data_t, apiakey),
	assert_cpu_data_pauth_stack_offset_mismatch);
#endif

#if CRASH_REPORTING
/* verify assembler offsets match data structures */
CASSERT(CPU_DATA_CRASH_BUF_OFFSET == __builtin_offsetof
	(cpu_data_t, crash_buf),
	assert_cpu_data_crash_stack_offset_mismatch);
#endif

#if defined(IMAGE_BL31) && EL3_EXCEPTION_HANDLING
CASSERT(CPU_DATA_EHF_DATA_BUF_OFFSET == __builtin_offsetof
	(cpu_data_t, ehf_data),
	assert_cpu_data_ehf_stack_offset_mismatch);
#endif

CASSERT(CPU_DATA_SIZE == sizeof(cpu_data_t),
		assert_cpu_data_size_mismatch);

CASSERT(CPU_DATA_CPU_OPS_PTR == __builtin_offsetof
		(cpu_data_t, cpu_ops_ptr),
		assert_cpu_data_cpu_ops_ptr_offset_mismatch);

#if ENABLE_RUNTIME_INSTRUMENTATION
CASSERT(CPU_DATA_PMF_TS0_OFFSET == __builtin_offsetof
		(cpu_data_t, cpu_data_pmf_ts[0]),
		assert_cpu_data_pmf_ts0_offset_mismatch);
#endif

struct cpu_data *_cpu_data_by_index(uint32_t cpu_index);

#ifdef __aarch64__
/* Return the cpu_data structure for the current CPU. */
static inline struct cpu_data *_cpu_data(void)
{
	return (cpu_data_t *)read_tpidr_el3();
}
#else
struct cpu_data *_cpu_data(void);
#endif

/*
 * Returns the index of the cpu_context array for the given security state.
 * All accesses to cpu_context should be through this helper to make sure
 * an access is not out-of-bounds. The function assumes security_state is
 * valid.
 */
static inline context_pas_t get_cpu_context_index(uint32_t security_state)
{
	if (security_state == SECURE) {
		return CPU_CONTEXT_SECURE;
	} else {
#if ENABLE_RME
		if (security_state == NON_SECURE) {
			return CPU_CONTEXT_NS;
		} else {
			assert(security_state == REALM);
			return CPU_CONTEXT_REALM;
		}
#else
		assert(security_state == NON_SECURE);
		return CPU_CONTEXT_NS;
#endif
	}
}

/**************************************************************************
 * APIs for initialising and accessing per-cpu data
 *************************************************************************/

void init_cpu_data_ptr(void);
void init_cpu_ops(void);

#define get_cpu_data(_m)		   _cpu_data()->_m
#define set_cpu_data(_m, _v)		   _cpu_data()->_m = (_v)
#define get_cpu_data_by_index(_ix, _m)	   _cpu_data_by_index(_ix)->_m
#define set_cpu_data_by_index(_ix, _m, _v) _cpu_data_by_index(_ix)->_m = (_v)
/* ((cpu_data_t *)0)->_m is a dummy to get the sizeof the struct member _m */
#define flush_cpu_data(_m)	   flush_dcache_range((uintptr_t)	  \
						&(_cpu_data()->_m), \
						sizeof(((cpu_data_t *)0)->_m))
#define inv_cpu_data(_m)	   inv_dcache_range((uintptr_t)	  	  \
						&(_cpu_data()->_m), \
						sizeof(((cpu_data_t *)0)->_m))
#define flush_cpu_data_by_index(_ix, _m)	\
				   flush_dcache_range((uintptr_t)	  \
					 &(_cpu_data_by_index(_ix)->_m),  \
						sizeof(((cpu_data_t *)0)->_m))


#endif /* __ASSEMBLER__ */
#endif /* CPU_DATA_H */
