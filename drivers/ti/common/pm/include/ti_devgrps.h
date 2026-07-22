/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device Group Types
 *
 * This header defines the device group (devgrp) type and constants used
 * to categorize and distinguish device groups in power management operations.
 */

#ifndef TI_DEVGRPS_H
#define TI_DEVGRPS_H

#include <stdint.h>


/*
 * Defines the power domain DEVGRP type. Used in code or data
 * structures that require distinction of devgrps.
 */
typedef uint8_t devgrp_t;

/* SoC defined SYSFW devgrp 00 */
#define TI_DEVGRP_00 ((0x01U) << 0U)

#endif /* TI_DEVGRPS_H */
