/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSFW_DOMGRPS_H
#define SYSFW_DOMGRPS_H

#include <types/errno.h>
#include <types/short_types.h>

/**
 * \brief Defines the sysfw DOMGRP type. This is meant to be used in code
 * or data structures that require distinction of domgrps.
 */
typedef uint8_t domgrp_t;

/* External definitions */

/** SoC SYSFW domgrp for backward compatibility */
#define DOMGRP_COMPATIBILITY	(0x00U)

/** SoC defined SYSFW domgrp 00 */
#define DOMGRP_00		((0x01U) << 0U)
/** SoC defined SYSFW domgrp 01 */
#define DOMGRP_01		((0x01U) << 1U)
/** SoC defined SYSFW domgrp 02 */
#define DOMGRP_02		((0x01U) << 2U)
/** SoC defined SYSFW domgrp 03 */
#define DOMGRP_03		((0x01U) << 3U)
/** SoC defined SYSFW domgrp 04 */
#define DOMGRP_04		((0x01U) << 4U)
/** SoC defined SYSFW domgrp 05 */
#define DOMGRP_05		((0x01U) << 5U)
/** SoC defined SYSFW domgrp 06 */
#define DOMGRP_06		((0x01U) << 6U)

/** SYSFW internal usage ONLY */
#define DOMGRP_07		((0x01U) << 7U)
/** Match everything - STRICTLY INTERNAL USAGE ONLY */
#define DOMGRP_ALL		(0xFFU)

/**
 * Maximum number of domgrps that are supported by SYSFW.
 * Derived from the above definitions
 */
#define MAX_NUM_DOMGRPS (8U)

#endif /* SYSFW_DOMGRPS_H */
