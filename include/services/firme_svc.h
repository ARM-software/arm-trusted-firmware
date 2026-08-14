/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FIRME_SVC_H
#define FIRME_SVC_H

#include <stdint.h>

#include <common/sha_common_macros.h>
#include <lib/smccc.h>
#include <lib/utils_def.h>
#include <services/firme/firme_abi.h>
#include <smccc_helpers.h>

/* FIRME service versions currently supported */
/* Version 0.0 returns not supported */
#define FIRME_BASE_VERSION_MAJOR			U(1)
#define FIRME_BASE_VERSION_MINOR			U(0)

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

/* Base service feature register definitions */
#define FIRME_BASE_FEATURE_REG_COUNT			U(2)
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
#define FIRME_BASE_SERVICE_IDE_KM_BIT			FIRME_BASE_SERVICE_BIT(1)
#define FIRME_BASE_SERVICE_MECID_BIT			FIRME_BASE_SERVICE_BIT(2)

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
#define FIRME_SERVICE_VERSION_FID	FIRME_FID(U(0))

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
#define FIRME_SERVICE_FEATURES_FID	FIRME_FID(U(1))

/*
 * Platform hooks will return generic error codes, this helper converts to
 * FIRME error status code.
 */
int firme_errno_from_generic_errno(int errno);

int firme_init(void);

int plat_firme_get_supported_svcs(uint16_t *svc_mask);

/* Top level handler for FIRME SMC calls. */
uint64_t firme_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2, uint64_t x3,
		       uint64_t x4, void *cookie, void *handle, uint64_t flags);

#endif /* FIRME_SVC_H */
