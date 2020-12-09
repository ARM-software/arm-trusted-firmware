/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TBBR_OID_H
#define TBBR_OID_H

#define	MAX_OID_NAME_LEN	30

/*
 * The following is a list of OID values defined and reserved by ARM, which
 * are used to define the extension fields of the certificate structure, as
 * defined in the Trusted Board Boot Requirements (TBBR) specification,
 * ARM DEN0006C-1.
 */


/* TrustedFirmwareNVCounter - Non-volatile counter extension */
#define TRUSTED_FW_NVCOUNTER_OID		"1.3.6.1.4.1.4128.2100.1"
/* NonTrustedFirmwareNVCounter - Non-volatile counter extension */
#define NON_TRUSTED_FW_NVCOUNTER_OID		"1.3.6.1.4.1.4128.2100.2"


/*
 * Non-Trusted Firmware Updater Certificate
 */

/* APFirmwareUpdaterConfigHash - BL2U */
#define AP_FWU_CFG_HASH_OID			"1.3.6.1.4.1.4128.2100.101"
/* SCPFirmwareUpdaterConfigHash - SCP_BL2U */
#define SCP_FWU_CFG_HASH_OID			"1.3.6.1.4.1.4128.2100.102"
/* FirmwareUpdaterHash - NS_BL2U */
#define FWU_HASH_OID				"1.3.6.1.4.1.4128.2100.103"
/* TrustedWatchdogRefreshTime */
#define TRUSTED_WATCHDOG_TIME_OID		"1.3.6.1.4.1.4128.2100.104"


/*
 * Trusted Boot Firmware Certificate
 */

/* TrustedBootFirmwareHash - BL2 */
#define TRUSTED_BOOT_FW_HASH_OID		"1.3.6.1.4.1.4128.2100.201"
#define TRUSTED_BOOT_FW_CONFIG_HASH_OID		"1.3.6.1.4.1.4128.2100.202"
#define HW_CONFIG_HASH_OID			"1.3.6.1.4.1.4128.2100.203"
#define FW_CONFIG_HASH_OID			"1.3.6.1.4.1.4128.2100.204"

/*
 * Trusted Key Certificate
 */

/* PrimaryDebugCertificatePK */
#define PRIMARY_DEBUG_PK_OID			"1.3.6.1.4.1.4128.2100.301"
/* TrustedWorldPK */
#define TRUSTED_WORLD_PK_OID			"1.3.6.1.4.1.4128.2100.302"
/* NonTrustedWorldPK */
#define NON_TRUSTED_WORLD_PK_OID		"1.3.6.1.4.1.4128.2100.303"


/*
 * Trusted Debug Certificate
 */

/* DebugScenario */
#define TRUSTED_DEBUG_SCENARIO_OID		"1.3.6.1.4.1.4128.2100.401"
/* SoC Specific */
#define TRUSTED_DEBUG_SOC_SPEC_OID		"1.3.6.1.4.1.4128.2100.402"
/* SecondaryDebugCertPK */
#define SECONDARY_DEBUG_PK_OID			"1.3.6.1.4.1.4128.2100.403"


/*
 * SoC Firmware Key Certificate
 */

/* SoCFirmwareContentCertPK */
#define SOC_FW_CONTENT_CERT_PK_OID		"1.3.6.1.4.1.4128.2100.501"

/*
 * SoC Firmware Content Certificate
 */

/* APRomPatchHash - BL1_PATCH */
#define APROM_PATCH_HASH_OID			"1.3.6.1.4.1.4128.2100.601"
/* SoCConfigHash */
#define SOC_CONFIG_HASH_OID			"1.3.6.1.4.1.4128.2100.602"
/* SoCAPFirmwareHash - BL31 */
#define SOC_AP_FW_HASH_OID			"1.3.6.1.4.1.4128.2100.603"
/* SoCFirmwareConfigHash = SOC_FW_CONFIG */
#define SOC_FW_CONFIG_HASH_OID			"1.3.6.1.4.1.4128.2100.604"

/*
 * SCP Firmware Key Certificate
 */

/* SCPFirmwareContentCertPK */
#define SCP_FW_CONTENT_CERT_PK_OID		"1.3.6.1.4.1.4128.2100.701"


/*
 * SCP Firmware Content Certificate
 */

/* SCPFirmwareHash - SCP_BL2 */
#define SCP_FW_HASH_OID				"1.3.6.1.4.1.4128.2100.801"
/* SCPRomPatchHash - SCP_BL1_PATCH */
#define SCP_ROM_PATCH_HASH_OID			"1.3.6.1.4.1.4128.2100.802"


/*
 * Trusted OS Firmware Key Certificate
 */

/* TrustedOSFirmwareContentCertPK */
#define TRUSTED_OS_FW_CONTENT_CERT_PK_OID	"1.3.6.1.4.1.4128.2100.901"


/*
 * Trusted OS Firmware Content Certificate
 */

/* TrustedOSFirmwareHash - BL32 */
#define TRUSTED_OS_FW_HASH_OID			"1.3.6.1.4.1.4128.2100.1001"
/* TrustedOSExtra1FirmwareHash - BL32 Extra1 */
#define TRUSTED_OS_FW_EXTRA1_HASH_OID		"1.3.6.1.4.1.4128.2100.1002"
/* TrustedOSExtra2FirmwareHash - BL32 Extra2 */
#define TRUSTED_OS_FW_EXTRA2_HASH_OID		"1.3.6.1.4.1.4128.2100.1003"
/* TrustedOSFirmwareConfigHash - TOS_FW_CONFIG */
#define TRUSTED_OS_FW_CONFIG_HASH_OID		"1.3.6.1.4.1.4128.2100.1004"


/*
 * Non-Trusted Firmware Key Certificate
 */

/* NonTrustedFirmwareContentCertPK */
#define NON_TRUSTED_FW_CONTENT_CERT_PK_OID	"1.3.6.1.4.1.4128.2100.1101"


/*
 * Non-Trusted Firmware Content Certificate
 */

/* NonTrustedWorldBootloaderHash - BL33 */
#define NON_TRUSTED_WORLD_BOOTLOADER_HASH_OID	"1.3.6.1.4.1.4128.2100.1201"
/* NonTrustedFirmwareConfigHash - NT_FW_CONFIG */
#define NON_TRUSTED_FW_CONFIG_HASH_OID		"1.3.6.1.4.1.4128.2100.1202"

/*
 * Secure Partitions Content Certificate
 */
#define SP_PKG1_HASH_OID			"1.3.6.1.4.1.4128.2100.1301"
#define SP_PKG2_HASH_OID			"1.3.6.1.4.1.4128.2100.1302"
#define SP_PKG3_HASH_OID			"1.3.6.1.4.1.4128.2100.1303"
#define SP_PKG4_HASH_OID			"1.3.6.1.4.1.4128.2100.1304"
#define SP_PKG5_HASH_OID			"1.3.6.1.4.1.4128.2100.1305"
#define SP_PKG6_HASH_OID			"1.3.6.1.4.1.4128.2100.1306"
#define SP_PKG7_HASH_OID			"1.3.6.1.4.1.4128.2100.1307"
#define SP_PKG8_HASH_OID			"1.3.6.1.4.1.4128.2100.1308"

#ifdef PLAT_DEF_OID
#include <platform_oid.h>
#endif
#endif /* TBBR_OID_H */
