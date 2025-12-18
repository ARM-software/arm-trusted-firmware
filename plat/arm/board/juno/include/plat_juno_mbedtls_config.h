/*
 * Copyright (c) 2025, Arm Limited.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_JUNO_MBEDTLS_CONFIG_H
#define PLAT_JUNO_MBEDTLS_CONFIG_H

#include <export/lib/utils_def_exp.h>
#include <default_mbedtls_config.h>

/*
 * Limit the enabled hash algorithm to the one selected via HASH_ALG when
 * measured boot is enabled in order to avoid pulling unused digests into the
 * build.
 */
#if MEASURED_BOOT
#if TF_MBEDTLS_HASH_ALG_ID == TF_MBEDTLS_SHA256
#undef MBEDTLS_SHA384_C
#undef MBEDTLS_SHA512_C
#elif TF_MBEDTLS_HASH_ALG_ID == TF_MBEDTLS_SHA384
#undef MBEDTLS_SHA256_C
#undef MBEDTLS_SHA256_SMALLER
#undef MBEDTLS_SHA512_C
#elif TF_MBEDTLS_HASH_ALG_ID == TF_MBEDTLS_SHA512
#undef MBEDTLS_SHA256_C
#undef MBEDTLS_SHA256_SMALLER
#undef MBEDTLS_SHA384_C
#else
#error "Unsupported TF_MBEDTLS_HASH_ALG_ID for Juno measured boot"
#endif
#endif /* MEASURED_BOOT */

#endif /* PLAT_JUNO_MBEDTLS_CONFIG_H */
