/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __RUNTIME_SVC_H__
#define __RUNTIME_SVC_H__

#include <bl_common.h>		/* to include exception types */
#include <cassert.h>
#include <smccc_helpers.h>	/* to include SMCCC definitions */
#include <utils_def.h>

/*******************************************************************************
 * Structure definition, typedefs & constants for the runtime service framework
 ******************************************************************************/

/*
 * Constants to allow the assembler access a runtime service
 * descriptor
 */
#ifdef AARCH32
#define RT_SVC_SIZE_LOG2	4
#define RT_SVC_DESC_INIT	8
#define RT_SVC_DESC_HANDLE	12
#else
#define RT_SVC_SIZE_LOG2	5
#define RT_SVC_DESC_INIT	16
#define RT_SVC_DESC_HANDLE	24
#endif /* AARCH32 */
#define SIZEOF_RT_SVC_DESC	(1 << RT_SVC_SIZE_LOG2)


/*
 * In SMCCC 1.X, the function identifier has 6 bits for the owning entity number
 * and a single bit for the type of smc call. When taken together, those values
 * limit the maximum number of runtime services to 128.
 *
 * In SMCCC 2.X the type bit is always 1 and there are only 4 OEN bits in the
 * compatibility namespace, so the total number of services is 16. The LSB of
 * namespace is also added to these 4 bits to make space for the vendor service
 * handler and so the total number of runtime services is 32.
 */
#if SMCCC_MAJOR_VERSION == 1
#define MAX_RT_SVCS		128
#elif SMCCC_MAJOR_VERSION == 2
#define MAX_RT_SVCS		32
#endif

#ifndef __ASSEMBLY__

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
#if SMCCC_MAJOR_VERSION == 1
	uint8_t call_type;
#elif SMCCC_MAJOR_VERSION == 2
	uint8_t is_vendor;
#endif
	const char *name;
	rt_svc_init_t init;
	rt_svc_handle_t handle;
} rt_svc_desc_t;

/*
 * Convenience macros to declare a service descriptor
 */
#if SMCCC_MAJOR_VERSION == 1

#define DECLARE_RT_SVC(_name, _start, _end, _type, _setup, _smch)	\
	static const rt_svc_desc_t __svc_desc_ ## _name			\
		__section("rt_svc_descs") __used = {			\
			.start_oen = _start,				\
			.end_oen = _end,				\
			.call_type = _type,				\
			.name = #_name,					\
			.init = _setup,					\
			.handle = _smch					\
		}

#elif SMCCC_MAJOR_VERSION == 2

#define DECLARE_RT_SVC(_name, _start, _end, _type, _setup, _smch)	\
	static const rt_svc_desc_t __svc_desc_ ## _name			\
		__section("rt_svc_descs") __used = {			\
			.start_oen = _start,				\
			.end_oen = _end,				\
			.is_vendor = 0,					\
			.name = #_name,					\
			.init = _setup,					\
			.handle = _smch,				\
		};							\
	CASSERT((_type) == SMC_TYPE_FAST, rt_svc_type_check_ ## _name)

/*
 * The higher 16 entries of the runtime services are used for the vendor
 * specific descriptor.
 */
#define DECLARE_RT_SVC_VENDOR(_setup, _smch)				\
	static const rt_svc_desc_t __svc_desc_vendor			\
		__section("rt_svc_descs") __used = {			\
			.start_oen = 0,					\
			.end_oen = 15,					\
			.is_vendor = 1,					\
			.name = "vendor_rt_svc",			\
			.init = _setup,					\
			.handle = _smch,				\
		}

#endif /* SMCCC_MAJOR_VERSION */

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


#if SMCCC_MAJOR_VERSION == 1
/*
 * This macro combines the call type and the owning entity number corresponding
 * to a runtime service to generate a unique owning entity number. This unique
 * oen is used to access an entry in the 'rt_svc_descs_indices' array. The entry
 * contains the index of the service descriptor in the 'rt_svc_descs' array.
 */
#define get_unique_oen(oen, call_type)				\
	(((uint32_t)(oen) & FUNCID_OEN_MASK) |			\
	(((uint32_t)(call_type) & FUNCID_TYPE_MASK) << FUNCID_OEN_WIDTH))

/*
 * This macro generates the unique owning entity number from the SMC Function
 * ID. This unique oen is used to access an entry in the 'rt_svc_descs_indices'
 * array to invoke the corresponding runtime service handler during SMC
 * handling.
 */
#define get_unique_oen_from_smc_fid(fid)			\
	get_unique_oen(GET_SMC_OEN(fid), GET_SMC_TYPE(fid))

#elif SMCCC_MAJOR_VERSION == 2

/*
 * This macro combines the owning entity number corresponding to a runtime
 * service with one extra bit for the vendor namespace to generate an index into
 * the 'rt_svc_descs_indices' array. The entry contains the index of the service
 * descriptor in the 'rt_svc_descs' array.
 */
#define get_rt_desc_idx(oen, is_vendor)				\
	(((uint32_t)(oen) & FUNCID_OEN_MASK) |			\
	(((uint32_t)(is_vendor) & 1U) << FUNCID_OEN_WIDTH))

#endif

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

#endif /*__ASSEMBLY__*/
#endif /* __RUNTIME_SVC_H__ */
