/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

/* mbed TLS headers */
#include <mbedtls/memory_buffer_alloc.h>

/*
 * mbed TLS heap
 */
#if (TBBR_KEY_ALG_ID == TBBR_ECDSA)
#define MBEDTLS_HEAP_SIZE		(14*1024)
#elif (TBBR_KEY_ALG_ID == TBBR_RSA)
#define MBEDTLS_HEAP_SIZE		(8*1024)
#endif
static unsigned char heap[MBEDTLS_HEAP_SIZE];

/*
 * mbed TLS initialization function
 */
void mbedtls_init(void)
{
	static int ready;

	if (!ready) {
		/* Initialize the mbed TLS heap */
		mbedtls_memory_buffer_alloc_init(heap, MBEDTLS_HEAP_SIZE);
		ready = 1;
	}
}
