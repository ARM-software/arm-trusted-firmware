/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef VOLTAGE_LEVEL_H
#define VOLTAGE_LEVEL_H

/*
 * Voltage level definitions for RPMh rails.
 *
 * Do not change the existing enum values for the defined corners. Newly added
 * corners must be defined with a new enum value without changing the existing
 * ones. Check with the power/system driver owners before updating this file.
 */
enum rail_voltage_level {
	RAIL_VOLTAGE_LEVEL_OFF		= 0x0,		/* 0 */
	RAIL_VOLTAGE_LEVEL_RET		= 0x10,		/* 16 */
	RAIL_VOLTAGE_LEVEL_LOW_SVS	= 0x40,		/* 64 */
	RAIL_VOLTAGE_LEVEL_SVS		= 0x80,		/* 128 */
	RAIL_VOLTAGE_LEVEL_NOM		= 0x100,	/* 256 */
	RAIL_VOLTAGE_LEVEL_TUR		= 0x180,	/* 384 */

	RAIL_VOLTAGE_LEVEL_MAX		= RAIL_VOLTAGE_LEVEL_TUR,
	RAIL_VOLTAGE_LEVEL_NUM_LEVELS	= 6,
	RAIL_VOLTAGE_LEVEL_INVALID	= -1,
	RAIL_VOLTAGE_LEVEL_OVERLIMIT	= -2
};

#endif /* VOLTAGE_LEVEL_H */
