/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMCCC_V1_H
#define SMCCC_V1_H

#ifndef __SMCCC_H__
#error "This file must only be included from smccc.h"
#endif

/*******************************************************************************
 * Bit definitions inside the function id as per the SMC calling convention
 ******************************************************************************/
#define FUNCID_TYPE_SHIFT		U(31)
#define FUNCID_TYPE_MASK		U(0x1)
#define FUNCID_TYPE_WIDTH		U(1)

#define FUNCID_CC_SHIFT			U(30)
#define FUNCID_CC_MASK			U(0x1)
#define FUNCID_CC_WIDTH			U(1)

#define FUNCID_OEN_SHIFT		U(24)
#define FUNCID_OEN_MASK			U(0x3f)
#define FUNCID_OEN_WIDTH		U(6)

#define FUNCID_NUM_SHIFT		U(0)
#define FUNCID_NUM_MASK			U(0xffff)
#define FUNCID_NUM_WIDTH		U(16)

#define GET_SMC_TYPE(id)		(((id) >> FUNCID_TYPE_SHIFT) & \
					 FUNCID_TYPE_MASK)
#define GET_SMC_CC(id)			(((id) >> FUNCID_CC_SHIFT) & \
					 FUNCID_CC_MASK)
#define GET_SMC_OEN(id)			(((id) >> FUNCID_OEN_SHIFT) & \
					 FUNCID_OEN_MASK)

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
#define OEN_STD_HYP_START		U(5)	/* Standard Hypervisor Service calls */
#define OEN_STD_HYP_END			U(5)
#define OEN_VEN_HYP_START		U(6)	/* Vendor Hypervisor Service calls */
#define OEN_VEN_HYP_END			U(6)
#define OEN_TAP_START			U(48)	/* Trusted Applications */
#define OEN_TAP_END			U(49)
#define OEN_TOS_START			U(50)	/* Trusted OS */
#define OEN_TOS_END			U(63)
#define OEN_LIMIT			U(64)

/* Flags and error codes */
#define SMC_64				U(1)
#define SMC_32				U(0)

#define SMC_TYPE_FAST			ULL(1)
#define SMC_TYPE_YIELD			ULL(0)

#define SMC_OK				ULL(0)
#define SMC_UNK				-1
#define SMC_PREEMPTED			-2	/* Not defined by the SMCCC */

#endif /* SMCCC_V1_H */
