/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier:    BSD-3-Clause
 *
 * DRTM service
 *
 * Authors:
 *	Lucian Paul-Trifu <lucian.paultrifu@gmail.com>
 *	Brian Nezvadovitz <brinez@microsoft.com> 2021-02-01
 *
 */

#ifndef ARM_DRTM_SVC_H
#define ARM_DRTM_SVC_H

/*
 * SMC function IDs for DRTM Service
 * Upper word bits set: Fast call, SMC64, Standard Secure Svc. Call (OEN = 4)
 */
#define DRTM_FID(func_num)				\
	((SMC_TYPE_FAST << FUNCID_TYPE_SHIFT) |		\
	(SMC_64 << FUNCID_CC_SHIFT) |			\
	(OEN_STD_START << FUNCID_OEN_SHIFT) |		\
	((func_num) << FUNCID_NUM_SHIFT))

#define DRTM_FNUM_SVC_VERSION		U(0x110)
#define DRTM_FNUM_SVC_FEATURES		U(0x111)
#define DRTM_FNUM_SVC_UNPROTECT_MEM	U(0x113)
#define DRTM_FNUM_SVC_DYNAMIC_LAUNCH	U(0x114)
#define DRTM_FNUM_SVC_CLOSE_LOCALITY	U(0x115)
#define DRTM_FNUM_SVC_GET_ERROR		U(0x116)
#define DRTM_FNUM_SVC_SET_ERROR		U(0x117)
#define DRTM_FNUM_SVC_SET_TCB_HASH	U(0x118)
#define DRTM_FNUM_SVC_LOCK_TCB_HASH	U(0x119)

#define ARM_DRTM_SVC_VERSION		DRTM_FID(DRTM_FNUM_SVC_VERSION)
#define ARM_DRTM_SVC_FEATURES		DRTM_FID(DRTM_FNUM_SVC_FEATURES)
#define ARM_DRTM_SVC_UNPROTECT_MEM	DRTM_FID(DRTM_FNUM_SVC_UNPROTECT_MEM)
#define ARM_DRTM_SVC_DYNAMIC_LAUNCH	DRTM_FID(DRTM_FNUM_SVC_DYNAMIC_LAUNCH)
#define ARM_DRTM_SVC_CLOSE_LOCALITY	DRTM_FID(DRTM_FNUM_SVC_CLOSE_LOCALITY)
#define ARM_DRTM_SVC_GET_ERROR		DRTM_FID(DRTM_FNUM_SVC_GET_ERROR)
#define ARM_DRTM_SVC_SET_ERROR		DRTM_FID(DRTM_FNUM_SVC_SET_ERROR)
#define ARM_DRTM_SVC_SET_TCB_HASH	DRTM_FID(DRTM_FNUM_SVC_SET_TCB_HASH)
#define ARM_DRTM_SVC_LOCK_TCB_HASH	DRTM_FID(DRTM_FNUM_SVC_LOCK_TCB_HASH)

#define is_drtm_fid(_fid) \
	(((_fid) >= ARM_DRTM_SVC_VERSION) && ((_fid) <= ARM_DRTM_SVC_LOCK_TCB_HASH))

/* ARM DRTM Service Calls version numbers */
#define ARM_DRTM_VERSION_MAJOR		U(0)
#define ARM_DRTM_VERSION_MAJOR_SHIFT	16
#define ARM_DRTM_VERSION_MAJOR_MASK	U(0x7FFF)
#define ARM_DRTM_VERSION_MINOR		U(1)
#define ARM_DRTM_VERSION_MINOR_SHIFT	0
#define ARM_DRTM_VERSION_MINOR_MASK	U(0xFFFF)

#define ARM_DRTM_VERSION						\
	((((ARM_DRTM_VERSION_MAJOR) & ARM_DRTM_VERSION_MAJOR_MASK) <<	\
	ARM_DRTM_VERSION_MAJOR_SHIFT)					\
	| (((ARM_DRTM_VERSION_MINOR) & ARM_DRTM_VERSION_MINOR_MASK) <<	\
	ARM_DRTM_VERSION_MINOR_SHIFT))

#define ARM_DRTM_FUNC_SHIFT	U(63)
#define ARM_DRTM_FUNC_MASK	U(0x1)
#define ARM_DRTM_FUNC_ID	U(0x0)
#define ARM_DRTM_FEAT_ID	U(0x1)

/* Initialization routine for the DRTM service */
int drtm_setup(void);

/* Handler to be called to handle DRTM SMC calls */
uint64_t drtm_smc_handler(uint32_t smc_fid,
			  uint64_t x1,
			  uint64_t x2,
			  uint64_t x3,
			  uint64_t x4,
			  void *cookie,
			  void *handle,
			  uint64_t flags);

#endif /* ARM_DRTM_SVC_H */
