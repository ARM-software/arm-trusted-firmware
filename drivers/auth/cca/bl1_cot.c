/*
 * Copyright (c) 2022-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include <mbedtls/version.h>

#include <common/tbbr/cot_def.h>
#include <drivers/auth/auth_mod.h>
#include <platform_def.h>
#include <tools_share/cca_oid.h>

/*
 * Allocate static buffers to store the authentication parameters extracted from
 * the certificates.
 */
static unsigned char fw_config_hash_buf[HASH_DER_LEN];
static unsigned char tb_fw_hash_buf[HASH_DER_LEN];
static unsigned char tb_fw_config_hash_buf[HASH_DER_LEN];

/*
 * Parameter type descriptors.
 */
static auth_param_type_desc_t cca_nv_ctr = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_NV_CTR, CCA_FW_NVCOUNTER_OID);
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

/* CCA Content Certificate */
static const auth_img_desc_t cca_content_cert = {
	.img_id = CCA_CONTENT_CERT_ID,
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
				.cert_nv_ctr = &cca_nv_ctr,
				.plat_nv_ctr = &cca_nv_ctr
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
	.parent = &cca_content_cert,
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

static const auth_img_desc_t tb_fw_config = {
	.img_id = TB_FW_CONFIG_ID,
	.img_type = IMG_RAW,
	.parent = &cca_content_cert,
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
	.parent = &cca_content_cert,
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

static const auth_img_desc_t * const cot_desc[] = {
	[CCA_CONTENT_CERT_ID]			=	&cca_content_cert,
	[BL2_IMAGE_ID]				=	&bl2_image,
	[TB_FW_CONFIG_ID]			=	&tb_fw_config,
	[FW_CONFIG_ID]				=	&fw_config,
};

REGISTER_COT(cot_desc);
