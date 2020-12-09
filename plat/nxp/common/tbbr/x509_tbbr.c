/*
 * Copyright 2018-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <lib/cassert.h>
#include <sfp.h>
#include <tools_share/tbbr_oid.h>

#include <plat/common/platform.h>
#include "plat_common.h"

extern char nxp_rotpk_hash[], nxp_rotpk_hash_end[];

int plat_get_rotpk_info(void *cookie, void **key_ptr, unsigned int *key_len,
			unsigned int *flags)
{
	*key_ptr = nxp_rotpk_hash;
	*key_len = nxp_rotpk_hash_end - nxp_rotpk_hash;
	*flags = ROTPK_IS_HASH;

	return 0;
}

int plat_get_nv_ctr(void *cookie, unsigned int *nv_ctr)
{
	const char *oid;
	uint32_t uid_num;
	uint32_t val = 0U;

	assert(cookie != NULL);
	assert(nv_ctr != NULL);

	oid = (const char *)cookie;
	if (strcmp(oid, TRUSTED_FW_NVCOUNTER_OID) == 0) {
		uid_num = 3U;
	} else if (strcmp(oid, NON_TRUSTED_FW_NVCOUNTER_OID) == 0) {
		uid_num = 4U;
	} else {
		return 1;
	}

	val = sfp_read_oem_uid(uid_num);

	INFO("SFP Value read is %x from UID %d\n", val, uid_num);
	if (val == 0U) {
		*nv_ctr = 0U;
	} else {
		*nv_ctr = (32U - __builtin_clz(val));
	}

	INFO("NV Counter value for UID %d is %d\n", uid_num, *nv_ctr);
	return 0;

}

int plat_set_nv_ctr(void *cookie, unsigned int nv_ctr)
{
	const char *oid;
	uint32_t uid_num, sfp_val;

	assert(cookie != NULL);

	/* Counter values upto 32 are supported */
	if (nv_ctr > 32U) {
		return 1;
	}

	oid = (const char *)cookie;
	if (strcmp(oid, TRUSTED_FW_NVCOUNTER_OID) == 0) {
		uid_num = 3U;
	} else if (strcmp(oid, NON_TRUSTED_FW_NVCOUNTER_OID) == 0) {
		uid_num = 4U;
	} else {
		return 1;
	}
	sfp_val = (1U << (nv_ctr - 1));

	if (sfp_write_oem_uid(uid_num, sfp_val) == 1) {
		/* Enable POVDD on board */
		if (board_enable_povdd()) {
			sfp_program_fuses();
		}

		/* Disable POVDD on board */
		board_disable_povdd();
	} else {
		ERROR("Invalid OEM UID sent.\n");
		return 1;
	}

	return 0;
}

int plat_get_mbedtls_heap(void **heap_addr, size_t *heap_size)
{
	return get_mbedtls_heap_helper(heap_addr, heap_size);
}
