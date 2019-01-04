/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef TEESMC_OPTEED_MACROS_H
#define TEESMC_OPTEED_MACROS_H

#include <common/runtime_svc.h>

#define TEESMC_OPTEED_RV(func_num) \
		((SMC_TYPE_FAST << FUNCID_TYPE_SHIFT) | \
		 ((SMC_32) << FUNCID_CC_SHIFT) | \
		 (62 << FUNCID_OEN_SHIFT) | \
		 ((func_num) & FUNCID_NUM_MASK))

#endif /* TEESMC_OPTEED_MACROS_H */
