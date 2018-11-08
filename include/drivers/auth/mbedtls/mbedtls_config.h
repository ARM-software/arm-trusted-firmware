/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef MBEDTLS_CONFIG_H
#define MBEDTLS_CONFIG_H

/*
 * Key algorithms currently supported on mbed TLS libraries
 */
#define TF_MBEDTLS_RSA			1
#define TF_MBEDTLS_ECDSA		2
#define TF_MBEDTLS_RSA_AND_ECDSA	3

/*
 * Hash algorithms currently supported on mbed TLS libraries
 */
#define TF_MBEDTLS_SHA256		1
#define TF_MBEDTLS_SHA384		2
#define TF_MBEDTLS_SHA512		3

/*
 * Configuration file to build mbed TLS with the required features for
 * Trusted Boot
 */

#define MBEDTLS_PLATFORM_MEMORY
#define MBEDTLS_PLATFORM_NO_STD_FUNCTIONS
/* Prevent mbed TLS from using snprintf so that it can use tf_snprintf. */
#define MBEDTLS_PLATFORM_SNPRINTF_ALT

#define MBEDTLS_PKCS1_V21

#define MBEDTLS_X509_ALLOW_UNSUPPORTED_CRITICAL_EXTENSION
#define MBEDTLS_X509_CHECK_KEY_USAGE
#define MBEDTLS_X509_CHECK_EXTENDED_KEY_USAGE

#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C

#define MBEDTLS_BASE64_C
#define MBEDTLS_BIGNUM_C

#define MBEDTLS_ERROR_C
#define MBEDTLS_MD_C

#define MBEDTLS_MEMORY_BUFFER_ALLOC_C
#define MBEDTLS_OID_C

#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_PK_WRITE_C

#define MBEDTLS_PLATFORM_C

#if (TF_MBEDTLS_KEY_ALG_ID == TF_MBEDTLS_ECDSA)
#define MBEDTLS_ECDSA_C
#define MBEDTLS_ECP_C
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#elif (TF_MBEDTLS_KEY_ALG_ID == TF_MBEDTLS_RSA)
#define MBEDTLS_RSA_C
#define MBEDTLS_X509_RSASSA_PSS_SUPPORT
#elif (TF_MBEDTLS_KEY_ALG_ID == TF_MBEDTLS_RSA_AND_ECDSA)
#define MBEDTLS_RSA_C
#define MBEDTLS_X509_RSASSA_PSS_SUPPORT
#define MBEDTLS_ECDSA_C
#define MBEDTLS_ECP_C
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#endif

#define MBEDTLS_SHA256_C
#if (TF_MBEDTLS_HASH_ALG_ID != TF_MBEDTLS_SHA256)
#define MBEDTLS_SHA512_C
#endif

#define MBEDTLS_VERSION_C

#define MBEDTLS_X509_USE_C
#define MBEDTLS_X509_CRT_PARSE_C

/* MPI / BIGNUM options */
#define MBEDTLS_MPI_WINDOW_SIZE              2
#define MBEDTLS_MPI_MAX_SIZE               256

/* Memory buffer allocator options */
#define MBEDTLS_MEMORY_ALIGN_MULTIPLE        8

#ifndef __ASSEMBLY__
/* System headers required to build mbed TLS with the current configuration */
#include <stdlib.h>
#include "mbedtls/check_config.h"
#endif

/*
 * Determine Mbed TLS heap size
 * 13312 = 13*1024
 * 7168 = 7*1024
 */
#if (TF_MBEDTLS_KEY_ALG_ID == TF_MBEDTLS_ECDSA) \
	|| (TF_MBEDTLS_KEY_ALG_ID == TF_MBEDTLS_RSA_AND_ECDSA)
#define TF_MBEDTLS_HEAP_SIZE		U(13312)
#elif (TF_MBEDTLS_KEY_ALG_ID == TF_MBEDTLS_RSA)
#define TF_MBEDTLS_HEAP_SIZE		U(7168)
#endif

#endif /* MBEDTLS_CONFIG_H */
