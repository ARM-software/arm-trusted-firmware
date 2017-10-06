/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Driver for GIC600-specific features. This driver only overrides APIs that are
 * different to those generic ones in GICv3 driver.
 *
 * GIC600 supports independently power-gating redistributor interface.
 */

#include <arch_helpers.h>
#include <assert.h>
#include <gicv3.h>

#include "gicv3_private.h"

/* GIC600-specific register offsets */
#define GICR_PWRR	0x24

/* GICR_PWRR fields */
#define PWRR_RDPD_SHIFT		0
#define PWRR_RDGPD_SHIFT	2
#define PWRR_RDGPO_SHIFT	3

#define PWRR_RDGPD	(1 << PWRR_RDGPD_SHIFT)
#define PWRR_RDGPO	(1 << PWRR_RDGPO_SHIFT)

/* Values to write to GICR_PWRR register to power redistributor */
#define PWRR_ON		(0 << PWRR_RDPD_SHIFT)
#define PWRR_OFF	(1 << PWRR_RDPD_SHIFT)

/* GIC600-specific accessor functions */
static void gicr_write_pwrr(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICR_PWRR, val);
}

static uint32_t gicr_read_pwrr(uintptr_t base)
{
	return mmio_read_32(base + GICR_PWRR);
}

static int gicr_group_powering_down(uint32_t pwrr)
{
	/*
	 * Whether the redistributor group power down operation is in transit:
	 * i.e. it's intending to, but not finished yet.
	 */
	return ((pwrr & PWRR_RDGPD) && !(pwrr & PWRR_RDGPO));
}

static void gic600_pwr_on(uintptr_t base)
{
	/* Power on redistributor */
	gicr_write_pwrr(base, PWRR_ON);

	/* Wait until the power on state is reflected */
	while (gicr_read_pwrr(base) & PWRR_RDGPO)
		;
}

static void gic600_pwr_off(uintptr_t base)
{
	/* Power off redistributor */
	gicr_write_pwrr(base, PWRR_OFF);

	/*
	 * If this is the last man, turning this redistributor frame off will
	 * result in the group itself being powered off. In that case, wait as
	 * long as it's in transition, or has aborted the transition altogether
	 * for any reason.
	 */
	if (gicr_read_pwrr(base) & PWRR_RDGPD) {
		while (gicr_group_powering_down(gicr_read_pwrr(base)))
			;
	}
}

void gicv3_distif_pre_save(unsigned int proc_num)
{
	arm_gicv3_distif_pre_save(proc_num);
}

void gicv3_distif_post_restore(unsigned int proc_num)
{
	arm_gicv3_distif_post_restore(proc_num);
}

/*
 * Power off GIC600 redistributor
 */
void gicv3_rdistif_off(unsigned int proc_num)
{
	uintptr_t gicr_base;

	assert(gicv3_driver_data);
	assert(proc_num < gicv3_driver_data->rdistif_num);
	assert(gicv3_driver_data->rdistif_base_addrs);

	gicr_base = gicv3_driver_data->rdistif_base_addrs[proc_num];
	assert(gicr_base);

	/* Attempt to power redistributor off */
	gic600_pwr_off(gicr_base);
}

/*
 * Power on GIC600 redistributor
 */
void gicv3_rdistif_on(unsigned int proc_num)
{
	uintptr_t gicr_base;

	assert(gicv3_driver_data);
	assert(proc_num < gicv3_driver_data->rdistif_num);
	assert(gicv3_driver_data->rdistif_base_addrs);

	gicr_base = gicv3_driver_data->rdistif_base_addrs[proc_num];
	assert(gicr_base);

	/* Power redistributor on */
	gic600_pwr_on(gicr_base);
}
