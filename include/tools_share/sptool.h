/*
 * Copyright (c) 2018-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPTOOL_H
#define SPTOOL_H

#include <stdint.h>

/* 4 Byte magic name "SPKG" */
#define SECURE_PARTITION_MAGIC		0x474B5053

/* Header for a secure partition package. */
struct sp_pkg_header {
	uint32_t magic;
	uint32_t version;
	uint32_t pm_offset;
	uint32_t pm_size;
	uint32_t img_offset;
	uint32_t img_size;
};

#endif /* SPTOOL_H */
