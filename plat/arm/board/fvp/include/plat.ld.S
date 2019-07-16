/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PLAT_LD_S
#define PLAT_LD_S

#include <plat/arm/common/arm_tzc_dram.ld.S>

#if RECLAIM_INIT_CODE
#include <plat/arm/common/arm_reclaim_init.ld.S>
#endif /* RECLAIM_INIT_CODE */

#endif /* PLAT_LD_S */
