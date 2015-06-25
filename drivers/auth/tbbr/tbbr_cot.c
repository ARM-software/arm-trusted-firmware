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
static unsigned char plat_bl2_hash_buf[HASH_DER_LEN];
static unsigned char plat_bl30_hash_buf[HASH_DER_LEN];
static unsigned char plat_bl31_hash_buf[HASH_DER_LEN];
static unsigned char plat_bl32_hash_buf[HASH_DER_LEN];
static unsigned char plat_bl33_hash_buf[HASH_DER_LEN];
static unsigned char plat_tz_world_pk_buf[PK_DER_LEN];
static unsigned char plat_ntz_world_pk_buf[PK_DER_LEN];
static unsigned char plat_content_pk[PK_DER_LEN];

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

static auth_param_type_desc_t tz_world_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, TZ_WORLD_PK_OID);
static auth_param_type_desc_t ntz_world_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, NTZ_WORLD_PK_OID);

static auth_param_type_desc_t bl30_content_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, BL30_CONTENT_CERT_PK_OID);
static auth_param_type_desc_t bl31_content_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, BL31_CONTENT_CERT_PK_OID);
static auth_param_type_desc_t bl32_content_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, BL32_CONTENT_CERT_PK_OID);
static auth_param_type_desc_t bl33_content_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, BL33_CONTENT_CERT_PK_OID);

static auth_param_type_desc_t bl2_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, BL2_HASH_OID);
static auth_param_type_desc_t bl30_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, BL30_HASH_OID);
static auth_param_type_desc_t bl31_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, BL31_HASH_OID);
static auth_param_type_desc_t bl32_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, BL32_HASH_OID);
static auth_param_type_desc_t bl33_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, BL33_HASH_OID);

/*
 * TBBR Chain of trust definition
 */
static const auth_img_desc_t cot_desc[] = {
	/*
	 * BL2
	 */
	[BL2_CERT_ID] = {
		.img_id = BL2_CERT_ID,
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
				.type_desc = &bl2_hash,
				.data = {
					.ptr = (void *)plat_bl2_hash_buf,
					.len = (unsigned int)HASH_DER_LEN
				}
			}
		}
	},
	[BL2_IMAGE_ID] = {
		.img_id = BL2_IMAGE_ID,
		.img_type = IMG_RAW,
		.parent = &cot_desc[BL2_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_HASH,
				.param.hash = {
					.data = &raw_data,
					.hash = &bl2_hash,
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
				.type_desc = &tz_world_pk,
				.data = {
					.ptr = (void *)plat_tz_world_pk_buf,
					.len = (unsigned int)PK_DER_LEN
				}
			},
			[1] = {
				.type_desc = &ntz_world_pk,
				.data = {
					.ptr = (void *)plat_ntz_world_pk_buf,
					.len = (unsigned int)PK_DER_LEN
				}
			}
		}
	},
	/*
	 * BL3-0
	 */
	[BL30_KEY_CERT_ID] = {
		.img_id = BL30_KEY_CERT_ID,
		.img_type = IMG_CERT,
		.parent = &cot_desc[TRUSTED_KEY_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_SIG,
				.param.sig = {
					.pk = &tz_world_pk,
					.sig = &sig,
					.alg = &sig_alg,
					.data = &raw_data,
				}
			}
		},
		.authenticated_data = {
			[0] = {
				.type_desc = &bl30_content_pk,
				.data = {
					.ptr = (void *)plat_content_pk,
					.len = (unsigned int)PK_DER_LEN
				}
			}
		}
	},
	[BL30_CERT_ID] = {
		.img_id = BL30_CERT_ID,
		.img_type = IMG_CERT,
		.parent = &cot_desc[BL30_KEY_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_SIG,
				.param.sig = {
					.pk = &bl30_content_pk,
					.sig = &sig,
					.alg = &sig_alg,
					.data = &raw_data,
				}
			}
		},
		.authenticated_data = {
			[0] = {
				.type_desc = &bl30_hash,
				.data = {
					.ptr = (void *)plat_bl30_hash_buf,
					.len = (unsigned int)HASH_DER_LEN
				}
			}
		}
	},
	[BL30_IMAGE_ID] = {
		.img_id = BL30_IMAGE_ID,
		.img_type = IMG_RAW,
		.parent = &cot_desc[BL30_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_HASH,
				.param.hash = {
					.data = &raw_data,
					.hash = &bl30_hash,
				}
			}
		}
	},
	/*
	 * BL3-1
	 */
	[BL31_KEY_CERT_ID] = {
		.img_id = BL31_KEY_CERT_ID,
		.img_type = IMG_CERT,
		.parent = &cot_desc[TRUSTED_KEY_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_SIG,
				.param.sig = {
					.pk = &tz_world_pk,
					.sig = &sig,
					.alg = &sig_alg,
					.data = &raw_data,
				}
			}
		},
		.authenticated_data = {
			[0] = {
				.type_desc = &bl31_content_pk,
				.data = {
					.ptr = (void *)plat_content_pk,
					.len = (unsigned int)PK_DER_LEN
				}
			}
		}
	},
	[BL31_CERT_ID] = {
		.img_id = BL31_CERT_ID,
		.img_type = IMG_CERT,
		.parent = &cot_desc[BL31_KEY_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_SIG,
				.param.sig = {
					.pk = &bl31_content_pk,
					.sig = &sig,
					.alg = &sig_alg,
					.data = &raw_data,
				}
			}
		},
		.authenticated_data = {
			[0] = {
				.type_desc = &bl31_hash,
				.data = {
					.ptr = (void *)plat_bl31_hash_buf,
					.len = (unsigned int)HASH_DER_LEN
				}
			}
		}
	},
	[BL31_IMAGE_ID] = {
		.img_id = BL31_IMAGE_ID,
		.img_type = IMG_RAW,
		.parent = &cot_desc[BL31_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_HASH,
				.param.hash = {
					.data = &raw_data,
					.hash = &bl31_hash,
				}
			}
		}
	},
	/*
	 * BL3-2
	 */
	[BL32_KEY_CERT_ID] = {
		.img_id = BL32_KEY_CERT_ID,
		.img_type = IMG_CERT,
		.parent = &cot_desc[TRUSTED_KEY_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_SIG,
				.param.sig = {
					.pk = &tz_world_pk,
					.sig = &sig,
					.alg = &sig_alg,
					.data = &raw_data,
				}
			}
		},
		.authenticated_data = {
			[0] = {
				.type_desc = &bl32_content_pk,
				.data = {
					.ptr = (void *)plat_content_pk,
					.len = (unsigned int)PK_DER_LEN
				}
			}
		}
	},
	[BL32_CERT_ID] = {
		.img_id = BL32_CERT_ID,
		.img_type = IMG_CERT,
		.parent = &cot_desc[BL32_KEY_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_SIG,
				.param.sig = {
					.pk = &bl32_content_pk,
					.sig = &sig,
					.alg = &sig_alg,
					.data = &raw_data,
				}
			}
		},
		.authenticated_data = {
			[0] = {
				.type_desc = &bl32_hash,
				.data = {
					.ptr = (void *)plat_bl32_hash_buf,
					.len = (unsigned int)HASH_DER_LEN
				}
			}
		}
	},
	[BL32_IMAGE_ID] = {
		.img_id = BL32_IMAGE_ID,
		.img_type = IMG_RAW,
		.parent = &cot_desc[BL32_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_HASH,
				.param.hash = {
					.data = &raw_data,
					.hash = &bl32_hash,
				}
			}
		}
	},
	/*
	 * BL3-3
	 */
	[BL33_KEY_CERT_ID] = {
		.img_id = BL33_KEY_CERT_ID,
		.img_type = IMG_CERT,
		.parent = &cot_desc[TRUSTED_KEY_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_SIG,
				.param.sig = {
					.pk = &ntz_world_pk,
					.sig = &sig,
					.alg = &sig_alg,
					.data = &raw_data,
				}
			}
		},
		.authenticated_data = {
			[0] = {
				.type_desc = &bl33_content_pk,
				.data = {
					.ptr = (void *)plat_content_pk,
					.len = (unsigned int)PK_DER_LEN
				}
			}
		}
	},
	[BL33_CERT_ID] = {
		.img_id = BL33_CERT_ID,
		.img_type = IMG_CERT,
		.parent = &cot_desc[BL33_KEY_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_SIG,
				.param.sig = {
					.pk = &bl33_content_pk,
					.sig = &sig,
					.alg = &sig_alg,
					.data = &raw_data,
				}
			}
		},
		.authenticated_data = {
			[0] = {
				.type_desc = &bl33_hash,
				.data = {
					.ptr = (void *)plat_bl33_hash_buf,
					.len = (unsigned int)HASH_DER_LEN
				}
			}
		}
	},
	[BL33_IMAGE_ID] = {
		.img_id = BL33_IMAGE_ID,
		.img_type = IMG_RAW,
		.parent = &cot_desc[BL33_CERT_ID],
		.img_auth_methods = {
			[0] = {
				.type = AUTH_METHOD_HASH,
				.param.hash = {
					.data = &raw_data,
					.hash = &bl33_hash,
				}
			}
		}
	}
};

/* Register the CoT in the authentication module */
REGISTER_COT(cot_desc);
