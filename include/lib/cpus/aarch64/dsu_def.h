/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DSU_DEF_H
#define DSU_DEF_H

#include <lib/utils_def.h>

/********************************************************************
 * DSU control registers definitions				    *
 ********************************************************************/
#define CLUSTERCFR_EL1		S3_0_C15_C3_0
#define CLUSTERIDR_EL1		S3_0_C15_C3_1
#define CLUSTERACTLR_EL1	S3_0_C15_C3_3

/********************************************************************
 * DSU control registers bit fields				    *
 ********************************************************************/
#define CLUSTERIDR_REV_SHIFT	U(0)
#define CLUSTERIDR_REV_BITS	U(4)
#define CLUSTERIDR_VAR_SHIFT	U(4)
#define CLUSTERIDR_VAR_BITS	U(4)
#define CLUSTERCFR_ACP_SHIFT	U(11)

/********************************************************************
 * Masks applied for DSU errata workarounds			    *
 ********************************************************************/
#define DSU_ERRATA_936184_MASK	(ULL(0x3) << 15)

#endif /* DSU_DEF_H */
