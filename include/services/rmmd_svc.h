/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RMMD_SVC_H
#define RMMD_SVC_H

#include <lib/smccc.h>
#include <lib/utils_def.h>

/* STD calls FNUM Min/Max ranges */
#define RMI_FNUM_MIN_VALUE	U(0x150)
#define RMI_FNUM_MAX_VALUE	U(0x18F)

/* Construct RMI fastcall std FID from offset */
#define SMC64_RMI_FID(_offset)					  \
	((SMC_TYPE_FAST << FUNCID_TYPE_SHIFT)			| \
	 (SMC_64 << FUNCID_CC_SHIFT)				| \
	 (OEN_STD_START << FUNCID_OEN_SHIFT)			| \
	 (((RMI_FNUM_MIN_VALUE + (_offset)) & FUNCID_NUM_MASK)	  \
	  << FUNCID_NUM_SHIFT))

#define is_rmi_fid(fid) __extension__ ({		\
	__typeof__(fid) _fid = (fid);			\
	((GET_SMC_NUM(_fid) >= RMI_FNUM_MIN_VALUE) &&	\
	 (GET_SMC_NUM(_fid) <= RMI_FNUM_MAX_VALUE) &&	\
	 (GET_SMC_TYPE(_fid) == SMC_TYPE_FAST)	   &&	\
	 (GET_SMC_CC(_fid) == SMC_64)              &&	\
	 (GET_SMC_OEN(_fid) == OEN_STD_START)      &&	\
	 ((_fid & 0x00FE0000) == 0U)); })

/*
 * RMI_FNUM_REQ_COMPLETE is the only function in the RMI range that originates
 * from the Realm world and is handled by the RMMD. The RMI functions are
 * always invoked by the Normal world, forwarded by RMMD and handled by the
 * RMM.
 */
					/* 0x18F */
#define RMM_RMI_REQ_COMPLETE		SMC64_RMI_FID(U(0x3F))

/* RMM_BOOT_COMPLETE arg0 error codes */
#define E_RMM_BOOT_SUCCESS				(0)
#define E_RMM_BOOT_UNKNOWN				(-1)
#define E_RMM_BOOT_VERSION_MISMATCH			(-2)
#define E_RMM_BOOT_CPUS_OUT_OF_RANGE			(-3)
#define E_RMM_BOOT_CPU_ID_OUT_OF_RANGE			(-4)
#define E_RMM_BOOT_INVALID_SHARED_BUFFER		(-5)
#define E_RMM_BOOT_MANIFEST_VERSION_NOT_SUPPORTED	(-6)
#define E_RMM_BOOT_MANIFEST_DATA_ERROR			(-7)

/* The SMC in the range 0x8400 0191 - 0x8400 01AF are reserved for RSIs.*/

/*
 * EL3 - RMM SMCs used for requesting RMMD services. These SMCs originate in Realm
 * world and return to Realm world.
 *
 * These are allocated from 0x8400 01B0 - 0x8400 01CF in the RMM Service range.
 */
#define RMMD_EL3_FNUM_MIN_VALUE		U(0x1B0)
#define RMMD_EL3_FNUM_MAX_VALUE		U(0x1CF)

/* Construct RMM_EL3 fastcall std FID from offset */
#define SMC64_RMMD_EL3_FID(_offset)					  \
	((SMC_TYPE_FAST << FUNCID_TYPE_SHIFT)				| \
	 (SMC_64 << FUNCID_CC_SHIFT)					| \
	 (OEN_STD_START << FUNCID_OEN_SHIFT)				| \
	 (((RMMD_EL3_FNUM_MIN_VALUE + (_offset)) & FUNCID_NUM_MASK)	  \
	  << FUNCID_NUM_SHIFT))

/* The macros below are used to identify GTSI calls from the SMC function ID */
#define is_rmmd_el3_fid(fid) __extension__ ({		\
	__typeof__(fid) _fid = (fid);			\
	((GET_SMC_NUM(_fid) >= RMMD_EL3_FNUM_MIN_VALUE) &&\
	(GET_SMC_NUM(_fid) <= RMMD_EL3_FNUM_MAX_VALUE)  &&\
	(GET_SMC_TYPE(_fid) == SMC_TYPE_FAST)	    &&	\
	(GET_SMC_CC(_fid) == SMC_64)                &&	\
	(GET_SMC_OEN(_fid) == OEN_STD_START)        &&	\
	((_fid & 0x00FE0000) == 0U)); })

					/* 0x1B0 - 0x1B1 */
#define RMM_GTSI_DELEGATE		SMC64_RMMD_EL3_FID(U(0))
#define RMM_GTSI_UNDELEGATE		SMC64_RMMD_EL3_FID(U(1))

/* Return error codes from RMM-EL3 SMCs */
#define E_RMM_OK			 0
#define E_RMM_UNK			-1
#define E_RMM_BAD_ADDR			-2
#define E_RMM_BAD_PAS			-3
#define E_RMM_NOMEM			-4
#define E_RMM_INVAL			-5

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
					/* 0x1B2 */
#define RMM_ATTEST_GET_REALM_KEY	SMC64_RMMD_EL3_FID(U(2))

/*
 * Retrieve Platform token from EL3.
 * The arguments to this SMC are :
 *    arg0 - Function ID.
 *    arg1 - Platform attestation token buffer Physical address. (The challenge
 *           object is passed in this buffer.)
 *    arg2 - Platform attestation token buffer size (in bytes).
 *    arg3 - Challenge object size (in bytes). It has to be one of the defined
 *           SHA hash sizes.
 * The return arguments are :
 *    ret0 - Status / error.
 *    ret1 - Size of the platform token if successful.
 */
					/* 0x1B3 */
#define RMM_ATTEST_GET_PLAT_TOKEN	SMC64_RMMD_EL3_FID(U(3))

/* ECC Curve types for attest key generation */
#define ATTEST_KEY_CURVE_ECC_SECP384R1		0

/*
 * RMM_BOOT_COMPLETE originates on RMM when the boot finishes (either cold
 * or warm boot). This is handled by the RMM-EL3 interface SMC handler.
 *
 * RMM_BOOT_COMPLETE FID is located at the end of the available range.
 */
					 /* 0x1CF */
#define RMM_BOOT_COMPLETE		SMC64_RMMD_EL3_FID(U(0x1F))

/*
 * The major version number of the RMM Boot Interface implementation.
 * Increase this whenever the semantics of the boot arguments change making it
 * backwards incompatible.
 */
#define RMM_EL3_IFC_VERSION_MAJOR	(U(0))

/*
 * The minor version number of the RMM Boot Interface implementation.
 * Increase this when a bug is fixed, or a feature is added without
 * breaking compatibility.
 */
#define RMM_EL3_IFC_VERSION_MINOR	(U(1))

#define RMM_EL3_INTERFACE_VERSION				\
	(((RMM_EL3_IFC_VERSION_MAJOR << 16) & 0x7FFFF) |	\
		RMM_EL3_IFC_VERSION_MINOR)

#define RMM_EL3_IFC_VERSION_GET_MAJOR(_version) (((_version) >> 16) \
								& 0x7FFF)
#define RMM_EL3_IFC_VERSION_GET_MAJOR_MINOR(_version) ((_version) & 0xFFFF)

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
