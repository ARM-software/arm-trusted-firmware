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

#ifndef __PMF_HELPERS_H__
#define __PMF_HELPERS_H__

#include <arch_helpers.h>
#include <assert.h>
#include <platform.h>
#include <pmf.h>
#include <stdint.h>
#include <stddef.h>

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

/*
 * Convenience macro to allocate memory for a PMF service.
 */
#define PMF_ALLOCATE_TIMESTAMP_MEMORY(_name, _total_id)		\
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
 */
#define PMF_DEFINE_CAPTURE_TIMESTAMP(_name, _flags)			\
	void pmf_capture_timestamp_ ## _name(				\
			unsigned int tid,				\
			unsigned long long ts)				\
	{								\
		CASSERT(_flags, select_proper_config);			\
		PMF_VALIDATE_TID(_name, tid);				\
		uintptr_t base_addr = (uintptr_t) pmf_ts_mem_ ## _name;	\
		if ((_flags) & PMF_STORE_ENABLE)			\
			__pmf_store_timestamp(base_addr, tid, ts);	\
		if ((_flags) & PMF_DUMP_ENABLE)				\
			__pmf_dump_timestamp(tid, ts);			\
	}								\
	void pmf_capture_timestamp_with_cache_maint_ ## _name(		\
			unsigned int tid,				\
			unsigned long long ts)				\
	{								\
		CASSERT(_flags, select_proper_config);			\
		PMF_VALIDATE_TID(_name, tid);				\
		uintptr_t base_addr = (uintptr_t) pmf_ts_mem_ ## _name;	\
		if ((_flags) & PMF_STORE_ENABLE)			\
			__pmf_store_timestamp_with_cache_maint(base_addr, tid, ts);\
		if ((_flags) & PMF_DUMP_ENABLE)				\
			__pmf_dump_timestamp(tid, ts);			\
	}

/*
 * Convenience macros for retrieving time-stamp.
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
#endif /* __PMF_HELPERS_H__ */
