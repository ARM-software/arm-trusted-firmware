/*
 * Copyright (c) 2013-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RUNTIME_SVC_H
#define RUNTIME_SVC_H

#include <common/bl_common.h>		/* to include exception types */
#include <lib/cassert.h>
#include <lib/utils_def.h>
#include <smccc_helpers.h>	/* to include SMCCC definitions */

/*******************************************************************************
 * Structure definition, typedefs & constants for the runtime service framework
 ******************************************************************************/

/*
 * Constants to allow the assembler access a runtime service
 * descriptor
 */
#ifdef __aarch64__
#define RT_SVC_SIZE_LOG2	U(5)
#define RT_SVC_DESC_INIT	U(16)
#define RT_SVC_DESC_HANDLE	U(24)
#else
#define RT_SVC_SIZE_LOG2	U(4)
#define RT_SVC_DESC_INIT	U(8)
#define RT_SVC_DESC_HANDLE	U(12)
#endif /* __aarch64__ */
#define SIZEOF_RT_SVC_DESC	(U(1) << RT_SVC_SIZE_LOG2)


/*
 * In SMCCC 1.X, the function identifier has 6 bits for the owning entity number
 * and a single bit for the type of smc call. When taken together, those values
 * limit the maximum number of runtime services to 128.
 */
#define MAX_RT_SVCS		U(128)

#ifndef __ASSEMBLER__

/* Prototype for runtime service initializing function */
typedef int32_t (*rt_svc_init_t)(void);

/*
 * Prototype for runtime service SMC handler function. x0 (SMC Function ID) to
 * x4 are as passed by the caller. Rest of the arguments to SMC and the context
 * can be accessed using the handle pointer. The cookie parameter is reserved
 * for future use
 */
typedef uintptr_t (*rt_svc_handle_t)(uint32_t smc_fid,
				  u_register_t x1,
				  u_register_t x2,
				  u_register_t x3,
				  u_register_t x4,
				  void *cookie,
				  void *handle,
				  u_register_t flags);
typedef struct rt_svc_desc {
	uint8_t start_oen;
	uint8_t end_oen;
	uint8_t call_type;
	const char *name;
	rt_svc_init_t init;
	rt_svc_handle_t handle;
} rt_svc_desc_t;

/*
 * Convenience macros to declare a service descriptor
 */
#define DECLARE_RT_SVC(_name, _start, _end, _type, _setup, _smch)	\
	static const rt_svc_desc_t __svc_desc_ ## _name			\
		__section("rt_svc_descs") __used = {			\
			.start_oen = (_start),				\
			.end_oen = (_end),				\
			.call_type = (_type),				\
			.name = #_name,					\
			.init = (_setup),				\
			.handle = (_smch)				\
		}

/*
 * Compile time assertions related to the 'rt_svc_desc' structure to:
 * 1. ensure that the assembler and the compiler view of the size
 *    of the structure are the same.
 * 2. ensure that the assembler and the compiler see the initialisation
 *    routine at the same offset.
 * 3. ensure that the assembler and the compiler see the handler
 *    routine at the same offset.
 */
CASSERT((sizeof(rt_svc_desc_t) == SIZEOF_RT_SVC_DESC), \
	assert_sizeof_rt_svc_desc_mismatch);
CASSERT(RT_SVC_DESC_INIT == __builtin_offsetof(rt_svc_desc_t, init), \
	assert_rt_svc_desc_init_offset_mismatch);
CASSERT(RT_SVC_DESC_HANDLE == __builtin_offsetof(rt_svc_desc_t, handle), \
	assert_rt_svc_desc_handle_offset_mismatch);


/*
 * This function combines the call type and the owning entity number
 * corresponding to a runtime service to generate a unique owning entity number.
 * This unique oen is used to access an entry in the 'rt_svc_descs_indices'
 * array. The entry contains the index of the service descriptor in the
 * 'rt_svc_descs' array.
 */
static inline uint32_t get_unique_oen(uint32_t oen, uint32_t call_type)
{
	return ((call_type & FUNCID_TYPE_MASK) << FUNCID_OEN_WIDTH) |
		(oen & FUNCID_OEN_MASK);
}

/*
 * This function generates the unique owning entity number from the SMC Function
 * ID. This unique oen is used to access an entry in the 'rt_svc_descs_indices'
 * array to invoke the corresponding runtime service handler during SMC
 * handling.
 */
static inline uint32_t get_unique_oen_from_smc_fid(uint32_t fid)
{
	return get_unique_oen(GET_SMC_OEN(fid), GET_SMC_TYPE(fid));
}

/*******************************************************************************
 * Function & variable prototypes
 ******************************************************************************/
void runtime_svc_init(void);
uintptr_t handle_runtime_svc(uint32_t smc_fid, void *cookie, void *handle,
						unsigned int flags);
IMPORT_SYM(uintptr_t, __RT_SVC_DESCS_START__,		RT_SVC_DESCS_START);
IMPORT_SYM(uintptr_t, __RT_SVC_DESCS_END__,		RT_SVC_DESCS_END);
void init_crash_reporting(void);

extern uint8_t rt_svc_descs_indices[MAX_RT_SVCS];

#endif /*__ASSEMBLER__*/
#endif /* RUNTIME_SVC_H */
