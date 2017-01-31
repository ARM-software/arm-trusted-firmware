/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <auth/auth_mod.h>
#include <platform.h>
#include <platform_oid.h>
#include <string.h>

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
