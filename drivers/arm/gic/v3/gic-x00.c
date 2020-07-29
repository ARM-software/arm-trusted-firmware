/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Driver for GIC-500 and GIC-600 specific features. This driver only
 * overrides APIs that are different to those generic ones in GICv3
 * driver.
 *
 * GIC-600 supports independently power-gating redistributor interface.
 */

#include <assert.h>

#include <arch_helpers.h>
#include <drivers/arm/gicv3.h>

#include "gicv3_private.h"

/* GIC-600 specific register offsets */
#define GICR_PWRR			0x24U
#define IIDR_MODEL_ARM_GIC_600		U(0x0200043b)
#define IIDR_MODEL_ARM_GIC_600AE	U(0x0300043b)
#define IIDR_MODEL_ARM_GIC_CLAYTON	U(0x0400043b)

/* GICR_PWRR fields */
#define PWRR_RDPD_SHIFT			0
#define PWRR_RDAG_SHIFT			1
#define PWRR_RDGPD_SHIFT		2
#define PWRR_RDGPO_SHIFT		3

#define PWRR_RDPD			(1U << PWRR_RDPD_SHIFT)
#define PWRR_RDAG			(1U << PWRR_RDAG_SHIFT)
#define PWRR_RDGPD			(1U << PWRR_RDGPD_SHIFT)
#define PWRR_RDGPO			(1U << PWRR_RDGPO_SHIFT)

/*
 * Values to write to GICR_PWRR register to power redistributor
 * for operating through the core (GICR_PWRR.RDAG = 0)
 */
#define PWRR_ON				(0U << PWRR_RDPD_SHIFT)
#define PWRR_OFF			(1U << PWRR_RDPD_SHIFT)

#if GICV3_SUPPORT_GIC600

/* GIC-600/Clayton specific accessor functions */
static void gicr_write_pwrr(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICR_PWRR, val);
}

static uint32_t gicr_read_pwrr(uintptr_t base)
{
	return mmio_read_32(base + GICR_PWRR);
}

static void gicr_wait_group_not_in_transit(uintptr_t base)
{
	uint32_t pwrr;

	do {
		pwrr = gicr_read_pwrr(base);

	/* Check group not transitioning: RDGPD == RDGPO */
	} while (((pwrr & PWRR_RDGPD) >> PWRR_RDGPD_SHIFT) !=
		 ((pwrr & PWRR_RDGPO) >> PWRR_RDGPO_SHIFT));
}

static void gic600_pwr_on(uintptr_t base)
{
	do {	/* Wait until group not transitioning */
		gicr_wait_group_not_in_transit(base);

		/* Power on redistributor */
		gicr_write_pwrr(base, PWRR_ON);

		/*
		 * Wait until the power on state is reflected.
		 * If RDPD == 0 then powered on.
		 */
	} while ((gicr_read_pwrr(base) & PWRR_RDPD) != PWRR_ON);
}

static void gic600_pwr_off(uintptr_t base)
{
	/* Wait until group not transitioning */
	gicr_wait_group_not_in_transit(base);

	/* Power off redistributor */
	gicr_write_pwrr(base, PWRR_OFF);

	/*
	 * If this is the last man, turning this redistributor frame off will
	 * result in the group itself being powered off and RDGPD = 1.
	 * In that case, wait as long as it's in transition, or has aborted
	 * the transition altogether for any reason.
	 */
	if ((gicr_read_pwrr(base) & PWRR_RDGPD) != 0U) {
		/* Wait until group not transitioning */
		gicr_wait_group_not_in_transit(base);
	}
}

static uintptr_t get_gicr_base(unsigned int proc_num)
{
	uintptr_t gicr_base;

	assert(gicv3_driver_data != NULL);
	assert(proc_num < gicv3_driver_data->rdistif_num);
	assert(gicv3_driver_data->rdistif_base_addrs != NULL);

	gicr_base = gicv3_driver_data->rdistif_base_addrs[proc_num];
	assert(gicr_base != 0UL);

	return gicr_base;
}

static bool gicv3_redists_need_power_mgmt(uintptr_t gicr_base)
{
	uint32_t reg = mmio_read_32(gicr_base + GICR_IIDR);

	/*
	 * The Arm GIC-600 and GIC-Clayton models have their redistributors
	 * powered down at reset.
	 */
	return (((reg & IIDR_MODEL_MASK) == IIDR_MODEL_ARM_GIC_600) ||
		((reg & IIDR_MODEL_MASK) == IIDR_MODEL_ARM_GIC_600AE) ||
		((reg & IIDR_MODEL_MASK) == IIDR_MODEL_ARM_GIC_CLAYTON));
}

#endif	/* GICV3_SUPPORT_GIC600 */

void gicv3_distif_pre_save(unsigned int proc_num)
{
	arm_gicv3_distif_pre_save(proc_num);
}

void gicv3_distif_post_restore(unsigned int proc_num)
{
	arm_gicv3_distif_post_restore(proc_num);
}

/*
 * Power off GIC-600 redistributor (if configured and detected)
 */
void gicv3_rdistif_off(unsigned int proc_num)
{
#if GICV3_SUPPORT_GIC600
	uintptr_t gicr_base = get_gicr_base(proc_num);

	/* Attempt to power redistributor off */
	if (gicv3_redists_need_power_mgmt(gicr_base)) {
		gic600_pwr_off(gicr_base);
	}
#endif
}

/*
 * Power on GIC-600 redistributor (if configured and detected)
 */
void gicv3_rdistif_on(unsigned int proc_num)
{
#if GICV3_SUPPORT_GIC600
	uintptr_t gicr_base = get_gicr_base(proc_num);

	/* Power redistributor on */
	if (gicv3_redists_need_power_mgmt(gicr_base)) {
		gic600_pwr_on(gicr_base);
	}
#endif
}
