/*
 * Copyright (c) 2025, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <default_mbedtls_config.h>

/* MPI / BIGNUM options */
#undef MBEDTLS_MPI_WINDOW_SIZE
#define MBEDTLS_MPI_WINDOW_SIZE			2

/*
 * Mbed TLS heap size is small as we only use the asn1
 * parsing functions
 * digest, signature and crypto algorithm are done by
 * other library.
 */
#undef TF_MBEDTLS_HEAP_SIZE
#define TF_MBEDTLS_HEAP_SIZE			U(5120)
