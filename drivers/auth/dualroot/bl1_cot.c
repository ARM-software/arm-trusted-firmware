/*
 * Copyright (c) 2020-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include <mbedtls/version.h>

#include <common/tbbr/cot_def.h>
#include <drivers/auth/auth_mod.h>
#include <platform_def.h>
#include <tools_share/dualroot_oid.h>

/*
 * Allocate static buffers to store the authentication parameters extracted from
 * the certificates.
 */
static unsigned char fw_config_hash_buf[HASH_DER_LEN];
static unsigned char tb_fw_hash_buf[HASH_DER_LEN];
static unsigned char tb_fw_config_hash_buf[HASH_DER_LEN];
static unsigned char scp_fw_hash_buf[HASH_DER_LEN];
static unsigned char nt_world_bl_hash_buf[HASH_DER_LEN];

/*
 * Parameter type descriptors.
 */
static auth_param_type_desc_t trusted_nv_ctr = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_NV_CTR, TRUSTED_FW_NVCOUNTER_OID);
static auth_param_type_desc_t subject_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, 0);
static auth_param_type_desc_t sig = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_SIG, 0);
static auth_param_type_desc_t sig_alg = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_SIG_ALG, 0);
static auth_param_type_desc_t raw_data = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_RAW_DATA, 0);

static auth_param_type_desc_t tb_fw_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, TRUSTED_BOOT_FW_HASH_OID);
static auth_param_type_desc_t tb_fw_config_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, TRUSTED_BOOT_FW_CONFIG_HASH_OID);
static auth_param_type_desc_t fw_config_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, FW_CONFIG_HASH_OID);
static auth_param_type_desc_t scp_bl2u_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, SCP_FWU_CFG_HASH_OID);
static auth_param_type_desc_t bl2u_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, AP_FWU_CFG_HASH_OID);
static auth_param_type_desc_t ns_bl2u_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, FWU_HASH_OID);

static const auth_img_desc_t trusted_boot_fw_cert = {
	.img_id = TRUSTED_BOOT_FW_CERT_ID,
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
		},
		[1] = {
			.type = AUTH_METHOD_NV_CTR,
			.param.nv_ctr = {
				.cert_nv_ctr = &trusted_nv_ctr,
				.plat_nv_ctr = &trusted_nv_ctr
			}
		}
	},
	.authenticated_data = (const auth_param_desc_t[COT_MAX_VERIFIED_PARAMS]) {
		[0] = {
			.type_desc = &tb_fw_hash,
			.data = {
				.ptr = (void *)tb_fw_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[1] = {
			.type_desc = &tb_fw_config_hash,
			.data = {
				.ptr = (void *)tb_fw_config_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[2] = {
			.type_desc = &fw_config_hash,
			.data = {
				.ptr = (void *)fw_config_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		}
	}
};

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

/* TB FW Config */
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

/* FWU auth descriptor */
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

/* SCP_BL2U */
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

/* BL2U */
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

/* NS_BL2U */
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

static const auth_img_desc_t * const cot_desc[] = {
	[TRUSTED_BOOT_FW_CERT_ID]		=	&trusted_boot_fw_cert,
	[BL2_IMAGE_ID]				=	&bl2_image,
	[TB_FW_CONFIG_ID]			=	&tb_fw_config,
	[FW_CONFIG_ID]				=	&fw_config,
	[FWU_CERT_ID]				=	&fwu_cert,
	[SCP_BL2U_IMAGE_ID]			=	&scp_bl2u_image,
	[BL2U_IMAGE_ID]				=	&bl2u_image,
	[NS_BL2U_IMAGE_ID]			=	&ns_bl2u_image
};

REGISTER_COT(cot_desc);
