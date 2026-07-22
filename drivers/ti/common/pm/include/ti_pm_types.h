/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TI Power Management Types
 *
 * This header defines core power management type definitions for power
 * domains, LPSC modules, PSC controllers, clocks, and device clock indexes
 * used throughout the TI platform firmware.
 */

#ifndef TI_PM_TYPES_H
#define TI_PM_TYPES_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Index of a power domain within a PSC.
 *
 * The PSC register layout supports up to 32 power domains.
 */
typedef uint8_t ti_pd_idx_t;

/*
 * Index of an LPSC module within a PSC.
 *
 * The PSC register layout supports up to 128 modules.
 */
typedef uint8_t ti_lpsc_idx_t;

/*
 * Index of a PSC controller within the SoC.
 *
 * Each SoC can have multiple PSC controllers. Special values:
 * - 6: TI_PSC_DEV_MULTIPLE - IP is associated with domains in multiple PSCs
 * - 7: TI_PSC_DEV_NONE - IP is not associated with any PSC
 */
typedef uint8_t ti_psc_idx_t;

/*
 * Index of a clock within the SoC clock tree.
 *
 * Each SoC clock has a unique index used for internal clock management.
 */
typedef uint16_t ti_clk_idx_t;

/*
 * Index of a clock connection to a device IP block.
 *
 * Device clocks are exported for external use and correspond to the clock
 * field in device clock management messages.
 */
typedef uint16_t ti_dev_clk_idx_t;

/*
 * Index of a device within the SoC device table.
 *
 * Each device managed by the firmware has a unique index used for
 * internal device management and TISCI communication.
 */
typedef uint8_t ti_dev_idx_t;

#endif /* TI_PM_TYPES_H */
