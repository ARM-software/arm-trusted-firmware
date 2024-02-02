/*
 * Copyright (c) 2024, The ChromiumOS Authors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/psci/psci.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <services/oem/chromeos/widevine_smc_handlers.h>
#include <tools_share/uuid.h>

#define CROS_OEM_TPM_AUTH_PK_MAX_LEN 128
#define CROS_OEM_HUK_LEN 32
#define CROS_OEM_ROT_LEN 32

static uint8_t cros_oem_tpm_auth_pk_buffer[CROS_OEM_TPM_AUTH_PK_MAX_LEN];
static uint8_t cros_oem_huk_buffer[CROS_OEM_HUK_LEN];
static uint8_t cros_oem_rot_len_buffer[CROS_OEM_ROT_LEN];

struct cros_oem_data cros_oem_tpm_auth_pk = {
	.buffer = cros_oem_tpm_auth_pk_buffer,
	.max_length = sizeof(cros_oem_tpm_auth_pk_buffer),
};

struct cros_oem_data cros_oem_huk = {
	.buffer = cros_oem_huk_buffer,
	.max_length = sizeof(cros_oem_huk_buffer),
};

struct cros_oem_data cros_oem_rot = {
	.buffer = cros_oem_rot_len_buffer,
	.max_length = sizeof(cros_oem_rot_len_buffer),
};

static uintptr_t cros_write_data(struct cros_oem_data *data,
				 u_register_t length, u_register_t address,
				 void *handle)
{
	uintptr_t aligned_address;
	uintptr_t aligned_size;
	int32_t rc;

	if (data->length) {
		SMC_RET1(handle, PSCI_E_ALREADY_ON);
	}

	if (length > data->max_length) {
		SMC_RET1(handle, PSCI_E_INVALID_PARAMS);
	}

	aligned_address = page_align(address, DOWN);
	aligned_size = page_align(length + (address - aligned_address), UP);

	/*
	 * We do not validate the passed in address because we are trusting the
	 * non-secure world at this point still.
	 */
	rc = mmap_add_dynamic_region(aligned_address, aligned_address,
				     aligned_size, MT_MEMORY | MT_RO | MT_NS);
	if (rc != 0) {
		SMC_RET1(handle, PSCI_E_INVALID_ADDRESS);
	}

	memcpy(data->buffer, (void *)address, length);
	data->length = length;

	mmap_remove_dynamic_region(aligned_address, aligned_size);
	SMC_RET1(handle, SMC_OK);
}

/* Handler for servicing specific SMC calls. */
static uintptr_t cros_oem_svc_smc_handler(uint32_t smc_fid, u_register_t x1,
					  u_register_t x2, u_register_t x3,
					  u_register_t x4, void *cookie,
					  void *handle, u_register_t flags)
{
	switch (smc_fid) {
	case CROS_OEM_SMC_DRM_SET_TPM_AUTH_PUB_FUNC_ID:
		return cros_write_data(&cros_oem_tpm_auth_pk, x1, x2, handle);
	case CROS_OEM_SMC_DRM_SET_HARDWARE_UNIQUE_KEY_FUNC_ID:
		return cros_write_data(&cros_oem_huk, x1, x2, handle);
	case CROS_OEM_SMC_DRM_SET_ROOT_OF_TRUST_FUNC_ID:
		return cros_write_data(&cros_oem_rot, x1, x2, handle);
	default:
		WARN("Unimplemented OEM Call: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}

/* Register OEM Service Calls as runtime service */
DECLARE_RT_SVC(cros_oem_svc_smc_handler, OEN_OEM_START, OEN_OEM_END,
	       SMC_TYPE_FAST, NULL, cros_oem_svc_smc_handler);
