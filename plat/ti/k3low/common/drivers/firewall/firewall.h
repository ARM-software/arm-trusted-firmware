/*
 * Copyright (C) 2026 Texas Instruments Incorporated - https://www.ti.com/
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
#define DDR_FWL_ID		1U
#define OSPI_FWL_ID		97U
#define ADC_MCASP_FWL_ID	160U

/* Firewall Regions to configure */
#define DDR_BG_FWL_REGION	0U
#define OSPI_FWL_REGION		0U
#define ADC_MCASP_FWL_REGION	0U

/* Firewall Control Register Values */
#define FWL_CTRL_EN		0xA
#define FWL_CTRL_EN_BG		(FWL_CTRL_EN | FW_BACKGROUND_BIT)

/* Firewall permission values */
#define FWL_PERM_ALL_RW		0xC3BBBB /* RW access to ALL hosts */

struct fwl_data {
	uint16_t fwl_id;
	uint8_t region;
};

/*
 * ROM configures some firewalls initially for its use. Re-configure these
 * firewalls with a permissive configuration so that other users can access
 * these regions after boot.
 *
 * Additionally, configure a permissive background firewall over the DDR
 */
void update_fwl_configs(void);

#endif /* FIREWALL_H */
