/*
 * Copyright (c) 2017, Cavium Inc. All rights reserved.<BR>
 * Copyright (c) 2018, Facebook, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <auth_mod.h>
#include <crypto_mod.h>
#include <platform_def.h>
#include <stddef.h>

/*
 * Maximum key and hash sizes (in DER format)
 */
#define PK_DER_LEN			294
#define HASH_DER_LEN			51

/*
 * The platform must allocate buffers to store the authentication parameters
 * extracted from the certificates. In this case, because of the way the CoT is
 * established, we can reuse some of the buffers on different stages
 */
static unsigned char tb_fw_hash_buf[HASH_DER_LEN];
static unsigned char soc_fw_hash_buf[HASH_DER_LEN];
static unsigned char nt_world_bl_hash_buf[HASH_DER_LEN];
static unsigned char trusted_world_pk_buf[PK_DER_LEN];
static unsigned char non_trusted_world_pk_buf[PK_DER_LEN];
static unsigned char content_pk_buf[PK_DER_LEN];

/*
 * Parameter type descriptors
 */
static auth_param_type_desc_t trusted_nv_ctr = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_NV_CTR, TRUSTED_FW_NVCOUNTER_OID);
static auth_param_type_desc_t non_trusted_nv_ctr = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_NV_CTR, NON_TRUSTED_FW_NVCOUNTER_OID);

static auth_param_type_desc_t subject_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, 0);
static auth_param_type_desc_t sig = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_SIG, 0);
static auth_param_type_desc_t sig_alg = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_SIG_ALG, 0);
static auth_param_type_desc_t raw_data = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_RAW_DATA, 0);

static auth_param_type_desc_t trusted_world_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, TRUSTED_WORLD_PK_OID);
static auth_param_type_desc_t non_trusted_world_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, NON_TRUSTED_WORLD_PK_OID);

static auth_param_type_desc_t soc_fw_content_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, SOC_FW_CONTENT_CERT_PK_OID);
static auth_param_type_desc_t nt_fw_content_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, NON_TRUSTED_FW_CONTENT_CERT_PK_OID);

static auth_param_type_desc_t tb_fw_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, TRUSTED_BOOT_FW_HASH_OID);
static auth_param_type_desc_t soc_fw_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, SOC_AP_FW_HASH_OID);
static auth_param_type_desc_t nt_world_bl_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, NON_TRUSTED_WORLD_BOOTLOADER_HASH_OID);


/*
 * TBBR Chain of trust definition
 */
static const auth_img_desc_t cot_desc[] = {
	/*
	 * BL2
	 */
	[TRUSTED_BOOT_FW_CERT_ID] = {
		.img_id = TRUSTED_BOOT_FW_CERT_ID,
		.img_type = IMG_CERT,
		.parent = NULL,
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_SIG,
				.param.sig = {
					.pk = &subject_pk,
					.sig = &sig,
					.alg = &sig_alg,
					.data = &raw_data,
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
		.authenticated_data = {
			[0] = {
				.type_desc = &tb_fw_hash,
				.data = {
					.ptr = (void *)tb_fw_hash_buf,
					.len = (unsigned int)HASH_DER_LEN
				}
			}
		}
	},
	[BL2_IMAGE_ID] = {
		.img_id = BL2_IMAGE_ID,
		.img_type = IMG_RAW,
		.parent = &cot_desc[TRUSTED_BOOT_FW_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_HASH,
				.param.hash = {
					.data = &raw_data,
					.hash = &tb_fw_hash,
				}
			}
		}
	},
	/*
	 * Trusted key certificate
	 */
	[TRUSTED_KEY_CERT_ID] = {
		.img_id = TRUSTED_KEY_CERT_ID,
		.img_type = IMG_CERT,
		.parent = NULL,
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_SIG,
				.param.sig = {
					.pk = &subject_pk,
					.sig = &sig,
					.alg = &sig_alg,
					.data = &raw_data,
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
		.authenticated_data = {
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
	},
	/*
	 * SCP Firmware, not used by Cavium platform
	 */
#if 0
	[SCP_FW_KEY_CERT_ID] = {
		.img_id = SCP_FW_KEY_CERT_ID,
		.img_type = IMG_CERT,
		.parent = &cot_desc[TRUSTED_KEY_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_SIG,
				.param.sig = {
					.pk = &trusted_world_pk,
					.sig = &sig,
					.alg = &sig_alg,
					.data = &raw_data,
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
		.authenticated_data = {
			[0] = {
				.type_desc = &scp_fw_content_pk,
				.data = {
					.ptr = (void *)content_pk_buf,
					.len = (unsigned int)PK_DER_LEN
				}
			}
		}
	},
	[SCP_FW_CONTENT_CERT_ID] = {
		.img_id = SCP_FW_CONTENT_CERT_ID,
		.img_type = IMG_CERT,
		.parent = &cot_desc[SCP_FW_KEY_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_SIG,
				.param.sig = {
					.pk = &scp_fw_content_pk,
					.sig = &sig,
					.alg = &sig_alg,
					.data = &raw_data,
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
		.authenticated_data = {
			[0] = {
				.type_desc = &scp_fw_hash,
				.data = {
					.ptr = (void *)scp_fw_hash_buf,
					.len = (unsigned int)HASH_DER_LEN
				}
			}
		}
	},
	[SCP_BL2_IMAGE_ID] = {
		.img_id = SCP_BL2_IMAGE_ID,
		.img_type = IMG_RAW,
		.parent = &cot_desc[SCP_FW_CONTENT_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_HASH,
				.param.hash = {
					.data = &raw_data,
					.hash = &scp_fw_hash,
				}
			}
		}
	},
#endif
	/*
	 * SoC Firmware
	 */
	[SOC_FW_KEY_CERT_ID] = {
		.img_id = SOC_FW_KEY_CERT_ID,
		.img_type = IMG_CERT,
		.parent = &cot_desc[TRUSTED_KEY_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_SIG,
				.param.sig = {
					.pk = &trusted_world_pk,
					.sig = &sig,
					.alg = &sig_alg,
					.data = &raw_data,
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
		.authenticated_data = {
			[0] = {
				.type_desc = &soc_fw_content_pk,
				.data = {
					.ptr = (void *)content_pk_buf,
					.len = (unsigned int)PK_DER_LEN
				}
			}
		}
	},
	[SOC_FW_CONTENT_CERT_ID] = {
		.img_id = SOC_FW_CONTENT_CERT_ID,
		.img_type = IMG_CERT,
		.parent = &cot_desc[SOC_FW_KEY_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_SIG,
				.param.sig = {
					.pk = &soc_fw_content_pk,
					.sig = &sig,
					.alg = &sig_alg,
					.data = &raw_data,
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
		.authenticated_data = {
			[0] = {
				.type_desc = &soc_fw_hash,
				.data = {
					.ptr = (void *)soc_fw_hash_buf,
					.len = (unsigned int)HASH_DER_LEN
				}
			}
		}
	},
	[BL31_IMAGE_ID] = {
		.img_id = BL31_IMAGE_ID,
		.img_type = IMG_RAW,
		.parent = &cot_desc[SOC_FW_CONTENT_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_HASH,
				.param.hash = {
					.data = &raw_data,
					.hash = &soc_fw_hash,
				}
			}
		}
	},
	/*
	 * Trusted OS Firmware, not used by Cavium platform
	 */
#if 0
	[TRUSTED_OS_FW_KEY_CERT_ID] = {
		.img_id = TRUSTED_OS_FW_KEY_CERT_ID,
		.img_type = IMG_CERT,
		.parent = &cot_desc[TRUSTED_KEY_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_SIG,
				.param.sig = {
					.pk = &trusted_world_pk,
					.sig = &sig,
					.alg = &sig_alg,
					.data = &raw_data,
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
		.authenticated_data = {
			[0] = {
				.type_desc = &tos_fw_content_pk,
				.data = {
					.ptr = (void *)content_pk_buf,
					.len = (unsigned int)PK_DER_LEN
				}
			}
		}
	},
	[TRUSTED_OS_FW_CONTENT_CERT_ID] = {
		.img_id = TRUSTED_OS_FW_CONTENT_CERT_ID,
		.img_type = IMG_CERT,
		.parent = &cot_desc[TRUSTED_OS_FW_KEY_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_SIG,
				.param.sig = {
					.pk = &tos_fw_content_pk,
					.sig = &sig,
					.alg = &sig_alg,
					.data = &raw_data,
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
		.authenticated_data = {
			[0] = {
				.type_desc = &tos_fw_hash,
				.data = {
					.ptr = (void *)tos_fw_hash_buf,
					.len = (unsigned int)HASH_DER_LEN
				}
			}
		}
	},
	[BL32_IMAGE_ID] = {
		.img_id = BL32_IMAGE_ID,
		.img_type = IMG_RAW,
		.parent = &cot_desc[TRUSTED_OS_FW_CONTENT_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_HASH,
				.param.hash = {
					.data = &raw_data,
					.hash = &tos_fw_hash,
				}
			}
		}
	},
#endif
	/*
	 * Non-Trusted Firmware
	 */
	[NON_TRUSTED_FW_KEY_CERT_ID] = {
		.img_id = NON_TRUSTED_FW_KEY_CERT_ID,
		.img_type = IMG_CERT,
		.parent = &cot_desc[TRUSTED_KEY_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_SIG,
				.param.sig = {
					.pk = &non_trusted_world_pk,
					.sig = &sig,
					.alg = &sig_alg,
					.data = &raw_data,
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
		.authenticated_data = {
			[0] = {
				.type_desc = &nt_fw_content_pk,
				.data = {
					.ptr = (void *)content_pk_buf,
					.len = (unsigned int)PK_DER_LEN
				}
			}
		}
	},
	[NON_TRUSTED_FW_CONTENT_CERT_ID] = {
		.img_id = NON_TRUSTED_FW_CONTENT_CERT_ID,
		.img_type = IMG_CERT,
		.parent = &cot_desc[NON_TRUSTED_FW_KEY_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_SIG,
				.param.sig = {
					.pk = &nt_fw_content_pk,
					.sig = &sig,
					.alg = &sig_alg,
					.data = &raw_data,
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
		.authenticated_data = {
			[0] = {
				.type_desc = &nt_world_bl_hash,
				.data = {
					.ptr = (void *)nt_world_bl_hash_buf,
					.len = (unsigned int)HASH_DER_LEN
				}
			}
		}
	},
	[BL33_IMAGE_ID] = {
		.img_id = BL33_IMAGE_ID,
		.img_type = IMG_RAW,
		.parent = &cot_desc[NON_TRUSTED_FW_CONTENT_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_HASH,
				.param.hash = {
					.data = &raw_data,
					.hash = &nt_world_bl_hash,
				}
			}
		}
	}
	/*
	 * FWU auth descriptor, not used by Cavium platform
	 */
#if 0
	[FWU_CERT_ID] = {
		.img_id = FWU_CERT_ID,
		.img_type = IMG_CERT,
		.parent = NULL,
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_SIG,
				.param.sig = {
					.pk = &subject_pk,
					.sig = &sig,
					.alg = &sig_alg,
					.data = &raw_data,
				}
			}
		},
		.authenticated_data = {
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
	},
	/*
	 * SCP_BL2U
	 */
	[SCP_BL2U_IMAGE_ID] = {
		.img_id = SCP_BL2U_IMAGE_ID,
		.img_type = IMG_RAW,
		.parent = &cot_desc[FWU_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_HASH,
				.param.hash = {
					.data = &raw_data,
					.hash = &scp_bl2u_hash,
				}
			}
		}
	},
	/*
	 * BL2U
	 */
	[BL2U_IMAGE_ID] = {
		.img_id = BL2U_IMAGE_ID,
		.img_type = IMG_RAW,
		.parent = &cot_desc[FWU_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_HASH,
				.param.hash = {
					.data = &raw_data,
					.hash = &bl2u_hash,
				}
			}
		}
	},
	/*
	 * NS_BL2U
	 */
	[NS_BL2U_IMAGE_ID] = {
		.img_id = NS_BL2U_IMAGE_ID,
		.img_type = IMG_RAW,
		.parent = &cot_desc[FWU_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_HASH,
				.param.hash = {
					.data = &raw_data,
					.hash = &ns_bl2u_hash,
				}
			}
		}
	}
#endif
};

/* Register the CoT in the authentication module */
REGISTER_COT(cot_desc);
