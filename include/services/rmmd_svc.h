/*
 * Copyright (c) 2021-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RMMD_SVC_H
#define RMMD_SVC_H

#include <lib/smccc.h>
#include <lib/utils_def.h>

/* Construct RMM fastcall std FID from function number */
#define RMM_FID(smc_cc, func_num)			\
	((SMC_TYPE_FAST << FUNCID_TYPE_SHIFT)	|	\
	((smc_cc) << FUNCID_CC_SHIFT)		|	\
	(OEN_STD_START << FUNCID_OEN_SHIFT)	|	\
	((func_num) << FUNCID_NUM_SHIFT))

/* The macros below are used to identify RMI calls from the SMC function ID */
#define RMI_FNUM_MIN_VALUE	U(0x150)
#define RMI_FNUM_MAX_VALUE	U(0x18F)

#define is_rmi_fid(fid) __extension__ ({		\
	__typeof__(fid) _fid = (fid);			\
	((GET_SMC_NUM(_fid) >= RMI_FNUM_MIN_VALUE) &&	\
	 (GET_SMC_NUM(_fid) <= RMI_FNUM_MAX_VALUE) &&	\
	 (GET_SMC_TYPE(_fid) == SMC_TYPE_FAST)	   &&	\
	 (GET_SMC_CC(_fid) == SMC_64)              &&	\
	 (GET_SMC_OEN(_fid) == OEN_STD_START)      &&	\
	 ((_fid & 0x00FE0000) == 0U)); })

/*
 * RMI_FNUM_REQ_COMPLETE is the only function in the RMI rnage that originates
 * from the Realm world and is handled by the RMMD. The RMI functions are
 * always invoked by the Normal world, forwarded by RMMD and handled by the
 * RMM
 */
#define RMI_FNUM_REQ_COMPLETE		U(0x18F)
#define RMMD_RMI_REQ_COMPLETE		RMM_FID(SMC_64, RMI_FNUM_REQ_COMPLETE)

/* The SMC in the range 0x8400 0190 - 0x8400 01AF are reserved for RSIs.*/

/*
 * EL3 - RMM SMCs used for requesting RMMD services. These SMCs originate in Realm
 * world and return to Realm world.
 *
 * These are allocated from 0x8400 01B0 - 0x8400 01CF in the RMM Service range.
 */
#define RMMD_EL3_FNUM_MIN_VALUE		U(0x1B0)
#define RMMD_EL3_FNUM_MAX_VALUE		U(0x1CF)

/* The macros below are used to identify GTSI calls from the SMC function ID */
#define is_rmmd_el3_fid(fid) __extension__ ({		\
	__typeof__(fid) _fid = (fid);			\
	((GET_SMC_NUM(_fid) >= RMMD_EL3_FNUM_MIN_VALUE) &&\
	(GET_SMC_NUM(_fid) <= RMMD_EL3_FNUM_MAX_VALUE)  &&\
	(GET_SMC_TYPE(_fid) == SMC_TYPE_FAST)	    &&	\
	(GET_SMC_CC(_fid) == SMC_64)                &&	\
	(GET_SMC_OEN(_fid) == OEN_STD_START)        &&	\
	((_fid & 0x00FE0000) == 0U)); })

/* RMMD Service Function NUmbers */
#define GTSI_DELEGATE			U(0x1B0)
#define GTSI_UNDELEGATE			U(0x1B1)
#define ATTEST_GET_REALM_KEY		U(0x1B2)
#define ATTEST_GET_PLAT_TOKEN		U(0x1B3)

#define RMMD_GTSI_DELEGATE		RMM_FID(SMC_64, GTSI_DELEGATE)
#define RMMD_GTSI_UNDELEGATE		RMM_FID(SMC_64, GTSI_UNDELEGATE)

/* Return error codes from RMM-EL3 SMCs */
#define RMMD_OK				0
#define RMMD_ERR_BAD_ADDR		-2
#define RMMD_ERR_BAD_PAS		-3
#define RMMD_ERR_NOMEM			-4
#define RMMD_ERR_INVAL			-5
#define RMMD_ERR_UNK			-6

/*
 * Retrieve Platform token from EL3.
 * The arguments to this SMC are :
 *    arg0 - Function ID.
 *    arg1 - Platform attestation token buffer Physical address. (The challenge
 *           object is passed in this buffer.)
 *    arg2 - Platform attestation token buffer size (in bytes).
 *    arg3 - Challenge object size (in bytes). It has be one of the defined SHA hash
 *           sizes.
 * The return arguments are :
 *    ret0 - Status / error.
 *    ret1 - Size of the platform token if successful.
 */
#define RMMD_ATTEST_GET_PLAT_TOKEN	RMM_FID(SMC_64, ATTEST_GET_PLAT_TOKEN)

/* Acceptable SHA sizes for Challenge object */
#define SHA256_DIGEST_SIZE	32U
#define SHA384_DIGEST_SIZE	48U
#define SHA512_DIGEST_SIZE	64U

/*
 * Retrieve Realm attestation key from EL3. Only P-384 ECC curve key is
 * supported. The arguments to this SMC are :
 *    arg0 - Function ID.
 *    arg1 - Realm attestation key buffer Physical address.
 *    arg2 - Realm attestation key buffer size (in bytes).
 *    arg3 - The type of the elliptic curve to which the requested
 *           attestation key belongs to. The value should be one of the
 *           defined curve types.
 * The return arguments are :
 *    ret0 - Status / error.
 *    ret1 - Size of the realm attestation key if successful.
 */
#define RMMD_ATTEST_GET_REALM_KEY	RMM_FID(SMC_64, ATTEST_GET_REALM_KEY)

/* ECC Curve types for attest key generation */
#define ATTEST_KEY_CURVE_ECC_SECP384R1		0


#ifndef __ASSEMBLER__
#include <stdint.h>

int rmmd_setup(void);
uint64_t rmmd_rmi_handler(uint32_t smc_fid,
		uint64_t x1,
		uint64_t x2,
		uint64_t x3,
		uint64_t x4,
		void *cookie,
		void *handle,
		uint64_t flags);

uint64_t rmmd_rmm_el3_handler(uint32_t smc_fid,
		uint64_t x1,
		uint64_t x2,
		uint64_t x3,
		uint64_t x4,
		void *cookie,
		void *handle,
		uint64_t flags);

#endif /* __ASSEMBLER__ */
#endif /* RMMD_SVC_H */
