/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DICE_PROT_ENV_H
#define DICE_PROT_ENV_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <drivers/measured_boot/metadata.h>

#define DPE_INVALID_ID	UINT32_MAX

struct dpe_metadata {
	unsigned int id;
	uint32_t cert_id;
	uint8_t signer_id[SIGNER_ID_MAX_SIZE];
	size_t  signer_id_size;
	uint8_t version[VERSION_MAX_SIZE];
	size_t  version_size;
	uint8_t sw_type[SW_TYPE_MAX_SIZE];
	size_t  sw_type_size;
	bool allow_new_context_to_derive;
	bool retain_parent_context;
	bool create_certificate;
	void *pk_oid;
};

void dpe_init(struct dpe_metadata *metadata);

/* Returns 0 in case of success otherwise -1. */
int dpe_measure_and_record(struct dpe_metadata *metadata,
			   uintptr_t data_base, uint32_t data_size,
			   uint32_t data_id);

int dpe_set_signer_id(struct dpe_metadata *metadata,
		      const void *pk_oid, const void *pk_ptr, size_t pk_len);

/* Child components inherit their first valid context handle from their parents.
 * How to share context handle is platform specific.
 */
void plat_dpe_share_context_handle(int *ctx_handle, int *parent_ctx_handle);
void plat_dpe_get_context_handle(int *ctx_handle);

#endif /* DICE_PROT_ENV_H */
