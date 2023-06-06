/*
 * Copyright (c) 2019-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef PSA_MANIFEST_SID_H
#define PSA_MANIFEST_SID_H

/******** RSS_SP_CRYPTO ********/
#define RSS_CRYPTO_HANDLE				(0x40000100U)

/******** RSS_SP_PLATFORM ********/
#define RSS_PLATFORM_SERVICE_HANDLE			(0x40000105U)

/******** PSA_SP_MEASURED_BOOT ********/
#define RSS_MEASURED_BOOT_HANDLE			(0x40000110U)

/******** PSA_SP_DELEGATED_ATTESTATION ********/
#define RSS_DELEGATED_SERVICE_HANDLE			(0x40000111U)

/******** PSA_SP_DICE_PROTECTION_ENVIRONMENT ********/
#define RSS_DPE_SERVICE_HANDLE				(0x40000112U)

#endif /* PSA_MANIFEST_SID_H */
