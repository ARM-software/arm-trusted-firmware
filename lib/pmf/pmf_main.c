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
#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <errno.h>
#include <platform.h>
#include <pmf.h>
#include <string.h>

/*******************************************************************************
 * The 'pmf_svc_descs' array holds the PMF service descriptors exported by
 * services by placing them in the 'pmf_svc_descs' linker section.
 * The 'pmf_svc_descs_indices' array holds the index of a descriptor in the
 * 'pmf_svc_descs' array. The TIF[15:10] bits in the time-stamp id are used
 * to get an index into the 'pmf_svc_descs_indices' array. This gives the
 * index of the descriptor in the 'pmf_svc_descs' array  which contains the
 * service function pointers.
 ******************************************************************************/
extern uintptr_t __PMF_SVC_DESCS_START__;
extern uintptr_t __PMF_SVC_DESCS_END__;
#define PMF_SVC_DESCS_START		((uintptr_t)(&__PMF_SVC_DESCS_START__))
#define PMF_SVC_DESCS_END		((uintptr_t)(&__PMF_SVC_DESCS_END__))
extern void *__PERCPU_TIMESTAMP_SIZE__;
#define PMF_PERCPU_TIMESTAMP_SIZE	((uintptr_t)&__PERCPU_TIMESTAMP_SIZE__)
extern uintptr_t __PMF_TIMESTAMP_START__;
#define PMF_TIMESTAMP_ARRAY_START	((uintptr_t)&__PMF_TIMESTAMP_START__)
extern uintptr_t __PMF_TIMESTAMP_END__;
#define PMF_TIMESTAMP_ARRAY_END		((uintptr_t)&__PMF_TIMESTAMP_END__)

#define PMF_SVC_DESCS_MAX		10

/*
 * This is used to traverse through registered PMF services.
 */
static pmf_svc_desc_t *pmf_svc_descs;

/*
 * This array is used to store registered PMF services in sorted order.
 */
static int pmf_svc_descs_indices[PMF_SVC_DESCS_MAX];

/*
 * This is used to track total number of successfully registered PMF services.
 */
static int pmf_num_services;

/*
 * This is the main PMF function that initialize registered
 * PMF services and also sort them in ascending order.
 */
int pmf_setup(void)
{
	int rc, ii, jj = 0;
	int pmf_svc_descs_num, temp_val;

	/* If no PMF services are registered then simply bail out */
	pmf_svc_descs_num = (PMF_SVC_DESCS_END - PMF_SVC_DESCS_START)/
				 sizeof(pmf_svc_desc_t);
	if (pmf_svc_descs_num == 0)
		return 0;

	assert(pmf_svc_descs_num < PMF_SVC_DESCS_MAX);

	pmf_svc_descs = (pmf_svc_desc_t *) PMF_SVC_DESCS_START;
	for (ii = 0; ii < pmf_svc_descs_num; ii++) {

		assert(pmf_svc_descs[ii].get_ts);

		/*
		 * Call the initialization routine for this
		 * PMF service, if it is defined.
		 */
		if (pmf_svc_descs[ii].init) {
			rc = pmf_svc_descs[ii].init();
			if (rc) {
				WARN("Could not initialize PMF"
					"service %s - skipping \n",
					pmf_svc_descs[ii].name);
				continue;
			}
		}

		/* Update the pmf_svc_descs_indices array */
		pmf_svc_descs_indices[jj++] = ii;
	}

	pmf_num_services = jj;

	/*
	 * Sort the successfully registered PMF services
	 * according to service ID
	 */
	for (ii = 1; ii < pmf_num_services; ii++) {
		for (jj = 0; jj < (pmf_num_services - ii); jj++) {
			if ((pmf_svc_descs[jj].svc_config & PMF_SVC_ID_MASK) >
				(pmf_svc_descs[jj + 1].svc_config &
						PMF_SVC_ID_MASK)) {
				temp_val = pmf_svc_descs_indices[jj];
				pmf_svc_descs_indices[jj] =
						pmf_svc_descs_indices[jj+1];
				pmf_svc_descs_indices[jj+1] = temp_val;
			}
		}
	}

	return 0;
}

/*
 * This function implements binary search to find registered
 * PMF service based on Service ID provided in `tid` argument.
 */
static pmf_svc_desc_t *get_service(unsigned int tid)
{
	int low = 0;
	int mid;
	int high = pmf_num_services;
	unsigned int svc_id = tid & PMF_SVC_ID_MASK;
	int index;
	unsigned int desc_svc_id;

	if (pmf_num_services == 0)
		return NULL;

	assert(pmf_svc_descs);

	do {
		mid = (low + high) / 2;
		index = pmf_svc_descs_indices[mid];

		desc_svc_id = pmf_svc_descs[index].svc_config & PMF_SVC_ID_MASK;
		if (svc_id < desc_svc_id)
			high = mid - 1;
		if (svc_id > desc_svc_id)
			low = mid + 1;
	} while ((svc_id != desc_svc_id) && (low <= high));

	/*
	 * Make sure the Service found supports the tid range.
	 */
	if ((svc_id == desc_svc_id) && ((tid & PMF_TID_MASK) <
		(pmf_svc_descs[index].svc_config & PMF_TID_MASK)))
		return (pmf_svc_desc_t *)&pmf_svc_descs[index];

	return NULL;
}

/*
 * This function gets the time-stamp value for the PMF services
 * registered for SMC interface based on `tid` and `mpidr`.
 */
int pmf_get_timestamp_smc(unsigned int tid,
		u_register_t mpidr,
		unsigned int flags,
		unsigned long long *ts_value)
{
	pmf_svc_desc_t *svc_desc;
	assert(ts_value);

	/* Search for registered service. */
	svc_desc = get_service(tid);

	if ((svc_desc == NULL) || (plat_core_pos_by_mpidr(mpidr) < 0)) {
		*ts_value = 0;
		return -EINVAL;
	} else {
		/* Call the service time-stamp handler. */
		*ts_value = svc_desc->get_ts(tid, mpidr, flags);
		return 0;
	}
}

/*
 * This function can be used to dump `ts` value for given `tid`.
 * Assumption is that the console is already initialized.
 */
void __pmf_dump_timestamp(unsigned int tid, unsigned long long ts)
{
	tf_printf("PMF:cpu %u	tid %u	ts %llu\n",
		plat_my_core_pos(), tid, ts);
}

/*
 * This function calculate the address identified by
 * `base_addr`, `tid` and `cpuid`.
 */
static inline uintptr_t calc_ts_addr(uintptr_t base_addr,
		unsigned int tid,
		unsigned int cpuid)
{
	assert(cpuid < PLATFORM_CORE_COUNT);
	assert(base_addr >= PMF_TIMESTAMP_ARRAY_START);
	assert(base_addr < ((PMF_TIMESTAMP_ARRAY_START +
		PMF_PERCPU_TIMESTAMP_SIZE) - ((tid & PMF_TID_MASK) *
		sizeof(unsigned long long))));

	base_addr += ((cpuid * PMF_PERCPU_TIMESTAMP_SIZE) +
		((tid & PMF_TID_MASK) * sizeof(unsigned long long)));

	return base_addr;
}

/*
 * This function stores the `ts` value to the storage identified by
 * `base_addr`, `tid` and current cpu id.
 * Note: The timestamp addresses are cache line aligned per cpu
 * and only the owning CPU would ever write into it.
 */
void __pmf_store_timestamp(uintptr_t base_addr,
			unsigned int tid,
			unsigned long long ts)
{
	unsigned long long *ts_addr = (unsigned long long *)calc_ts_addr(base_addr,
				 tid, plat_my_core_pos());
	*ts_addr = ts;
}

/*
 * This is the cached version of `pmf_store_my_timestamp`
 * Note: The timestamp addresses are cache line aligned per cpu
 * and only the owning CPU would ever write into it.
 */
void __pmf_store_timestamp_with_cache_maint(uintptr_t base_addr,
			unsigned int tid,
			unsigned long long ts)
{
	unsigned long long *ts_addr = (unsigned long long *)calc_ts_addr(base_addr,
				 tid, plat_my_core_pos());
	*ts_addr = ts;
	flush_dcache_range((uintptr_t)ts_addr, sizeof(unsigned long long));
}

/*
 * This function retrieves the `ts` value from the storage identified by
 * `base_addr`, `tid` and `cpuid`.
 * Note: The timestamp addresses are cache line aligned per cpu.
 */
unsigned long long __pmf_get_timestamp(uintptr_t base_addr,
			unsigned int tid,
			unsigned int cpuid,
			unsigned int flags)
{
	assert(cpuid < PLATFORM_CORE_COUNT);
	unsigned long long *ts_addr = (unsigned long long *)calc_ts_addr(base_addr,
				tid, cpuid);

	if (flags & PMF_CACHE_MAINT)
		inv_dcache_range((uintptr_t)ts_addr, sizeof(unsigned long long));

	return *ts_addr;
}
