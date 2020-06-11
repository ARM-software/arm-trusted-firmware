/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include <platform_def.h>
#include <drivers/auth/mbedtls/mbedtls_config.h>

#include <drivers/auth/auth_mod.h>
#include <drivers/auth/tbbr_cot_common.h>
#if USE_TBBR_DEFS
#include <tools_share/tbbr_oid.h>
#else
#include <platform_oid.h>
#endif

static auth_param_type_desc_t scp_bl2u_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, SCP_FWU_CFG_HASH_OID);
static auth_param_type_desc_t bl2u_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, AP_FWU_CFG_HASH_OID);
static auth_param_type_desc_t ns_bl2u_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, FWU_HASH_OID);

static const auth_img_desc_t bl2_image = {
	.img_id = BL2_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &trusted_boot_fw_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &tb_fw_hash
			}
		}
	}
};

/*
 * FWU auth descriptor.
 */
static const auth_img_desc_t fwu_cert = {
	.img_id = FWU_CERT_ID,
	.img_type = IMG_CERT,
	.parent = NULL,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &subject_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &raw_data
			}
		}
	},
	.authenticated_data = (const auth_param_desc_t[COT_MAX_VERIFIED_PARAMS]) {
		[0] = {
			.type_desc = &scp_bl2u_hash,
			.data = {
				.ptr = (void *)scp_fw_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[1] = {
			.type_desc = &bl2u_hash,
			.data = {
				.ptr = (void *)tb_fw_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[2] = {
			.type_desc = &ns_bl2u_hash,
			.data = {
				.ptr = (void *)nt_world_bl_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		}
	}
};
/*
 * SCP_BL2U
 */
static const auth_img_desc_t scp_bl2u_image = {
	.img_id = SCP_BL2U_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &fwu_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &scp_bl2u_hash
			}
		}
	}
};
/*
 * BL2U
 */
static const auth_img_desc_t bl2u_image = {
	.img_id = BL2U_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &fwu_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &bl2u_hash
			}
		}
	}
};
/*
 * NS_BL2U
 */
static const auth_img_desc_t ns_bl2u_image = {
	.img_id = NS_BL2U_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &fwu_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &ns_bl2u_hash
				}
			}
		}
};
/*
 * TB_FW_CONFIG
 */
static const auth_img_desc_t tb_fw_config = {
	.img_id = TB_FW_CONFIG_ID,
	.img_type = IMG_RAW,
	.parent = &trusted_boot_fw_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &tb_fw_config_hash
			}
		}
	}
};

static const auth_img_desc_t fw_config = {
	.img_id = FW_CONFIG_ID,
	.img_type = IMG_RAW,
	.parent = &trusted_boot_fw_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &fw_config_hash
			}
		}
	}
};

/*
 * TBBR Chain of trust definition
 */
static const auth_img_desc_t * const cot_desc[] = {
	[TRUSTED_BOOT_FW_CERT_ID]		=	&trusted_boot_fw_cert,
	[BL2_IMAGE_ID]				=	&bl2_image,
	[HW_CONFIG_ID]				=	&hw_config,
	[TB_FW_CONFIG_ID]			=	&tb_fw_config,
	[FW_CONFIG_ID]				=	&fw_config,
	[FWU_CERT_ID]				=	&fwu_cert,
	[SCP_BL2U_IMAGE_ID]			=	&scp_bl2u_image,
	[BL2U_IMAGE_ID]				=	&bl2u_image,
	[NS_BL2U_IMAGE_ID]			=	&ns_bl2u_image
};

/* Register the CoT in the authentication module */
REGISTER_COT(cot_desc);
