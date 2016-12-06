/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
#define GICR_PWRR		0x24

/* GICR_PWRR fields */
#define PWRR_RDPD_SHIFT		0
#define PWRR_RDGPD_SHIFT	2
#define PWRR_RDGPO_SHIFT	3

#define PWRR_RDGPD		(1 << PWRR_RDGPD_SHIFT)
#define PWRR_RDGPO		(1 << PWRR_RDGPO_SHIFT)

/* Values to write to GICR_PWRR register to power redistributor */
#define PWRR_ON			(0 << PWRR_RDPD_SHIFT)
#define PWRR_OFF		(1 << PWRR_RDPD_SHIFT)

/* Generic GICv3 resources */
extern const gicv3_driver_data_t *gicv3_driver_data;

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
