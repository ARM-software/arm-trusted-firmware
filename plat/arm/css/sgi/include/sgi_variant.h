/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SGI_VARIANT_H
#define SGI_VARIANT_H

/* SSC_VERSION values for SGI575 */
#define SGI575_SSC_VER_PART_NUM		0x0783

/* SID Version values for RD-N1E1-Edge */
#define RD_N1E1_EDGE_SID_VER_PART_NUM		0x0786
#define RD_E1_EDGE_CONFIG_ID			0x2

/* Structure containing SGI platform variant information */
typedef struct sgi_platform_info {
	unsigned int platform_id;	/* Part Number of the platform */
	unsigned int config_id;		/* Config Id of the platform */
} sgi_platform_info_t;

extern sgi_platform_info_t sgi_plat_info;

/* returns the part number of the platform*/
unsigned int plat_arm_sgi_get_platform_id(void);

/* returns the configuration id of the platform */
unsigned int plat_arm_sgi_get_config_id(void);

#endif /* SGI_VARIANT_H */
