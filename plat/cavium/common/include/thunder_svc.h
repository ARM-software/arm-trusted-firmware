/*
 * Copyright (c) 2016-2017, Cavium Inc. All rights reserved.<BR>
 * Copyright (c) 2018     , Facebook, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __THUNDER_SVC_H__
#define __THUNDER_SVC_H__

/* SMC function IDs for Standard Service queries */

#define THUNDERX_SVC_CALL_COUNT		0x4300ff00
#define THUNDERX_SVC_UID		0x4300ff01

#define THUNDERX_SVC_VERSION		0x4300ff03

/* ThunderX Service Calls version numbers */
#define THUNDERX_VERSION_MAJOR		0x0
#define THUNDERX_VERSION_MINOR		0x1

#define THUNDERX_NOR_READ		0x43000111
#define THUNDERX_NOR_WRITE		0x43000112
#define THUNDERX_NOR_ERASE		0x43000113

#define THUNDERX_DRAM_SIZE		0x43000301
/* x1 - node number
 */

#define THUNDERX_NODE_COUNT		0x43000601

/* x1 - character to print */
#define THUNDERX_PUTC			0x43000a01

/* X1 - gpio_num, X2 - sp, X3 - cpu, X4 - ttbr0 */
#define THUNDERX_INSTALL_GPIO_INT	0x43000801
/* X1 - gpio_num */
#define THUNDERX_REMOVE_GPIO_INT	0x43000802

#define THUNDERX_NUM_CALLS		12
#endif /* __THUNDER_SVC_H__ */
