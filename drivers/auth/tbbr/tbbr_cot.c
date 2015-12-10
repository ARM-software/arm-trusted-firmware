/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <auth_mod.h>
#include <platform_def.h>
#include <platform_oid.h>
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
static unsigned char scp_fw_hash_buf[HASH_DER_LEN];
static unsigned char soc_fw_hash_buf[HASH_DER_LEN];
static unsigned char tos_fw_hash_buf[HASH_DER_LEN];
static unsigned char nt_world_bl_hash_buf[HASH_DER_LEN];
static unsigned char trusted_world_pk_buf[PK_DER_LEN];
static unsigned char non_trusted_world_pk_buf[PK_DER_LEN];
static unsigned char content_pk_buf[PK_DER_LEN];

/*
 * Parameter type descriptors
 */
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

static auth_param_type_desc_t scp_fw_content_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, SCP_FW_CONTENT_CERT_PK_OID);
static auth_param_type_desc_t soc_fw_content_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, SOC_FW_CONTENT_CERT_PK_OID);
static auth_param_type_desc_t tos_fw_content_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, TRUSTED_OS_FW_CONTENT_CERT_PK_OID);
static auth_param_type_desc_t nt_fw_content_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, NON_TRUSTED_FW_CONTENT_CERT_PK_OID);

static auth_param_type_desc_t tb_fw_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, TRUSTED_BOOT_FW_HASH_OID);
static auth_param_type_desc_t scp_fw_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, SCP_FW_HASH_OID);
static auth_param_type_desc_t soc_fw_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, SOC_AP_FW_HASH_OID);
static auth_param_type_desc_t tos_fw_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, TRUSTED_OS_FW_HASH_OID);
static auth_param_type_desc_t nt_world_bl_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, NON_TRUSTED_WORLD_BOOTLOADER_HASH_OID);
static auth_param_type_desc_t scp_bl2u_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, SCP_FWU_CFG_HASH_OID);
static auth_param_type_desc_t bl2u_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, AP_FWU_CFG_HASH_OID);
static auth_param_type_desc_t ns_bl2u_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, FWU_HASH_OID);

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
	 * SCP Firmware
	 */
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
	 * Trusted OS Firmware
	 */
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
	},
	/*
	 * FWU auth descriptor.
	 */
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
};

/* Register the CoT in the authentication module */
REGISTER_COT(cot_desc);
