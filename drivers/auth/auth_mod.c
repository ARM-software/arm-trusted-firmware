/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <auth_common.h>
#include <auth_mod.h>
#include <cot_def.h>
#include <crypto_mod.h>
#include <debug.h>
#include <img_parser_mod.h>
#include <platform.h>
#include <platform_def.h>
#include <stdint.h>
#include <string.h>

#define return_if_error(rc) \
	do { \
		if (rc != 0) { \
			return rc; \
		} \
	} while (0)

/* Pointer to CoT */
extern const auth_img_desc_t *const cot_desc_ptr;
extern unsigned int auth_img_flags[];

static int cmp_auth_param_type_desc(const auth_param_type_desc_t *a,
		const auth_param_type_desc_t *b)
{
	if ((a->type == b->type) && (a->cookie == b->cookie)) {
		return 0;
	}
	return 1;
}

/*
 * This function obtains the requested authentication parameter data from the
 * information extracted from the parent image after its authentication.
 */
static int auth_get_param(const auth_param_type_desc_t *param_type_desc,
			  const auth_img_desc_t *img_desc,
			  void **param, unsigned int *len)
{
	int i;

	for (i = 0 ; i < COT_MAX_VERIFIED_PARAMS ; i++) {
		if (0 == cmp_auth_param_type_desc(param_type_desc,
				img_desc->authenticated_data[i].type_desc)) {
			*param = img_desc->authenticated_data[i].data.ptr;
			*len = img_desc->authenticated_data[i].data.len;
			return 0;
		}
	}

	return 1;
}

/*
 * Authenticate an image by matching the data hash
 *
 * This function implements 'AUTH_METHOD_HASH'. To authenticate an image using
 * this method, the image must contain:
 *
 *   - The data to calculate the hash from
 *
 * The parent image must contain:
 *
 *   - The hash to be matched with (including hash algorithm)
 *
 * For a successful authentication, both hashes must match. The function calls
 * the crypto-module to check this matching.
 *
 * Parameters:
 *   param: parameters to perform the hash authentication
 *   img_desc: pointer to image descriptor so we can know the image type
 *             and parent image
 *   img: pointer to image in memory
 *   img_len: length of image (in bytes)
 *
 * Return:
 *   0 = success, Otherwise = error
 */
static int auth_hash(const auth_method_param_hash_t *param,
		     const auth_img_desc_t *img_desc,
		     void *img, unsigned int img_len)
{
	void *data_ptr, *hash_der_ptr;
	unsigned int data_len, hash_der_len;
	int rc = 0;

	/* Get the hash from the parent image. This hash will be DER encoded
	 * and contain the hash algorithm */
	rc = auth_get_param(param->hash, img_desc->parent,
			&hash_der_ptr, &hash_der_len);
	return_if_error(rc);

	/* Get the data to be hashed from the current image */
	rc = img_parser_get_auth_param(img_desc->img_type, param->data,
			img, img_len, &data_ptr, &data_len);
	return_if_error(rc);

	/* Ask the crypto module to verify this hash */
	rc = crypto_mod_verify_hash(data_ptr, data_len,
				    hash_der_ptr, hash_der_len);

	return rc;
}

/*
 * Authenticate by digital signature
 *
 * This function implements 'AUTH_METHOD_SIG'. To authenticate an image using
 * this method, the image must contain:
 *
 *   - Data to be signed
 *   - Signature
 *   - Signature algorithm
 *
 * We rely on the image parser module to extract this data from the image.
 * The parent image must contain:
 *
 *   - Public key (or a hash of it)
 *
 * If the parent image contains only a hash of the key, we will try to obtain
 * the public key from the image itself (i.e. self-signed certificates). In that
 * case, the signature verification is considered just an integrity check and
 * the authentication is established by calculating the hash of the key and
 * comparing it with the hash obtained from the parent.
 *
 * If the image has no parent (NULL), it means it has to be authenticated using
 * the ROTPK stored in the platform. Again, this ROTPK could be the key itself
 * or a hash of it.
 *
 * Return: 0 = success, Otherwise = error
 */
static int auth_signature(const auth_method_param_sig_t *param,
			  const auth_img_desc_t *img_desc,
			  void *img, unsigned int img_len)
{
	void *data_ptr, *pk_ptr, *pk_hash_ptr, *sig_ptr, *sig_alg_ptr;
	unsigned int data_len, pk_len, pk_hash_len, sig_len, sig_alg_len;
	unsigned int flags = 0;
	int rc = 0;

	/* Get the data to be signed from current image */
	rc = img_parser_get_auth_param(img_desc->img_type, param->data,
			img, img_len, &data_ptr, &data_len);
	return_if_error(rc);

	/* Get the signature from current image */
	rc = img_parser_get_auth_param(img_desc->img_type, param->sig,
			img, img_len, &sig_ptr, &sig_len);
	return_if_error(rc);

	/* Get the signature algorithm from current image */
	rc = img_parser_get_auth_param(img_desc->img_type, param->alg,
			img, img_len, &sig_alg_ptr, &sig_alg_len);
	return_if_error(rc);

	/* Get the public key from the parent. If there is no parent (NULL),
	 * the certificate has been signed with the ROTPK, so we have to get
	 * the PK from the platform */
	if (img_desc->parent) {
		rc = auth_get_param(param->pk, img_desc->parent,
				&pk_ptr, &pk_len);
	} else {
		rc = plat_get_rotpk_info(param->pk->cookie, &pk_ptr, &pk_len,
				&flags);
	}
	return_if_error(rc);

	/* If the PK is a hash of the key, retrieve the key from the image */
	if (flags & ROTPK_IS_HASH) {
		pk_hash_ptr = pk_ptr;
		pk_hash_len = pk_len;
		rc = img_parser_get_auth_param(img_desc->img_type,
					param->pk, img, img_len,
					&pk_ptr, &pk_len);
		return_if_error(rc);

		/* Ask the crypto module to verify the signature */
		rc = crypto_mod_verify_signature(data_ptr, data_len,
						 sig_ptr, sig_len,
						 sig_alg_ptr, sig_alg_len,
						 pk_ptr, pk_len);
		return_if_error(rc);

		/* Ask the crypto-module to verify the key hash */
		rc = crypto_mod_verify_hash(pk_ptr, pk_len,
					    pk_hash_ptr, pk_hash_len);
	} else {
		/* Ask the crypto module to verify the signature */
		rc = crypto_mod_verify_signature(data_ptr, data_len,
						 sig_ptr, sig_len,
						 sig_alg_ptr, sig_alg_len,
						 pk_ptr, pk_len);
	}

	return rc;
}

/*
 * Return the parent id in the output parameter '*parent_id'
 *
 * Return value:
 *   0 = Image has parent, 1 = Image has no parent or parent is authenticated
 */
int auth_mod_get_parent_id(unsigned int img_id, unsigned int *parent_id)
{
	const auth_img_desc_t *img_desc = NULL;

	assert(parent_id != NULL);

	/* Get the image descriptor */
	img_desc = &cot_desc_ptr[img_id];

	/* Check if the image has no parent (ROT) */
	if (img_desc->parent == NULL) {
		*parent_id = 0;
		return 1;
	}

	/* Check if the parent has already been authenticated */
	if (auth_img_flags[img_desc->parent->img_id] & IMG_FLAG_AUTHENTICATED) {
		*parent_id = 0;
		return 1;
	}

	*parent_id = img_desc->parent->img_id;
	return 0;
}

/*
 * Initialize the different modules in the authentication framework
 */
void auth_mod_init(void)
{
	/* Check we have a valid CoT registered */
	assert(cot_desc_ptr != NULL);

	/* Crypto module */
	crypto_mod_init();

	/* Image parser module */
	img_parser_init();
}

/*
 * Authenticate a certificate/image
 *
 * Return: 0 = success, Otherwise = error
 */
int auth_mod_verify_img(unsigned int img_id,
			void *img_ptr,
			unsigned int img_len)
{
	const auth_img_desc_t *img_desc = NULL;
	const auth_method_desc_t *auth_method = NULL;
	void *param_ptr;
	unsigned int param_len;
	int rc, i;

	/* Get the image descriptor from the chain of trust */
	img_desc = &cot_desc_ptr[img_id];

	/* Ask the parser to check the image integrity */
	rc = img_parser_check_integrity(img_desc->img_type, img_ptr, img_len);
	return_if_error(rc);

	/* Authenticate the image using the methods indicated in the image
	 * descriptor. */
	for (i = 0 ; i < AUTH_METHOD_NUM ; i++) {
		auth_method = &img_desc->img_auth_methods[i];
		switch (auth_method->type) {
		case AUTH_METHOD_NONE:
			rc = 0;
			break;
		case AUTH_METHOD_HASH:
			rc = auth_hash(&auth_method->param.hash,
					img_desc, img_ptr, img_len);
			break;
		case AUTH_METHOD_SIG:
			rc = auth_signature(&auth_method->param.sig,
					img_desc, img_ptr, img_len);
			break;
		default:
			/* Unknown authentication method */
			rc = 1;
			break;
		}
		return_if_error(rc);
	}

	/* Extract the parameters indicated in the image descriptor to
	 * authenticate the children images. */
	for (i = 0 ; i < COT_MAX_VERIFIED_PARAMS ; i++) {
		if (img_desc->authenticated_data[i].type_desc == NULL) {
			continue;
		}

		/* Get the parameter from the image parser module */
		rc = img_parser_get_auth_param(img_desc->img_type,
				img_desc->authenticated_data[i].type_desc,
				img_ptr, img_len, &param_ptr, &param_len);
		return_if_error(rc);

		/* Check parameter size */
		if (param_len > img_desc->authenticated_data[i].data.len) {
			return 1;
		}

		/* Copy the parameter for later use */
		memcpy((void *)img_desc->authenticated_data[i].data.ptr,
				(void *)param_ptr, param_len);
	}

	/* Mark image as authenticated */
	auth_img_flags[img_desc->img_id] |= IMG_FLAG_AUTHENTICATED;

	return 0;
}
