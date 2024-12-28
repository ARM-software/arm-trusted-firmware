/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PM_TYPES_H
#define PM_TYPES_H

#include <types/short_types.h>
#include <stdbool.h>

/**
 * This defines the index of a powerdomain within a PSC. The current PSC
 * register layout limits the max number of powerdomains to 32. This value
 * can be compressed to a 5-bit bitfield.
 *
 * This index is internal to the DSMC firmware.
 */
typedef uint8_t pd_idx_t;

/**
 * This defines the index of a module within a PSC. The current PSC register
 * layout limits the max number of modules to 128. This value can be
 * compressed to a 7-bit bitfield.
 *
 * This index is internal to the DSMC firmware.
 */
typedef uint8_t lpsc_idx_t;

/**
 * This defines the arbitrary ID of PSC within an SoC. The ID is assigned
 * by the specdata autogen scripts. We are currently limiting the maximum
 * number of PSCs within an SoC to 6 and this field can be compressed to
 * a 3-bit bitfield. The remaining two values have special meanings:
 *
 * 6 PSC_DEV_MULTIPLE - IP is associated with domains in multiple PSCs.
 * 7 PSC_DEV_NONE -	IP is not associated with any PSC.
 *
 * This index is internal to the DSMC firmware.
 */
typedef uint8_t psc_idx_t;

/**
 * This defines the arbitrary ID of a clock within the SoC. The ID is
 * assigned by the specdata autogen scripts. We only export clock connections
 * to device IP via the ABI. Each clock connection is then associated
 * internally with an SoC clock. We currently have fewer than 1024 clocks on
 * every SoC so this field can be compressed to a 10-bit bitfield
 *
 * This index is internal to the DSMC firmware.
 */
typedef uint16_t clk_idx_t;

/**
 * This defines a clock connected to a device IP. IDs are assigned sequentially
 * and their name and ID are exported for use via the ABI.
 *
 * This index has a 1-to-1 correspondence with the clk field in the TISCI
 * device clock messages.
 */
typedef uint16_t dev_clk_idx_t;

#endif
