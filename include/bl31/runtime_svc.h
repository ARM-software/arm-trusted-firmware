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

#ifndef __RUNTIME_SVC_H__
#define __RUNTIME_SVC_H__

/*******************************************************************************
 * Bit definitions inside the function id as per the SMC calling convention
 ******************************************************************************/
#define FUNCID_TYPE_SHIFT		31
#define FUNCID_CC_SHIFT			30
#define FUNCID_OEN_SHIFT		24
#define FUNCID_NUM_SHIFT		0

#define FUNCID_TYPE_MASK		0x1
#define FUNCID_CC_MASK			0x1
#define FUNCID_OEN_MASK			0x3f
#define FUNCID_NUM_MASK			0xffff

#define FUNCID_TYPE_WIDTH		1
#define FUNCID_CC_WIDTH			1
#define FUNCID_OEN_WIDTH		6
#define FUNCID_NUM_WIDTH		16

#define GET_SMC_CC(id)			((id >> FUNCID_CC_SHIFT) & \
					 FUNCID_CC_MASK)
#define GET_SMC_TYPE(id)		((id >> FUNCID_TYPE_SHIFT) & \
					 FUNCID_TYPE_MASK)

#define SMC_64				1
#define SMC_32				0
#define SMC_UNK				0xffffffff
#define SMC_TYPE_FAST			1
#define SMC_TYPE_STD			0
#define SMC_PREEMPTED		0xfffffffe
/*******************************************************************************
 * Owning entity number definitions inside the function id as per the SMC
 * calling convention
 ******************************************************************************/
#define OEN_ARM_START			0
#define OEN_ARM_END			0
#define OEN_CPU_START			1
#define OEN_CPU_END			1
#define OEN_SIP_START			2
#define OEN_SIP_END			2
#define OEN_OEM_START			3
#define OEN_OEM_END			3
#define OEN_STD_START			4	/* Standard Calls */
#define OEN_STD_END			4
#define OEN_TAP_START			48	/* Trusted Applications */
#define OEN_TAP_END			49
#define OEN_TOS_START			50	/* Trusted OS */
#define OEN_TOS_END			63
#define OEN_LIMIT			64

/*******************************************************************************
 * Constants to indicate type of exception to the common exception handler.
 ******************************************************************************/
#define SYNC_EXCEPTION_SP_EL0		0x0
#define IRQ_SP_EL0			0x1
#define FIQ_SP_EL0			0x2
#define SERROR_SP_EL0			0x3
#define SYNC_EXCEPTION_SP_ELX		0x4
#define IRQ_SP_ELX			0x5
#define FIQ_SP_ELX			0x6
#define SERROR_SP_ELX			0x7
#define SYNC_EXCEPTION_AARCH64		0x8
#define IRQ_AARCH64			0x9
#define FIQ_AARCH64			0xa
#define SERROR_AARCH64			0xb
#define SYNC_EXCEPTION_AARCH32		0xc
#define IRQ_AARCH32			0xd
#define FIQ_AARCH32			0xe
#define SERROR_AARCH32			0xf

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

#include <cassert.h>
#include <context.h>
#include <stdint.h>

/* Various flags passed to SMC handlers */
#define SMC_FROM_SECURE		(0 << 0)
#define SMC_FROM_NON_SECURE	(1 << 0)

#define is_caller_non_secure(_f)	(!!(_f & SMC_FROM_NON_SECURE))
#define is_caller_secure(_f)		(!(is_caller_non_secure(_f)))

/* Prototype for runtime service initializing function */
typedef int32_t (*rt_svc_init_t)(void);

/* Convenience macros to return from SMC handler */
#define SMC_RET0(_h)	{ \
	return (uint64_t) (_h);		\
}
#define SMC_RET1(_h, _x0)	{ \
	write_ctx_reg(get_gpregs_ctx(_h), CTX_GPREG_X0, (_x0)); \
	SMC_RET0(_h);						\
}
#define SMC_RET2(_h, _x0, _x1)	{ \
	write_ctx_reg(get_gpregs_ctx(_h), CTX_GPREG_X1, (_x1)); \
	SMC_RET1(_h, (_x0)); \
}
#define SMC_RET3(_h, _x0, _x1, _x2)	{ \
	write_ctx_reg(get_gpregs_ctx(_h), CTX_GPREG_X2, (_x2)); \
	SMC_RET2(_h, (_x0), (_x1)); \
}
#define SMC_RET4(_h, _x0, _x1, _x2, _x3)	{ \
	write_ctx_reg(get_gpregs_ctx(_h), CTX_GPREG_X3, (_x3)); \
	SMC_RET3(_h, (_x0), (_x1), (_x2)); \
}


/*
 * Convenience macros to access general purpose registers using handle provided
 * to SMC handler. These takes the offset values defined in context.h
 */
#define SMC_GET_GP(_h, _g) \
	read_ctx_reg(get_gpregs_ctx(_h), (_g));
#define SMC_SET_GP(_h, _g, _v) \
	write_ctx_reg(get_gpregs_ctx(_h), (_g), (_v));

/*
 * Convenience macros to access EL3 context registers using handle provided to
 * SMC handler. These takes the offset values defined in context.h
 */
#define SMC_GET_EL3(_h, _e) \
	read_ctx_reg(get_el3state_ctx(_h), (_e));
#define SMC_SET_EL3(_h, _e, _v) \
	write_ctx_reg(get_el3state_ctx(_h), (_e), (_v));

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


/*
 * Macro to define UUID for services. Apart from defining and initializing a
 * uuid_t structure, this macro verifies that the first word of the defined UUID
 * does not equal SMC_UNK. This is to ensure that the caller won't mistake the
 * returned UUID in x0 for an invalid SMC error return
 */
#define DEFINE_SVC_UUID(_name, _tl, _tm, _th, _cl, _ch, \
		_n0, _n1, _n2, _n3, _n4, _n5) \
	CASSERT(_tl != SMC_UNK, invalid_svc_uuid);\
	static const uuid_t _name = { \
		_tl, _tm, _th, _cl, _ch, \
		{ _n0, _n1, _n2, _n3, _n4, _n5 } \
	}

/* Return a UUID in the SMC return registers */
#define SMC_UUID_RET(_h, _uuid) \
	SMC_RET4(handle, ((const uint32_t *) &(_uuid))[0], \
			 ((const uint32_t *) &(_uuid))[1], \
			 ((const uint32_t *) &(_uuid))[2], \
			 ((const uint32_t *) &(_uuid))[3])

/*******************************************************************************
 * Function & variable prototypes
 ******************************************************************************/
void runtime_svc_init();
extern uint64_t __RT_SVC_DESCS_START__;
extern uint64_t __RT_SVC_DESCS_END__;
uint64_t get_crash_stack(uint64_t mpidr);
void runtime_exceptions(void);
#endif /*__ASSEMBLY__*/
#endif /* __RUNTIME_SVC_H__ */
