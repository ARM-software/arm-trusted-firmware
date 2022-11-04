/*
 * Copyright (c) 2017-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HIKEY960_DEF_H
#define HIKEY960_DEF_H

#include <common/tbbr/tbbr_img_def.h>
#include <plat/common/common_def.h>

#define DDR_BASE			0x0
#define DDR_SIZE			0xE0000000

#define DEVICE_BASE			0xE0000000
#define DEVICE_SIZE			0x20000000

/* Memory location options for TSP */
#define HIKEY960_SRAM_ID	0
#define HIKEY960_DRAM_ID	1

/*
 * DDR for TEE (80MB from 0x3E00000-0x43000FFF) is divided into several
 * regions:
 *   - SPMC manifest (4KB at the top) used by SPMC_AT_EL3 and the TEE
 *   - Datastore for SPMC_AT_EL3 (4MB at the top) used by BL31
 *   - Secure DDR (default is the top 60MB) used by OP-TEE
 *   - Non-secure DDR used by OP-TEE (shared memory and padding) (4MB)
 *   - Secure DDR (4MB aligned on 4MB) for OP-TEE's "Secure Data Path" feature
 *   - Non-secure DDR (8MB) reserved for OP-TEE's future use
 */
#define DDR_SEC_SIZE			0x03C00000 /* reserve 60MB secure memory */
#define DDR_SEC_BASE			0x3F000000
#define DDR2_SEC_SIZE			0x00400000 /* SPMC_AT_EL3: 4MB for BL31 RAM2 */
#define DDR2_SEC_BASE			0x42C00000
#define DDR_SEC_CONFIG_SIZE		0x00001000 /* SPMC_AT_EL3: SPMC manifest */
#define DDR_SEC_CONFIG_BASE		0x43000000

#define DDR_SDP_SIZE			0x00400000
#define DDR_SDP_BASE			(DDR_SEC_BASE - 0x400000 /* align */ - \
					DDR_SDP_SIZE)

/*
 * PL011 related constants
 */
#define PL011_UART5_BASE		0xFDF05000
#define PL011_UART6_BASE		0xFFF32000
#define PL011_BAUDRATE			115200
#define PL011_UART_CLK_IN_HZ		19200000

#define UFS_BASE			0

#define HIKEY960_UFS_DESC_BASE		0x20000000
#define HIKEY960_UFS_DESC_SIZE		0x00200000	/* 2MB */
#define HIKEY960_UFS_DATA_BASE		0x10000000
#define HIKEY960_UFS_DATA_SIZE		0x0A000000	/* 160MB */

#if defined(SPMC_AT_EL3)
/*
 * Number of Secure Partitions supported.
 * SPMC at EL3, uses this count to configure the maximum number of supported
 * secure partitions.
 */
#define SECURE_PARTITION_COUNT      1

/*
 * Number of Nwld Partitions supported.
 * SPMC at EL3, uses this count to configure the maximum number of supported
 * nwld partitions.
 */
#define NS_PARTITION_COUNT      1
/*
 * Number of Logical Partitions supported.
 * SPMC at EL3, uses this count to configure the maximum number of supported
 * logical partitions.
 */
#define MAX_EL3_LP_DESCS_COUNT		1

#endif /* SPMC_AT_EL3 */

#endif /* HIKEY960_DEF_H */
