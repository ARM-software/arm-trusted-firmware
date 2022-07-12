/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_STARTUP_H
#define PLAT_STARTUP_H

/* For FSBL handover */
enum fsbl_handoff {
	FSBL_HANDOFF_SUCCESS = 0,
	FSBL_HANDOFF_NO_STRUCT,
	FSBL_HANDOFF_INVAL_STRUCT,
	FSBL_HANDOFF_TOO_MANY_PARTS
};

#define FSBL_MAX_PARTITIONS		8U

/* Structure corresponding to each partition entry */
struct xfsbl_partition {
	uint64_t entry_point;
	uint64_t flags;
};

/* Structure for handoff parameters to ARM Trusted Firmware (ATF) */
struct xfsbl_atf_handoff_params {
	uint8_t magic[4];
	uint32_t num_entries;
	struct xfsbl_partition partition[FSBL_MAX_PARTITIONS];
};

#define ATF_HANDOFF_PARAMS_MAX_SIZE	sizeof(struct xfsbl_atf_handoff_params)

enum fsbl_handoff fsbl_atf_handover(entry_point_info_t *bl32,
					entry_point_info_t *bl33,
					uint64_t atf_handoff_addr);

#endif /* PLAT_STARTUP_H */
