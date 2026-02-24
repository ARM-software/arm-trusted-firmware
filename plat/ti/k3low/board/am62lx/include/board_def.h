/*
 * Copyright (C) 2024-2026 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BOARD_DEF_H
#define BOARD_DEF_H

/* The ports must be in order and contiguous */
#define K3_CLUSTER0_CORE_COUNT          U(2)
#define K3_CLUSTER1_CORE_COUNT          U(0)
#define K3_CLUSTER2_CORE_COUNT          U(0)
#define K3_CLUSTER3_CORE_COUNT          U(0)

#define SCMI_SHMEM_ADDR		(0x70800000UL)
#define SCMI_SHMEM_SIZE		(0x1000UL)	/* 4096 bytes */

/*
 * Fuse writebuff
 * K3_FUSE_WRITEBUFF_BASE - buffer content base addr in memory
 * K3_FUSE_WRITEBUFF_SIZE - Maximum allocated size for the buffer
 */
#define K3_FUSE_WRITEBUFF_BASE	(0x82000000UL)
#define K3_FUSE_WRITEBUFF_SIZE	(0x1000UL)

#endif /* BOARD_DEF_H */
