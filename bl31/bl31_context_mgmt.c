/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
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

#include <assert.h>
#include <bl31.h>
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

	return cm_get_context_by_index(platform_get_core_pos(mpidr), security_state);
}

/*******************************************************************************
 * This function sets the pointer to the current 'cpu_context' structure for the
 * specified security state for the CPU identified by MPIDR
 ******************************************************************************/
void cm_set_context_by_mpidr(uint64_t mpidr, void *context, uint32_t security_state)
{
	assert(sec_state_is_valid(security_state));

	cm_set_context_by_index(platform_get_core_pos(mpidr),
						 context, security_state);
}

/*******************************************************************************
 * The following function provides a compatibility function for SPDs using the
 * existing cm library routines. This function is expected to be invoked for
 * initializing the cpu_context for the CPU specified by MPIDR for first use.
 ******************************************************************************/
void cm_init_context(unsigned long mpidr, const entry_point_info_t *ep)
{
	if ((mpidr & MPIDR_AFFINITY_MASK) ==
			(read_mpidr_el1() & MPIDR_AFFINITY_MASK))
		cm_init_my_context(ep);
	else
		cm_init_context_by_index(platform_get_core_pos(mpidr), ep);
}
#endif