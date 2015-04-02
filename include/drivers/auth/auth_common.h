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

#ifndef __AUTH_COMMON_H__
#define __AUTH_COMMON_H__

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
	auth_param_type_desc_t *nv_ctr;	/* NV counter value */
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

#endif /* __AUTH_COMMON_H__ */
