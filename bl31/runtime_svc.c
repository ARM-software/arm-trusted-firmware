/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

#include <debug.h>
#include <errno.h>
#include <runtime_svc.h>
#include <string.h>

/*******************************************************************************
 * The 'rt_svc_descs' array holds the runtime service descriptors exported by
 * services by placing them in the 'rt_svc_descs' linker section.
 * The 'rt_svc_descs_indices' array holds the index of a descriptor in the
 * 'rt_svc_descs' array. When an SMC arrives, the OEN[29:24] bits and the call
 * type[31] bit in the function id are combined to get an index into the
 * 'rt_svc_descs_indices' array. This gives the index of the descriptor in the
 * 'rt_svc_descs' array which contains the SMC handler.
 ******************************************************************************/
#define RT_SVC_DESCS_START	((uint64_t) (&__RT_SVC_DESCS_START__))
#define RT_SVC_DESCS_END	((uint64_t) (&__RT_SVC_DESCS_END__))
uint8_t rt_svc_descs_indices[MAX_RT_SVCS];
static rt_svc_desc_t *rt_svc_descs;

/*******************************************************************************
 * Simple routine to sanity check a runtime service descriptor before using it
 ******************************************************************************/
static int32_t validate_rt_svc_desc(rt_svc_desc_t *desc)
{
	if (desc == NULL)
		return -EINVAL;

	if (desc->start_oen > desc->end_oen)
		return -EINVAL;

	if (desc->end_oen >= OEN_LIMIT)
		return -EINVAL;

	if (desc->call_type != SMC_TYPE_FAST && desc->call_type != SMC_TYPE_STD)
		return -EINVAL;

	/* A runtime service having no init or handle function doesn't make sense */
	if (desc->init == NULL && desc->handle == NULL)
		return -EINVAL;

	return 0;
}

/*******************************************************************************
 * This function calls the initialisation routine in the descriptor exported by
 * a runtime service. Once a descriptor has been validated, its start & end
 * owning entity numbers and the call type are combined to form a unique oen.
 * The unique oen is used as an index into the 'rt_svc_descs_indices' array.
 * The index of the runtime service descriptor is stored at this index.
 ******************************************************************************/
void runtime_svc_init()
{
	int32_t rc = 0;
	uint32_t index, start_idx, end_idx;
	uint64_t rt_svc_descs_num;

	/* If no runtime services are implemented then simply bail out */
	rt_svc_descs_num = RT_SVC_DESCS_END - RT_SVC_DESCS_START;
	rt_svc_descs_num /= sizeof(rt_svc_desc_t);
	if (rt_svc_descs_num == 0)
		return;

	/* Initialise internal variables to invalid state */
	memset(rt_svc_descs_indices, -1, sizeof(rt_svc_descs_indices));

	rt_svc_descs = (rt_svc_desc_t *) RT_SVC_DESCS_START;
	for (index = 0; index < rt_svc_descs_num; index++) {

		/*
		 * An invalid descriptor is an error condition since it is
		 * difficult to predict the system behaviour in the absence
		 * of this service.
		 */
		rc = validate_rt_svc_desc(&rt_svc_descs[index]);
		if (rc) {
			ERROR("Invalid runtime service descriptor 0x%x (%s)\n",
					&rt_svc_descs[index],
					rt_svc_descs[index].name);
			goto error;
		}

		/*
		 * The runtime service may have seperate rt_svc_desc_t
		 * for its fast smc and standard smc. Since the service itself
		 * need to be initialized only once, only one of them will have
		 * an initialisation routine defined. Call the initialisation
		 * routine for this runtime service, if it is defined.
		 */
		if (rt_svc_descs[index].init) {
			rc = rt_svc_descs[index].init();
			if (rc) {
				ERROR("Error initializing runtime service %s\n",
						rt_svc_descs[index].name);
				continue;
			}
		}

		/*
		 * Fill the indices corresponding to the start and end
		 * owning entity numbers with the index of the
		 * descriptor which will handle the SMCs for this owning
		 * entity range.
		 */
		start_idx = get_unique_oen(rt_svc_descs[index].start_oen,
				rt_svc_descs[index].call_type);
		end_idx = get_unique_oen(rt_svc_descs[index].end_oen,
				rt_svc_descs[index].call_type);

		for (; start_idx <= end_idx; start_idx++)
			rt_svc_descs_indices[start_idx] = index;
	}

	return;
error:
	panic();
}
