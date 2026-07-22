/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * PSC SoC Device Definitions
 *
 * This header defines the SoC-specific device data structure for Power
 * and Sleep Controller (PSC) devices, including power domain, module
 * index, and PSC controller information.
 */

#ifndef TI_PSC_SOC_DEVICE_H
#define TI_PSC_SOC_DEVICE_H

#include <ti_pm_types.h>

/*
 * SoC specific const device data.
 *
 * This stores the SoC specific const data for each device.
 */
struct ti_soc_device_data {
	/* PSC index */
	uint8_t psc_idx;
	/* PSC powerdomain */
	uint8_t pd;
	/* PSC module index */
	ti_lpsc_idx_t mod;
};

#endif /* TI_PSC_SOC_DEVICE_H */
