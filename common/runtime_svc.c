/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
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
#define RT_SVC_DESCS_START	((uintptr_t) (&__RT_SVC_DESCS_START__))
#define RT_SVC_DESCS_END	((uintptr_t) (&__RT_SVC_DESCS_END__))
uint8_t rt_svc_descs_indices[MAX_RT_SVCS];
static rt_svc_desc_t *rt_svc_descs;

#define RT_SVC_DECS_NUM		((RT_SVC_DESCS_END - RT_SVC_DESCS_START)\
					/ sizeof(rt_svc_desc_t))

/*******************************************************************************
 * Function to invoke the registered `handle` corresponding to the smc_fid.
 ******************************************************************************/
uintptr_t handle_runtime_svc(uint32_t smc_fid,
			     void *cookie,
			     void *handle,
			     unsigned int flags)
{
	u_register_t x1, x2, x3, x4;
	int index;
	unsigned int idx;
	const rt_svc_desc_t *rt_svc_descs;

	assert(handle);
	idx = get_unique_oen_from_smc_fid(smc_fid);
	assert(idx < MAX_RT_SVCS);

	index = rt_svc_descs_indices[idx];
	if (index < 0 || index >= (int)RT_SVC_DECS_NUM)
		SMC_RET1(handle, SMC_UNK);

	rt_svc_descs = (rt_svc_desc_t *) RT_SVC_DESCS_START;

	get_smc_params_from_ctx(handle, x1, x2, x3, x4);

	return rt_svc_descs[index].handle(smc_fid, x1, x2, x3, x4, cookie,
						handle, flags);
}

/*******************************************************************************
 * Simple routine to sanity check a runtime service descriptor before using it
 ******************************************************************************/
static int32_t validate_rt_svc_desc(const rt_svc_desc_t *desc)
{
	if (desc == NULL)
		return -EINVAL;

	if (desc->start_oen > desc->end_oen)
		return -EINVAL;

	if (desc->end_oen >= OEN_LIMIT)
		return -EINVAL;

	if (desc->call_type != SMC_TYPE_FAST &&
			desc->call_type != SMC_TYPE_YIELD)
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
void runtime_svc_init(void)
{
	int rc = 0;
	unsigned int index, start_idx, end_idx;

	/* Assert the number of descriptors detected are less than maximum indices */
	assert((RT_SVC_DESCS_END >= RT_SVC_DESCS_START) &&
			(RT_SVC_DECS_NUM < MAX_RT_SVCS));

	/* If no runtime services are implemented then simply bail out */
	if (RT_SVC_DECS_NUM == 0)
		return;

	/* Initialise internal variables to invalid state */
	memset(rt_svc_descs_indices, -1, sizeof(rt_svc_descs_indices));

	rt_svc_descs = (rt_svc_desc_t *) RT_SVC_DESCS_START;
	for (index = 0; index < RT_SVC_DECS_NUM; index++) {
		rt_svc_desc_t *service = &rt_svc_descs[index];

		/*
		 * An invalid descriptor is an error condition since it is
		 * difficult to predict the system behaviour in the absence
		 * of this service.
		 */
		rc = validate_rt_svc_desc(service);
		if (rc) {
			ERROR("Invalid runtime service descriptor %p\n",
				(void *) service);
			panic();
		}

		/*
		 * The runtime service may have separate rt_svc_desc_t
		 * for its fast smc and yielding smc. Since the service itself
		 * need to be initialized only once, only one of them will have
		 * an initialisation routine defined. Call the initialisation
		 * routine for this runtime service, if it is defined.
		 */
		if (service->init) {
			rc = service->init();
			if (rc) {
				ERROR("Error initializing runtime service %s\n",
						service->name);
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
				service->call_type);
		assert(start_idx < MAX_RT_SVCS);
		end_idx = get_unique_oen(rt_svc_descs[index].end_oen,
				service->call_type);
		assert(end_idx < MAX_RT_SVCS);
		for (; start_idx <= end_idx; start_idx++)
			rt_svc_descs_indices[start_idx] = index;
	}
}
