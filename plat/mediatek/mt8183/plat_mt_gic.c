/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/gicv3.h>
#include <bl31/interrupt_mgmt.h>
#include <mt_gic_v3.h>
#include <mtk_plat_common.h>
#include "plat_private.h"
#include <plat/common/platform.h>
#include <platform_def.h>
#include <stdint.h>
#include <stdio.h>

#define NR_INT_POL_CTL         20

uintptr_t rdistif_base_addrs[PLATFORM_CORE_COUNT];

/* we save and restore the GICv3 context on system suspend */
gicv3_redist_ctx_t rdist_ctx;
gicv3_dist_ctx_t dist_ctx;

static unsigned int mt_mpidr_to_core_pos(u_register_t mpidr)
{
	return plat_core_pos_by_mpidr(mpidr);
}

gicv3_driver_data_t mt_gicv3_data = {
	.gicd_base = MT_GIC_BASE,
	.gicr_base = MT_GIC_RDIST_BASE,
	.rdistif_num = PLATFORM_CORE_COUNT,
	.rdistif_base_addrs = rdistif_base_addrs,
	.mpidr_to_core_pos = mt_mpidr_to_core_pos,
};

void clear_sec_pol_ctl_en(void)
{
	unsigned int i;

	/* total 19 polarity ctrl registers */
	for (i = 0; i <= NR_INT_POL_CTL - 1; i++) {
		mmio_write_32((SEC_POL_CTL_EN0 + (i * 4)), 0);
	}
	dsb();
}

void mt_gic_driver_init(void)
{
	gicv3_driver_init(&mt_gicv3_data);
}

void mt_gic_init(void)
{
	gicv3_distif_init();
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());

	clear_sec_pol_ctl_en();
}

void mt_gic_set_pending(uint32_t irq)
{
	gicv3_set_interrupt_pending(irq, plat_my_core_pos());
}

void mt_gic_cpuif_enable(void)
{
	gicv3_cpuif_enable(plat_my_core_pos());
}

void mt_gic_cpuif_disable(void)
{
	gicv3_cpuif_disable(plat_my_core_pos());
}

void mt_gic_pcpu_init(void)
{
	gicv3_rdistif_init(plat_my_core_pos());
}

void mt_gic_irq_save(void)
{
	gicv3_rdistif_save(plat_my_core_pos(), &rdist_ctx);
	gicv3_distif_save(&dist_ctx);
}

void mt_gic_irq_restore(void)
{
	gicv3_distif_init_restore(&dist_ctx);
	gicv3_rdistif_init_restore(plat_my_core_pos(), &rdist_ctx);
}

void mt_gic_sync_dcm_enable(void)
{
	unsigned int val = mmio_read_32(GIC_SYNC_DCM);

	val &= ~GIC_SYNC_DCM_MASK;
	mmio_write_32(GIC_SYNC_DCM, val | GIC_SYNC_DCM_ON);
}

void mt_gic_sync_dcm_disable(void)
{
	unsigned int val = mmio_read_32(GIC_SYNC_DCM);

	val &= ~GIC_SYNC_DCM_MASK;
	mmio_write_32(GIC_SYNC_DCM, val | GIC_SYNC_DCM_OFF);
}
