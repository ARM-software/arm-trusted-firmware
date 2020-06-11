/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tbbr/tbb_cert.h"
#include "tbbr/tbb_ext.h"
#include "tbbr/tbb_key.h"

/*
 * Certificates used in the chain of trust
 *
 * The order of the certificates must follow the enumeration specified in
 * tbb_cert.h. All certificates are self-signed, so the issuer certificate
 * field points to itself.
 */
static cert_t tbb_certs[] = {
	[TRUSTED_BOOT_FW_CERT] = {
		.id = TRUSTED_BOOT_FW_CERT,
		.opt = "tb-fw-cert",
		.help_msg = "Trusted Boot FW Certificate (output file)",
		.fn = NULL,
		.cn = "Trusted Boot FW Certificate",
		.key = ROT_KEY,
		.issuer = TRUSTED_BOOT_FW_CERT,
		.ext = {
			TRUSTED_FW_NVCOUNTER_EXT,
			TRUSTED_BOOT_FW_HASH_EXT,
			TRUSTED_BOOT_FW_CONFIG_HASH_EXT,
			HW_CONFIG_HASH_EXT,
			FW_CONFIG_HASH_EXT
		},
		.num_ext = 5
	},
	[TRUSTED_KEY_CERT] = {
		.id = TRUSTED_KEY_CERT,
		.opt = "trusted-key-cert",
		.help_msg = "Trusted Key Certificate (output file)",
		.fn = NULL,
		.cn = "Trusted Key Certificate",
		.key = ROT_KEY,
		.issuer = TRUSTED_KEY_CERT,
		.ext = {
			TRUSTED_FW_NVCOUNTER_EXT,
			TRUSTED_WORLD_PK_EXT,
			NON_TRUSTED_WORLD_PK_EXT
		},
		.num_ext = 3
	},
	[SCP_FW_KEY_CERT] = {
		.id = SCP_FW_KEY_CERT,
		.opt = "scp-fw-key-cert",
		.help_msg = "SCP Firmware Key Certificate (output file)",
		.fn = NULL,
		.cn = "SCP Firmware Key Certificate",
		.key = TRUSTED_WORLD_KEY,
		.issuer = SCP_FW_KEY_CERT,
		.ext = {
			TRUSTED_FW_NVCOUNTER_EXT,
			SCP_FW_CONTENT_CERT_PK_EXT
		},
		.num_ext = 2
	},
	[SCP_FW_CONTENT_CERT] = {
		.id = SCP_FW_CONTENT_CERT,
		.opt = "scp-fw-cert",
		.help_msg = "SCP Firmware Content Certificate (output file)",
		.fn = NULL,
		.cn = "SCP Firmware Content Certificate",
		.key = SCP_FW_CONTENT_CERT_KEY,
		.issuer = SCP_FW_CONTENT_CERT,
		.ext = {
			TRUSTED_FW_NVCOUNTER_EXT,
			SCP_FW_HASH_EXT
		},
		.num_ext = 2
	},
	[SOC_FW_KEY_CERT] = {
		.id = SOC_FW_KEY_CERT,
		.opt = "soc-fw-key-cert",
		.help_msg = "SoC Firmware Key Certificate (output file)",
		.fn = NULL,
		.cn = "SoC Firmware Key Certificate",
		.key = TRUSTED_WORLD_KEY,
		.issuer = SOC_FW_KEY_CERT,
		.ext = {
			TRUSTED_FW_NVCOUNTER_EXT,
			SOC_FW_CONTENT_CERT_PK_EXT
		},
		.num_ext = 2
	},
	[SOC_FW_CONTENT_CERT] = {
		.id = SOC_FW_CONTENT_CERT,
		.opt = "soc-fw-cert",
		.help_msg = "SoC Firmware Content Certificate (output file)",
		.fn = NULL,
		.cn = "SoC Firmware Content Certificate",
		.key = SOC_FW_CONTENT_CERT_KEY,
		.issuer = SOC_FW_CONTENT_CERT,
		.ext = {
			TRUSTED_FW_NVCOUNTER_EXT,
			SOC_AP_FW_HASH_EXT,
			SOC_FW_CONFIG_HASH_EXT,
		},
		.num_ext = 3
	},
	[TRUSTED_OS_FW_KEY_CERT] = {
		.id = TRUSTED_OS_FW_KEY_CERT,
		.opt = "tos-fw-key-cert",
		.help_msg = "Trusted OS Firmware Key Certificate (output file)",
		.fn = NULL,
		.cn = "Trusted OS Firmware Key Certificate",
		.key = TRUSTED_WORLD_KEY,
		.issuer = TRUSTED_OS_FW_KEY_CERT,
		.ext = {
			TRUSTED_FW_NVCOUNTER_EXT,
			TRUSTED_OS_FW_CONTENT_CERT_PK_EXT
		},
		.num_ext = 2
	},
	[TRUSTED_OS_FW_CONTENT_CERT] = {
		.id = TRUSTED_OS_FW_CONTENT_CERT,
		.opt = "tos-fw-cert",
		.help_msg = "Trusted OS Firmware Content Certificate (output file)",
		.fn = NULL,
		.cn = "Trusted OS Firmware Content Certificate",
		.key = TRUSTED_OS_FW_CONTENT_CERT_KEY,
		.issuer = TRUSTED_OS_FW_CONTENT_CERT,
		.ext = {
			TRUSTED_FW_NVCOUNTER_EXT,
			TRUSTED_OS_FW_HASH_EXT,
			TRUSTED_OS_FW_EXTRA1_HASH_EXT,
			TRUSTED_OS_FW_EXTRA2_HASH_EXT,
			TRUSTED_OS_FW_CONFIG_HASH_EXT,
		},
		.num_ext = 5
	},
	[NON_TRUSTED_FW_KEY_CERT] = {
		.id = NON_TRUSTED_FW_KEY_CERT,
		.opt = "nt-fw-key-cert",
		.help_msg = "Non-Trusted Firmware Key Certificate (output file)",
		.fn = NULL,
		.cn = "Non-Trusted Firmware Key Certificate",
		.key = NON_TRUSTED_WORLD_KEY,
		.issuer = NON_TRUSTED_FW_KEY_CERT,
		.ext = {
			NON_TRUSTED_FW_NVCOUNTER_EXT,
			NON_TRUSTED_FW_CONTENT_CERT_PK_EXT
		},
		.num_ext = 2
	},
	[NON_TRUSTED_FW_CONTENT_CERT] = {
		.id = NON_TRUSTED_FW_CONTENT_CERT,
		.opt = "nt-fw-cert",
		.help_msg = "Non-Trusted Firmware Content Certificate (output file)",
		.fn = NULL,
		.cn = "Non-Trusted Firmware Content Certificate",
		.key = NON_TRUSTED_FW_CONTENT_CERT_KEY,
		.issuer = NON_TRUSTED_FW_CONTENT_CERT,
		.ext = {
			NON_TRUSTED_FW_NVCOUNTER_EXT,
			NON_TRUSTED_WORLD_BOOTLOADER_HASH_EXT,
			NON_TRUSTED_FW_CONFIG_HASH_EXT,
		},
		.num_ext = 3
	},
	[SIP_SECURE_PARTITION_CONTENT_CERT] = {
		.id = SIP_SECURE_PARTITION_CONTENT_CERT,
		.opt = "sip-sp-cert",
		.help_msg = "SiP owned Secure Partition Content Certificate (output file)",
		.fn = NULL,
		.cn = "SiP owned Secure Partition Content Certificate",
		.key = TRUSTED_WORLD_KEY,
		.issuer = SIP_SECURE_PARTITION_CONTENT_CERT,
		.ext = {
			TRUSTED_FW_NVCOUNTER_EXT,
			SP_PKG1_HASH_EXT,
			SP_PKG2_HASH_EXT,
			SP_PKG3_HASH_EXT,
			SP_PKG4_HASH_EXT,
			SP_PKG5_HASH_EXT,
			SP_PKG6_HASH_EXT,
			SP_PKG7_HASH_EXT,
			SP_PKG8_HASH_EXT,
		},
		.num_ext = 9
	},
	[FWU_CERT] = {
		.id = FWU_CERT,
		.opt = "fwu-cert",
		.help_msg = "Firmware Update Certificate (output file)",
		.fn = NULL,
		.cn = "Firmware Update Certificate",
		.key = ROT_KEY,
		.issuer = FWU_CERT,
		.ext = {
			SCP_FWU_CFG_HASH_EXT,
			AP_FWU_CFG_HASH_EXT,
			FWU_HASH_EXT
		},
		.num_ext = 3
	}
};

REGISTER_COT(tbb_certs);
