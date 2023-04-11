/*
 * Copyright (c) 2024, The ChromiumOS Authors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CROS_WIDEVINE_SMC_HANDLERS_H
#define CROS_WIDEVINE_SMC_HANDLERS_H

#include <lib/smccc.h>

/*******************************************************************************
 * Defines for CrOS OEM Service queries
 ******************************************************************************/

/* 0xC300C050 - 0xC300C05F are CrOS OEM service calls */
#define CROS_OEM_SMC_ID 0xC050
#define CROS_OEM_SMC_CALL_ID(func_num)                                         \
	((SMC_TYPE_FAST << FUNCID_TYPE_SHIFT) |                                \
	 ((SMC_64) << FUNCID_CC_SHIFT) | (OEN_OEM_START << FUNCID_OEN_SHIFT) | \
	 (CROS_OEM_SMC_ID) | ((func_num) & FUNCID_NUM_MASK))

enum cros_drm_set {
	CROS_DRM_SET_TPM_AUTH_PUB = 0U,
	CROS_DRM_SET_HARDWARE_UNIQUE_KEY = 1U,
	CROS_DRM_SET_ROOT_OF_TRUST = 2U,
};

/*******************************************************************************
 * Defines for runtime services func ids
 ******************************************************************************/

/* Sets the TPM auth public key. The maximum size is 128 bytes.
 * |x1| is the length of the data, |x2| is the physical address of the data.
 */
#define CROS_OEM_SMC_DRM_SET_TPM_AUTH_PUB_FUNC_ID \
	CROS_OEM_SMC_CALL_ID(CROS_DRM_SET_TPM_AUTH_PUB)

/* Sets the hardware unique key. The maximum size is 32 bytes.
 * |x1| is the length of the data, |x2| is the physical address of the data.
 */
#define CROS_OEM_SMC_DRM_SET_HARDWARE_UNIQUE_KEY_FUNC_ID \
	CROS_OEM_SMC_CALL_ID(CROS_DRM_SET_HARDWARE_UNIQUE_KEY)

/* Sets the widevine root of trust. The maximum size is 32 bytes.
 * |x1| is the length of the data, |x2| is the physical address of the data.
 */
#define CROS_OEM_SMC_DRM_SET_ROOT_OF_TRUST_FUNC_ID \
	CROS_OEM_SMC_CALL_ID(CROS_DRM_SET_ROOT_OF_TRUST)

#define is_cros_oem_smc(_call_id) (((_call_id) & 0xFFF0U) == CROS_OEM_SMC_ID)

struct cros_oem_data {
	uint8_t *buffer;
	const uint32_t max_length;
	uint32_t length;
};

extern struct cros_oem_data cros_oem_tpm_auth_pk;

extern struct cros_oem_data cros_oem_huk;

extern struct cros_oem_data cros_oem_rot;

#endif /* CROS_WIDEVINE_SMC_HANDLERS_H */
