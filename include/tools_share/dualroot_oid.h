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

#endif /* DUALROOT_OID_H */
