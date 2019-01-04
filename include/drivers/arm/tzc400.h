/*
 * Copyright (c) 2014-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TZC400_H
#define TZC400_H

#include <drivers/arm/tzc_common.h>
#include <lib/utils_def.h>

#define BUILD_CONFIG_OFF			U(0x000)
#define GATE_KEEPER_OFF				U(0x008)
#define SPECULATION_CTRL_OFF			U(0x00c)
#define INT_STATUS				U(0x010)
#define INT_CLEAR				U(0x014)

#define FAIL_ADDRESS_LOW_OFF			U(0x020)
#define FAIL_ADDRESS_HIGH_OFF			U(0x024)
#define FAIL_CONTROL_OFF			U(0x028)
#define FAIL_ID					U(0x02c)

/* ID registers not common across different varieties of TZC */
#define PID5					U(0xFD4)
#define PID6					U(0xFD8)
#define PID7					U(0xFDC)

#define BUILD_CONFIG_NF_SHIFT			24
#define BUILD_CONFIG_NF_MASK			U(0x3)
#define BUILD_CONFIG_AW_SHIFT			8
#define BUILD_CONFIG_AW_MASK			U(0x3f)
#define BUILD_CONFIG_NR_SHIFT			0
#define BUILD_CONFIG_NR_MASK			U(0x1f)

/*
 * Number of gate keepers is implementation defined. But we know the max for
 * this device is 4. Get implementation details from BUILD_CONFIG.
 */
#define GATE_KEEPER_OS_SHIFT			16
#define GATE_KEEPER_OS_MASK			U(0xf)
#define GATE_KEEPER_OR_SHIFT			0
#define GATE_KEEPER_OR_MASK			U(0xf)
#define GATE_KEEPER_FILTER_MASK			U(0x1)

/* Speculation is enabled by default. */
#define SPECULATION_CTRL_WRITE_DISABLE		BIT_32(1)
#define SPECULATION_CTRL_READ_DISABLE		BIT_32(0)

/* Max number of filters allowed is 4. */
#define INT_STATUS_OVERLAP_SHIFT		16
#define INT_STATUS_OVERLAP_MASK			U(0xf)
#define INT_STATUS_OVERRUN_SHIFT		8
#define INT_STATUS_OVERRUN_MASK			U(0xf)
#define INT_STATUS_STATUS_SHIFT			0
#define INT_STATUS_STATUS_MASK			U(0xf)

#define INT_CLEAR_CLEAR_SHIFT			0
#define INT_CLEAR_CLEAR_MASK			U(0xf)

#define FAIL_CONTROL_DIR_SHIFT			24
#define FAIL_CONTROL_DIR_READ			U(0)
#define FAIL_CONTROL_DIR_WRITE			U(1)
#define FAIL_CONTROL_NS_SHIFT			21
#define FAIL_CONTROL_NS_SECURE			U(0)
#define FAIL_CONTROL_NS_NONSECURE		U(1)
#define FAIL_CONTROL_PRIV_SHIFT			20
#define FAIL_CONTROL_PRIV_PRIV			U(0)
#define FAIL_CONTROL_PRIV_UNPRIV		U(1)

/*
 * FAIL_ID_ID_MASK depends on AID_WIDTH which is platform specific.
 * Platform should provide the value on initialisation.
 */
#define FAIL_ID_VNET_SHIFT			24
#define FAIL_ID_VNET_MASK			U(0xf)
#define FAIL_ID_ID_SHIFT			0

#define TZC_400_PERIPHERAL_ID			U(0x460)

/* Filter enable bits in a TZC */
#define TZC_400_REGION_ATTR_F_EN_MASK		U(0xf)
#define TZC_400_REGION_ATTR_FILTER_BIT(x)				\
				((U(1) << (x)) << TZC_REGION_ATTR_F_EN_SHIFT)
#define TZC_400_REGION_ATTR_FILTER_BIT_ALL				\
				(TZC_400_REGION_ATTR_F_EN_MASK <<	\
				TZC_REGION_ATTR_F_EN_SHIFT)

/*
 * All TZC region configuration registers are placed one after another. It
 * depicts size of block of registers for programming each region.
 */
#define TZC_400_REGION_SIZE			U(0x20)
#define TZC_400_ACTION_OFF			U(0x4)

#ifndef __ASSEMBLY__

#include <cdefs.h>
#include <stdint.h>

/*******************************************************************************
 * Function & variable prototypes
 ******************************************************************************/
void tzc400_init(uintptr_t base);
void tzc400_configure_region0(unsigned int sec_attr,
			   unsigned int ns_device_access);
void tzc400_configure_region(unsigned int filters,
			  unsigned int region,
			  unsigned long long region_base,
			  unsigned long long region_top,
			  unsigned int sec_attr,
			  unsigned int nsaid_permissions);
void tzc400_set_action(unsigned int action);
void tzc400_enable_filters(void);
void tzc400_disable_filters(void);

static inline void tzc_init(uintptr_t base)
{
	tzc400_init(base);
}

static inline void tzc_configure_region0(
			unsigned int sec_attr,
			unsigned int ns_device_access)
{
	tzc400_configure_region0(sec_attr, ns_device_access);
}

static inline void tzc_configure_region(
			  unsigned int filters,
			  unsigned int region,
			  unsigned long long region_base,
			  unsigned long long region_top,
			  unsigned int sec_attr,
			  unsigned int ns_device_access)
{
	tzc400_configure_region(filters, region, region_base,
			region_top, sec_attr, ns_device_access);
}

static inline void tzc_set_action(unsigned int action)
{
	tzc400_set_action(action);
}


static inline void tzc_enable_filters(void)
{
	tzc400_enable_filters();
}

static inline void tzc_disable_filters(void)
{
	tzc400_disable_filters();
}

#endif /* __ASSEMBLY__ */

#endif /* TZC400_H */
