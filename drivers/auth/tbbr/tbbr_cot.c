/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include <platform_def.h>

#include <drivers/auth/auth_mod.h>
#if USE_TBBR_DEFS
#include <tools_share/tbbr_oid.h>
#else
#include <platform_oid.h>
#endif


/*
 * Maximum key and hash sizes (in DER format)
 */
#define PK_DER_LEN			294
#define HASH_DER_LEN			83

/*
 * The platform must allocate buffers to store the authentication parameters
 * extracted from the certificates. In this case, because of the way the CoT is
 * established, we can reuse some of the buffers on different stages
 */

static unsigned char tb_fw_hash_buf[HASH_DER_LEN];
static unsigned char tb_fw_config_hash_buf[HASH_DER_LEN];
static unsigned char hw_config_hash_buf[HASH_DER_LEN];
static unsigned char scp_fw_hash_buf[HASH_DER_LEN];
static unsigned char nt_world_bl_hash_buf[HASH_DER_LEN];

#ifdef IMAGE_BL2
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


static auth_param_type_desc_t tb_fw_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, TRUSTED_BOOT_FW_HASH_OID);
static auth_param_type_desc_t tb_fw_config_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, TRUSTED_BOOT_FW_CONFIG_HASH_OID);
static auth_param_type_desc_t hw_config_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, HW_CONFIG_HASH_OID);
#ifdef IMAGE_BL1
static auth_param_type_desc_t scp_bl2u_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, SCP_FWU_CFG_HASH_OID);
static auth_param_type_desc_t bl2u_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, AP_FWU_CFG_HASH_OID);
static auth_param_type_desc_t ns_bl2u_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, FWU_HASH_OID);
#endif /* IMAGE_BL1 */

#ifdef IMAGE_BL2
static auth_param_type_desc_t non_trusted_nv_ctr = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_NV_CTR, NON_TRUSTED_FW_NVCOUNTER_OID);
static auth_param_type_desc_t trusted_world_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, TRUSTED_WORLD_PK_OID);
static auth_param_type_desc_t non_trusted_world_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, NON_TRUSTED_WORLD_PK_OID);
static auth_param_type_desc_t scp_fw_content_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, SCP_FW_CONTENT_CERT_PK_OID);
static auth_param_type_desc_t soc_fw_content_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, SOC_FW_CONTENT_CERT_PK_OID);
static auth_param_type_desc_t tos_fw_content_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, TRUSTED_OS_FW_CONTENT_CERT_PK_OID);
static auth_param_type_desc_t nt_fw_content_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, NON_TRUSTED_FW_CONTENT_CERT_PK_OID);
static auth_param_type_desc_t scp_fw_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, SCP_FW_HASH_OID);
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

#endif /* IMAGE_BL2 */


	/*
	 * BL2
	 */
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
			.type_desc = &hw_config_hash,
			.data = {
				.ptr = (void *)hw_config_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		}
	}
	};
#ifdef IMAGE_BL1
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
#endif /* IMAGE_BL1 */
/* HW Config */
static const auth_img_desc_t hw_config = {
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
/* TB FW Config */
#ifdef IMAGE_BL1
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
#endif /* IMAGE_BL1 */
#ifdef IMAGE_BL2
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
 * SCP Firmware
 */
static const auth_img_desc_t scp_fw_key_cert = {
	.img_id = SCP_FW_KEY_CERT_ID,
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
			.type_desc = &scp_fw_content_pk,
			.data = {
				.ptr = (void *)content_pk_buf,
				.len = (unsigned int)PK_DER_LEN
			}
		}
	}
};
static const auth_img_desc_t scp_fw_content_cert = {
	.img_id = SCP_FW_CONTENT_CERT_ID,
	.img_type = IMG_CERT,
	.parent = &scp_fw_key_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &scp_fw_content_pk,
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
			.type_desc = &scp_fw_hash,
			.data = {
				.ptr = (void *)scp_fw_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		}
	}
};
static const auth_img_desc_t scp_bl2_image = {
	.img_id = SCP_BL2_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &scp_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &scp_fw_hash
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
#else  /* IMAGE_BL2 */
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
#endif /* IMAGE_BL2 */
/*
 * TBBR Chain of trust definition
 */

#ifdef IMAGE_BL1
static const auth_img_desc_t * const cot_desc[] = {
	[TRUSTED_BOOT_FW_CERT_ID]		=	&trusted_boot_fw_cert,
	[BL2_IMAGE_ID]				=	&bl2_image,
	[HW_CONFIG_ID]				=	&hw_config,
	[TB_FW_CONFIG_ID]			=	&tb_fw_config,
	[FWU_CERT_ID]				=	&fwu_cert,
	[SCP_BL2U_IMAGE_ID]			=	&scp_bl2u_image,
	[BL2U_IMAGE_ID]				=	&bl2u_image,
	[NS_BL2U_IMAGE_ID]			=	&ns_bl2u_image
};
#else /* IMAGE_BL2 */
static const auth_img_desc_t * const cot_desc[] = {
	[TRUSTED_BOOT_FW_CERT_ID]		=	&trusted_boot_fw_cert,
	[HW_CONFIG_ID]				=	&hw_config,
	[TRUSTED_KEY_CERT_ID]			=	&trusted_key_cert,
	[SCP_FW_KEY_CERT_ID]			=	&scp_fw_key_cert,
	[SCP_FW_CONTENT_CERT_ID]		=	&scp_fw_content_cert,
	[SCP_BL2_IMAGE_ID]			=	&scp_bl2_image,
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
};
#endif

/* Register the CoT in the authentication module */
REGISTER_COT(cot_desc);
