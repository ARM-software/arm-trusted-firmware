/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PWR_UTILS_H
#define PWR_UTILS_H

#include <stdint.h>

#include <drivers/qti/pwr_utils/voltage_level.h>

/*
 * Enums of all SW corners. These values are expected to remain the same
 * throughout the lifetime of the chip.
 *
 * Voltage rail SW corners are already defined in voltage_level.h, so they are
 * not repeated here.
 */

/* DDR resource levels/corners */
enum {
	DDRSS_LEVEL_OFF			= 0x0,	/* D3 + SHRM Gated + SC flush */
	DDRSS_LEVEL_PHY_COLLAPSED	= 0x20,	/* D3 PHY Collapsed */
	DDRSS_LEVEL_PLL_DISABLED	= 0x80,	/* PLL Disabled */
	DDRSS_LEVEL_ACTIVE		= 0xA0,
	DDRSS_LEVEL_MAX			= DDRSS_LEVEL_ACTIVE,
	DDRSS_LEVEL_INVALID		= -1,
	DDRSS_LEVEL_NUM_LEVELS		= 4
};

/* XO resource levels/corners */
enum {
	XO_LEVEL_CRYSTAL_OFF		= 0x0,
	XO_LEVEL_PMIC_BUFFER_OFF	= 0x20,
	XO_LEVEL_SOC_BUFFER_OFF		= 0x50,
	XO_LEVEL_ON			= 0x80,
	XO_LEVEL_MAX			= XO_LEVEL_ON,
	XO_LEVEL_INVALID		= -1,
	XO_LEVEL_NUM_LEVELS		= 4
};

/*
 * qti_pwr_utils_init - set up rails and supported corners for the target.
 *
 * Must be called once during subsystem initialization before any other
 * pwr_utils API is used.
 */
#ifdef QTI_PWR_UTILS_ENABLED
void qti_pwr_utils_init(void);
#else
static inline void qti_pwr_utils_init(void) {}
#endif

/*
 * pwr_utils_lvl_resource_idx - return the resource index for a resource name.
 *
 * The returned index is meaningful only to the pwr_utils lvl APIs and can be
 * reused for subsequent hlvl/vlvl queries.
 *
 * @res_name: resource name (e.g. "cx.lvl", "mx.lvl").
 *
 * Returns the resource index (>= 0) on success, -1 otherwise.
 */
int pwr_utils_lvl_resource_idx(const char *res_name);

/*
 * pwr_utils_hlvl_named_resource - map a SW corner to a HW corner by name.
 *
 * Performs an internal name lookup on each call.
 *
 * @resource:    resource name (e.g. "cx.lvl", "mx.lvl").
 * @vlvl:        SW corner to map.
 * @mapped_vlvl: if non-NULL, on success set to the vlvl used for the returned
 *               hlvl:
 *                 == vlvl when the exact vlvl is supported,
 *                 >  vlvl when a round-up occurred,
 *                 == RAIL_VOLTAGE_LEVEL_OVERLIMIT when vlvl exceeds the max
 *                    supported vlvl (error).
 *
 * Returns the HW corner (>= 0) on success, -1 otherwise.
 */
int pwr_utils_hlvl_named_resource(const char *resource, int vlvl,
				  int *mapped_vlvl);

#endif /* PWR_UTILS_H */
