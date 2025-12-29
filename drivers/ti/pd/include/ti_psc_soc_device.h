/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
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

#ifndef PSC_SOC_DEVICE_H
#define PSC_SOC_DEVICE_H

#include <ti_pm_types.h>

/**
 * \brief SoC specific const device data.
 *
 * This stores the SoC specific const data for each device.
 */
struct ti_soc_device_data {
	uint8_t psc_idx;        /** PSC index */
	uint8_t pd;             /** PSC powerdomain */
	lpsc_idx_t mod;         /** PSC module index */
};

#endif
