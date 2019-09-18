/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/common/platform.h>

extern char qemu_rotpk_hash[], qemu_rotpk_hash_end[];

int plat_get_rotpk_info(void *cookie, void **key_ptr, unsigned int *key_len,
			unsigned int *flags)
{
	*key_ptr = qemu_rotpk_hash;
	*key_len = qemu_rotpk_hash_end - qemu_rotpk_hash;
	*flags = ROTPK_IS_HASH;

	return 0;
}

int plat_get_nv_ctr(void *cookie, unsigned int *nv_ctr)
{
	*nv_ctr = 0;

	return 0;
}

int plat_set_nv_ctr(void *cookie, unsigned int nv_ctr)
{
	return 1;
}

int plat_get_mbedtls_heap(void **heap_addr, size_t *heap_size)
{
	return get_mbedtls_heap_helper(heap_addr, heap_size);
}
