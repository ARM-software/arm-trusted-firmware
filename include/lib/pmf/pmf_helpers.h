/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PMF_HELPERS_H
#define PMF_HELPERS_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <plat/common/platform.h>

/*
 * Prototype for PMF service functions.
 */
typedef int (*pmf_svc_init_t)(void);
typedef unsigned long long (*pmf_svc_get_ts_t)(unsigned int tid,
		 u_register_t mpidr,
		 unsigned int flags);

/*
 * This is the definition of PMF service desc.
 */
typedef struct pmf_svc_desc {
	/* Structure version information */
	param_header_t h;

	/* Name of the PMF service */
	const char *name;

	/* PMF service config: Implementer id, Service id and total id*/
	unsigned int svc_config;

	/* PMF service initialization handler */
	pmf_svc_init_t init;

	/* PMF service time-stamp retrieval handler */
	pmf_svc_get_ts_t get_ts;
} pmf_svc_desc_t;

#if ENABLE_PMF
/*
 * Convenience macros for capturing time-stamp.
 */
#define PMF_DECLARE_CAPTURE_TIMESTAMP(_name)			\
	void pmf_capture_timestamp_with_cache_maint_ ## _name(	\
				unsigned int tid,		\
				unsigned long long ts);		\
	void pmf_capture_timestamp_ ## _name(			\
				unsigned int tid,		\
				unsigned long long ts);

#define PMF_CAPTURE_TIMESTAMP(_name, _tid, _flags)			\
	do {								\
		unsigned long long ts = read_cntpct_el0();		\
		if (((_flags) & PMF_CACHE_MAINT) != 0U)			\
			pmf_capture_timestamp_with_cache_maint_ ## _name((_tid), ts);\
		else							\
			pmf_capture_timestamp_ ## _name((_tid), ts);	\
	} while (0)

#define PMF_CAPTURE_AND_GET_TIMESTAMP(_name, _tid, _flags, _tsval)	\
	do {								\
		(_tsval) = read_cntpct_el0();				\
		CASSERT(sizeof(_tsval) == sizeof(unsigned long long), invalid_tsval_size);\
		if (((_flags) & PMF_CACHE_MAINT) != 0U)			\
			pmf_capture_timestamp_with_cache_maint_ ## _name((_tid), (_tsval));\
		else							\
			pmf_capture_timestamp_ ## _name((_tid), (_tsval));\
	} while (0)

#define PMF_WRITE_TIMESTAMP(_name, _tid, _flags, _wrval)		\
	do {								\
		CASSERT(sizeof(_wrval) == sizeof(unsigned long long), invalid_wrval_size);\
		if (((_flags) & PMF_CACHE_MAINT) != 0U)			\
			pmf_capture_timestamp_with_cache_maint_ ## _name((_tid), (_wrval));\
		else							\
			pmf_capture_timestamp_ ## _name((_tid), (_wrval));\
	} while (0)

/*
 * Convenience macros for retrieving time-stamp.
 */
#define PMF_DECLARE_GET_TIMESTAMP(_name)			\
	unsigned long long pmf_get_timestamp_by_index_ ## _name(\
		unsigned int tid,				\
		unsigned int cpuid,				\
		unsigned int flags);				\
	unsigned long long pmf_get_timestamp_by_mpidr_ ## _name(\
		unsigned int tid,				\
		u_register_t mpidr,				\
		unsigned int flags);

#define PMF_GET_TIMESTAMP_BY_MPIDR(_name, _tid, _mpidr, _flags, _tsval)\
	_tsval = pmf_get_timestamp_by_mpidr_ ## _name(_tid, _mpidr, _flags)

#define PMF_GET_TIMESTAMP_BY_INDEX(_name, _tid, _cpuid, _flags, _tsval)\
	_tsval = pmf_get_timestamp_by_index_ ## _name(_tid, _cpuid, _flags)

/* Convenience macros to register a PMF service.*/
/*
 * This macro is used to register a PMF Service. It allocates PMF memory
 * and defines default service-specific PMF functions.
 */
#define PMF_REGISTER_SERVICE(_name, _svcid, _totalid, _flags)	\
	PMF_ALLOCATE_TIMESTAMP_MEMORY(_name, _totalid)		\
	PMF_DEFINE_CAPTURE_TIMESTAMP(_name, _flags)		\
	PMF_DEFINE_GET_TIMESTAMP(_name)

/*
 * This macro is used to register a PMF service, including an
 * SMC interface to that service.
 */
#define PMF_REGISTER_SERVICE_SMC(_name, _svcid, _totalid, _flags)\
	PMF_REGISTER_SERVICE(_name, _svcid, _totalid, _flags)	\
	PMF_DEFINE_SERVICE_DESC(_name, PMF_ARM_TIF_IMPL_ID,	\
			_svcid, _totalid, NULL,			\
			pmf_get_timestamp_by_mpidr_ ## _name)

/*
 * This macro is used to register a PMF service that has an SMC interface
 * but provides its own service-specific PMF functions.
 */
#define PMF_REGISTER_SERVICE_SMC_OWN(_name, _implid, _svcid, _totalid,	\
		 _init, _getts)						\
	PMF_DEFINE_SERVICE_DESC(_name, _implid, _svcid, _totalid,	\
		 _init, _getts)

#else

#define PMF_REGISTER_SERVICE(_name, _svcid, _totalid, _flags)
#define PMF_REGISTER_SERVICE_SMC(_name, _svcid, _totalid, _flags)
#define PMF_REGISTER_SERVICE_SMC_OWN(_name, _implid, _svcid, _totalid,	\
				_init, _getts)
#define PMF_DECLARE_CAPTURE_TIMESTAMP(_name)
#define PMF_DECLARE_GET_TIMESTAMP(_name)
#define PMF_CAPTURE_TIMESTAMP(_name, _tid, _flags)
#define PMF_GET_TIMESTAMP_BY_MPIDR(_name, _tid, _mpidr, _flags, _tsval)
#define PMF_GET_TIMESTAMP_BY_INDEX(_name, _tid, _cpuid, _flags, _tsval)

#endif /* ENABLE_PMF */

/*
 * Convenience macro to allocate memory for a PMF service.
 *
 * The extern declaration is there to satisfy MISRA C-2012 rule 8.4.
 */
#define PMF_ALLOCATE_TIMESTAMP_MEMORY(_name, _total_id)		\
	extern unsigned long long pmf_ts_mem_ ## _name[_total_id];	\
	unsigned long long pmf_ts_mem_ ## _name[_total_id]	\
	__aligned(CACHE_WRITEBACK_GRANULE)			\
	__section("pmf_timestamp_array")			\
	__used;

/*
 * Convenience macro to validate tid index for the given TS array.
 */
#define PMF_VALIDATE_TID(_name, _tid)	\
	assert((_tid & PMF_TID_MASK) < (ARRAY_SIZE(pmf_ts_mem_ ## _name)))

/*
 * Convenience macros for capturing time-stamp.
 *
 * The extern declaration is there to satisfy MISRA C-2012 rule 8.4.
 */
#define PMF_DEFINE_CAPTURE_TIMESTAMP(_name, _flags)			\
	void pmf_capture_timestamp_ ## _name(				\
			unsigned int tid,				\
			unsigned long long ts)				\
	{								\
		CASSERT(_flags, select_proper_config);			\
		PMF_VALIDATE_TID(_name, tid);				\
		uintptr_t base_addr = (uintptr_t) pmf_ts_mem_ ## _name;	\
		if (((_flags) & PMF_STORE_ENABLE) != 0)			\
			__pmf_store_timestamp(base_addr, tid, ts);	\
		if (((_flags) & PMF_DUMP_ENABLE) != 0)			\
			__pmf_dump_timestamp(tid, ts);			\
	}								\
	void pmf_capture_timestamp_with_cache_maint_ ## _name(		\
			unsigned int tid,				\
			unsigned long long ts)				\
	{								\
		CASSERT(_flags, select_proper_config);			\
		PMF_VALIDATE_TID(_name, tid);				\
		uintptr_t base_addr = (uintptr_t) pmf_ts_mem_ ## _name;	\
		if (((_flags) & PMF_STORE_ENABLE) != 0)			\
			__pmf_store_timestamp_with_cache_maint(base_addr, tid, ts);\
		if (((_flags) & PMF_DUMP_ENABLE) != 0)			\
			__pmf_dump_timestamp(tid, ts);			\
	}

/*
 * Convenience macros for retrieving time-stamp.
 *
 * The extern declaration is there to satisfy MISRA C-2012 rule 8.4.
 */
#define PMF_DEFINE_GET_TIMESTAMP(_name)					\
	unsigned long long pmf_get_timestamp_by_index_ ## _name(	\
		unsigned int tid, unsigned int cpuid, unsigned int flags)\
	{								\
		PMF_VALIDATE_TID(_name, tid);				\
		uintptr_t base_addr = (uintptr_t) pmf_ts_mem_ ## _name;	\
		return __pmf_get_timestamp(base_addr, tid, cpuid, flags);\
	}								\
	unsigned long long pmf_get_timestamp_by_mpidr_ ## _name(	\
		unsigned int tid, u_register_t mpidr, unsigned int flags)\
	{								\
		PMF_VALIDATE_TID(_name, tid);				\
		uintptr_t base_addr = (uintptr_t) pmf_ts_mem_ ## _name;	\
		return __pmf_get_timestamp(base_addr, tid,		\
			plat_core_pos_by_mpidr(mpidr), flags);		\
	}

/*
 * Convenience macro to register a PMF service.
 * This is needed for services that require SMC handling.
 */
#define PMF_DEFINE_SERVICE_DESC(_name, _implid, _svcid, _totalid,	\
		_init, _getts_by_mpidr) 				\
	static const pmf_svc_desc_t __pmf_desc_ ## _name 		\
	__section("pmf_svc_descs") __used = {		 		\
		.h.type = PARAM_EP, 					\
		.h.version = VERSION_1, 				\
		.h.size = sizeof(pmf_svc_desc_t),			\
		.h.attr = 0,						\
		.name = #_name, 					\
		.svc_config = ((((_implid) << PMF_IMPL_ID_SHIFT) &	\
						PMF_IMPL_ID_MASK) |	\
				(((_svcid) << PMF_SVC_ID_SHIFT) &	\
						PMF_SVC_ID_MASK) |	\
				(((_totalid) << PMF_TID_SHIFT) &	\
						PMF_TID_MASK)),		\
		.init = _init,						\
		.get_ts = _getts_by_mpidr				\
	};

/* PMF internal functions */
void __pmf_dump_timestamp(unsigned int tid, unsigned long long ts);
void __pmf_store_timestamp(uintptr_t base_addr,
		unsigned int tid,
		unsigned long long ts);
void __pmf_store_timestamp_with_cache_maint(uintptr_t base_addr,
		unsigned int tid,
		unsigned long long ts);
unsigned long long __pmf_get_timestamp(uintptr_t base_addr,
		unsigned int tid,
		unsigned int cpuid,
		unsigned int flags);
#endif /* PMF_HELPERS_H */
