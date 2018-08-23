/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/********************************************************************
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/

#ifndef __QTI_PLAT_PARAMS_H__
#define __QTI_PLAT_PARAMS_H__

#include <stdint.h>

/*
 * We defined several plat parameter structs for BL2 to pass platform related
 * parameters to QTI BL31 platform code.  All plat parameters start with
 * a common header, which has a type field to indicate the parameter type, and
 * a next pointer points to next parameter. If the parameter is the last one in
 * the list, next pointer will points to NULL.  After the header comes the
 * variable-sized members that describe the parameter. The picture below shows
 * how the parameters are kept in memory.
 *
 * head of list  ---> +----------------+ --+
 *                    |      type      |   |
 *                    +----------------+   |--> struct bl31_plat_param
 *               +----|      next      |   |
 *               |    +----------------+ --+
 *               |    | parameter data |
 *               |    +----------------+
 *               |
 *               +--> +----------------+ --+
 *                    |      type      |   |
 *                    +----------------+   |--> struct bl31_plat_param
 *           NULL <---|      next      |   |
 *                    +----------------+ --+
 *                    | parameter data |
 *                    +----------------+
 *
 * Note: The SCTLR_EL3.A bit (Alignment fault check enable) of ARM TF is set,
 * so be sure each parameter struct starts on 64-bit aligned address. If not,
 * alignment fault will occur during accessing its data member.
 */

/* param type */
enum {
	PARAM_NONE = 0,
	PARAM_COREBOOT_TABLE,
};

/* common header for all plat parameter type */
struct bl31_plat_param {
	uint64_t type;
	void *next;
};

struct bl31_u64_param {
	struct bl31_plat_param h;
	uint64_t value;
};

#endif /* __QTI_PLAT_PARAMS_H__ */
