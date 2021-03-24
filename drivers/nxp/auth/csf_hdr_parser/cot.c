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


static auth_param_type_desc_t sig = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_SIG, 0);
static auth_param_type_desc_t sig_alg = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_SIG_ALG, 0);
static auth_param_type_desc_t sig_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, 0);

static auth_param_type_desc_t non_trusted_world_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, NON_TRUSTED_WORLD_PK_OID);

/*
 * TBBR Chain of trust definition
 */
static const auth_img_desc_t bl31_image = {
	.img_id = BL31_IMAGE_ID,
	.img_type = IMG_PLAT,
	.parent = NULL,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &non_trusted_world_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &sig_hash
			}
		}
	}
};
static const auth_img_desc_t scp_bl2_image = {
	.img_id = SCP_BL2_IMAGE_ID,
	.img_type = IMG_PLAT,
	.parent = NULL,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &non_trusted_world_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &sig_hash
			}
		}
	}
};
static const auth_img_desc_t bl32_image = {
	.img_id = BL32_IMAGE_ID,
	.img_type = IMG_PLAT,
	.parent = NULL,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &non_trusted_world_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &sig_hash
			}
		}
	}
};
static const auth_img_desc_t bl33_image = {
	.img_id = BL33_IMAGE_ID,
	.img_type = IMG_PLAT,
	.parent = NULL,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &non_trusted_world_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &sig_hash
			}
		}
	}
};
#ifdef POLICY_FUSE_PROVISION
static const auth_img_desc_t fuse_prov_img = {
	.img_id = FUSE_PROV_IMAGE_ID,
	.img_type = IMG_PLAT,
	.parent = NULL,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &non_trusted_world_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &sig_hash
			}
		}
	}
};
static const auth_img_desc_t fuse_upgrade_img = {
	.img_id = FUSE_UP_IMAGE_ID,
	.img_type = IMG_PLAT,
	.parent = NULL,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &non_trusted_world_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &sig_hash
			}
		}
	}
};
#endif
#ifdef CONFIG_DDR_FIP_IMAGE
static const auth_img_desc_t ddr_imem_udimm_1d_img = {
	.img_id = DDR_IMEM_UDIMM_1D_IMAGE_ID,
	.img_type = IMG_PLAT,
	.parent = NULL,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &non_trusted_world_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &sig_hash
			}
		}
	}
};
static const auth_img_desc_t ddr_imem_udimm_2d_img = {
	.img_id = DDR_IMEM_UDIMM_2D_IMAGE_ID,
	.img_type = IMG_PLAT,
	.parent = NULL,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &non_trusted_world_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &sig_hash
			}
		}
	}
};
static const auth_img_desc_t ddr_dmem_udimm_1d_img = {
	.img_id = DDR_DMEM_UDIMM_1D_IMAGE_ID,
	.img_type = IMG_PLAT,
	.parent = NULL,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &non_trusted_world_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &sig_hash
			}
		}
	}
};
static const auth_img_desc_t ddr_dmem_udimm_2d_img = {
	.img_id = DDR_DMEM_UDIMM_2D_IMAGE_ID,
	.img_type = IMG_PLAT,
	.parent = NULL,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &non_trusted_world_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &sig_hash
			}
		}
	}
};
static const auth_img_desc_t ddr_imem_rdimm_1d_img = {
	.img_id = DDR_IMEM_RDIMM_1D_IMAGE_ID,
	.img_type = IMG_PLAT,
	.parent = NULL,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &non_trusted_world_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &sig_hash
			}
		}
	}
};
static const auth_img_desc_t ddr_imem_rdimm_2d_img = {
	.img_id = DDR_IMEM_RDIMM_2D_IMAGE_ID,
	.img_type = IMG_PLAT,
	.parent = NULL,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &non_trusted_world_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &sig_hash
			}
		}
	}
};
static const auth_img_desc_t ddr_dmem_rdimm_1d_img = {
	.img_id = DDR_DMEM_RDIMM_1D_IMAGE_ID,
	.img_type = IMG_PLAT,
	.parent = NULL,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &non_trusted_world_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &sig_hash
			}
		}
	}
};
static const auth_img_desc_t ddr_dmem_rdimm_2d_img = {
	.img_id = DDR_DMEM_RDIMM_2D_IMAGE_ID,
	.img_type = IMG_PLAT,
	.parent = NULL,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &non_trusted_world_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &sig_hash
			}
		}
	}
};
#endif

static const auth_img_desc_t * const cot_desc[] = {
	[BL31_IMAGE_ID]			=	&bl31_image,
	[SCP_BL2_IMAGE_ID]		=	&scp_bl2_image,
	[BL32_IMAGE_ID]			=	&bl32_image,
	[BL33_IMAGE_ID]			=	&bl33_image,
#ifdef POLICY_FUSE_PROVISION
	[FUSE_PROV_IMAGE_ID]		=	&fuse_prov_img,
	[FUSE_UP_IMAGE_ID]		=	&fuse_upgrade_img,
#endif
#ifdef CONFIG_DDR_FIP_IMAGE
	[DDR_IMEM_UDIMM_1D_IMAGE_ID]	=	&ddr_imem_udimm_1d_img,
	[DDR_IMEM_UDIMM_2D_IMAGE_ID]	=	&ddr_imem_udimm_2d_img,
	[DDR_DMEM_UDIMM_1D_IMAGE_ID]	=	&ddr_dmem_udimm_1d_img,
	[DDR_DMEM_UDIMM_2D_IMAGE_ID]	=	&ddr_dmem_udimm_2d_img,
	[DDR_IMEM_RDIMM_1D_IMAGE_ID]	=	&ddr_imem_rdimm_1d_img,
	[DDR_IMEM_RDIMM_2D_IMAGE_ID]	=	&ddr_imem_rdimm_2d_img,
	[DDR_DMEM_RDIMM_1D_IMAGE_ID]	=	&ddr_dmem_rdimm_1d_img,
	[DDR_DMEM_RDIMM_2D_IMAGE_ID]	=	&ddr_dmem_rdimm_2d_img,
#endif
};

/* Register the CoT in the authentication module */
REGISTER_COT(cot_desc);
