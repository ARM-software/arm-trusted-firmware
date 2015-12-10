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

#ifndef __BOARD_ARM_OID_H__
#define __BOARD_ARM_OID_H__

/*
 * The following is a list of OID values defined and reserved by ARM, which
 * are used to define the extension fields of the certificate structure, as
 * defined in the Trusted Board Boot Requirements (TBBR) specification,
 * ARM DEN0006C-1.
 *
 * Non-ARM platform owners that wish to align with the TBBR should define
 * constants with the same name in their own platform port(s), using their
 * own OIDs obtained from the ITU-T.
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

#endif /* __BOARD_ARM_OID_H__ */
