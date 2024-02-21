/*
 * Copyright (c) 2022-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RSE_MEASURED_BOOT_H
#define RSE_MEASURED_BOOT_H

#include <stdint.h>

#include <common/debug.h>
#include <drivers/measured_boot/metadata.h>

#define RSE_MBOOT_INVALID_ID	UINT32_MAX

struct rse_mboot_metadata {
	unsigned int id;
	uint8_t slot;
	uint8_t signer_id[SIGNER_ID_MAX_SIZE];
	size_t  signer_id_size;
	uint8_t version[VERSION_MAX_SIZE];
	size_t  version_size;
	uint8_t sw_type[SW_TYPE_MAX_SIZE];
	size_t  sw_type_size;
	void    *pk_oid;
	bool    lock_measurement;
};

/* Functions' declarations */
void rse_measured_boot_init(struct rse_mboot_metadata *metadata_ptr);
int rse_mboot_measure_and_record(struct rse_mboot_metadata *metadata_ptr,
				 uintptr_t data_base, uint32_t data_size,
				 uint32_t data_id);

int rse_mboot_set_signer_id(struct rse_mboot_metadata *metadata_ptr,
			    const void *pk_oid, const void *pk_ptr,
			    size_t pk_len);

#endif /* RSE_MEASURED_BOOT_H */
