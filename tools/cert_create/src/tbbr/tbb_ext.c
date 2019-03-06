/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>

#if USE_TBBR_DEFS
#include <tbbr_oid.h>
#else
#include <platform_oid.h>
#endif

#include "ext.h"
#include "tbbr/tbb_ext.h"
#include "tbbr/tbb_key.h"

static ext_t tbb_ext[] = {
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
	[TRUSTED_WORLD_PK_EXT] = {
		.oid = TRUSTED_WORLD_PK_OID,
		.sn = "TrustedWorldPublicKey",
		.ln = "Trusted World Public Key",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_PKEY,
		.attr.key = TRUSTED_WORLD_KEY
	},
	[NON_TRUSTED_WORLD_PK_EXT] = {
		.oid = NON_TRUSTED_WORLD_PK_OID,
		.sn = "NonTrustedWorldPublicKey",
		.ln = "Non-Trusted World Public Key",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_PKEY,
		.attr.key = NON_TRUSTED_WORLD_KEY
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
	[NON_TRUSTED_FW_CONTENT_CERT_PK_EXT] = {
		.oid = NON_TRUSTED_FW_CONTENT_CERT_PK_OID,
		.sn = "NonTrustedFirmwareContentCertPK",
		.ln = "Non-Trusted Firmware content certificate public key",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_PKEY,
		.attr.key = NON_TRUSTED_FW_CONTENT_CERT_KEY
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
	}
};

REGISTER_EXTENSIONS(tbb_ext);
