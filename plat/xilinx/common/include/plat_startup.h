/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_STARTUP_H
#define PLAT_STARTUP_H

#include <common/bl_common.h>

/* For Xilinx bootloader XBL handover */
enum xbl_handoff {
	XBL_HANDOFF_SUCCESS = 0,
	XBL_HANDOFF_NO_STRUCT,
	XBL_HANDOFF_INVAL_STRUCT,
	XBL_HANDOFF_TOO_MANY_PARTS
};

#define XBL_MAX_PARTITIONS		8U

/* Structure corresponding to each partition entry */
struct xbl_partition {
	uint64_t entry_point;
	uint64_t flags;
};

/* Structure for handoff parameters to TrustedFirmware-A (TF-A) */
struct xbl_handoff_params {
	uint8_t magic[4];
	uint32_t num_entries;
	struct xbl_partition partition[XBL_MAX_PARTITIONS];
};

#define HANDOFF_PARAMS_MAX_SIZE	 sizeof(struct xbl_handoff_params)

enum xbl_handoff xbl_handover(entry_point_info_t *bl32,
					entry_point_info_t *bl33,
					uint64_t handoff_addr);

/* JEDEC Standard Manufacturer's Identification Code and Bank ID JEP106 */
#define JEDEC_XILINX_MFID	U(0x49)
#define JEDEC_XILINX_BKID	U(0)

#endif /* PLAT_STARTUP_H */
