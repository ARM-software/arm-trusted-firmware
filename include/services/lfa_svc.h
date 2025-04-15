/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LFA_SVC_H
#define LFA_SVC_H

#include <lib/smccc.h>

/*
 * SMC function IDs for LFA Service
 * Upper word bits set: Fast call, SMC64, Standard Secure Svc. Call (OEN = 4)
 */
#define LFA_FID(func_num)				\
	((SMC_TYPE_FAST << FUNCID_TYPE_SHIFT) |		\
	(SMC_64 << FUNCID_CC_SHIFT) |			\
	(OEN_STD_START << FUNCID_OEN_SHIFT) |		\
	((func_num) << FUNCID_NUM_SHIFT))

#define LFA_VERSION			LFA_FID(0x2E0)
#define LFA_FEATURES			LFA_FID(0x2E1)
#define LFA_GET_INFO			LFA_FID(0x2E2)
#define LFA_GET_INVENTORY		LFA_FID(0x2E3)
#define LFA_PRIME			LFA_FID(0x2E4)
#define LFA_ACTIVATE			LFA_FID(0x2E5)
#define LFA_CANCEL			LFA_FID(0x2E6)

/* Check whether FID is in the range */
#define is_lfa_fid(_fid)	\
	((_fid >= LFA_VERSION) && (_fid <= LFA_CANCEL))

/* LFA Service Calls version numbers */
#define LFA_VERSION_MAJOR		U(1)
#define LFA_VERSION_MAJOR_SHIFT		16
#define LFA_VERSION_MAJOR_MASK		U(0x7FFF)
#define LFA_VERSION_MINOR		U(0)
#define LFA_VERSION_MINOR_SHIFT		0
#define LFA_VERSION_MINOR_MASK		U(0xFFFF)

#define LFA_VERSION_VAL						\
	((((LFA_VERSION_MAJOR) & LFA_VERSION_MAJOR_MASK) <<	\
	LFA_VERSION_MAJOR_SHIFT)				\
	| (((LFA_VERSION_MINOR) & LFA_VERSION_MINOR_MASK) <<	\
	LFA_VERSION_MINOR_SHIFT))

/* List of errors as per the specification */
enum lfa_retc {
	LFA_SUCCESS			=  0,
	LFA_NOT_SUPPORTED		= -1,
	LFA_BUSY			= -2,
	LFA_AUTH_ERROR			= -3,
	LFA_NO_MEMORY			= -4,
	LFA_CRITICAL_ERROR		= -5,
	LFA_DEVICE_ERROR		= -6,
	LFA_WRONG_STATE			= -7,
	LFA_INVALID_PARAMETERS		= -8,
	LFA_COMPONENT_WRONG_STATE	= -9,
	LFA_INVALID_ADDRESS		= -10,
	LFA_ACTIVATION_FAILED		= -11,
};

/* Initialization routine for the LFA service */
int lfa_setup(void);

uint64_t lfa_smc_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
			 u_register_t x3, u_register_t x4, void *cookie,
			 void *handle, u_register_t flags);

#endif /* LFA_SVC_H */
