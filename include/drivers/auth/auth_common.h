/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AUTH_COMMON_H
#define AUTH_COMMON_H

/*
 * Authentication framework common types
 */

/*
 * Type of parameters that can be extracted from an image and
 * used for authentication
 */
typedef enum auth_param_type_enum {
	AUTH_PARAM_NONE,
	AUTH_PARAM_RAW_DATA,		/* Raw image data */
	AUTH_PARAM_SIG,			/* The image signature */
	AUTH_PARAM_SIG_ALG,		/* The image signature algorithm */
	AUTH_PARAM_HASH,		/* A hash (including the algorithm) */
	AUTH_PARAM_PUB_KEY,		/* A public key */
	AUTH_PARAM_NV_CTR,		/* A non-volatile counter */
} auth_param_type_t;

/*
 * Defines an authentication parameter. The cookie will be interpreted by the
 * image parser module.
 */
typedef struct auth_param_type_desc_s {
	auth_param_type_t type;
	void *cookie;
} auth_param_type_desc_t;

/*
 * Store a pointer to the authentication parameter and its length
 */
typedef struct auth_param_data_desc_s {
	void *ptr;
	unsigned int len;
} auth_param_data_desc_t;

/*
 * Authentication parameter descriptor, including type and value
 */
typedef struct auth_param_desc_s {
	auth_param_type_desc_t *type_desc;
	auth_param_data_desc_t data;
} auth_param_desc_t;

/*
 * The method type defines how an image is authenticated
 */
typedef enum auth_method_type_enum {
	AUTH_METHOD_NONE = 0,
	AUTH_METHOD_HASH,	/* Authenticate by hash matching */
	AUTH_METHOD_SIG,	/* Authenticate by PK operation */
	AUTH_METHOD_NV_CTR,	/* Authenticate by Non-Volatile Counter */
	AUTH_METHOD_NUM 	/* Number of methods */
} auth_method_type_t;

/*
 * Parameters for authentication by hash matching
 */
typedef struct auth_method_param_hash_s {
	auth_param_type_desc_t *data;	/* Data to hash */
	auth_param_type_desc_t *hash;	/* Hash to match with */
} auth_method_param_hash_t;

/*
 * Parameters for authentication by signature
 */
typedef struct auth_method_param_sig_s {
	auth_param_type_desc_t *pk;	/* Public key */
	auth_param_type_desc_t *sig;	/* Signature to check */
	auth_param_type_desc_t *alg;	/* Signature algorithm */
	auth_param_type_desc_t *data;	/* Data signed */
} auth_method_param_sig_t;

/*
 * Parameters for authentication by NV counter
 */
typedef struct auth_method_param_nv_ctr_s {
	auth_param_type_desc_t *cert_nv_ctr;	/* NV counter in certificate */
	auth_param_type_desc_t *plat_nv_ctr;	/* NV counter in platform */
} auth_method_param_nv_ctr_t;

/*
 * Authentication method descriptor
 */
typedef struct auth_method_desc_s {
	auth_method_type_t type;
	union {
		auth_method_param_hash_t hash;
		auth_method_param_sig_t sig;
		auth_method_param_nv_ctr_t nv_ctr;
	} param;
} auth_method_desc_t;

/*
 * Helper macro to define an authentication parameter type descriptor
 */
#define AUTH_PARAM_TYPE_DESC(_type, _cookie) \
	{ \
		.type = _type, \
		.cookie = (void *)_cookie \
	}

/*
 * Helper macro to define an authentication parameter data descriptor
 */
#define AUTH_PARAM_DATA_DESC(_ptr, _len) \
	{ \
		.ptr = (void *)_ptr, \
		.len = (unsigned int)_len \
	}

#endif /* AUTH_COMMON_H */
