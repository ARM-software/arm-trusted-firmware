/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <drivers/auth/auth_mod.h>
#include <plat/common/platform.h>
#if USE_TBBR_DEFS
#include <tools_share/tbbr_oid.h>
#else
#include <platform_oid.h>
#endif

/*
 * Store a new non-volatile counter value. This implementation
 * only allows updating of the platform's Trusted NV counter when a
 * certificate protected by the Trusted NV counter is signed with
 * the ROT key. This avoids a compromised secondary certificate from
 * updating the platform's Trusted NV counter, which could lead to the
 * platform becoming unusable. The function is suitable for all TBBR
 * compliant platforms.
 *
 * Return: 0 = success, Otherwise = error
 */
int plat_set_nv_ctr2(void *cookie, const auth_img_desc_t *img_desc,
		unsigned int nv_ctr)
{
	int trusted_nv_ctr;

	assert(cookie != NULL);
	assert(img_desc != NULL);

	trusted_nv_ctr = strcmp(cookie, TRUSTED_FW_NVCOUNTER_OID) == 0;

	/*
	 * Only update the Trusted NV Counter if the certificate
	 * has been signed with the ROT key. Non Trusted NV counter
	 * updates are unconditional.
	 */
	if (!trusted_nv_ctr || img_desc->parent == NULL)
		return plat_set_nv_ctr(cookie, nv_ctr);

	/*
	 * Trusted certificates not signed with the ROT key are not
	 * allowed to update the Trusted NV Counter.
	 */
	return 1;
}
