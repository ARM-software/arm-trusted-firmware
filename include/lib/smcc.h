/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SMCC_H__
#define __SMCC_H__

#include <utils_def.h>

/*******************************************************************************
 * Bit definitions inside the function id as per the SMC calling convention
 ******************************************************************************/
#define FUNCID_TYPE_SHIFT		U(31)
#define FUNCID_CC_SHIFT			U(30)
#define FUNCID_OEN_SHIFT		U(24)
#define FUNCID_NUM_SHIFT		U(0)

#define FUNCID_TYPE_MASK		U(0x1)
#define FUNCID_CC_MASK			U(0x1)
#define FUNCID_OEN_MASK			U(0x3f)
#define FUNCID_NUM_MASK			U(0xffff)

#define FUNCID_TYPE_WIDTH		U(1)
#define FUNCID_CC_WIDTH			U(1)
#define FUNCID_OEN_WIDTH		U(6)
#define FUNCID_NUM_WIDTH		U(16)

#define GET_SMC_CC(id)			((id >> FUNCID_CC_SHIFT) & \
					 FUNCID_CC_MASK)
#define GET_SMC_TYPE(id)		((id >> FUNCID_TYPE_SHIFT) & \
					 FUNCID_TYPE_MASK)

#define SMC_64				U(1)
#define SMC_32				U(0)
#define SMC_OK				U(0)
#define SMC_UNK				U(0xffffffff)
#define SMC_TYPE_FAST			ULL(1)
#if !ERROR_DEPRECATED
#define SMC_TYPE_STD			ULL(0)
#endif
#define SMC_TYPE_YIELD			U(0)
#define SMC_PREEMPTED		U(0xfffffffe)
/*******************************************************************************
 * Owning entity number definitions inside the function id as per the SMC
 * calling convention
 ******************************************************************************/
#define OEN_ARM_START			U(0)
#define OEN_ARM_END			U(0)
#define OEN_CPU_START			U(1)
#define OEN_CPU_END			U(1)
#define OEN_SIP_START			U(2)
#define OEN_SIP_END			U(2)
#define OEN_OEM_START			U(3)
#define OEN_OEM_END			U(3)
#define OEN_STD_START			U(4)	/* Standard Service Calls */
#define OEN_STD_END			U(4)
#define OEN_TAP_START			U(48)	/* Trusted Applications */
#define OEN_TAP_END			U(49)
#define OEN_TOS_START			U(50)	/* Trusted OS */
#define OEN_TOS_END			U(63)
#define OEN_LIMIT			U(64)

#ifndef __ASSEMBLY__

#include <cassert.h>
#include <stdint.h>

#define SMCCC_MAJOR_VERSION U(1)
#define SMCCC_MINOR_VERSION U(1)

#define MAKE_SMCCC_VERSION(_major, _minor) (((_major) << 16) | (_minor))

/* Various flags passed to SMC handlers */
#define SMC_FROM_SECURE		(U(0) << 0)
#define SMC_FROM_NON_SECURE	(U(1) << 0)

#define is_caller_non_secure(_f)	(!!(_f & SMC_FROM_NON_SECURE))
#define is_caller_secure(_f)		(!(is_caller_non_secure(_f)))

/* The macro below is used to identify a Standard Service SMC call */
#define is_std_svc_call(_fid)		((((_fid) >> FUNCID_OEN_SHIFT) & \
					   FUNCID_OEN_MASK) == OEN_STD_START)

/* The macro below is used to identify a Arm Architectural Service SMC call */
#define is_arm_arch_svc_call(_fid)	((((_fid) >> FUNCID_OEN_SHIFT) & \
					   FUNCID_OEN_MASK) == OEN_ARM_START)

/* The macro below is used to identify a valid Fast SMC call */
#define is_valid_fast_smc(_fid)		((!(((_fid) >> 16) & U(0xff))) && \
					   (GET_SMC_TYPE(_fid) == SMC_TYPE_FAST))

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

#endif /*__ASSEMBLY__*/
#endif /* __SMCC_H__ */
