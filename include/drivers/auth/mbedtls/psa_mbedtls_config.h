/*
 * Copyright (c) 2023, Arm Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PSA_MBEDTLS_CONFIG_H
#define PSA_MBEDTLS_CONFIG_H

#include "mbedtls_config-3.h"

#define MBEDTLS_PSA_CRYPTO_C

/*
 * Using PSA crypto API requires an RNG right now. If we don't define the macro
 * below then we get build errors.
 *
 * This is a functionality gap in mbedTLS. The technical limitation is that
 * psa_crypto_init() is all-or-nothing, and fixing that would require separate
 * initialization of the keystore, the RNG, etc.
 *
 * By defining MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG, we pretend using an external
 * RNG. As a result, the PSA crypto init code does nothing when it comes to
 * initializing the RNG, as we are supposed to take care of that ourselves.
 */
#define MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG

#endif /* PSA_MBEDTLS_CONFIG_H */
