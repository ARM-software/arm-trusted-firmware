/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CPU_SVC_H
#define CPU_SVC_H

/*
 * Function Identifier value ranges for CPU Service Calls.
 */
/* CPU_SMC_32			0x81000000U */
/* CPU_SMC_64			0xC1000000U */

/* Function Identifier values of general queries */
#define CPU_SVC_UID		0x8100ff01
/*				0x8100ff02 is reserved */
#define CPU_SVC_VERSION         0x8100ff03

#define CPU_SVC_VERSION_MAJOR	1
#define CPU_SVC_VERSION_MINOR	0

#endif /* CPU_SVC_H */
