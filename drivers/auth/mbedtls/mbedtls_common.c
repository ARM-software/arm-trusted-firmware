/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <debug.h>
/* mbed TLS headers */
#include <mbedtls/memory_buffer_alloc.h>
#include <mbedtls/platform.h>
#include <mbedtls_common.h>
#include <mbedtls_config.h>
#include <platform.h>
#include <stddef.h>

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
	void *heap_addr;
	size_t heap_size = 0;
	int err;

	if (!ready) {
		if (atexit(cleanup))
			panic();

		err = plat_get_mbedtls_heap(&heap_addr, &heap_size);

		/* Ensure heap setup is proper */
		if (err < 0) {
			ERROR("Mbed TLS failed to get a heap\n");
			panic();
		}
		assert(heap_size >= TF_MBEDTLS_HEAP_SIZE);

		/* Initialize the mbed TLS heap */
		mbedtls_memory_buffer_alloc_init(heap_addr, heap_size);

#ifdef MBEDTLS_PLATFORM_SNPRINTF_ALT
		mbedtls_platform_set_snprintf(snprintf);
#endif
		ready = 1;
	}
}
