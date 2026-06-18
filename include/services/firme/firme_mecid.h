/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FIRME_MEC_H
#define FIRME_MEC_H

#include <stdint.h>

#include <services/firme/firme_abi.h>

#define FIRME_MECID_MGMT_VERSION_MAJOR	U(1)
#define FIRME_MECID_MGMT_VERSION_MINOR	U(0)

#define FIRME_MEC_FNUM_REFRESH		U(0x7)

/*
 * FIRME_MEC_REFRESH
 *
 * This function refreshes the MEC linked to a given MECID by invalidating the
 * existing MEC, generating a new one, and associating the new MEC with that MECID.
 *
 * Supported from v1.0
 *
 * Arguments
 *	arg0(w0): Function ID 0xC4000407
 *	arg1(x1): MEC params Bits[63:48]: Reserved (SBZ).
 *			     Bits[47:32]: MECID.
 *			     Bits[31:1]: Reserved (SBZ).
 *			     Bits[0]: MEC refresh reason.
 *			     – 0b’0: Realm creation.
 *			     – 0b’1: Realm destruction
 *
 * Return
 *	ret0(x0): Status FIRME_SUCCESS
 *			 FIRME_NOT_SUPPORTED
 *			 FIRME_INVALID_PARAMETERS
 *			 FIRME_RETRY
 */
#define FIRME_MEC_REFRESH_FID			FIRME_FID(FIRME_MEC_FNUM_REFRESH)

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

int32_t firme_mecid_service_init(void);
int plat_firme_mec_refresh(uint16_t mecid, uint8_t reason);
uint8_t plat_firme_get_common_mecid_width(void);

#endif /* FIRME_MEC_H */
