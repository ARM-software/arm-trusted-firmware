/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOC_TZASC_H
#define SOC_TZASC_H

#define MAX_NUM_TZC_REGION	3

/* TZASC related constants */
#define TZASC_CONFIGURATION_REG		0x000
#define TZASC_SECURITY_INV_REG		0x034
#define TZASC_SECURITY_INV_EN		0x1
#define TZASC_REGIONS_REG		0x100
/* As region address should address atleast 32KB memory. */
#define TZASC_REGION_LOWADDR_MASK	0xFFFF8000
#define TZASC_REGION_LOWADDR_OFFSET	0x0
#define TZASC_REGION_HIGHADDR_OFFSET	0x4
#define TZASC_REGION_ATTR_OFFSET	0x8
#define TZASC_REGION_ENABLED		1
#define TZASC_REGION_DISABLED		0
#define TZASC_REGION_SIZE_32KB		0xE
#define TZASC_REGION_SIZE_64KB		0xF
#define TZASC_REGION_SIZE_128KB		0x10
#define TZASC_REGION_SIZE_256KB		0x11
#define TZASC_REGION_SIZE_512KB		0x12
#define TZASC_REGION_SIZE_1MB		0x13
#define TZASC_REGION_SIZE_2MB		0x14
#define TZASC_REGION_SIZE_4MB		0x15
#define TZASC_REGION_SIZE_8MB		0x16
#define TZASC_REGION_SIZE_16MB		0x17
#define TZASC_REGION_SIZE_32MB		0x18
#define TZASC_REGION_SIZE_64MB		0x19
#define TZASC_REGION_SIZE_128MB		0x1A
#define TZASC_REGION_SIZE_256MB		0x1B
#define TZASC_REGION_SIZE_512MB		0x1C
#define TZASC_REGION_SIZE_1GB		0x1D
#define TZASC_REGION_SIZE_2GB		0x1E
#define TZASC_REGION_SIZE_4GB		0x1F
#define TZASC_REGION_SIZE_8GB		0x20
#define TZASC_REGION_SIZE_16GB		0x21
#define TZASC_REGION_SIZE_32GB		0x22
#define TZASC_REGION_SECURITY_SR	(1 << 3)
#define TZASC_REGION_SECURITY_SW	(1 << 2)
#define TZASC_REGION_SECURITY_SRW	(TZASC_REGION_SECURITY_SR| \
						TZASC_REGION_SECURITY_SW)
#define TZASC_REGION_SECURITY_NSR	(1 << 1)
#define TZASC_REGION_SECURITY_NSW	1
#define TZASC_REGION_SECURITY_NSRW	(TZASC_REGION_SECURITY_NSR| \
						TZASC_REGION_SECURITY_NSW)

#define CSU_SEC_ACCESS_REG_OFFSET	0x21C
#define TZASC_BYPASS_MUX_DISABLE	0x4
#define CCI_TERMINATE_BARRIER_TX	0x8
#define CONFIG_SYS_FSL_TZASC_ADDR	0x1500000

struct tzc380_reg {
	unsigned int secure;
	unsigned int enabled;
	unsigned int low_addr;
	unsigned int high_addr;
	unsigned int size;
	unsigned int sub_mask;
};

/* List of MAX_NUM_TZC_REGION TZC regions' boundaries and configurations. */

static const struct tzc380_reg tzc380_reg_list[] = {
	{
		TZASC_REGION_SECURITY_NSRW,	/* .secure attr */
		0x0,			/* .enabled */
		0x0,			/* .lowaddr */
		0x0,			/* .highaddr */
		0x0,			/* .size */
		0x0,			/* .submask */
	},
	{
		TZASC_REGION_SECURITY_SRW,
		TZASC_REGION_ENABLED,
		0xFC000000,
		0x0,
		TZASC_REGION_SIZE_64MB,
		0x80,			/* Disable region 7 */
	},
	/* reserve 2M non-scure memory for OPTEE public memory */
	{
		TZASC_REGION_SECURITY_SRW,
		TZASC_REGION_ENABLED,
		0xFF800000,
		0x0,
		TZASC_REGION_SIZE_8MB,
		0xC0,			/* Disable region 6 & 7 */
	},

	{}
};

#endif /* SOC_TZASC_H */
