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

#ifndef __PMF_H__
#define __PMF_H__

#include <cassert.h>
#include <pmf_helpers.h>

/*
 * Constants used for/by PMF services.
 */
#define PMF_ARM_TIF_IMPL_ID	(0x41000000)
#define PMF_TID_SHIFT		0
#define PMF_TID_MASK		(0xFF << PMF_TID_SHIFT)
#define PMF_SVC_ID_SHIFT	10
#define PMF_SVC_ID_MASK		(0x3F << PMF_SVC_ID_SHIFT)
#define PMF_IMPL_ID_SHIFT	24
#define PMF_IMPL_ID_MASK	(0xFF << PMF_IMPL_ID_SHIFT)

/*
 * Flags passed to PMF_REGISTER_SERVICE
 */
#define PMF_STORE_ENABLE	(1 << 0)
#define PMF_DUMP_ENABLE		(1 << 1)

/*
 * Flags passed to PMF_GET_TIMESTAMP_XXX
 * and PMF_CAPTURE_TIMESTAMP
 */
#define PMF_CACHE_MAINT		(1 << 0)
#define PMF_NO_CACHE_MAINT	0

/*
 * Defines for PMF SMC function ids.
 */
#define PMF_SMC_GET_TIMESTAMP_32	0x82000010
#define PMF_SMC_GET_TIMESTAMP_64	0xC2000010
#define PMF_NUM_SMC_CALLS		2

/*
 * The macros below are used to identify
 * PMF calls from the SMC function ID.
 */
#define PMF_FID_MASK	0xffe0u
#define PMF_FID_VALUE	0u
#define is_pmf_fid(_fid)	(((_fid) & PMF_FID_MASK) == PMF_FID_VALUE)

/* Following are the supported PMF service IDs */
#define PMF_PSCI_STAT_SVC_ID	0

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
		if ((_flags) & PMF_CACHE_MAINT)				\
			pmf_capture_timestamp_with_cache_maint_ ## _name((_tid), ts);\
		else							\
			pmf_capture_timestamp_ ## _name((_tid), ts);	\
	} while (0)

#define PMF_CAPTURE_AND_GET_TIMESTAMP(_name, _tid, _flags, _tsval)	\
	do {								\
		(_tsval) = read_cntpct_el0();				\
		CASSERT(sizeof(_tsval) == sizeof(unsigned long long), invalid_tsval_size);\
		if ((_flags) & PMF_CACHE_MAINT)				\
			pmf_capture_timestamp_with_cache_maint_ ## _name((_tid), (_tsval));\
		else							\
			pmf_capture_timestamp_ ## _name((_tid), (_tsval));\
	} while (0)

#define PMF_WRITE_TIMESTAMP(_name, _tid, _flags, _wrval)		\
	do {								\
		CASSERT(sizeof(_wrval) == sizeof(unsigned long long), invalid_wrval_size);\
		if ((_flags) & PMF_CACHE_MAINT)				\
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

/*******************************************************************************
 * Function & variable prototypes
 ******************************************************************************/
/* PMF common functions */
int pmf_get_timestamp_smc(unsigned int tid,
		u_register_t mpidr,
		unsigned int flags,
		unsigned long long *ts);
int pmf_setup(void);
uintptr_t pmf_smc_handler(unsigned int smc_fid,
		u_register_t x1,
		u_register_t x2,
		u_register_t x3,
		u_register_t x4,
		void *cookie,
		void *handle,
		u_register_t flags);

#endif /* __PMF_H__ */
