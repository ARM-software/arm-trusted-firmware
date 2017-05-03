/*
 * Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __TZC400_H__
#define __TZC400_H__

#include <tzc_common.h>

#define BUILD_CONFIG_OFF			0x000
#define GATE_KEEPER_OFF				0x008
#define SPECULATION_CTRL_OFF			0x00c
#define INT_STATUS				0x010
#define INT_CLEAR				0x014

#define FAIL_ADDRESS_LOW_OFF			0x020
#define FAIL_ADDRESS_HIGH_OFF			0x024
#define FAIL_CONTROL_OFF			0x028
#define FAIL_ID					0x02c

/* ID registers not common across different varieties of TZC */
#define PID5					0xFD4
#define PID6					0xFD8
#define PID7					0xFDC

#define BUILD_CONFIG_NF_SHIFT			24
#define BUILD_CONFIG_NF_MASK			0x3
#define BUILD_CONFIG_AW_SHIFT			8
#define BUILD_CONFIG_AW_MASK			0x3f
#define BUILD_CONFIG_NR_SHIFT			0
#define BUILD_CONFIG_NR_MASK			0x1f

/*
 * Number of gate keepers is implementation defined. But we know the max for
 * this device is 4. Get implementation details from BUILD_CONFIG.
 */
#define GATE_KEEPER_OS_SHIFT			16
#define GATE_KEEPER_OS_MASK			0xf
#define GATE_KEEPER_OR_SHIFT			0
#define GATE_KEEPER_OR_MASK			0xf
#define GATE_KEEPER_FILTER_MASK			0x1

/* Speculation is enabled by default. */
#define SPECULATION_CTRL_WRITE_DISABLE		(1 << 1)
#define SPECULATION_CTRL_READ_DISABLE		(1 << 0)

/* Max number of filters allowed is 4. */
#define INT_STATUS_OVERLAP_SHIFT		16
#define INT_STATUS_OVERLAP_MASK			0xf
#define INT_STATUS_OVERRUN_SHIFT		8
#define INT_STATUS_OVERRUN_MASK			0xf
#define INT_STATUS_STATUS_SHIFT			0
#define INT_STATUS_STATUS_MASK			0xf

#define INT_CLEAR_CLEAR_SHIFT			0
#define INT_CLEAR_CLEAR_MASK			0xf

#define FAIL_CONTROL_DIR_SHIFT			(1 << 24)
#define FAIL_CONTROL_DIR_READ			0x0
#define FAIL_CONTROL_DIR_WRITE			0x1
#define FAIL_CONTROL_NS_SHIFT			(1 << 21)
#define FAIL_CONTROL_NS_SECURE			0x0
#define FAIL_CONTROL_NS_NONSECURE		0x1
#define FAIL_CONTROL_PRIV_SHIFT			(1 << 20)
#define FAIL_CONTROL_PRIV_PRIV			0x0
#define FAIL_CONTROL_PRIV_UNPRIV		0x1

/*
 * FAIL_ID_ID_MASK depends on AID_WIDTH which is platform specific.
 * Platform should provide the value on initialisation.
 */
#define FAIL_ID_VNET_SHIFT			24
#define FAIL_ID_VNET_MASK			0xf
#define FAIL_ID_ID_SHIFT			0

#define TZC_400_PERIPHERAL_ID			0x460

/* Filter enable bits in a TZC */
#define TZC_400_REGION_ATTR_F_EN_MASK		0xf
#define TZC_400_REGION_ATTR_FILTER_BIT(x)	((1 << x)		\
					<< TZC_REGION_ATTR_F_EN_SHIFT)
#define TZC_400_REGION_ATTR_FILTER_BIT_ALL				\
				(TZC_400_REGION_ATTR_F_EN_MASK <<	\
				TZC_REGION_ATTR_F_EN_SHIFT)

/*
 * Define some macros for backward compatibility with existing tzc400 clients.
 */
#if !ERROR_DEPRECATED
#define REG_ATTR_FILTER_BIT(x)			((1 << x)		\
					<< TZC_REGION_ATTR_F_EN_SHIFT)
#define REG_ATTR_FILTER_BIT_ALL	(TZC_400_REGION_ATTR_F_EN_MASK <<	\
					TZC_REGION_ATTR_F_EN_SHIFT)
#endif /* __ERROR_DEPRECATED__ */

/*
 * All TZC region configuration registers are placed one after another. It
 * depicts size of block of registers for programming each region.
 */
#define TZC_400_REGION_SIZE			0x20
#define TZC_400_ACTION_OFF			0x4

#ifndef __ASSEMBLY__

#include <cdefs.h>
#include <stdint.h>

/*******************************************************************************
 * Function & variable prototypes
 ******************************************************************************/
void tzc400_init(uintptr_t base);
void tzc400_configure_region0(tzc_region_attributes_t sec_attr,
			   unsigned int ns_device_access);
void tzc400_configure_region(unsigned int filters,
			  int region,
			  unsigned long long region_base,
			  unsigned long long region_top,
			  tzc_region_attributes_t sec_attr,
			  unsigned int ns_device_access);
void tzc400_set_action(tzc_action_t action);
void tzc400_enable_filters(void);
void tzc400_disable_filters(void);

/*
 * Deprecated APIs
 */
static inline void tzc_init(uintptr_t base) __deprecated;
static inline void tzc_configure_region0(
			tzc_region_attributes_t sec_attr,
			unsigned int ns_device_access) __deprecated;
static inline void tzc_configure_region(
			  unsigned int filters,
			  int region,
			  unsigned long long region_base,
			  unsigned long long region_top,
			  tzc_region_attributes_t sec_attr,
			  unsigned int ns_device_access) __deprecated;
static inline void tzc_set_action(tzc_action_t action) __deprecated;
static inline void tzc_enable_filters(void) __deprecated;
static inline void tzc_disable_filters(void) __deprecated;

static inline void tzc_init(uintptr_t base)
{
	tzc400_init(base);
}

static inline void tzc_configure_region0(
			tzc_region_attributes_t sec_attr,
			unsigned int ns_device_access)
{
	tzc400_configure_region0(sec_attr, ns_device_access);
}

static inline void tzc_configure_region(
			  unsigned int filters,
			  int region,
			  unsigned long long region_base,
			  unsigned long long region_top,
			  tzc_region_attributes_t sec_attr,
			  unsigned int ns_device_access)
{
	tzc400_configure_region(filters, region, region_base,
			region_top, sec_attr, ns_device_access);
}

static inline void tzc_set_action(tzc_action_t action)
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

#endif /* __TZC400__ */
