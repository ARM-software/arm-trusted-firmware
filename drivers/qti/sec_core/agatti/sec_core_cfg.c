/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <drivers/qti/sec_core/sec_core.h>
#include <lib/utils_def.h>
#include <sec_core_defs.h>

/* The secure access controls stay as XBL configured them. */
const struct sec_core_cfg_s qti_sec_core_cfg[] = {};
const size_t qti_sec_core_cfg_count = ARRAY_SIZE(qti_sec_core_cfg);

const uintptr_t qti_sec_core_rvbaraddr_lo = APCS_COMMON_RVBARADDR_LO_ADDR;
const uintptr_t qti_sec_core_rvbaraddr_hi = APCS_COMMON_RVBARADDR_HI_ADDR;
