/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <dualroot_oid.h>

#include "cert.h"
#include "ext.h"
#include "key.h"

#include "dualroot/cot.h"

/*
 * Certificates used in the chain of trust.
 *
 * All certificates are self-signed so the issuer certificate field points to
 * itself.
 */
static cert_t cot_certs[] = {
	[TRUSTED_BOOT_FW_CERT] = {
		.id = TRUSTED_BOOT_FW_CERT,
		.opt = "tb-fw-cert",
		.help_msg = "Trusted Boot FW Certificate (output file)",
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
		.cn = "Trusted Key Certificate",
		.key = ROT_KEY,
		.issuer = TRUSTED_KEY_CERT,
		.ext = {
			TRUSTED_FW_NVCOUNTER_EXT,
			TRUSTED_WORLD_PK_EXT,
		},
		.num_ext = 2
	},

	[SCP_FW_KEY_CERT] = {
		.id = SCP_FW_KEY_CERT,
		.opt = "scp-fw-key-cert",
		.help_msg = "SCP Firmware Key Certificate (output file)",
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
		},
		.num_ext = 5
	},

	[PLAT_SECURE_PARTITION_CONTENT_CERT] = {
		.id = PLAT_SECURE_PARTITION_CONTENT_CERT,
		.opt = "plat-sp-cert",
		.help_msg = "Platform owned Secure Partition Content Certificate (output file)",
		.fn = NULL,
		.cn = "Platform owned Secure Partition Content Certificate",
		.key = PROT_KEY,
		.issuer = PLAT_SECURE_PARTITION_CONTENT_CERT,
		.ext = {
			NON_TRUSTED_FW_NVCOUNTER_EXT,
			SP_PKG5_HASH_EXT,
			SP_PKG6_HASH_EXT,
			SP_PKG7_HASH_EXT,
			SP_PKG8_HASH_EXT,
			PROT_PK_EXT,
		},
		.num_ext = 6
	},

	[FWU_CERT] = {
		.id = FWU_CERT,
		.opt = "fwu-cert",
		.help_msg = "Firmware Update Certificate (output file)",
		.cn = "Firmware Update Certificate",
		.key = ROT_KEY,
		.issuer = FWU_CERT,
		.ext = {
			SCP_FWU_CFG_HASH_EXT,
			AP_FWU_CFG_HASH_EXT,
			FWU_HASH_EXT
		},
		.num_ext = 3
	},

	[NON_TRUSTED_FW_CONTENT_CERT] = {
		.id = NON_TRUSTED_FW_CONTENT_CERT,
		.opt = "nt-fw-cert",
		.help_msg = "Non-Trusted Firmware Content Certificate (output file)",
		.cn = "Non-Trusted Firmware Content Certificate",
		.key = PROT_KEY,
		.issuer = NON_TRUSTED_FW_CONTENT_CERT,
		.ext = {
			NON_TRUSTED_FW_NVCOUNTER_EXT,
			NON_TRUSTED_WORLD_BOOTLOADER_HASH_EXT,
			NON_TRUSTED_FW_CONFIG_HASH_EXT,
			PROT_PK_EXT,
		},
		.num_ext = 4
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

	[TRUSTED_WORLD_PK_EXT] = {
		.oid = TRUSTED_WORLD_PK_OID,
		.sn = "TrustedWorldPublicKey",
		.ln = "Trusted World Public Key",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_PKEY,
		.attr.key = TRUSTED_WORLD_KEY
	},

	[SCP_FW_CONTENT_CERT_PK_EXT] = {
		.oid = SCP_FW_CONTENT_CERT_PK_OID,
		.sn = "SCPFirmwareContentCertPK",
		.ln = "SCP Firmware content certificate public key",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_PKEY,
		.attr.key = SCP_FW_CONTENT_CERT_KEY
	},

	[SCP_FW_HASH_EXT] = {
		.oid = SCP_FW_HASH_OID,
		.opt = "scp-fw",
		.help_msg = "SCP Firmware image file",
		.sn = "SCPFirmwareHash",
		.ln = "SCP Firmware hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH
	},

	[SOC_FW_CONTENT_CERT_PK_EXT] = {
		.oid = SOC_FW_CONTENT_CERT_PK_OID,
		.sn = "SoCFirmwareContentCertPK",
		.ln = "SoC Firmware content certificate public key",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_PKEY,
		.attr.key = SOC_FW_CONTENT_CERT_KEY
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

	[TRUSTED_OS_FW_CONTENT_CERT_PK_EXT] = {
		.oid = TRUSTED_OS_FW_CONTENT_CERT_PK_OID,
		.sn = "TrustedOSFirmwareContentCertPK",
		.ln = "Trusted OS Firmware content certificate public key",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_PKEY,
		.attr.key = TRUSTED_OS_FW_CONTENT_CERT_KEY
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

	[TRUSTED_OS_FW_EXTRA1_HASH_EXT] = {
		.oid = TRUSTED_OS_FW_EXTRA1_HASH_OID,
		.opt = "tos-fw-extra1",
		.help_msg = "Trusted OS Extra1 image file",
		.sn = "TrustedOSExtra1Hash",
		.ln = "Trusted OS Extra1 hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH,
		.optional = 1
	},

	[TRUSTED_OS_FW_EXTRA2_HASH_EXT] = {
		.oid = TRUSTED_OS_FW_EXTRA2_HASH_OID,
		.opt = "tos-fw-extra2",
		.help_msg = "Trusted OS Extra2 image file",
		.sn = "TrustedOSExtra2Hash",
		.ln = "Trusted OS Extra2 hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH,
		.optional = 1
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

	[SCP_FWU_CFG_HASH_EXT] = {
		.oid = SCP_FWU_CFG_HASH_OID,
		.opt = "scp-fwu-cfg",
		.help_msg = "SCP Firmware Update Config image file",
		.sn = "SCPFWUpdateConfig",
		.ln = "SCP Firmware Update Config hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH,
		.optional = 1
	},

	[AP_FWU_CFG_HASH_EXT] = {
		.oid = AP_FWU_CFG_HASH_OID,
		.opt = "ap-fwu-cfg",
		.help_msg = "AP Firmware Update Config image file",
		.sn = "APFWUpdateConfig",
		.ln = "AP Firmware Update Config hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH,
		.optional = 1
	},

	[FWU_HASH_EXT] = {
		.oid = FWU_HASH_OID,
		.opt = "fwu",
		.help_msg = "Firmware Updater image file",
		.sn = "FWUpdaterHash",
		.ln = "Firmware Updater hash (SHA256)",
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

	[TRUSTED_WORLD_KEY] = {
		.id = TRUSTED_WORLD_KEY,
		.opt = "trusted-world-key",
		.help_msg = "Trusted World key (input/output file)",
		.desc = "Trusted World key"
	},

	[SCP_FW_CONTENT_CERT_KEY] = {
		.id = SCP_FW_CONTENT_CERT_KEY,
		.opt = "scp-fw-key",
		.help_msg = "SCP Firmware Content Certificate key (input/output file)",
		.desc = "SCP Firmware Content Certificate key"
	},

	[SOC_FW_CONTENT_CERT_KEY] = {
		.id = SOC_FW_CONTENT_CERT_KEY,
		.opt = "soc-fw-key",
		.help_msg = "SoC Firmware Content Certificate key (input/output file)",
		.desc = "SoC Firmware Content Certificate key"
	},

	[TRUSTED_OS_FW_CONTENT_CERT_KEY] = {
		.id = TRUSTED_OS_FW_CONTENT_CERT_KEY,
		.opt = "tos-fw-key",
		.help_msg = "Trusted OS Firmware Content Certificate key (input/output file)",
		.desc = "Trusted OS Firmware Content Certificate key"
	},

	[PROT_KEY] = {
		.id = PROT_KEY,
		.opt = "prot-key",
		.help_msg = "Platform Root of Trust key",
		.desc = "Platform Root of Trust key"
	},
};

REGISTER_KEYS(cot_keys);
