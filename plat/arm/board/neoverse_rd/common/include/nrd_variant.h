/*
 * Copyright (c) 2018-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NRD_VARIANT_H
#define NRD_VARIANT_H

/* SID Version values for RD-N2 */
#define RD_N2_SID_VER_PART_NUM			0x07B7

/* SID Version values for RD-N2 variants */
#define RD_N2_CFG1_SID_VER_PART_NUM		0x07B6
#define RD_N2_CFG3_SID_VER_PART_NUM		0x07F1

/* SID Version values for RD-V2 */
#define RD_V2_SID_VER_PART_NUM			0x07F2
#define RD_V2_CONFIG_ID				0x1

/* SID Version values for RD-V3 */
#define RD_V3_SID_VER_PART_NUM		0x07EE
#define RD_V3_CONFIG_ID			0x0

/* SID Version values for RD-V3 variants */
#define RD_V3_CFG1_SID_VER_PART_NUM	0x07F9
#define RD_V3_CFG2_SID_VER_PART_NUM	0x07EE

/* Structure containing Neoverse RD platform variant information */
typedef struct nrd_platform_info {
	unsigned int platform_id;	/* Part Number of the platform */
	unsigned int config_id;		/* Config Id of the platform */
	unsigned int chip_id;		/* Chip Id or Node number */
	unsigned int multi_chip_mode;	/* Multi-chip mode availability */
} nrd_platform_info_t;

extern nrd_platform_info_t nrd_plat_info;

/* returns the part number of the platform*/
unsigned int plat_arm_nrd_get_platform_id(void);

/* returns the configuration id of the platform */
unsigned int plat_arm_nrd_get_config_id(void);

/* returns true if operating in multi-chip configuration */
unsigned int plat_arm_nrd_get_multi_chip_mode(void);

#endif /* NRD_VARIANT_H */
