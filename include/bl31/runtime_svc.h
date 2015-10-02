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

#ifndef __RUNTIME_SVC_H__
#define __RUNTIME_SVC_H__

#include <bl_common.h>		/* to include exception types */
#include <smcc_helpers.h>	/* to include SMCC definitions */


/*******************************************************************************
 * Structure definition, typedefs & constants for the runtime service framework
 ******************************************************************************/

/*
 * Constants to allow the assembler access a runtime service
 * descriptor
 */
#define RT_SVC_SIZE_LOG2	5
#define SIZEOF_RT_SVC_DESC	(1 << RT_SVC_SIZE_LOG2)
#define RT_SVC_DESC_INIT	16
#define RT_SVC_DESC_HANDLE	24

/*
 * The function identifier has 6 bits for the owning entity number and
 * single bit for the type of smc call. When taken together these
 * values limit the maximum number of runtime services to 128.
 */
#define MAX_RT_SVCS		128

#ifndef __ASSEMBLY__

/* Prototype for runtime service initializing function */
typedef int32_t (*rt_svc_init_t)(void);

/*
 * Prototype for runtime service SMC handler function. x0 (SMC Function ID) to
 * x4 are as passed by the caller. Rest of the arguments to SMC and the context
 * can be accessed using the handle pointer. The cookie parameter is reserved
 * for future use
 */
typedef uint64_t (*rt_svc_handle_t)(uint32_t smc_fid,
				  uint64_t x1,
				  uint64_t x2,
				  uint64_t x3,
				  uint64_t x4,
				  void *cookie,
				  void *handle,
				  uint64_t flags);
typedef struct rt_svc_desc {
	uint8_t start_oen;
	uint8_t end_oen;
	uint8_t call_type;
	const char *name;
	rt_svc_init_t init;
	rt_svc_handle_t handle;
} rt_svc_desc_t;

/*
 * Convenience macro to declare a service descriptor
 */
#define DECLARE_RT_SVC(_name, _start, _end, _type, _setup, _smch) \
	static const rt_svc_desc_t __svc_desc_ ## _name \
		__attribute__ ((section("rt_svc_descs"), used)) = { \
			_start, \
			_end, \
			_type, \
			#_name, \
			_setup, \
			_smch }

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
 * This macro combines the call type and the owning entity number corresponding
 * to a runtime service to generate a unique owning entity number. This unique
 * oen is used to access an entry in the 'rt_svc_descs_indices' array. The entry
 * contains the index of the service descriptor in the 'rt_svc_descs' array.
 */
#define get_unique_oen(oen, call_type)	((oen & FUNCID_OEN_MASK) |	\
					((call_type & FUNCID_TYPE_MASK) \
					 << FUNCID_OEN_WIDTH))

/*******************************************************************************
 * Function & variable prototypes
 ******************************************************************************/
void runtime_svc_init(void);
extern uint64_t __RT_SVC_DESCS_START__;
extern uint64_t __RT_SVC_DESCS_END__;
void init_crash_reporting(void);

#endif /*__ASSEMBLY__*/
#endif /* __RUNTIME_SVC_H__ */
