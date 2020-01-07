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

enum fsbl_handoff fsbl_atf_handover(entry_point_info_t *bl32_image_ep_info,
					entry_point_info_t *bl33_image_ep_info,
					uint64_t atf_handoff_addr);

#endif /* PLAT_STARTUP_H */
