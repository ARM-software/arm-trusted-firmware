/*
 * Copyright (c) 2022-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/auth/crypto_mod.h>
#include <drivers/measured_boot/rss/rss_measured_boot.h>
#include <lib/psa/measured_boot.h>
#include <psa/crypto_types.h>
#include <psa/crypto_values.h>
#include <psa/error.h>

#define MBOOT_ALG_SHA512 0
#define MBOOT_ALG_SHA384 1
#define MBOOT_ALG_SHA256 2

#if MBOOT_ALG_ID == MBOOT_ALG_SHA512
#define	CRYPTO_MD_ID		CRYPTO_MD_SHA512
#define PSA_CRYPTO_MD_ID	PSA_ALG_SHA_512
#elif MBOOT_ALG_ID == MBOOT_ALG_SHA384
#define	CRYPTO_MD_ID		CRYPTO_MD_SHA384
#define PSA_CRYPTO_MD_ID	PSA_ALG_SHA_384
#elif MBOOT_ALG_ID == MBOOT_ALG_SHA256
#define	CRYPTO_MD_ID		CRYPTO_MD_SHA256
#define PSA_CRYPTO_MD_ID	PSA_ALG_SHA_256
#else
#  error Invalid Measured Boot algorithm.
#endif /* MBOOT_ALG_ID */

/* Functions' declarations */
void rss_measured_boot_init(struct rss_mboot_metadata *metadata_ptr)
{
	assert(metadata_ptr != NULL);

	/* Init the non-const members of the metadata structure */
	while (metadata_ptr->id != RSS_MBOOT_INVALID_ID) {
		metadata_ptr->sw_type_size =
			strlen((const char *)&metadata_ptr->sw_type) + 1;
		metadata_ptr++;
	}
}

int rss_mboot_measure_and_record(struct rss_mboot_metadata *metadata_ptr,
				 uintptr_t data_base, uint32_t data_size,
				 uint32_t data_id)
{
	unsigned char hash_data[CRYPTO_MD_MAX_SIZE];
	int rc;
	psa_status_t ret;

	assert(metadata_ptr != NULL);

	/* Get the metadata associated with this image. */
	while ((metadata_ptr->id != RSS_MBOOT_INVALID_ID) &&
		(metadata_ptr->id != data_id)) {
		metadata_ptr++;
	}

	/* If image is not present in metadata array then skip */
	if (metadata_ptr->id == RSS_MBOOT_INVALID_ID) {
		return 0;
	}

	/* Calculate hash */
	rc = crypto_mod_calc_hash(CRYPTO_MD_ID,
				  (void *)data_base, data_size, hash_data);
	if (rc != 0) {
		return rc;
	}

	ret = rss_measured_boot_extend_measurement(
						metadata_ptr->slot,
						metadata_ptr->signer_id,
						metadata_ptr->signer_id_size,
						metadata_ptr->version,
						metadata_ptr->version_size,
						PSA_CRYPTO_MD_ID,
						metadata_ptr->sw_type,
						metadata_ptr->sw_type_size,
						hash_data,
						MBOOT_DIGEST_SIZE,
						metadata_ptr->lock_measurement);
	if (ret != PSA_SUCCESS) {
		return ret;
	}

	return 0;
}

int rss_mboot_set_signer_id(struct rss_mboot_metadata *metadata_ptr,
			    unsigned int img_id,
			    const void *pk_ptr,
			    size_t pk_len)
{
	unsigned char hash_data[CRYPTO_MD_MAX_SIZE];
	int rc;

	assert(metadata_ptr != NULL);

	/* Get the metadata associated with this image. */
	while ((metadata_ptr->id != RSS_MBOOT_INVALID_ID) &&
		(metadata_ptr->id != img_id)) {
		metadata_ptr++;
	}

	/* If image is not present in metadata array then skip */
	if (metadata_ptr->id == RSS_MBOOT_INVALID_ID) {
		return 0;
	}

	/* Calculate public key hash */
	rc = crypto_mod_calc_hash(CRYPTO_MD_ID, (void *)pk_ptr,
				  pk_len, hash_data);
	if (rc != 0) {
		return rc;
	}

	/* Update metadata struct with the received signer_id */
	(void)memcpy(metadata_ptr->signer_id, hash_data, MBOOT_DIGEST_SIZE);
	metadata_ptr->signer_id_size = MBOOT_DIGEST_SIZE;

	return 0;
}
