/*
 * Copyright (c) 2024, Arm Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_ARM_MBEDTLS_CONFIG_H
#define PLAT_ARM_MBEDTLS_CONFIG_H

#include <default_mbedtls_config.h>

/**
 * On Arm platforms, the ROTPK is always hashed using the SHA-256
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

#endif /* PLAT_ARM_MBEDTLS_CONFIG_H */
