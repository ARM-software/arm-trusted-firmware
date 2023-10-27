/*
 * Copyright (c) 2015-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef COT_DEF_H
#define COT_DEF_H

/*
 * Guard here with availability of mbedtls config since PLAT=lx2162aqds
 * uses custom tbbr from 'drivers/nxp/auth/tbbr/tbbr_cot.c'  and also may
 * build without mbedtls folder only with TRUSTED_BOOT enabled.
 */
#ifdef MBEDTLS_CONFIG_FILE
#include <mbedtls/version.h>
#endif

/* TBBR CoT definitions */
#if defined(SPD_spmd)
#define COT_MAX_VERIFIED_PARAMS		8
#elif defined(ARM_COT_cca)
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
 *
 * If the platform employs its own mbedTLS configuration, it is the platform's
 * responsibility to define TF_MBEDTLS_USE_RSA or TF_MBEDTLS_USE_ECDSA to
 * establish the appropriate PK_DER_LEN size.
 */
#ifdef MBEDTLS_CONFIG_FILE
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
#elif TF_MBEDTLS_USE_ECDSA
#if TF_MBEDTLS_KEY_SIZE == 384
#define PK_DER_LEN                      120
#elif TF_MBEDTLS_KEY_SIZE == 256
#define PK_DER_LEN                      92
#else
#error "Invalid value for TF_MBEDTLS_KEY_SIZE"
#endif
#else
#error "Invalid value of algorithm"
#endif /* TF_MBEDTLS_USE_RSA */

#if TF_MBEDTLS_HASH_ALG_ID == TF_MBEDTLS_SHA256
#define HASH_DER_LEN                    51
#elif TF_MBEDTLS_HASH_ALG_ID == TF_MBEDTLS_SHA384
#define HASH_DER_LEN                    67
#elif TF_MBEDTLS_HASH_ALG_ID == TF_MBEDTLS_SHA512
#define HASH_DER_LEN                    83
#else
#error "Invalid value for TF_MBEDTLS_HASH_ALG_ID"
#endif
#endif /* MBEDTLS_CONFIG_FILE */

#endif /* COT_DEF_H */
