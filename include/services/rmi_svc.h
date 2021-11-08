/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RMI_SVC_H
#define RMI_SVC_H

#include <lib/smccc.h>
#include <lib/utils_def.h>

/* RMI error codes. */
#define RMI_SUCCESS			0
#define RMI_ERROR_NOT_SUPPORTED		-1
#define RMI_ERROR_INVALID_ADDRESS	-2
#define RMI_ERROR_INVALID_PAS		-3

/* The macros below are used to identify RMI calls from the SMC function ID */
#define RMI_FNUM_MIN_VALUE	U(0x00)
#define RMI_FNUM_MAX_VALUE	U(0x20)
#define is_rmi_fid(fid) __extension__ ({		\
	__typeof__(fid) _fid = (fid);			\
	((GET_SMC_NUM(_fid) >= RMI_FNUM_MIN_VALUE) &&	\
	 (GET_SMC_NUM(_fid) <= RMI_FNUM_MAX_VALUE) &&	\
	 (GET_SMC_TYPE(_fid) == SMC_TYPE_FAST)	   &&	\
	 (GET_SMC_CC(_fid) == SMC_64)              &&	\
	 (GET_SMC_OEN(_fid) == OEN_ARM_START)      &&	\
	 ((_fid & 0x00FE0000) == 0U)); })

/* Get RMI fastcall std FID from function number */
#define RMI_FID(smc_cc, func_num)			\
	((SMC_TYPE_FAST << FUNCID_TYPE_SHIFT)	|	\
	((smc_cc) << FUNCID_CC_SHIFT)		|	\
	(OEN_ARM_START << FUNCID_OEN_SHIFT)	|	\
	((func_num) << FUNCID_NUM_SHIFT))

/*
 * SMC_RMM_INIT_COMPLETE is the only function in the RMI that originates from
 * the Realm world and is handled by the RMMD. The remaining functions are
 * always invoked by the Normal world, forwarded by RMMD and handled by the
 * RMM
 */
#define RMI_FNUM_REQ_COMPLETE		U(0x10)
#define RMI_FNUM_VERSION_REQ		U(0x00)

#define RMI_FNUM_GRAN_NS_REALM		U(0x01)
#define RMI_FNUM_GRAN_REALM_NS		U(0x02)

/* RMI SMC64 FIDs handled by the RMMD */
#define RMI_RMM_REQ_COMPLETE		RMI_FID(SMC_64, RMI_FNUM_REQ_COMPLETE)
#define RMI_RMM_REQ_VERSION		RMI_FID(SMC_64, RMI_FNUM_VERSION_REQ)

#define RMI_RMM_GRANULE_DELEGATE	RMI_FID(SMC_64, RMI_FNUM_GRAN_NS_REALM)
#define RMI_RMM_GRANULE_UNDELEGATE	RMI_FID(SMC_64, RMI_FNUM_GRAN_REALM_NS)


#define RMI_ABI_VERSION_GET_MAJOR(_version) ((_version) >> 16)
#define RMI_ABI_VERSION_GET_MINOR(_version) ((_version) & 0xFFFF)

/* Reserve a special value for MBZ parameters. */
#define RMI_PARAM_MBZ			U(0x0)

#endif /* RMI_SVC_H */
