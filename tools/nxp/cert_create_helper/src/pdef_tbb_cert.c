/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <pdef_tbb_cert.h>
#include <pdef_tbb_ext.h>
#include <pdef_tbb_key.h>

static cert_t pdef_tbb_certs[] = {
	[DDR_FW_KEY_CERT - DDR_FW_KEY_CERT] = {
		.id = DDR_FW_KEY_CERT,
		.opt = "ddr-fw-key-cert",
		.help_msg = "DDR Firmware Key Certificate (output file)",
		.fn = NULL,
		.cn = "DDR Firmware Key Certificate",
		.key = TRUSTED_WORLD_KEY,
		.issuer = DDR_FW_KEY_CERT,
		.ext = {
			TRUSTED_FW_NVCOUNTER_EXT,
			DDR_FW_CONTENT_CERT_PK_EXT,
		},
		.num_ext = 2
	},
	[DDR_UDIMM_FW_CONTENT_CERT - DDR_FW_KEY_CERT] = {
		.id = DDR_UDIMM_FW_CONTENT_CERT,
		.opt = "ddr-udimm-fw-cert",
		.help_msg = "DDR UDIMM Firmware Content Certificate (output file)",
		.fn = NULL,
		.cn = "DDR UDIMM Firmware Content Certificate",
		.key = DDR_FW_CONTENT_KEY,
		.issuer = DDR_UDIMM_FW_CONTENT_CERT,
		.ext = {
			TRUSTED_FW_NVCOUNTER_EXT,
			DDR_IMEM_UDIMM_1D_HASH_EXT,
			DDR_IMEM_UDIMM_2D_HASH_EXT,
			DDR_DMEM_UDIMM_1D_HASH_EXT,
			DDR_DMEM_UDIMM_2D_HASH_EXT,
		},
		.num_ext = 5
	},
	[DDR_RDIMM_FW_CONTENT_CERT - DDR_FW_KEY_CERT] = {
		.id = DDR_RDIMM_FW_CONTENT_CERT,
		.opt = "ddr-rdimm-fw-cert",
		.help_msg = "DDR RDIMM Firmware Content Certificate (output file)",
		.fn = NULL,
		.cn = "DDR RDIMM Firmware Content Certificate",
		.key = DDR_FW_CONTENT_KEY,
		.issuer = DDR_RDIMM_FW_CONTENT_CERT,
		.ext = {
			TRUSTED_FW_NVCOUNTER_EXT,
			DDR_IMEM_RDIMM_1D_HASH_EXT,
			DDR_IMEM_RDIMM_2D_HASH_EXT,
			DDR_DMEM_RDIMM_1D_HASH_EXT,
			DDR_DMEM_RDIMM_2D_HASH_EXT,
		},
		.num_ext = 5
	}
};

PLAT_REGISTER_COT(pdef_tbb_certs);
