/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCMI_SQ_H
#define SCMI_SQ_H

#include <stddef.h>
#include <stdint.h>

#include <sq_common.h>

/* Structure to represent available DRAM region */
struct dram_info_resp {
	int status;
	int reserved;
	struct draminfo info;
};

/* API to get the available DRAM region */
int scmi_get_draminfo(void *p, struct draminfo *info);

#endif /* SCMI_SQ_H */
