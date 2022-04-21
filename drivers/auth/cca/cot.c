/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include <drivers/auth/auth_mod.h>
#include MBEDTLS_CONFIG_FILE
#include <tools_share/cca_oid.h>

#include <platform_def.h>

/*
 * Allocate static buffers to store the authentication parameters extracted from
 * the certificates.
 */
static unsigned char fw_config_hash_buf[HASH_DER_LEN];
static unsigned char tb_fw_hash_buf[HASH_DER_LEN];
static unsigned char tb_fw_config_hash_buf[HASH_DER_LEN];
static unsigned char hw_config_hash_buf[HASH_DER_LEN];
static unsigned char soc_fw_hash_buf[HASH_DER_LEN];
static unsigned char soc_fw_config_hash_buf[HASH_DER_LEN];
static unsigned char rmm_hash_buf[HASH_DER_LEN];

#ifdef IMAGE_BL2
static unsigned char nt_world_bl_hash_buf[HASH_DER_LEN];
static unsigned char tos_fw_hash_buf[HASH_DER_LEN];
static unsigned char tos_fw_config_hash_buf[HASH_DER_LEN];
static unsigned char nt_fw_config_hash_buf[HASH_DER_LEN];
#if defined(SPD_spmd)
static unsigned char sp_pkg_hash_buf[MAX_SP_IDS][HASH_DER_LEN];
#endif /* SPD_spmd */

static unsigned char core_swd_pk_buf[PK_DER_LEN];
static unsigned char plat_pk_buf[PK_DER_LEN];
#endif /* IMAGE_BL2 */

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
static auth_param_type_desc_t hw_config_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, HW_CONFIG_HASH_OID);
static auth_param_type_desc_t fw_config_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, FW_CONFIG_HASH_OID);
static auth_param_type_desc_t soc_fw_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, SOC_AP_FW_HASH_OID);
static auth_param_type_desc_t soc_fw_config_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, SOC_FW_CONFIG_HASH_OID);
static auth_param_type_desc_t rmm_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, RMM_HASH_OID);

#ifdef IMAGE_BL2
static auth_param_type_desc_t non_trusted_nv_ctr = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_NV_CTR, NON_TRUSTED_FW_NVCOUNTER_OID);

static auth_param_type_desc_t prot_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, PROT_PK_OID);
static auth_param_type_desc_t swd_rot_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, SWD_ROT_PK_OID);
static auth_param_type_desc_t core_swd_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, CORE_SWD_PK_OID);
static auth_param_type_desc_t plat_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, PLAT_PK_OID);

static auth_param_type_desc_t tos_fw_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, TRUSTED_OS_FW_HASH_OID);
static auth_param_type_desc_t tos_fw_config_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, TRUSTED_OS_FW_CONFIG_HASH_OID);
static auth_param_type_desc_t nt_world_bl_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, NON_TRUSTED_WORLD_BOOTLOADER_HASH_OID);
static auth_param_type_desc_t nt_fw_config_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, NON_TRUSTED_FW_CONFIG_HASH_OID);
#if defined(SPD_spmd)
static auth_param_type_desc_t sp_pkg1_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, SP_PKG1_HASH_OID);
static auth_param_type_desc_t sp_pkg2_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, SP_PKG2_HASH_OID);
static auth_param_type_desc_t sp_pkg3_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, SP_PKG3_HASH_OID);
static auth_param_type_desc_t sp_pkg4_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, SP_PKG4_HASH_OID);
static auth_param_type_desc_t sp_pkg5_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, SP_PKG5_HASH_OID);
static auth_param_type_desc_t sp_pkg6_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, SP_PKG6_HASH_OID);
static auth_param_type_desc_t sp_pkg7_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, SP_PKG7_HASH_OID);
static auth_param_type_desc_t sp_pkg8_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, SP_PKG8_HASH_OID);
#endif /* SPD_spmd */
#endif /* IMAGE_BL2 */

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
		},
		[3] = {
			.type_desc = &hw_config_hash,
			.data = {
				.ptr = (void *)hw_config_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[4] = {
			.type_desc = &soc_fw_hash,
			.data = {
				.ptr = (void *)soc_fw_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[5] = {
			.type_desc = &soc_fw_config_hash,
			.data = {
				.ptr = (void *)soc_fw_config_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[6] = {
			.type_desc = &rmm_hash,
			.data = {
				.ptr = (void *)rmm_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		}
	}
};

#ifdef IMAGE_BL1
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
#endif /* IMAGE_BL1 */

#ifdef IMAGE_BL2
/* HW Config */
static const auth_img_desc_t hw_config = {
	.img_id = HW_CONFIG_ID,
	.img_type = IMG_RAW,
	.parent = &cca_content_cert,
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

/* BL31 */
static const auth_img_desc_t bl31_image = {
	.img_id = BL31_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &cca_content_cert,
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

/* BL31 Config */
static const auth_img_desc_t soc_fw_config = {
	.img_id = SOC_FW_CONFIG_ID,
	.img_type = IMG_RAW,
	.parent = &cca_content_cert,
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

/* RMM */
static const auth_img_desc_t rmm_image = {
	.img_id = RMM_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &cca_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &rmm_hash
			}
		}
	}
};

/* Core SWD Key Certificate */
static const auth_img_desc_t core_swd_key_cert = {
	.img_id = CORE_SWD_KEY_CERT_ID,
	.img_type = IMG_CERT,
	.parent = NULL, /* SWD ROOT CERT */
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &swd_rot_pk,
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
			.type_desc = &core_swd_pk,
			.data = {
				.ptr = (void *)core_swd_pk_buf,
				.len = (unsigned int)PK_DER_LEN
			}
		}
	}
};

/* SPMC Content Certificate */
static const auth_img_desc_t trusted_os_fw_content_cert = {
	.img_id = TRUSTED_OS_FW_CONTENT_CERT_ID,
	.img_type = IMG_CERT,
	.parent = &core_swd_key_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &core_swd_pk,
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
			.type_desc = &tos_fw_config_hash,
			.data = {
				.ptr = (void *)tos_fw_config_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		}
	}
};

/* SPMC */
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

/* SPM Config */
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

/* Platform Key Certificate */
static const auth_img_desc_t plat_key_cert = {
	.img_id = PLAT_KEY_CERT_ID,
	.img_type = IMG_CERT,
	.parent = NULL, /* PLATFORM ROOT CERT */
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &prot_pk,
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
			.type_desc = &plat_pk,
			.data = {
				.ptr = (void *)plat_pk_buf,
				.len = (unsigned int)PK_DER_LEN
			}
		}
	}
};

/* Non-Trusted Firmware */
static const auth_img_desc_t non_trusted_fw_content_cert = {
	.img_id = NON_TRUSTED_FW_CONTENT_CERT_ID,
	.img_type = IMG_CERT,
	.parent = &plat_key_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &plat_pk,
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

/*
 * Secure Partitions
 */
#if defined(SPD_spmd)
static const auth_img_desc_t sip_sp_content_cert = {
	.img_id = SIP_SP_CONTENT_CERT_ID,
	.img_type = IMG_CERT,
	.parent = &core_swd_key_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &core_swd_pk,
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
			.type_desc = &sp_pkg1_hash,
			.data = {
				.ptr = (void *)sp_pkg_hash_buf[0],
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[1] = {
			.type_desc = &sp_pkg2_hash,
			.data = {
				.ptr = (void *)sp_pkg_hash_buf[1],
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[2] = {
			.type_desc = &sp_pkg3_hash,
			.data = {
				.ptr = (void *)sp_pkg_hash_buf[2],
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[3] = {
			.type_desc = &sp_pkg4_hash,
			.data = {
				.ptr = (void *)sp_pkg_hash_buf[3],
				.len = (unsigned int)HASH_DER_LEN
			}
		}
	}
};

DEFINE_SIP_SP_PKG(1);
DEFINE_SIP_SP_PKG(2);
DEFINE_SIP_SP_PKG(3);
DEFINE_SIP_SP_PKG(4);

static const auth_img_desc_t plat_sp_content_cert = {
	.img_id = PLAT_SP_CONTENT_CERT_ID,
	.img_type = IMG_CERT,
	.parent = &plat_key_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &plat_pk,
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
			.type_desc = &sp_pkg5_hash,
			.data = {
				.ptr = (void *)sp_pkg_hash_buf[4],
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[1] = {
			.type_desc = &sp_pkg6_hash,
			.data = {
				.ptr = (void *)sp_pkg_hash_buf[5],
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[2] = {
			.type_desc = &sp_pkg7_hash,
			.data = {
				.ptr = (void *)sp_pkg_hash_buf[6],
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[3] = {
			.type_desc = &sp_pkg8_hash,
			.data = {
				.ptr = (void *)sp_pkg_hash_buf[7],
				.len = (unsigned int)HASH_DER_LEN
			}
		}
	}
};

DEFINE_PLAT_SP_PKG(5);
DEFINE_PLAT_SP_PKG(6);
DEFINE_PLAT_SP_PKG(7);
DEFINE_PLAT_SP_PKG(8);
#endif /* SPD_spmd */
#endif /* IMAGE_BL2 */
/*
 * Chain of trust definition
 */
#ifdef IMAGE_BL1
static const auth_img_desc_t * const cot_desc[] = {
	[CCA_CONTENT_CERT_ID]			=	&cca_content_cert,
	[BL2_IMAGE_ID]				=	&bl2_image,
	[TB_FW_CONFIG_ID]			=	&tb_fw_config,
	[FW_CONFIG_ID]				=	&fw_config,
};
#else /* IMAGE_BL2 */
static const auth_img_desc_t * const cot_desc[] = {
	[CCA_CONTENT_CERT_ID]			=	&cca_content_cert,
	[HW_CONFIG_ID]				=	&hw_config,
	[BL31_IMAGE_ID]				=	&bl31_image,
	[SOC_FW_CONFIG_ID]			=	&soc_fw_config,
	[RMM_IMAGE_ID]				=	&rmm_image,
	[CORE_SWD_KEY_CERT_ID]			=	&core_swd_key_cert,
	[TRUSTED_OS_FW_CONTENT_CERT_ID]		=	&trusted_os_fw_content_cert,
	[BL32_IMAGE_ID]				=	&bl32_image,
	[TOS_FW_CONFIG_ID]			=	&tos_fw_config,
	[PLAT_KEY_CERT_ID]			=	&plat_key_cert,
	[NON_TRUSTED_FW_CONTENT_CERT_ID]	=	&non_trusted_fw_content_cert,
	[BL33_IMAGE_ID]				=	&bl33_image,
	[NT_FW_CONFIG_ID]			=	&nt_fw_config,
#if defined(SPD_spmd)
	[SIP_SP_CONTENT_CERT_ID]		=	&sip_sp_content_cert,
	[PLAT_SP_CONTENT_CERT_ID]		=	&plat_sp_content_cert,
	[SP_PKG1_ID]				=	&sp_pkg1,
	[SP_PKG2_ID]				=	&sp_pkg2,
	[SP_PKG3_ID]				=	&sp_pkg3,
	[SP_PKG4_ID]				=	&sp_pkg4,
	[SP_PKG5_ID]				=	&sp_pkg5,
	[SP_PKG6_ID]				=	&sp_pkg6,
	[SP_PKG7_ID]				=	&sp_pkg7,
	[SP_PKG8_ID]				=       &sp_pkg8,
#endif
};
#endif /* IMAGE_BL1 */

/* Register the CoT in the authentication module */
REGISTER_COT(cot_desc);
