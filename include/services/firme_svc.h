/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FIRME_SVC_H
#define FIRME_SVC_H

#include <common/sha_common_macros.h>
#include <lib/smccc.h>
#include <lib/utils_def.h>

/* FIRME service versions currently supported */
/* Version 0.0 returns not supported */
#define FIRME_BASE_VERSION_MAJOR			U(1)
#define FIRME_BASE_VERSION_MINOR			U(0)
#define FIRME_GRANULE_MGMT_VERSION_MAJOR		U(1)
#define FIRME_GRANULE_MGMT_VERSION_MINOR		U(0)
#define FIRME_IDE_KEY_MGMT_VERSION_MAJOR		U(0)
#define FIRME_IDE_KEY_MGMT_VERSION_MINOR		U(0)
#define FIRME_MECID_MGMT_VERSION_MAJOR			U(0)
#define FIRME_MECID_MGMT_VERSION_MINOR			U(0)
#define FIRME_ATTESTATION_VERSION_MAJOR			U(0)
#define FIRME_ATTESTATION_VERSION_MINOR			U(0)
#define FIRME_INTEGRATED_DEVICE_MGMT_VERSION_MAJOR	U(0)
#define FIRME_INTEGRATED_DEVICE_MGMT_VERSION_MINOR	U(0)

#define FIRME_VERSION_MAJOR_SHIFT			U(16)
#define FIRME_VERSION_MAJOR_MASK			U(0x7FFF)
#define FIRME_VERSION_MINOR_SHIFT			U(0)
#define FIRME_VERSION_MINOR_MASK			U(0xFFFF)
#define FIRME_VERSION(major, minor)			((((major) & FIRME_VERSION_MAJOR_MASK) << \
							FIRME_VERSION_MAJOR_SHIFT) | \
							(((minor) & FIRME_VERSION_MINOR_MASK) << \
							FIRME_VERSION_MINOR_SHIFT))


/* FIRME service ID definitions */
typedef enum {
	FIRME_BASE_ID = 0,
	FIRME_GRANULE_MGMT_ID = 1,
	FIRME_IDE_KEY_MGMT_ID = 2,
	FIRME_MECID_MGMT_ID = 3,
	FIRME_ATTESTATION_ID = 4,
	FIRME_INTEGRATED_DEVICE_MGMT_ID = 5,
	FIRME_SERVICE_ID_MAX = 6,
} firme_service_id_e;

typedef enum {
	FIRME_SECURE = 0,
	FIRME_NONSECURE = 1,
	FIRME_REALM = 2
} firme_instance_e;

typedef struct {
	uint32_t version;
	uint8_t instance_support;
	uint8_t num_feature_regs;
	uint64_t feature_reg[16];
} firme_service_info_t;

/* FIRME SMC return codes */
#define FIRME_SUCCESS			0
#define FIRME_NOT_SUPPORTED		-1
#define FIRME_INVALID_PARAMETERS	-2
#define FIRME_ABORTED			-3
#define FIRME_INCOMPLETE		-4
#define FIRME_DENIED			-5
#define FIRME_RETRY			-6
#define FIRME_IN_PROGRESS		-7
#define FIRME_EXISTS			-8
#define FIRME_NO_ENTRY			-9
#define FIRME_NO_MEMORY			-10
#define FIRME_BAD_DATA			-11

/* Range of function IDs used by FIRME interface */
#define FIRME_FNUM_MIN_VALUE		U(0x400)
#define FIRME_FNUM_MAX_VALUE		U(0x412)

/* Construct FIRME fastcall std FID from offset */
#define SMC64_FIRME_FID(_offset)                                              \
	((SMC_TYPE_FAST << FUNCID_TYPE_SHIFT) | (SMC_64 << FUNCID_CC_SHIFT) | \
	 (OEN_STD_START << FUNCID_OEN_SHIFT) |                                \
	 (((FIRME_FNUM_MIN_VALUE + (_offset)) & FUNCID_NUM_MASK)              \
	  << FUNCID_NUM_SHIFT))

#define is_firme_fid(fid)                                       \
	__extension__({                                         \
		__typeof__(fid) _fid = (fid);                   \
		((GET_SMC_NUM(_fid) >= FIRME_FNUM_MIN_VALUE) && \
		 (GET_SMC_NUM(_fid) <= FIRME_FNUM_MAX_VALUE) && \
		 (GET_SMC_TYPE(_fid) == SMC_TYPE_FAST) &&       \
		 (GET_SMC_CC(_fid) == SMC_64) &&                \
		 (GET_SMC_OEN(_fid) == OEN_STD_START) &&        \
		 ((_fid & 0x00FE0000) == 0U));                  \
	})

/* FIRME Granule Transition Bit Fields */

#define FIRME_GM_GPI_SET_TGT_GPI_SHIFT			U(0)
#define FIRME_GM_GPI_SET_TGT_GPI_MASK			U(0xF)

/* Base service feature register definitions */
#define FIRME_BASE_VERSION_BIT				BIT(0)
#define FIRME_BASE_FEATURES_BIT				BIT(1)
#define FIRME_BASE_MIN_SH_BUF_SZ_SHIFT			U(0)
#define FIRME_BASE_MIN_SH_BUF_SZ_MASK			U(0x3)
#define FIRME_BASE_MAX_SH_BUF_PG_CNT_SHIFT		U(2)
#define FIRME_BASE_MAX_SH_BUF_PG_CNT_MASK		U(0x3FFF)
#define FIRME_BASE_SERVICE_LIST_SHIFT			U(16)
#define FIRME_BASE_SERVICE_LIST_MASK			U(0xFFFF)
#define FIRME_BASE_SERVICE_BIT(_id)			BIT((_id) + \
							    FIRME_BASE_SERVICE_LIST_SHIFT)
#define FIRME_BASE_SERVICE_GRANULE_MGMT_BIT		FIRME_BASE_SERVICE_BIT(0)
#define FIRME_BASE_SERVICE_MECID_BIT			FIRME_BASE_SERVICE_BIT(2)

/* Granule management service feature register definitions. */
#define FIRME_GM_GPI_SET_BIT				BIT(0)
#define FIRME_GM_GPI_OP_CONTINUE_BIT			BIT(1)
#define FIRME_GM_L1_CREATE_BIT				BIT(2)
#define FIRME_GM_L1_DESTROY_BIT				BIT(3)
#define FIRME_GM_PPS_SHIFT				U(6)
#define FIRME_GM_PPS_MASK				U(0x7)
#define FIRME_GM_L0GPTSZ_SHIFT				U(2)
#define FIRME_GM_L0GPTSZ_MASK				U(0xF)
#define FIRME_GM_PGS_SHIFT				U(0)
#define FIRME_GM_PGS_MASK				U(0x3)

/*
 * FIRME_SERVICE_VERSION
 *
 * This function returns the supported version for a specified
 * FIRME service.
 *
 * Arguments
 *    arg0(w0): Function ID 0xC4000400
 *    arg1(x1): Service ID  bits[63:0] Unused
 *                          bits[7:0]  ID of FIRME service
 *
 * Return
 *    ret0(w0): Success  bits[31]    Set to indicate negative value
 *                       bits[30:16] Major version
 *                       bits[15:0]  Minor version
 *              Failure  Negative value encodes FIRME_NOT_SUPPORTED
 */
#define FIRME_SERVICE_VERSION_FID	SMC64_FIRME_FID(U(0))

/*
 * FIRME_FEATURES
 *
 * This function returns the features supported by the FIRME interface.
 *
 * Supported from v1.0
 *
 * Arguments
 *    arg0(w0): Function ID 0xC4000401.
 *    arg1(w1): Index       Index of the feature register to be returned to the
 *                          caller. Current supported values are 0, 1, and 2.
 *                          All others reserved.
 *
 * Return
 *    ret0(x0): Status      FIRME_SUCCESS
 *                          FIRME_NOT_SUPPORTED
 *                          FIRME_INVALID_PARAMETERS
 *    ret1(x1): Requested feature register if successful.
 */
#define FIRME_SERVICE_FEATURES_FID	SMC64_FIRME_FID(U(1))

/*
 * FIRME_GM_GPI_SET
 *
 * This function requests a GPI transition for a set of physically contiguous
 * granules of memory.
 *
 * Supported from v1.0
 *
 * Arguments
 *    arg0(w0): Function ID   0xC4000402
 *    arg1(x1): Base Address  Physical base address of the granules whose GPI
 *                            encoding must be changed.
 *    arg2(x2): Granule Count Number of granules GPI to change starting from
 *                            base address.
 *    arg3(x3): Attributes    bits[63:4] Reserved
 *                            bits[3:0]  Target GPI encoding of the granules.
 *
 * Return
 *    ret0(x0): Status        FIRME_SUCCESS
 *                            FIRME_NOT_SUPPORTED
 *                            FIRME_INVALID_PARAMETERS
 *                            FIRME_DENIED
 *                            FIRME_RETRY
 *    ret1(x1): Count of granules whose GPI encoding was change.
 */
#define FIRME_GM_GPI_SET_FID		SMC64_FIRME_FID(U(0x2))

/* These are unimplemented so far and will be added in the future. */

/* Granule management service ABIs */
#define FIRME_GM_GPI_OP_CONTINUE_FID		SMC64_FIRME_FID(U(0x12))
#define FIRME_GM_L1_GPT_CREATE_FID		SMC64_FIRME_FID(U(0xE))
#define FIRME_GM_L1_GPT_DESTROY_FID		SMC64_FIRME_FID(U(0xF))

/* IDE key management service */
#define FIRME_IDE_KEYSET_PROG_FID		SMC64_FIRME_FID(U(0x3))
#define FIRME_IDE_KEYSET_GO_FID			SMC64_FIRME_FID(U(0x4))
#define FIRME_IDE_KEYSET_STOP_FID		SMC64_FIRME_FID(U(0x5))
#define FIRME_IDE_KEYSET_POLL_FID		SMC64_FIRME_FID(U(0x6))

/* MECID management service */
#define FIRME_MEC_REFRESH_FID			SMC64_FIRME_FID(U(0x7))

#define MEC_REFRESH_REASON_REALM_CREATE		U(0)
#define MEC_REFRESH_REASON_REALM_DESTROY	U(1)

#define MEC_PARAM_MECID_SHIFT			U(32)
#define MEC_PARAM_MECID_WIDTH			U(16)
#define MEC_PARAM_MECID_MASK			MASK(MEC_PARAM_MECID)

#define FIRME_MECID_FEATURE_REG_COUNT		U(2)
#define FIRME_MECID_FEAT_REG0_MEC_REFRESH_BIT	BIT(0)
#define FIRME_MECID_FEAT_REG1_COMMON_MECID_WIDTH_BITS_SHIFT	U(0)
#define FIRME_MECID_FEAT_REG1_COMMON_MECID_WIDTH_BITS_WIDTH	U(4)
#define FIRME_MECID_FEAT_REG1_COMMON_MECID_WIDTH_BITS_MASK	MASK(FIRME_MECID_FEAT_REG1_COMMON_MECID_WIDTH_BITS)

/* Attestation service */
#define FIRME_ATTEST_PAT_GET_FID		SMC64_FIRME_FID(U(0x8))
#define FIRME_ATTEST_RAK_GET_FID		SMC64_FIRME_FID(U(0x9))
#define FIRME_ATTEST_RAT_SIGN_FID		SMC64_FIRME_FID(U(0xA))
#define FIRME_ATTEST_PAT_EXT_CLAIMS_STAGE_FID	SMC64_FIRME_FID(U(0xB))
#define FIRME_ATTEST_PAT_EXT_CLAIMS_CLEAR_FID	SMC64_FIRME_FID(U(0xC))
#define FIRME_ATTEST_PAT_EXT_CLAIMS_FINALISE_FID	SMC64_FIRME_FID(U(0xD))

/* Integrated device management service */
#define FIRME_IDEV_OP_START_FID			SMC64_FIRME_FID(U(0x10))
#define FIRME_IDEV_OP_CONTINUE_FID		SMC64_FIRME_FID(U(0x11))

/* Top level handler for FIRME SMC calls. */
int32_t firme_init(void);

uint64_t firme_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2, uint64_t x3,
		       uint64_t x4, void *cookie, void *handle, uint64_t flags);

firme_service_info_t *firme_granule_mgmt_service_get_info(void);
firme_service_info_t *firme_mecid_service_get_info(void);

int32_t firme_mecid_service_init(void);
int plat_firme_mec_refresh(uint16_t mecid, uint8_t reason);
uint8_t plat_firme_get_common_mecid_width(void);

u_register_t firme_base_service_handler(firme_instance_e instance, uint32_t smc_fid,
					uint64_t x1, uint64_t x2, uint64_t x3,
					uint64_t x4, void *cookie, void *handle,
					uint64_t flags);

u_register_t firme_granule_mgmt_service_handler(firme_instance_e instance,
						uint32_t smc_fid, uint64_t x1,
						uint64_t x2, uint64_t x3,
						uint64_t x4, void *cookie,
						void *handle, uint64_t flags);

u_register_t firme_mecid_service_handler(firme_instance_e instance,
					 uint32_t smc_fid, uint64_t x1,
					 uint64_t x2, uint64_t x3,
					 uint64_t x4, void *cookie,
					 void *handle, uint64_t flags);

#endif /* FIRME_SVC_H */
