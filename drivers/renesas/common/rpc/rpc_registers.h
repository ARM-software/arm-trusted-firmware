/*
 * Copyright (c) 2015-2017, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RPC_REGISTERS_H
#define RPC_REGISTERS_H

#define RPC_BASE	(0xEE200000U)
#define RPC_CMNCR	(RPC_BASE + 0x0000U)
#define RPC_SSLDR	(RPC_BASE + 0x0004U)
#define RPC_DRCR	(RPC_BASE + 0x000CU)
#define RPC_DRCMR	(RPC_BASE + 0x0010U)
#define RPC_DRENR	(RPC_BASE + 0x001CU)
#define RPC_SMCR	(RPC_BASE + 0x0020U)
#define RPC_SMCMR	(RPC_BASE + 0x0024U)
#define RPC_SMENR	(RPC_BASE + 0x0030U)
#define RPC_CMNSR	(RPC_BASE + 0x0048U)
#define RPC_DRDMCR	(RPC_BASE + 0x0058U)
#define RPC_DRDRENR	(RPC_BASE + 0x005CU)
#define RPC_PHYCNT	(RPC_BASE + 0x007CU)
#define RPC_PHYINT	(RPC_BASE + 0x0088U)

#endif /* RPC_REGISTERS_H */
