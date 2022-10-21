/*
 * Copyright (c) 2022, Arm Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_TC_MBEDTLS_CONFIG_H
#define PLAT_TC_MBEDTLS_CONFIG_H

#include <mbedtls_config.h>
#include <export/lib/utils_def_exp.h>

#ifndef TF_MBEDTLS_HEAP_SIZE
#error TF_MBEDTLS_HEAP_SIZE is not defined
#else
#define PLATFORM_TEST_MIN_MBEDTLS_HEAP_SIZE	(8 * 1024)
/* Only change heap size if it is less then the minimum required. */
#if TF_MBEDTLS_HEAP_SIZE < PLATFORM_TEST_MIN_MBEDTLS_HEAP_SIZE
#undef TF_MBEDTLS_HEAP_SIZE
#define TF_MBEDTLS_HEAP_SIZE	PLATFORM_TEST_MIN_MBEDTLS_HEAP_SIZE
#endif
#endif

#define MBEDTLS_PSA_CRYPTO_C
#define MBEDTLS_HMAC_DRBG_C
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES
#define MBEDTLS_NO_PLATFORM_ENTROPY
#define MBEDTLS_TEST_NULL_ENTROPY
#define MBEDTLS_ECP_C
#define MBEDTLS_ECP_DP_SECP384R1_ENABLED

#endif /* PLAT_TC_MBEDTLS_CONFIG_H */
