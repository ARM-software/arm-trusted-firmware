/*
 * Copyright (c) 2019-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef PSA_MANIFEST_SID_H
#define PSA_MANIFEST_SID_H

/******** RSE_SP_CRYPTO ********/
#define RSE_CRYPTO_HANDLE				(0x40000100U)

/******** RSE_SP_PLATFORM ********/
#define RSE_PLATFORM_SERVICE_HANDLE			(0x40000105U)

/******** PSA_SP_MEASURED_BOOT ********/
#define RSE_MEASURED_BOOT_HANDLE			(0x40000110U)

/******** PSA_SP_DELEGATED_ATTESTATION ********/
#define RSE_DELEGATED_SERVICE_HANDLE			(0x40000111U)

/******** PSA_SP_DICE_PROTECTION_ENVIRONMENT ********/
#define RSE_DPE_SERVICE_HANDLE				(0x40000112U)

#endif /* PSA_MANIFEST_SID_H */
