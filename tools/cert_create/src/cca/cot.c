/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "cca/cca_cot.h"

#include <cca_oid.h>

#include "cert.h"
#include "ext.h"
#include "key.h"

/*
 * Certificates used in the chain of trust.
 *
 * All certificates are self-signed so the issuer certificate field points to
 * itself.
 */
static cert_t cot_certs[] = {
	[CCA_CONTENT_CERT] = {
		.id = CCA_CONTENT_CERT,
		.opt = "cca-cert",
		.help_msg = "CCA Content Certificate (output file)",
		.cn = "CCA Content Certificate",
		.key = ROT_KEY,
		.issuer = CCA_CONTENT_CERT,
		.ext = {
			TRUSTED_FW_NVCOUNTER_EXT,
			SOC_AP_FW_HASH_EXT,
			SOC_FW_CONFIG_HASH_EXT,
			RMM_HASH_EXT,
			TRUSTED_BOOT_FW_HASH_EXT,
			TRUSTED_BOOT_FW_CONFIG_HASH_EXT,
			HW_CONFIG_HASH_EXT,
			FW_CONFIG_HASH_EXT,
		},
		.num_ext = 8
	},

	[CORE_SWD_KEY_CERT] = {
		.id = CORE_SWD_KEY_CERT,
		.opt = "core-swd-cert",
		.help_msg = "Core Secure World Key Certificate (output file)",
		.cn = "Core Secure World Key Certificate",
		.key = SWD_ROT_KEY,
		.issuer = CORE_SWD_KEY_CERT,
		.ext = {
			TRUSTED_FW_NVCOUNTER_EXT,
			SWD_ROT_PK_EXT,
			CORE_SWD_PK_EXT,
		},
		.num_ext = 3
	},

	[SPMC_CONTENT_CERT] = {
		.id = SPMC_CONTENT_CERT,
		.opt = "tos-fw-cert",
		.help_msg = "SPMC Content Certificate (output file)",
		.cn = "SPMC Content Certificate",
		.key = CORE_SWD_KEY,
		.issuer = SPMC_CONTENT_CERT,
		.ext = {
			TRUSTED_FW_NVCOUNTER_EXT,
			TRUSTED_OS_FW_HASH_EXT,
			TRUSTED_OS_FW_CONFIG_HASH_EXT,
		},
		.num_ext = 3
	},

	[SIP_SECURE_PARTITION_CONTENT_CERT] = {
		.id = SIP_SECURE_PARTITION_CONTENT_CERT,
		.opt = "sip-sp-cert",
		.help_msg = "SiP owned Secure Partition Content Certificate (output file)",
		.cn = "SiP owned Secure Partition Content Certificate",
		.key = CORE_SWD_KEY,
		.issuer = SIP_SECURE_PARTITION_CONTENT_CERT,
		.ext = {
			TRUSTED_FW_NVCOUNTER_EXT,
			SP_PKG1_HASH_EXT,
			SP_PKG2_HASH_EXT,
			SP_PKG3_HASH_EXT,
			SP_PKG4_HASH_EXT,
		},
		.num_ext = 5
	},

	[PLAT_KEY_CERT] = {
		.id = PLAT_KEY_CERT,
		.opt = "plat-key-cert",
		.help_msg = "Platform Key Certificate (output file)",
		.cn = "Platform Key Certificate",
		.key = PROT_KEY,
		.issuer = PLAT_KEY_CERT,
		.ext = {
			NON_TRUSTED_FW_NVCOUNTER_EXT,
			PROT_PK_EXT,
			PLAT_PK_EXT,
		},
		.num_ext = 3
	},

	[PLAT_SECURE_PARTITION_CONTENT_CERT] = {
		.id = PLAT_SECURE_PARTITION_CONTENT_CERT,
		.opt = "plat-sp-cert",
		.help_msg = "Platform owned Secure Partition Content Certificate (output file)",
		.cn = "Platform owned Secure Partition Content Certificate",
		.key = PLAT_KEY,
		.issuer = PLAT_SECURE_PARTITION_CONTENT_CERT,
		.ext = {
			NON_TRUSTED_FW_NVCOUNTER_EXT,
			SP_PKG5_HASH_EXT,
			SP_PKG6_HASH_EXT,
			SP_PKG7_HASH_EXT,
			SP_PKG8_HASH_EXT,
		},
		.num_ext = 5
	},

	[NON_TRUSTED_FW_CONTENT_CERT] = {
		.id = NON_TRUSTED_FW_CONTENT_CERT,
		.opt = "nt-fw-cert",
		.help_msg = "Non-Trusted Firmware Content Certificate (output file)",
		.cn = "Non-Trusted Firmware Content Certificate",
		.key = PLAT_KEY,
		.issuer = NON_TRUSTED_FW_CONTENT_CERT,
		.ext = {
			NON_TRUSTED_FW_NVCOUNTER_EXT,
			NON_TRUSTED_WORLD_BOOTLOADER_HASH_EXT,
			NON_TRUSTED_FW_CONFIG_HASH_EXT,
		},
		.num_ext = 3
	},
};

REGISTER_COT(cot_certs);


/* Certificate extensions. */
static ext_t cot_ext[] = {
	[TRUSTED_FW_NVCOUNTER_EXT] = {
		.oid = TRUSTED_FW_NVCOUNTER_OID,
		.opt = "tfw-nvctr",
		.help_msg = "Trusted Firmware Non-Volatile counter value",
		.sn = "TrustedWorldNVCounter",
		.ln = "Trusted World Non-Volatile counter",
		.asn1_type = V_ASN1_INTEGER,
		.type = EXT_TYPE_NVCOUNTER,
		.attr.nvctr_type = NVCTR_TYPE_TFW
	},

	[TRUSTED_BOOT_FW_HASH_EXT] = {
		.oid = TRUSTED_BOOT_FW_HASH_OID,
		.opt = "tb-fw",
		.help_msg = "Trusted Boot Firmware image file",
		.sn = "TrustedBootFirmwareHash",
		.ln = "Trusted Boot Firmware hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH
	},

	[TRUSTED_BOOT_FW_CONFIG_HASH_EXT] = {
		.oid = TRUSTED_BOOT_FW_CONFIG_HASH_OID,
		.opt = "tb-fw-config",
		.help_msg = "Trusted Boot Firmware Config file",
		.sn = "TrustedBootFirmwareConfigHash",
		.ln = "Trusted Boot Firmware Config hash",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH,
		.optional = 1
	},

	[HW_CONFIG_HASH_EXT] = {
		.oid = HW_CONFIG_HASH_OID,
		.opt = "hw-config",
		.help_msg = "HW Config file",
		.sn = "HWConfigHash",
		.ln = "HW Config hash",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH,
		.optional = 1
	},

	[FW_CONFIG_HASH_EXT] = {
		.oid = FW_CONFIG_HASH_OID,
		.opt = "fw-config",
		.help_msg = "Firmware Config file",
		.sn = "FirmwareConfigHash",
		.ln = "Firmware Config hash",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH,
		.optional = 1
	},

	[SWD_ROT_PK_EXT] = {
		.oid = SWD_ROT_PK_OID,
		.sn = "SWDRoTKey",
		.ln = "Secure World Root of Trust Public Key",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_PKEY,
		.attr.key = SWD_ROT_KEY
	},

	[CORE_SWD_PK_EXT] = {
		.oid = CORE_SWD_PK_OID,
		.sn = "CORESWDKey",
		.ln = "Core Secure World Public Key",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_PKEY,
		.attr.key = CORE_SWD_KEY
	},

	[SOC_AP_FW_HASH_EXT] = {
		.oid = SOC_AP_FW_HASH_OID,
		.opt = "soc-fw",
		.help_msg = "SoC AP Firmware image file",
		.sn = "SoCAPFirmwareHash",
		.ln = "SoC AP Firmware hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH
	},

	[SOC_FW_CONFIG_HASH_EXT] = {
		.oid = SOC_FW_CONFIG_HASH_OID,
		.opt = "soc-fw-config",
		.help_msg = "SoC Firmware Config file",
		.sn = "SocFirmwareConfigHash",
		.ln = "SoC Firmware Config hash",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH,
		.optional = 1
	},

	[RMM_HASH_EXT] = {
		.oid = RMM_HASH_OID,
		.opt = "rmm-fw",
		.help_msg = "RMM Firmware image file",
		.sn = "RMMFirmwareHash",
		.ln = "RMM Firmware hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH
	},

	[TRUSTED_OS_FW_HASH_EXT] = {
		.oid = TRUSTED_OS_FW_HASH_OID,
		.opt = "tos-fw",
		.help_msg = "Trusted OS image file",
		.sn = "TrustedOSHash",
		.ln = "Trusted OS hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH
	},

	[TRUSTED_OS_FW_CONFIG_HASH_EXT] = {
		.oid = TRUSTED_OS_FW_CONFIG_HASH_OID,
		.opt = "tos-fw-config",
		.help_msg = "Trusted OS Firmware Config file",
		.sn = "TrustedOSFirmwareConfigHash",
		.ln = "Trusted OS Firmware Config hash",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH,
		.optional = 1
	},

	[SP_PKG1_HASH_EXT] = {
		.oid = SP_PKG1_HASH_OID,
		.opt = "sp-pkg1",
		.help_msg = "Secure Partition Package1 file",
		.sn = "SPPkg1Hash",
		.ln = "SP Pkg1 hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH,
		.optional = 1
	},
	[SP_PKG2_HASH_EXT] = {
		.oid = SP_PKG2_HASH_OID,
		.opt = "sp-pkg2",
		.help_msg = "Secure Partition Package2 file",
		.sn = "SPPkg2Hash",
		.ln = "SP Pkg2 hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH,
		.optional = 1
	},
	[SP_PKG3_HASH_EXT] = {
		.oid = SP_PKG3_HASH_OID,
		.opt = "sp-pkg3",
		.help_msg = "Secure Partition Package3 file",
		.sn = "SPPkg3Hash",
		.ln = "SP Pkg3 hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH,
		.optional = 1
	},
	[SP_PKG4_HASH_EXT] = {
		.oid = SP_PKG4_HASH_OID,
		.opt = "sp-pkg4",
		.help_msg = "Secure Partition Package4 file",
		.sn = "SPPkg4Hash",
		.ln = "SP Pkg4 hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH,
		.optional = 1
	},

	[PROT_PK_EXT] = {
		.oid = PROT_PK_OID,
		.sn = "PlatformRoTKey",
		.ln = "Platform Root of Trust Public Key",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_PKEY,
		.attr.key = PROT_KEY
	},

	[PLAT_PK_EXT] = {
		.oid = PLAT_PK_OID,
		.sn = "PLATKey",
		.ln = "Platform Public Key",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_PKEY,
		.attr.key = PLAT_KEY
	},

	[SP_PKG5_HASH_EXT] = {
		.oid = SP_PKG5_HASH_OID,
		.opt = "sp-pkg5",
		.help_msg = "Secure Partition Package5 file",
		.sn = "SPPkg5Hash",
		.ln = "SP Pkg5 hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH,
		.optional = 1
	},
	[SP_PKG6_HASH_EXT] = {
		.oid = SP_PKG6_HASH_OID,
		.opt = "sp-pkg6",
		.help_msg = "Secure Partition Package6 file",
		.sn = "SPPkg6Hash",
		.ln = "SP Pkg6 hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH,
		.optional = 1
	},
	[SP_PKG7_HASH_EXT] = {
		.oid = SP_PKG7_HASH_OID,
		.opt = "sp-pkg7",
		.help_msg = "Secure Partition Package7 file",
		.sn = "SPPkg7Hash",
		.ln = "SP Pkg7 hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH,
		.optional = 1
	},
	[SP_PKG8_HASH_EXT] = {
		.oid = SP_PKG8_HASH_OID,
		.opt = "sp-pkg8",
		.help_msg = "Secure Partition Package8 file",
		.sn = "SPPkg8Hash",
		.ln = "SP Pkg8 hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH,
		.optional = 1
	},

	[NON_TRUSTED_FW_NVCOUNTER_EXT] = {
		.oid = NON_TRUSTED_FW_NVCOUNTER_OID,
		.opt = "ntfw-nvctr",
		.help_msg = "Non-Trusted Firmware Non-Volatile counter value",
		.sn = "NormalWorldNVCounter",
		.ln = "Non-Trusted Firmware Non-Volatile counter",
		.asn1_type = V_ASN1_INTEGER,
		.type = EXT_TYPE_NVCOUNTER,
		.attr.nvctr_type = NVCTR_TYPE_NTFW
	},

	[NON_TRUSTED_WORLD_BOOTLOADER_HASH_EXT] = {
		.oid = NON_TRUSTED_WORLD_BOOTLOADER_HASH_OID,
		.opt = "nt-fw",
		.help_msg = "Non-Trusted World Bootloader image file",
		.sn = "NonTrustedWorldBootloaderHash",
		.ln = "Non-Trusted World hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH
	},

	[NON_TRUSTED_FW_CONFIG_HASH_EXT] = {
		.oid = NON_TRUSTED_FW_CONFIG_HASH_OID,
		.opt = "nt-fw-config",
		.help_msg = "Non Trusted OS Firmware Config file",
		.sn = "NonTrustedOSFirmwareConfigHash",
		.ln = "Non-Trusted OS Firmware Config hash",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH,
		.optional = 1
	},
};

REGISTER_EXTENSIONS(cot_ext);

/* Keys used to establish the chain of trust. */
static key_t cot_keys[] = {
	[ROT_KEY] = {
		.id = ROT_KEY,
		.opt = "rot-key",
		.help_msg = "Root Of Trust key (input/output file)",
		.desc = "Root Of Trust key"
	},

	[SWD_ROT_KEY] = {
		.id = SWD_ROT_KEY,
		.opt = "swd-rot-key",
		.help_msg = "Secure World Root of Trust key",
		.desc = "Secure World Root of Trust key"
	},

	[CORE_SWD_KEY] = {
		.id = CORE_SWD_KEY,
		.opt = "core-swd-key",
		.help_msg = "Core Secure World key",
		.desc = "Core Secure World key"
	},

	[PROT_KEY] = {
		.id = PROT_KEY,
		.opt = "prot-key",
		.help_msg = "Platform Root of Trust key",
		.desc = "Platform Root of Trust key"
	},

	[PLAT_KEY] = {
		.id = PLAT_KEY,
		.opt = "plat-key",
		.help_msg = "Platform key",
		.desc = "Platform key"
	},
};

REGISTER_KEYS(cot_keys);
