/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device Group Types
 *
 * This header defines the device group (devgrp) type and constants used
 * to categorize and distinguish device groups in power management operations.
 */

#ifndef SYSFW_DEVGRPS_H
#define SYSFW_DEVGRPS_H

#include <stdint.h>


/**
 * \brief Defines the sysfw DEVGRP type. This is meant to be used in code
 * or data structures that require distinction of devgrps.
 */
typedef uint8_t devgrp_t;

/** SoC defined SYSFW devgrp 00 */
#define DEVGRP_00 ((0x01U) << 0U)

#endif /* SYSFW_DEVGRPS_H */
