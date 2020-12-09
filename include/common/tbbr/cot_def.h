/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef COT_DEF_H
#define COT_DEF_H

#ifdef MBEDTLS_CONFIG_FILE
#include MBEDTLS_CONFIG_FILE
#endif

/* TBBR CoT definitions */
#if defined(SPD_spmd)
#define COT_MAX_VERIFIED_PARAMS		8
#else
#define COT_MAX_VERIFIED_PARAMS		4
#endif

/*
 * Maximum key and hash sizes (in DER format).
 *
 * Both RSA and ECDSA keys may be used at the same time. In this case, the key
 * buffers must be big enough to hold either. As RSA keys are bigger than ECDSA
 * ones for all key sizes we support, they impose the minimum size of these
 * buffers.
 */
#if TF_MBEDTLS_USE_RSA
#if TF_MBEDTLS_KEY_SIZE == 1024
#define PK_DER_LEN                      162
#elif TF_MBEDTLS_KEY_SIZE == 2048
#define PK_DER_LEN                      294
#elif TF_MBEDTLS_KEY_SIZE == 3072
#define PK_DER_LEN                      422
#elif TF_MBEDTLS_KEY_SIZE == 4096
#define PK_DER_LEN                      550
#else
#error "Invalid value for TF_MBEDTLS_KEY_SIZE"
#endif
#else /* Only using ECDSA keys. */
#define PK_DER_LEN                      91
#endif

#if TF_MBEDTLS_HASH_ALG_ID == TF_MBEDTLS_SHA256
#define HASH_DER_LEN                    51
#elif TF_MBEDTLS_HASH_ALG_ID == TF_MBEDTLS_SHA384
#define HASH_DER_LEN                    67
#elif TF_MBEDTLS_HASH_ALG_ID == TF_MBEDTLS_SHA512
#define HASH_DER_LEN                    83
#else
#error "Invalid value for TF_MBEDTLS_HASH_ALG_ID"
#endif

#endif /* COT_DEF_H */
