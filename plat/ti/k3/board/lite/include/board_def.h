/*
 * Copyright (c) 2020-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BOARD_DEF_H
#define BOARD_DEF_H

#include <lib/utils_def.h>

/* The ports must be in order and contiguous */
#define K3_CLUSTER0_CORE_COUNT		U(4)
#define K3_CLUSTER1_CORE_COUNT		U(0)
#define K3_CLUSTER2_CORE_COUNT		U(0)
#define K3_CLUSTER3_CORE_COUNT		U(0)

#define PLAT_PROC_START_ID		U(32)
#define PLAT_PROC_DEVICE_START_ID	U(135)
#define PLAT_CLUSTER_DEVICE_START_ID	U(134)
#define PLAT_BOARD_DEVICE_ID		U(157)

#endif /* BOARD_DEF_H */
