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
#if defined(SPD_spmd)
static unsigned char sp_pkg_hash_buf[MAX_SP_IDS][HASH_DER_LEN];
#endif /* SPD_spmd */

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
/* Secure Partitions */
#if defined(SPD_spmd)
static const auth_img_desc_t sip_sp_content_cert = {
	.img_id = SIP_SP_CONTENT_CERT_ID,
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
		},
		[4] = {
			.type_desc = &sp_pkg5_hash,
			.data = {
				.ptr = (void *)sp_pkg_hash_buf[4],
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[5] = {
			.type_desc = &sp_pkg6_hash,
			.data = {
				.ptr = (void *)sp_pkg_hash_buf[5],
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[6] = {
			.type_desc = &sp_pkg7_hash,
			.data = {
				.ptr = (void *)sp_pkg_hash_buf[6],
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[7] = {
			.type_desc = &sp_pkg8_hash,
			.data = {
				.ptr = (void *)sp_pkg_hash_buf[7],
				.len = (unsigned int)HASH_DER_LEN
			}
		}
	}
};

DEFINE_SIP_SP_PKG(1);
DEFINE_SIP_SP_PKG(2);
DEFINE_SIP_SP_PKG(3);
DEFINE_SIP_SP_PKG(4);
DEFINE_SIP_SP_PKG(5);
DEFINE_SIP_SP_PKG(6);
DEFINE_SIP_SP_PKG(7);
DEFINE_SIP_SP_PKG(8);
#endif /* SPD_spmd */

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
#if defined(SPD_spmd)
	[SIP_SP_CONTENT_CERT_ID]		=	&sip_sp_content_cert,
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

/* Register the CoT in the authentication module */
REGISTER_COT(cot_desc);
