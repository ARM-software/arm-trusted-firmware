/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CCA_COT_H
#define CCA_COT_H

/* Certificates. */
enum {
	/* Certificates owned by the silicon provider. */
	CCA_CONTENT_CERT,
	CORE_SWD_KEY_CERT,
	SPMC_CONTENT_CERT,
	SIP_SECURE_PARTITION_CONTENT_CERT,

	/* Certificates owned by the platform owner. */
	PLAT_KEY_CERT,
	PLAT_SECURE_PARTITION_CONTENT_CERT,
	NON_TRUSTED_FW_CONTENT_CERT,
};

/* Certificate extensions. */
enum {
	/* Extensions used in certificates owned by the silicon provider. */
	TRUSTED_FW_NVCOUNTER_EXT,
	TRUSTED_BOOT_FW_HASH_EXT,
	TRUSTED_BOOT_FW_CONFIG_HASH_EXT,
	HW_CONFIG_HASH_EXT,
	FW_CONFIG_HASH_EXT,
	SWD_ROT_PK_EXT,
	CORE_SWD_PK_EXT,
	SOC_AP_FW_HASH_EXT,
	SOC_FW_CONFIG_HASH_EXT,
	RMM_HASH_EXT,
	TRUSTED_OS_FW_HASH_EXT,
	TRUSTED_OS_FW_CONFIG_HASH_EXT,
	SP_PKG1_HASH_EXT,
	SP_PKG2_HASH_EXT,
	SP_PKG3_HASH_EXT,
	SP_PKG4_HASH_EXT,

	/* Extensions used in certificates owned by the platform owner. */
	PROT_PK_EXT,
	PLAT_PK_EXT,
	SP_PKG5_HASH_EXT,
	SP_PKG6_HASH_EXT,
	SP_PKG7_HASH_EXT,
	SP_PKG8_HASH_EXT,
	NON_TRUSTED_FW_NVCOUNTER_EXT,
	NON_TRUSTED_WORLD_BOOTLOADER_HASH_EXT,
	NON_TRUSTED_FW_CONFIG_HASH_EXT,
};

/* Keys. */
enum {
	/* Keys owned by the silicon provider. */
	ROT_KEY,
	SWD_ROT_KEY,
	CORE_SWD_KEY,

	/* Keys owned by the platform owner. */
	PROT_KEY,
	PLAT_KEY,
};

#endif /* CCA_COT_H */
