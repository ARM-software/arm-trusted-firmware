/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 *
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include <drivers/auth/auth_mod.h>

#if USE_TBBR_DEFS
#include <tools_share/tbbr_oid.h>
#else
#include <platform_oid.h>
#endif


#if TF_MBEDTLS_HASH_ALG_ID == TF_MBEDTLS_SHA256
#define HASH_DER_LEN			51
#elif TF_MBEDTLS_HASH_ALG_ID == TF_MBEDTLS_SHA384
#define HASH_DER_LEN			67
#elif TF_MBEDTLS_HASH_ALG_ID == TF_MBEDTLS_SHA512
#define HASH_DER_LEN			83
#else
#error "Invalid value for TF_MBEDTLS_HASH_ALG_ID"
#endif

/*
 * The platform must allocate buffers to store the authentication parameters
 * extracted from the certificates. In this case, because of the way the CoT is
 * established, we can reuse some of the buffers on different stages
 */

static unsigned char nt_world_bl_hash_buf[HASH_DER_LEN];

static unsigned char soc_fw_hash_buf[HASH_DER_LEN];
static unsigned char tos_fw_hash_buf[HASH_DER_LEN];
static unsigned char tos_fw_extra1_hash_buf[HASH_DER_LEN];
static unsigned char tos_fw_extra2_hash_buf[HASH_DER_LEN];
static unsigned char trusted_world_pk_buf[PK_DER_LEN];
static unsigned char non_trusted_world_pk_buf[PK_DER_LEN];
static unsigned char content_pk_buf[PK_DER_LEN];
static unsigned char soc_fw_config_hash_buf[HASH_DER_LEN];
static unsigned char tos_fw_config_hash_buf[HASH_DER_LEN];
static unsigned char nt_fw_config_hash_buf[HASH_DER_LEN];

#ifdef CONFIG_DDR_FIP_IMAGE
static unsigned char ddr_fw_content_pk_buf[PK_DER_LEN];
static unsigned char ddr_imem_udimm_1d_hash_buf[HASH_DER_LEN];
static unsigned char ddr_imem_udimm_2d_hash_buf[HASH_DER_LEN];
static unsigned char ddr_dmem_udimm_1d_hash_buf[HASH_DER_LEN];
static unsigned char ddr_dmem_udimm_2d_hash_buf[HASH_DER_LEN];

static unsigned char ddr_imem_rdimm_1d_hash_buf[HASH_DER_LEN];
static unsigned char ddr_imem_rdimm_2d_hash_buf[HASH_DER_LEN];
static unsigned char ddr_dmem_rdimm_1d_hash_buf[HASH_DER_LEN];
static unsigned char ddr_dmem_rdimm_2d_hash_buf[HASH_DER_LEN];
#endif

/*
 * Parameter type descriptors
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


static auth_param_type_desc_t non_trusted_nv_ctr = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_NV_CTR, NON_TRUSTED_FW_NVCOUNTER_OID);
static auth_param_type_desc_t trusted_world_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, TRUSTED_WORLD_PK_OID);
static auth_param_type_desc_t non_trusted_world_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, NON_TRUSTED_WORLD_PK_OID);
static auth_param_type_desc_t soc_fw_content_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, SOC_FW_CONTENT_CERT_PK_OID);
static auth_param_type_desc_t tos_fw_content_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, TRUSTED_OS_FW_CONTENT_CERT_PK_OID);
static auth_param_type_desc_t nt_fw_content_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, NON_TRUSTED_FW_CONTENT_CERT_PK_OID);
static auth_param_type_desc_t soc_fw_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, SOC_AP_FW_HASH_OID);
static auth_param_type_desc_t soc_fw_config_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, SOC_FW_CONFIG_HASH_OID);
static auth_param_type_desc_t tos_fw_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, TRUSTED_OS_FW_HASH_OID);
static auth_param_type_desc_t tos_fw_config_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, TRUSTED_OS_FW_CONFIG_HASH_OID);
static auth_param_type_desc_t tos_fw_extra1_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, TRUSTED_OS_FW_EXTRA1_HASH_OID);
static auth_param_type_desc_t tos_fw_extra2_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, TRUSTED_OS_FW_EXTRA2_HASH_OID);
static auth_param_type_desc_t nt_world_bl_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, NON_TRUSTED_WORLD_BOOTLOADER_HASH_OID);
static auth_param_type_desc_t nt_fw_config_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, NON_TRUSTED_FW_CONFIG_HASH_OID);

#ifdef CONFIG_DDR_FIP_IMAGE
static auth_param_type_desc_t ddr_fw_content_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, DDR_FW_CONTENT_CERT_PK_OID);

static auth_param_type_desc_t ddr_imem_udimm_1d_fw_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, DDR_IMEM_UDIMM_1D_HASH_OID);
static auth_param_type_desc_t ddr_imem_udimm_2d_fw_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, DDR_IMEM_UDIMM_2D_HASH_OID);
static auth_param_type_desc_t ddr_dmem_udimm_1d_fw_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, DDR_DMEM_UDIMM_1D_HASH_OID);
static auth_param_type_desc_t ddr_dmem_udimm_2d_fw_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, DDR_DMEM_UDIMM_2D_HASH_OID);

static auth_param_type_desc_t ddr_imem_rdimm_1d_fw_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, DDR_IMEM_RDIMM_1D_HASH_OID);
static auth_param_type_desc_t ddr_imem_rdimm_2d_fw_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, DDR_IMEM_RDIMM_2D_HASH_OID);
static auth_param_type_desc_t ddr_dmem_rdimm_1d_fw_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, DDR_DMEM_RDIMM_1D_HASH_OID);
static auth_param_type_desc_t ddr_dmem_rdimm_2d_fw_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, DDR_DMEM_RDIMM_2D_HASH_OID);
#endif


/*
 * Trusted key certificate
 */
static const auth_img_desc_t trusted_key_cert = {
	.img_id = TRUSTED_KEY_CERT_ID,
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
			.type_desc = &trusted_world_pk,
			.data = {
				.ptr = (void *)trusted_world_pk_buf,
				.len = (unsigned int)PK_DER_LEN
			}
		},
		[1] = {
			.type_desc = &non_trusted_world_pk,
			.data = {
				.ptr = (void *)non_trusted_world_pk_buf,
				.len = (unsigned int)PK_DER_LEN
			}
		}
	}
};

/*
 * SoC Firmware
 */
static const auth_img_desc_t soc_fw_key_cert = {
	.img_id = SOC_FW_KEY_CERT_ID,
	.img_type = IMG_CERT,
	.parent = &trusted_key_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &trusted_world_pk,
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
			.type_desc = &soc_fw_content_pk,
			.data = {
				.ptr = (void *)content_pk_buf,
				.len = (unsigned int)PK_DER_LEN
			}
		}
	}
};
static const auth_img_desc_t soc_fw_content_cert = {
	.img_id = SOC_FW_CONTENT_CERT_ID,
	.img_type = IMG_CERT,
	.parent = &soc_fw_key_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &soc_fw_content_pk,
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
			.type_desc = &soc_fw_hash,
			.data = {
				.ptr = (void *)soc_fw_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[1] = {
			.type_desc = &soc_fw_config_hash,
			.data = {
				.ptr = (void *)soc_fw_config_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		}
	}
};
static const auth_img_desc_t bl31_image = {
	.img_id = BL31_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &soc_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &soc_fw_hash
			}
		}
	}
};
/* SOC FW Config */
static const auth_img_desc_t soc_fw_config = {
	.img_id = SOC_FW_CONFIG_ID,
	.img_type = IMG_RAW,
	.parent = &soc_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &soc_fw_config_hash
			}
		}
	}
};
/*
 * Trusted OS Firmware
 */
static const auth_img_desc_t trusted_os_fw_key_cert = {
	.img_id = TRUSTED_OS_FW_KEY_CERT_ID,
	.img_type = IMG_CERT,
	.parent = &trusted_key_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &trusted_world_pk,
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
			.type_desc = &tos_fw_content_pk,
			.data = {
				.ptr = (void *)content_pk_buf,
				.len = (unsigned int)PK_DER_LEN
			}
		}
	}
};
static const auth_img_desc_t trusted_os_fw_content_cert = {
	.img_id = TRUSTED_OS_FW_CONTENT_CERT_ID,
	.img_type = IMG_CERT,
	.parent = &trusted_os_fw_key_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &tos_fw_content_pk,
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
			.type_desc = &tos_fw_hash,
			.data = {
				.ptr = (void *)tos_fw_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[1] = {
			.type_desc = &tos_fw_extra1_hash,
			.data = {
				.ptr = (void *)tos_fw_extra1_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[2] = {
			.type_desc = &tos_fw_extra2_hash,
			.data = {
				.ptr = (void *)tos_fw_extra2_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[3] = {
			.type_desc = &tos_fw_config_hash,
			.data = {
				.ptr = (void *)tos_fw_config_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		}
	}
};
static const auth_img_desc_t bl32_image = {
	.img_id = BL32_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &trusted_os_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &tos_fw_hash
			}
		}
	}
};
static const auth_img_desc_t bl32_extra1_image = {
	.img_id = BL32_EXTRA1_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &trusted_os_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &tos_fw_extra1_hash
			}
		}
	}
};
static const auth_img_desc_t bl32_extra2_image = {
	.img_id = BL32_EXTRA2_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &trusted_os_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &tos_fw_extra2_hash
			}
		}
	}
};
/* TOS FW Config */
static const auth_img_desc_t tos_fw_config = {
	.img_id = TOS_FW_CONFIG_ID,
	.img_type = IMG_RAW,
	.parent = &trusted_os_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &tos_fw_config_hash
			}
		}
	}
};
/*
 * Non-Trusted Firmware
 */
static const auth_img_desc_t non_trusted_fw_key_cert = {
	.img_id = NON_TRUSTED_FW_KEY_CERT_ID,
	.img_type = IMG_CERT,
	.parent = &trusted_key_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &non_trusted_world_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &raw_data
			}
		},
		[1] = {
			.type = AUTH_METHOD_NV_CTR,
			.param.nv_ctr = {
				.cert_nv_ctr = &non_trusted_nv_ctr,
				.plat_nv_ctr = &non_trusted_nv_ctr
			}
		}
	},
	.authenticated_data = (const auth_param_desc_t[COT_MAX_VERIFIED_PARAMS]) {
		[0] = {
			.type_desc = &nt_fw_content_pk,
			.data = {
				.ptr = (void *)content_pk_buf,
				.len = (unsigned int)PK_DER_LEN
			}
		}
	}
};
static const auth_img_desc_t non_trusted_fw_content_cert = {
	.img_id = NON_TRUSTED_FW_CONTENT_CERT_ID,
	.img_type = IMG_CERT,
	.parent = &non_trusted_fw_key_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &nt_fw_content_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &raw_data
			}
		},
		[1] = {
			.type = AUTH_METHOD_NV_CTR,
			.param.nv_ctr = {
				.cert_nv_ctr = &non_trusted_nv_ctr,
				.plat_nv_ctr = &non_trusted_nv_ctr
			}
		}
	},
	.authenticated_data = (const auth_param_desc_t[COT_MAX_VERIFIED_PARAMS]) {
		[0] = {
			.type_desc = &nt_world_bl_hash,
			.data = {
				.ptr = (void *)nt_world_bl_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[1] = {
			.type_desc = &nt_fw_config_hash,
			.data = {
				.ptr = (void *)nt_fw_config_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		}
	}
};
static const auth_img_desc_t bl33_image = {
	.img_id = BL33_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &non_trusted_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &nt_world_bl_hash
			}
		}
	}
};
/* NT FW Config */
static const auth_img_desc_t nt_fw_config = {
	.img_id = NT_FW_CONFIG_ID,
	.img_type = IMG_RAW,
	.parent = &non_trusted_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &nt_fw_config_hash
			}
		}
	}
};
#ifdef CONFIG_DDR_FIP_IMAGE
/*
 * DDR Firmware
 */
static const auth_img_desc_t ddr_fw_key_cert = {
	.img_id = DDR_FW_KEY_CERT_ID,
	.img_type = IMG_CERT,
	.parent = &trusted_key_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &trusted_world_pk,
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
			.type_desc = &ddr_fw_content_pk,
			.data = {
				.ptr = (void *)ddr_fw_content_pk_buf,
				.len = (unsigned int)PK_DER_LEN
			}
		}
	}
};
static const auth_img_desc_t ddr_udimm_fw_content_cert = {
	.img_id = DDR_UDIMM_FW_CONTENT_CERT_ID,
	.img_type = IMG_CERT,
	.parent = &ddr_fw_key_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &ddr_fw_content_pk,
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
			.type_desc = &ddr_imem_udimm_1d_fw_hash,
			.data = {
				.ptr = (void *)ddr_imem_udimm_1d_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[1] = {
			.type_desc = &ddr_imem_udimm_2d_fw_hash,
			.data = {
				.ptr = (void *)ddr_imem_udimm_2d_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[2] = {
			.type_desc = &ddr_dmem_udimm_1d_fw_hash,
			.data = {
				.ptr = (void *)ddr_dmem_udimm_1d_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[3] = {
			.type_desc = &ddr_dmem_udimm_2d_fw_hash,
			.data = {
				.ptr = (void *)ddr_dmem_udimm_2d_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
	}
};

static const auth_img_desc_t ddr_imem_udimm_1d_img = {
	.img_id = DDR_IMEM_UDIMM_1D_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &ddr_udimm_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &ddr_imem_udimm_1d_fw_hash
			}
		}
	}
};
static const auth_img_desc_t ddr_imem_udimm_2d_img = {
	.img_id = DDR_IMEM_UDIMM_2D_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &ddr_udimm_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &ddr_imem_udimm_2d_fw_hash
			}
		}
	}
};
static const auth_img_desc_t ddr_dmem_udimm_1d_img = {
	.img_id = DDR_DMEM_UDIMM_1D_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &ddr_udimm_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &ddr_dmem_udimm_1d_fw_hash
			}
		}
	}
};
static const auth_img_desc_t ddr_dmem_udimm_2d_img = {
	.img_id = DDR_DMEM_UDIMM_2D_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &ddr_udimm_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &ddr_dmem_udimm_2d_fw_hash
			}
		}
	}
};

static const auth_img_desc_t ddr_rdimm_fw_content_cert = {
	.img_id = DDR_RDIMM_FW_CONTENT_CERT_ID,
	.img_type = IMG_CERT,
	.parent = &ddr_fw_key_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &ddr_fw_content_pk,
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
			.type_desc = &ddr_imem_rdimm_1d_fw_hash,
			.data = {
				.ptr = (void *)ddr_imem_rdimm_1d_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[1] = {
			.type_desc = &ddr_imem_rdimm_2d_fw_hash,
			.data = {
				.ptr = (void *)ddr_imem_rdimm_2d_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[2] = {
			.type_desc = &ddr_dmem_rdimm_1d_fw_hash,
			.data = {
				.ptr = (void *)ddr_dmem_rdimm_1d_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[3] = {
			.type_desc = &ddr_dmem_rdimm_2d_fw_hash,
			.data = {
				.ptr = (void *)ddr_dmem_rdimm_2d_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
	}
};

static const auth_img_desc_t ddr_imem_rdimm_1d_img = {
	.img_id = DDR_IMEM_RDIMM_1D_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &ddr_rdimm_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &ddr_imem_rdimm_1d_fw_hash
			}
		}
	}
};
static const auth_img_desc_t ddr_imem_rdimm_2d_img = {
	.img_id = DDR_IMEM_RDIMM_2D_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &ddr_rdimm_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &ddr_imem_rdimm_2d_fw_hash
			}
		}
	}
};
static const auth_img_desc_t ddr_dmem_rdimm_1d_img = {
	.img_id = DDR_DMEM_RDIMM_1D_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &ddr_rdimm_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &ddr_dmem_rdimm_1d_fw_hash
			}
		}
	}
};
static const auth_img_desc_t ddr_dmem_rdimm_2d_img = {
	.img_id = DDR_DMEM_RDIMM_2D_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &ddr_rdimm_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &ddr_dmem_rdimm_2d_fw_hash
			}
		}
	}
};
#endif

/*
 * TBBR Chain of trust definition
 */

static const auth_img_desc_t * const cot_desc[] = {
	[TRUSTED_KEY_CERT_ID]			=	&trusted_key_cert,
	[SOC_FW_KEY_CERT_ID]			=	&soc_fw_key_cert,
	[SOC_FW_CONTENT_CERT_ID]		=	&soc_fw_content_cert,
	[BL31_IMAGE_ID]				=	&bl31_image,
	[SOC_FW_CONFIG_ID]			=	&soc_fw_config,
	[TRUSTED_OS_FW_KEY_CERT_ID]		=	&trusted_os_fw_key_cert,
	[TRUSTED_OS_FW_CONTENT_CERT_ID]		=	&trusted_os_fw_content_cert,
	[BL32_IMAGE_ID]				=	&bl32_image,
	[BL32_EXTRA1_IMAGE_ID]			=	&bl32_extra1_image,
	[BL32_EXTRA2_IMAGE_ID]			=	&bl32_extra2_image,
	[TOS_FW_CONFIG_ID]			=	&tos_fw_config,
	[NON_TRUSTED_FW_KEY_CERT_ID]		=	&non_trusted_fw_key_cert,
	[NON_TRUSTED_FW_CONTENT_CERT_ID]	=	&non_trusted_fw_content_cert,
	[BL33_IMAGE_ID]				=	&bl33_image,
	[NT_FW_CONFIG_ID]			=	&nt_fw_config,
#ifdef CONFIG_DDR_FIP_IMAGE
	[DDR_FW_KEY_CERT_ID]			=	&ddr_fw_key_cert,
	[DDR_UDIMM_FW_CONTENT_CERT_ID]		=	&ddr_udimm_fw_content_cert,
	[DDR_RDIMM_FW_CONTENT_CERT_ID]		=	&ddr_rdimm_fw_content_cert,
	[DDR_IMEM_UDIMM_1D_IMAGE_ID]		=	&ddr_imem_udimm_1d_img,
	[DDR_IMEM_UDIMM_2D_IMAGE_ID]		=	&ddr_imem_udimm_2d_img,
	[DDR_DMEM_UDIMM_1D_IMAGE_ID]		=	&ddr_dmem_udimm_1d_img,
	[DDR_DMEM_UDIMM_2D_IMAGE_ID]		=	&ddr_dmem_udimm_2d_img,
	[DDR_IMEM_RDIMM_1D_IMAGE_ID]		=	&ddr_imem_rdimm_1d_img,
	[DDR_IMEM_RDIMM_2D_IMAGE_ID]		=	&ddr_imem_rdimm_2d_img,
	[DDR_DMEM_RDIMM_1D_IMAGE_ID]		=	&ddr_dmem_rdimm_1d_img,
	[DDR_DMEM_RDIMM_2D_IMAGE_ID]		=	&ddr_dmem_rdimm_2d_img,
#endif
};

/* Register the CoT in the authentication module */
REGISTER_COT(cot_desc);
