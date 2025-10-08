/*
 * Copyright (c) 2024-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <psa/crypto_types.h>
#include <psa/crypto_values.h>

#include <common/debug.h>
#include <drivers/auth/crypto_mod.h>
#include <drivers/measured_boot/rse/dice_prot_env.h>
#include <lib/cassert.h>
#include <lib/psa/dice_protection_environment.h>

#include <platform_def.h>

#define DPE_ALG_SHA512 0
#define DPE_ALG_SHA384 1
#define DPE_ALG_SHA256 2

#if DPE_ALG_ID == DPE_ALG_SHA512
#define	CRYPTO_MD_ID		CRYPTO_MD_SHA512
#define PSA_CRYPTO_MD_ID	PSA_ALG_SHA_512
#elif DPE_ALG_ID == DPE_ALG_SHA384
#define	CRYPTO_MD_ID		CRYPTO_MD_SHA384
#define PSA_CRYPTO_MD_ID	PSA_ALG_SHA_384
#elif DPE_ALG_ID == DPE_ALG_SHA256
#define	CRYPTO_MD_ID		CRYPTO_MD_SHA256
#define PSA_CRYPTO_MD_ID	PSA_ALG_SHA_256
#else
#  error Invalid DPE hash algorithm.
#endif /* DPE_ALG_ID */

/* Ensure that computed hash values fits into the DiceInputValues structure */
CASSERT(DICE_HASH_SIZE >= DPE_DIGEST_SIZE,
	assert_digest_size_bigger_than_allocated_buffer);

static int initial_context_handle;

static void map_metadata_to_dice_inputs(struct dpe_metadata *metadata,
					DiceInputValues  *dice_inputs)
{
	/* Hash of the content certificate signing key (public part) */
	memcpy(dice_inputs->authority_hash, metadata->signer_id,
	       DPE_DIGEST_SIZE);

	/* SW type string identifier */
	assert(metadata->sw_type_size < DICE_CODE_DESCRIPTOR_MAX_SIZE);
	dice_inputs->code_descriptor = metadata->sw_type;
	dice_inputs->code_descriptor_size = metadata->sw_type_size;
}

void dpe_init(struct dpe_metadata *metadata)
{
	assert(metadata != NULL);

	/* Init the non-const members of the metadata structure */
	while (metadata->id != DPE_INVALID_ID) {
		/* Terminating 0 character is not needed due to CBOR encoding */
		metadata->sw_type_size =
			strlen((const char *)&metadata->sw_type);
		metadata++;
	}

	plat_dpe_get_context_handle(&initial_context_handle);
}

int dpe_measure_and_record(struct dpe_metadata *metadata,
			   uintptr_t data_base, uint32_t data_size,
			   uint32_t data_id)
{
	static int current_context_handle;
	DiceInputValues dice_inputs = { 0 };
	int new_parent_context_handle = 0;
	int new_context_handle;
	dpe_error_t ret;
	int rc;

	assert(metadata != NULL);

	/* Get the metadata associated with this image. */
	while ((metadata->id != DPE_INVALID_ID) && (metadata->id != data_id)) {
		metadata++;
	}

	/* If image is not present in metadata array then skip */
	if (metadata->id == DPE_INVALID_ID) {
		return 0;
	}

	/* Calculate hash */
	rc = crypto_mod_calc_hash(CRYPTO_MD_ID,
				  (void *)data_base, data_size,
				   dice_inputs.code_hash);
	if (rc != 0) {
		return rc;
	}

	map_metadata_to_dice_inputs(metadata, &dice_inputs);

	/* Only at the first call */
	if (current_context_handle == 0) {
		current_context_handle = initial_context_handle;
	}

	VERBOSE("Calling dpe_derive_context, image_id: %d\n", metadata->id);
	ret = dpe_derive_context(current_context_handle,
				 metadata->cert_id,
				 metadata->retain_parent_context,
				 metadata->allow_new_context_to_derive,
				 metadata->create_certificate,
				 &dice_inputs,
				 metadata->target_locality,
				 false, /* return_certificate */
				 true, /* allow_new_context_to_export */
				 false, /* export_cdi */
				 &new_context_handle,
				 &new_parent_context_handle,
				 NULL, 0, NULL,  /* new_certificate_* */
				 NULL, 0, NULL); /* exported_cdi_* */
	if (ret == DPE_NO_ERROR) {
		current_context_handle = new_parent_context_handle;
		if (metadata->allow_new_context_to_derive == true) {
			/* Share new_context_handle with child component:
			 * e.g: BL2, BL33.
			 */
			VERBOSE("Share new_context_handle with child: 0x%x\n",
				new_context_handle);
			plat_dpe_share_context_handle(&new_context_handle,
						      &new_parent_context_handle);
		}
	} else {
		ERROR("dpe_derive_context failed: %d\n", ret);
	}

	return (ret == DPE_NO_ERROR) ? 0 : -1;
}

int dpe_set_signer_id(struct dpe_metadata *metadata,
		      const void *pk_oid,
		      const void *pk_ptr,
		      size_t pk_len)
{
	unsigned char hash_data[CRYPTO_MD_MAX_SIZE];
	int rc;
	bool hash_calc_done = false;

	assert(metadata != NULL);

	/*
	 * Do an exhaustive search over the platform metadata to find
	 * all images whose key OID matches the one passed in argument.
	 *
	 * Note that it is not an error if do not get any matches.
	 * The platform may decide not to measure all of the images
	 * in the system.
	 */
	while (metadata->id != DPE_INVALID_ID) {
		/* Get the metadata associated with this key-oid */
		if (metadata->pk_oid == pk_oid) {
			if (hash_calc_done == false) {
				/* Calculate public key hash */
				rc = crypto_mod_calc_hash(CRYPTO_MD_ID,
							  (void *)pk_ptr,
							  pk_len, hash_data);
				if (rc != 0) {
					return rc;
				}

				hash_calc_done = true;
			}

			/*
			 * Fill the signer-ID field with the newly/already
			 * computed hash of the public key and update its
			 * signer ID size field with compile-time decided
			 * digest size.
			 */
			(void)memcpy(metadata->signer_id,
				     hash_data,
				     DPE_DIGEST_SIZE);
			metadata->signer_id_size = DPE_DIGEST_SIZE;
		}

		metadata++;
	}

	return 0;
}
