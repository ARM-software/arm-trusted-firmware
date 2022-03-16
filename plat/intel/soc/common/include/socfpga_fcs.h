/*
 * Copyright (c) 2020-2022, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOCFPGA_FCS_H
#define SOCFPGA_FCS_H

/* FCS Definitions */

#define FCS_RANDOM_WORD_SIZE		8U
#define FCS_PROV_DATA_WORD_SIZE		44U
#define FCS_SHA384_WORD_SIZE		12U

#define FCS_RANDOM_BYTE_SIZE		(FCS_RANDOM_WORD_SIZE * 4U)
#define FCS_PROV_DATA_BYTE_SIZE		(FCS_PROV_DATA_WORD_SIZE * 4U)
#define FCS_SHA384_BYTE_SIZE		(FCS_SHA384_WORD_SIZE * 4U)

#define FCS_CRYPTION_DATA_0		0x10100

/* FCS Payload Structure */

typedef struct fcs_crypt_payload_t {
	uint32_t first_word;
	uint32_t src_addr;
	uint32_t src_size;
	uint32_t dst_addr;
	uint32_t dst_size;
} fcs_crypt_payload;

/* Functions Definitions */

uint32_t intel_fcs_random_number_gen(uint64_t addr, uint64_t *ret_size,
				uint32_t *mbox_error);
uint32_t intel_fcs_send_cert(uint64_t addr, uint64_t size,
				uint32_t *send_id);
uint32_t intel_fcs_get_provision_data(uint32_t *send_id);
uint32_t intel_fcs_cryption(uint32_t mode, uint32_t src_addr,
			uint32_t src_size, uint32_t dst_addr,
			uint32_t dst_size, uint32_t *send_id);

uint32_t intel_fcs_get_rom_patch_sha384(uint64_t addr, uint64_t *ret_size,
				uint32_t *mbox_error);

#endif /* SOCFPGA_FCS_H */
