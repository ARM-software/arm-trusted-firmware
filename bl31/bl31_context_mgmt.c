/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <bl31.h>
#include <bl_common.h>
#include <context.h>
#include <context_mgmt.h>
#include <cpu_data.h>
#include <platform.h>


/*******************************************************************************
 * This function returns a pointer to the most recent 'cpu_context' structure
 * for the calling CPU that was set as the context for the specified security
 * state. NULL is returned if no such structure has been specified.
 ******************************************************************************/
void *cm_get_context(uint32_t security_state)
{
	assert(security_state <= NON_SECURE);

	return get_cpu_data(cpu_context[security_state]);
}

/*******************************************************************************
 * This function sets the pointer to the current 'cpu_context' structure for the
 * specified security state for the calling CPU
 ******************************************************************************/
void cm_set_context(void *context, uint32_t security_state)
{
	assert(security_state <= NON_SECURE);

	set_cpu_data(cpu_context[security_state], context);
}

/*******************************************************************************
 * This function returns a pointer to the most recent 'cpu_context' structure
 * for the CPU identified by `cpu_idx` that was set as the context for the
 * specified security state. NULL is returned if no such structure has been
 * specified.
 ******************************************************************************/
void *cm_get_context_by_index(unsigned int cpu_idx,
				unsigned int security_state)
{
	assert(sec_state_is_valid(security_state));

	return get_cpu_data_by_index(cpu_idx, cpu_context[security_state]);
}

/*******************************************************************************
 * This function sets the pointer to the current 'cpu_context' structure for the
 * specified security state for the CPU identified by CPU index.
 ******************************************************************************/
void cm_set_context_by_index(unsigned int cpu_idx, void *context,
				unsigned int security_state)
{
	assert(sec_state_is_valid(security_state));

	set_cpu_data_by_index(cpu_idx, cpu_context[security_state], context);
}

#if !ERROR_DEPRECATED
/*
 * These context management helpers are deprecated but are maintained for use
 * by SPDs which have not migrated to the new API. If ERROR_DEPRECATED
 * is enabled, these are excluded from the build so as to force users to
 * migrate to the new API.
 */

/*******************************************************************************
 * This function returns a pointer to the most recent 'cpu_context' structure
 * for the CPU identified by MPIDR that was set as the context for the specified
 * security state. NULL is returned if no such structure has been specified.
 ******************************************************************************/
void *cm_get_context_by_mpidr(uint64_t mpidr, uint32_t security_state)
{
	assert(sec_state_is_valid(security_state));

	/*
	 * Suppress deprecated declaration warning in compatibility function
	 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	return cm_get_context_by_index(platform_get_core_pos(mpidr), security_state);
#pragma GCC diagnostic pop
}

/*******************************************************************************
 * This function sets the pointer to the current 'cpu_context' structure for the
 * specified security state for the CPU identified by MPIDR
 ******************************************************************************/
void cm_set_context_by_mpidr(uint64_t mpidr, void *context, uint32_t security_state)
{
	assert(sec_state_is_valid(security_state));

	/*
	 * Suppress deprecated declaration warning in compatibility function
	 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	cm_set_context_by_index(platform_get_core_pos(mpidr),
						 context, security_state);
#pragma GCC diagnostic pop
}

/*******************************************************************************
 * The following function provides a compatibility function for SPDs using the
 * existing cm library routines. This function is expected to be invoked for
 * initializing the cpu_context for the CPU specified by MPIDR for first use.
 ******************************************************************************/
void cm_init_context(uint64_t mpidr, const entry_point_info_t *ep)
{
	if ((mpidr & MPIDR_AFFINITY_MASK) ==
			(read_mpidr_el1() & MPIDR_AFFINITY_MASK))
		cm_init_my_context(ep);
	else {
		/*
		 * Suppress deprecated declaration warning in compatibility
		 * function
		 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
		cm_init_context_by_index(platform_get_core_pos(mpidr), ep);
#pragma GCC diagnostic pop
	}
}
#endif /* ERROR_DEPRECATED */
