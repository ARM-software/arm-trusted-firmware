/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <stdlib.h>

/* mbed TLS headers */
#include <mbedtls/memory_buffer_alloc.h>
#include <mbedtls/platform.h>
#include <mbedtls_config.h>
#include <mbedtls_common.h>

/*
 * mbed TLS heap
 */
#if (TF_MBEDTLS_KEY_ALG_ID == TF_MBEDTLS_ECDSA) \
	|| (TF_MBEDTLS_KEY_ALG_ID == TF_MBEDTLS_RSA_AND_ECDSA)
#define MBEDTLS_HEAP_SIZE		(13*1024)
#elif (TF_MBEDTLS_KEY_ALG_ID == TF_MBEDTLS_RSA)
#define MBEDTLS_HEAP_SIZE		(7*1024)
#endif
static unsigned char heap[MBEDTLS_HEAP_SIZE];

static void cleanup(void)
{
	ERROR("EXIT from BL2\n");
	panic();
}

/*
 * mbed TLS initialization function
 */
void mbedtls_init(void)
{
	static int ready;

	if (!ready) {
		if (atexit(cleanup))
			panic();

		/* Initialize the mbed TLS heap */
		mbedtls_memory_buffer_alloc_init(heap, MBEDTLS_HEAP_SIZE);

#ifdef MBEDTLS_PLATFORM_SNPRINTF_ALT
		/* Use reduced version of snprintf to save space. */
		mbedtls_platform_set_snprintf(tf_snprintf);
#endif

		ready = 1;
	}
}
