/*
 * Copyright (c) 2022-2024, Arm Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_TC_MBEDTLS_CONFIG_H
#define PLAT_TC_MBEDTLS_CONFIG_H

#include <export/lib/utils_def_exp.h>
#include <default_mbedtls_config.h>

#ifndef TF_MBEDTLS_HEAP_SIZE
#error TF_MBEDTLS_HEAP_SIZE is not defined
#else
#define PLATFORM_TEST_MIN_MBEDTLS_HEAP_SIZE	(8 * 1024)
/* Only change heap size if it is less then the minimum required. */
#if TF_MBEDTLS_HEAP_SIZE < PLATFORM_TEST_MIN_MBEDTLS_HEAP_SIZE
#undef TF_MBEDTLS_HEAP_SIZE
#define TF_MBEDTLS_HEAP_SIZE	PLATFORM_TEST_MIN_MBEDTLS_HEAP_SIZE
#endif
#endif /* TF_MBEDTLS_HEAP_SIZE */

/**
 * On Arm TC platforms, the ROTPK is always hashed using the SHA-256
 * algorithm.
 * TODO: Update to hash the ROTPK with the selected HASH_ALG to avoid
 * the need for explicitly enabling the SHA-256 configuration in mbedTLS.
 */
#define MBEDTLS_SHA256_C

/*
 * Use an implementation of SHA-256 with a smaller memory footprint
 * but reduced speed.
 */
#define MBEDTLS_SHA256_SMALLER

#define MBEDTLS_PSA_CRYPTO_C
#define MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG
#define MBEDTLS_ECP_C
#define MBEDTLS_ECP_DP_SECP384R1_ENABLED
#define MBEDTLS_ECP_NO_INTERNAL_RNG

#endif /* PLAT_TC_MBEDTLS_CONFIG_H */
