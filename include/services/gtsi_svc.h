/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GTSI_SVC_H
#define GTSI_SVC_H

/* GTSI error codes. */
#define GTSI_SUCCESS			0
#define GTSI_ERROR_NOT_SUPPORTED	-1
#define GTSI_ERROR_INVALID_ADDRESS	-2
#define GTSI_ERROR_INVALID_PAS		-3

/* The macros below are used to identify GTSI calls from the SMC function ID */
#define GTSI_FNUM_MIN_VALUE	U(0x100)
#define GTSI_FNUM_MAX_VALUE	U(0x101)
#define is_gtsi_fid(fid) __extension__ ({		\
	__typeof__(fid) _fid = (fid);			\
	((GET_SMC_NUM(_fid) >= GTSI_FNUM_MIN_VALUE) &&	\
	 (GET_SMC_NUM(_fid) <= GTSI_FNUM_MAX_VALUE)); })

/* Get GTSI fastcall std FID from function number */
#define GTSI_FID(smc_cc, func_num)			\
	((SMC_TYPE_FAST << FUNCID_TYPE_SHIFT)	|	\
	 ((smc_cc) << FUNCID_CC_SHIFT)		|	\
	 (OEN_STD_START << FUNCID_OEN_SHIFT)	|	\
	 ((func_num) << FUNCID_NUM_SHIFT))

#define GRAN_TRANS_TO_REALM_FNUM	U(0x100)
#define GRAN_TRANS_TO_NS_FNUM		U(0x101)

#define SMC_ASC_MARK_REALM	GTSI_FID(SMC_64, GRAN_TRANS_TO_REALM_FNUM)
#define SMC_ASC_MARK_NONSECURE	GTSI_FID(SMC_64, GRAN_TRANS_TO_NS_FNUM)

#define GRAN_TRANS_RET_BAD_ADDR		-2
#define GRAN_TRANS_RET_BAD_PAS		-3

#endif /* GTSI_SVC_H */
