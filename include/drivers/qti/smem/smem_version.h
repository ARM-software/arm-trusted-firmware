/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMEM_VERSION_H
#define SMEM_VERSION_H

/*
 * SMEM_VERSION_INFO_SIZE defines the size of a version array.
 * SMEM_VERSION_TZ_HYP_OFFSET is the index into a version array for this
 * processor.
 *
 * DO NOT CHANGE THE FOLLOWING VALUES AS IT MAY BREAK VERSION CHECKING.
 */
#define SMEM_VERSION_INFO_SIZE		32

#define SMEM_VERSION_TZ_HYP_OFFSET	0

/* Allocate from cached heap */
#define SMEM_ALLOC_FLAG_CACHED		0x80000000

#endif /*SMEM_VERSION_H*/
