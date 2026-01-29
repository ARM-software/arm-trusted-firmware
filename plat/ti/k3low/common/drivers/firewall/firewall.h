/*
 * Copyright (c) 2026, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FIREWALL_H
#define FIREWALL_H

#include <stdint.h>

#include <lib/utils_def.h>

#define TFA_HOST_ID		10U
#define A53_PRIV_ID		4U
#define FW_BACKGROUND_BIT	BIT(8U)

/* Firewall IDs */
#define OSPI_FWL_ID		97U
#define ADC_MCASP_FWL_ID	160U

/* Number of firewall regions */
#define OSPI_FWL_NUM_REGIONS		8U
#define ADC_MCASP_FWL_NUM_REGIONS	16U

enum k3_fwl_region_type {
	K3_FWL_REGION_FOREGROUND = 0,
	K3_FWL_REGION_BACKGROUND = FW_BACKGROUND_BIT,
};

struct fwl_data {
	uint16_t fwl_id;
	uint8_t num_regions;
};

/*
 * Updates firewall configurations by disabling ROM-configured firewalls.
 * This function should be called during boot initialization and after
 * resume from low power mode to ensure firewall regions that were
 * configured by ROM for the boot phase are properly disabled.
 */
void update_fwl_configs(void);

#endif /* FIREWALL_H */
