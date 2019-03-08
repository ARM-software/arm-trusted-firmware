/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>
#include <drivers/auth/auth_mod.h>
#include <stddef.h>

#if USE_TBBR_DEFS
#include <tools_share/tbbr_oid.h>
#else
#include <platform_oid.h>
#endif


/*
 * The platform must allocate buffers to store the authentication parameters
 * extracted from the certificates. In this case, because of the way the CoT is
 * established, we can reuse some of the buffers on different stages
 */
static auth_param_type_desc_t sig = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_SIG, 0);
static auth_param_type_desc_t sig_alg = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_SIG_ALG, 0);
static auth_param_type_desc_t sig_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, 0);

static auth_param_type_desc_t non_trusted_world_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, NON_TRUSTED_WORLD_PK_OID);

/*
 * TBBR Chain of trust definition
 */
static const auth_img_desc_t cot_desc[] = {
	/*
	 * Non-Trusted Firmware
	 */
	/* CSF header for Bl33 - Non Trusted FW ..chain ends at Platform RoT */
	[BL31_IMAGE_ID] = {
		.img_id = BL31_IMAGE_ID,
		.img_type = IMG_PLAT,
		.parent = NULL,
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_SIG,
				.param.sig = {
					.pk = &non_trusted_world_pk,
					.sig = &sig,
					.alg = &sig_alg,
					.data = &sig_hash,
				}
			}
		}
	},
	[SCP_BL2_IMAGE_ID] = {
		.img_id = SCP_BL2_IMAGE_ID,
		.img_type = IMG_PLAT,
		.parent = NULL,
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_SIG,
				.param.sig = {
					.pk = &non_trusted_world_pk,
					.sig = &sig,
					.alg = &sig_alg,
					.data = &sig_hash,
				}
			}
		}
	},
	[BL32_IMAGE_ID] = {
		.img_id = BL32_IMAGE_ID,
		.img_type = IMG_PLAT,
		.parent = NULL,
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_SIG,
				.param.sig = {
					.pk = &non_trusted_world_pk,
					.sig = &sig,
					.alg = &sig_alg,
					.data = &sig_hash,
				}
			}
		}
	},
	[BL33_IMAGE_ID] = {
		.img_id = BL33_IMAGE_ID,
		.img_type = IMG_PLAT,
		.parent = NULL,
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_SIG,
				.param.sig = {
					.pk = &non_trusted_world_pk,
					.sig = &sig,
					.alg = &sig_alg,
					.data = &sig_hash,
				}
			}
		}
	},
};

/* Register the CoT in the authentication module */
REGISTER_COT(cot_desc);
