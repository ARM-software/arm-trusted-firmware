/*
 * Copyright (c) 2019-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef PSA_MANIFEST_SID_H
#define PSA_MANIFEST_SID_H

/******** PSA_SP_INITIAL_ATTESTATION ********/
#define RSS_ATTESTATION_SERVICE_SID			(0x00000020U)
#define RSS_ATTESTATION_SERVICE_VERSION			(1U)
#define RSS_ATTESTATION_SERVICE_HANDLE			(0x40000103U)

/******** PSA_SP_MEASURED_BOOT ********/
#define RSS_MEASURED_BOOT_SID				(0x000000E0U)
#define RSS_MEASURED_BOOT_VERSION			(1U)
#define RSS_MEASURED_BOOT_HANDLE			(0x40000104U)

#endif /* PSA_MANIFEST_SID_H */
