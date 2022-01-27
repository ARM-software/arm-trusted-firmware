/*
 * Copyright (c) 2015-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include <platform_def.h>
#include MBEDTLS_CONFIG_FILE

#include <drivers/auth/auth_mod.h>
#include <drivers/auth/tbbr_cot_common.h>
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

static unsigned char fw_config_hash_buf[HASH_DER_LEN];
static unsigned char tb_fw_config_hash_buf[HASH_DER_LEN];
static unsigned char hw_config_hash_buf[HASH_DER_LEN];
unsigned char tb_fw_hash_buf[HASH_DER_LEN];
unsigned char scp_fw_hash_buf[HASH_DER_LEN];
unsigned char nt_world_bl_hash_buf[HASH_DER_LEN];

/*
 * common Parameter type descriptors across BL1 and BL2
 */
auth_param_type_desc_t trusted_nv_ctr = AUTH_PARAM_TYPE_DESC(
	AUTH_PARAM_NV_CTR, TRUSTED_FW_NVCOUNTER_OID);
auth_param_type_desc_t subject_pk = AUTH_PARAM_TYPE_DESC(
	AUTH_PARAM_PUB_KEY, 0);
auth_param_type_desc_t sig = AUTH_PARAM_TYPE_DESC(
	AUTH_PARAM_SIG, 0);
auth_param_type_desc_t sig_alg = AUTH_PARAM_TYPE_DESC(
	AUTH_PARAM_SIG_ALG, 0);
auth_param_type_desc_t raw_data = AUTH_PARAM_TYPE_DESC(
	AUTH_PARAM_RAW_DATA, 0);

/* common hash used across BL1 and BL2 */
auth_param_type_desc_t tb_fw_hash = AUTH_PARAM_TYPE_DESC(
	AUTH_PARAM_HASH, TRUSTED_BOOT_FW_HASH_OID);
auth_param_type_desc_t tb_fw_config_hash = AUTH_PARAM_TYPE_DESC(
	AUTH_PARAM_HASH, TRUSTED_BOOT_FW_CONFIG_HASH_OID);
auth_param_type_desc_t fw_config_hash = AUTH_PARAM_TYPE_DESC(
	AUTH_PARAM_HASH, FW_CONFIG_HASH_OID);
static auth_param_type_desc_t hw_config_hash = AUTH_PARAM_TYPE_DESC(
	AUTH_PARAM_HASH, HW_CONFIG_HASH_OID);

/* trusted_boot_fw_cert */
const auth_img_desc_t trusted_boot_fw_cert = {
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
			.type_desc = &hw_config_hash,
			.data = {
				.ptr = (void *)hw_config_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[3] = {
			.type_desc = &fw_config_hash,
			.data = {
				.ptr = (void *)fw_config_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		}
	}
};

/* HW Config */
const auth_img_desc_t hw_config = {
	.img_id = HW_CONFIG_ID,
	.img_type = IMG_RAW,
	.parent = &trusted_boot_fw_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &hw_config_hash
			}
		}
	}
};
