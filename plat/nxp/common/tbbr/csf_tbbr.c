/*
 * Copyright 2018-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 *
 */

#include <errno.h>

#include <common/debug.h>
#include <csf_hdr.h>
#include <dcfg.h>
#include <drivers/auth/crypto_mod.h>
#include <snvs.h>

#include <plat/common/platform.h>
#include "plat_common.h"

extern bool rotpk_not_dpld;
extern uint8_t rotpk_hash_table[MAX_KEY_ENTRIES][SHA256_BYTES];
extern uint32_t num_rotpk_hash_entries;

/*
 * In case of secure boot, return ptr of rotpk_hash table in key_ptr and
 * number of hashes in key_len
 */
int plat_get_rotpk_info(void *cookie, void **key_ptr, unsigned int *key_len,
			unsigned int *flags)
{
	uint32_t mode = 0U;
	*flags = ROTPK_NOT_DEPLOYED;

	/* ROTPK hash table must be available for secure boot */
	if (rotpk_not_dpld == true) {
		if (check_boot_mode_secure(&mode) == true) {
			/* Production mode, don;t continue further */
			if (mode == 1U) {
				return -EAUTH;
			}

			/* For development mode, rotpk flag false
			 * indicates that SRK hash comparison might
			 * have failed. This is not fatal error.
			 * Continue in this case but transition SNVS
			 * to non-secure state
			 */
			transition_snvs_non_secure();
			return 0;
		} else {
			return 0;
		}
	}

	/*
	 * We return the complete hash table and number of entries in
	 * table for NXP platform specific implementation.
	 * Here hash is always assume as SHA-256
	 */
	*key_ptr = rotpk_hash_table;
	*key_len = num_rotpk_hash_entries;
	*flags = ROTPK_IS_HASH;

	return 0;
}

int plat_get_nv_ctr(void *cookie, unsigned int *nv_ctr)
{
	/*
	 * No support for non-volatile counter. Update the ROT key to protect
	 * the system against rollback.
	 */
	*nv_ctr = 0U;

	return 0;
}

int plat_set_nv_ctr(void *cookie, unsigned int nv_ctr)
{
	return 0;
}
