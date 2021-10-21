/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TZC380_H
#define TZC380_H

#include <drivers/arm/tzc_common.h>
#include <lib/utils_def.h>

#define TZC380_CONFIGURATION_OFF	U(0x000)
#define ACTION_OFF			U(0x004)
#define LOCKDOWN_RANGE_OFF		U(0x008)
#define LOCKDOWN_SELECT_OFF		U(0x00C)
#define INT_STATUS			U(0x010)
#define INT_CLEAR			U(0x014)

#define FAIL_ADDRESS_LOW_OFF		U(0x020)
#define FAIL_ADDRESS_HIGH_OFF		U(0x024)
#define FAIL_CONTROL_OFF		U(0x028)
#define FAIL_ID				U(0x02c)

#define SPECULATION_CTRL_OFF		U(0x030)
#define SECURITY_INV_EN_OFF		U(0x034)

#define REGION_SETUP_LOW_OFF(n)		U(0x100 + (n) * 0x10)
#define REGION_SETUP_HIGH_OFF(n)	U(0x104 + (n) * 0x10)
#define REGION_ATTRIBUTES_OFF(n)	U(0x108 + (n) * 0x10)

#define BUILD_CONFIG_AW_SHIFT		8
#define BUILD_CONFIG_AW_MASK		U(0x3f)
#define BUILD_CONFIG_NR_SHIFT		0
#define BUILD_CONFIG_NR_MASK		U(0xf)

#define ACTION_RV_SHIFT			0
#define ACTION_RV_MASK			U(0x3)
#define ACTION_RV_LOWOK			U(0x0)
#define ACTION_RV_LOWERR		U(0x1)
#define ACTION_RV_HIGHOK		U(0x2)
#define ACTION_RV_HIGHERR		U(0x3)

/* Speculation is enabled by default. */
#define SPECULATION_CTRL_WRITE_DISABLE	BIT_32(1)
#define SPECULATION_CTRL_READ_DISABLE	BIT_32(0)

#define INT_STATUS_OVERRUN_SHIFT	1
#define INT_STATUS_OVERRUN_MASK		U(0x1)
#define INT_STATUS_STATUS_SHIFT		0
#define INT_STATUS_STATUS_MASK		U(0x1)

#define INT_CLEAR_CLEAR_SHIFT		0
#define INT_CLEAR_CLEAR_MASK		U(0x1)

#define TZC380_COMPONENT_ID		U(0xb105f00d)
#define TZC380_PERIPH_ID_LOW		U(0x001bb380)
#define TZC380_PERIPH_ID_HIGH		U(0x00000004)

#define TZC_SP_NS_W			BIT_32(0)
#define TZC_SP_NS_R			BIT_32(1)
#define TZC_SP_S_W			BIT_32(2)
#define TZC_SP_S_R			BIT_32(3)

#define TZC_ATTR_SP_SHIFT		28
#define TZC_ATTR_SP_ALL			((TZC_SP_S_W | TZC_SP_S_R | TZC_SP_NS_W | \
					TZC_SP_NS_R) << TZC_ATTR_SP_SHIFT)
#define TZC_ATTR_SP_S_RW		((TZC_SP_S_W | TZC_SP_S_R) << \
					 TZC_ATTR_SP_SHIFT)
#define TZC_ATTR_SP_NS_RW		((TZC_SP_NS_W | TZC_SP_NS_R) << \
					TZC_ATTR_SP_SHIFT)

#define TZC_REGION_SIZE_32K		U(0xe)
#define TZC_REGION_SIZE_64K		U(0xf)
#define TZC_REGION_SIZE_128K		U(0x10)
#define TZC_REGION_SIZE_256K		U(0x11)
#define TZC_REGION_SIZE_512K		U(0x12)
#define TZC_REGION_SIZE_1M		U(0x13)
#define TZC_REGION_SIZE_2M		U(0x14)
#define TZC_REGION_SIZE_4M		U(0x15)
#define TZC_REGION_SIZE_8M		U(0x16)
#define TZC_REGION_SIZE_16M		U(0x17)
#define TZC_REGION_SIZE_32M		U(0x18)
#define TZC_REGION_SIZE_64M		U(0x19)
#define TZC_REGION_SIZE_128M		U(0x1a)
#define TZC_REGION_SIZE_256M		U(0x1b)
#define TZC_REGION_SIZE_512M		U(0x1c)
#define TZC_REGION_SIZE_1G		U(0x1d)
#define TZC_REGION_SIZE_2G		U(0x1e)
#define TZC_REGION_SIZE_4G		U(0x1f)
#define TZC_REGION_SIZE_8G		U(0x20)
#define TZC_REGION_SIZE_16G		U(0x21)
#define TZC_REGION_SIZE_32G		U(0x22)
#define TZC_REGION_SIZE_64G		U(0x23)
#define TZC_REGION_SIZE_128G		U(0x24)
#define TZC_REGION_SIZE_256G		U(0x25)
#define TZC_REGION_SIZE_512G		U(0x26)
#define TZC_REGION_SIZE_1T		U(0x27)
#define TZC_REGION_SIZE_2T		U(0x28)
#define TZC_REGION_SIZE_4T		U(0x29)
#define TZC_REGION_SIZE_8T		U(0x2a)
#define TZC_REGION_SIZE_16T		U(0x2b)
#define TZC_REGION_SIZE_32T		U(0x2c)
#define TZC_REGION_SIZE_64T		U(0x2d)
#define TZC_REGION_SIZE_128T		U(0x2e)
#define TZC_REGION_SIZE_256T		U(0x2f)
#define TZC_REGION_SIZE_512T		U(0x30)
#define TZC_REGION_SIZE_1P		U(0x31)
#define TZC_REGION_SIZE_2P		U(0x32)
#define TZC_REGION_SIZE_4P		U(0x33)
#define TZC_REGION_SIZE_8P		U(0x34)
#define TZC_REGION_SIZE_16P		U(0x35)
#define TZC_REGION_SIZE_32P		U(0x36)
#define TZC_REGION_SIZE_64P		U(0x37)
#define TZC_REGION_SIZE_128P		U(0x38)
#define TZC_REGION_SIZE_256P		U(0x39)
#define TZC_REGION_SIZE_512P		U(0x3a)
#define TZC_REGION_SIZE_1E		U(0x3b)
#define TZC_REGION_SIZE_2E		U(0x3c)
#define TZC_REGION_SIZE_4E		U(0x3d)
#define TZC_REGION_SIZE_8E		U(0x3e)
#define TZC_REGION_SIZE_16E		U(0x3f)

#define TZC_SUBREGION_DIS_SHIFT		0x8
#define TZC_SUBREGION_DIS_MASK		U(0xff)
#define TZC_ATTR_SUBREG_DIS(s)		(((s) & TZC_SUBREGION_DIS_MASK) \
						<< TZC_SUBREGION_DIS_SHIFT)

#define TZC_REGION_SIZE_SHIFT		0x1
#define TZC_REGION_SIZE_MASK		U(0x7e)
#define TZC_ATTR_REGION_SIZE(s)		((s) << TZC_REGION_SIZE_SHIFT)

#define TZC_ATTR_REGION_EN_SHIFT	0x0
#define TZC_ATTR_REGION_EN_MASK		U(0x1)

#define TZC_ATTR_REGION_EN
#define TZC_ATTR_REGION_ENABLE		U(0x1)
#define TZC_ATTR_REGION_DISABLE		U(0x0)

#define	REGION_MAX	16

void tzc380_init(uintptr_t base);
void tzc380_configure_region(uint8_t region,
			     uintptr_t region_base,
			     unsigned int attr);
void tzc380_set_action(unsigned int action);
static inline void tzc_init(uintptr_t base)
{
	tzc380_init(base);
}

static inline void tzc_configure_region(uint8_t region,
					uintptr_t region_base,
					unsigned int attr)
{
	tzc380_configure_region(region, region_base, attr);
}

static inline void tzc_set_action(unsigned int action)
{
	tzc380_set_action(action);
}

#endif /* TZC380_H */
