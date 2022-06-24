/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CCA_OID_H
#define CCA_OID_H

/* Reuse the Object IDs defined by TBBR for certificate extensions. */
#include "tbbr_oid.h"

/*
 * Assign arbitrary Object ID values that do not conflict with any of the
 * TBBR reserved OIDs.
 */
/* Platform root-of-trust public key */
#define PROT_PK_OID				"1.3.6.1.4.1.4128.2100.1102"
/* Secure World root-of-trust public key */
#define SWD_ROT_PK_OID				"1.3.6.1.4.1.4128.2100.1103"
/* Core Secure World public key */
#define CORE_SWD_PK_OID				"1.3.6.1.4.1.4128.2100.1104"
/* Platform public key */
#define PLAT_PK_OID				"1.3.6.1.4.1.4128.2100.1105"
/* Realm Monitor Manager (RMM) Hash */
#define RMM_HASH_OID				"1.3.6.1.4.1.4128.2100.1106"

#endif /* CCA_OID_H */
