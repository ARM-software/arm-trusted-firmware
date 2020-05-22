/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DUALROOT_OID_H
#define DUALROOT_OID_H

/* Reuse the Object IDs defined by TBBR for certificate extensions. */
#include "tbbr_oid.h"

/*
 * Platform root-of-trust public key.
 * Arbitrary value that does not conflict with any of the TBBR reserved OIDs.
 */
#define PROT_PK_OID				"1.3.6.1.4.1.4128.2100.1102"

/*
 * Secure Partitions Content Certificate
 */
#define SP_PKG1_HASH_OID                        "1.3.6.1.4.1.4128.2100.1301"
#define SP_PKG2_HASH_OID                        "1.3.6.1.4.1.4128.2100.1302"
#define SP_PKG3_HASH_OID                        "1.3.6.1.4.1.4128.2100.1303"
#define SP_PKG4_HASH_OID                        "1.3.6.1.4.1.4128.2100.1304"
#define SP_PKG5_HASH_OID                        "1.3.6.1.4.1.4128.2100.1305"
#define SP_PKG6_HASH_OID                        "1.3.6.1.4.1.4128.2100.1306"
#define SP_PKG7_HASH_OID                        "1.3.6.1.4.1.4128.2100.1307"
#define SP_PKG8_HASH_OID                        "1.3.6.1.4.1.4128.2100.1308"

#endif /* DUALROOT_OID_H */
