/*
 * Copyright (c) 2020-2024, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "../drivers/arm/gic/v3/gicv3_private.h"
#include <bl31/interrupt_mgmt.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/mtk_init/mtk_init.h>
#include <mt_gic_v3.h>
#include <mtk_plat_common.h>
#include <plat/common/platform.h>
#include <plat_private.h>
#include <platform_def.h>

#define SGI_MASK 0xffff

uintptr_t rdistif_base_addrs[PLATFORM_CORE_COUNT];
static uint32_t rdist_has_saved[PLATFORM_CORE_COUNT];

/* we save and restore the GICv3 context on system suspend */
gicv3_dist_ctx_t dist_ctx;

static const interrupt_prop_t mtk_interrupt_props[] = {
	PLAT_MTK_G1S_IRQ_PROPS(INTR_GROUP1S)
};

static unsigned int mt_mpidr_to_core_pos(u_register_t mpidr)
{
	return plat_core_pos_by_mpidr(mpidr);
}

gicv3_driver_data_t mt_gicv3_data = {
	.gicd_base = MT_GIC_BASE,
	.gicr_base = MT_GIC_RDIST_BASE,
	.interrupt_props = mtk_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(mtk_interrupt_props),
	.rdistif_num = PLATFORM_CORE_COUNT,
	.rdistif_base_addrs = rdistif_base_addrs,
	.mpidr_to_core_pos = mt_mpidr_to_core_pos,
};

struct gic_chip_data {
	/* All cores share the same configuration */
	unsigned int saved_ctlr;
	unsigned int saved_group;
	unsigned int saved_enable;
	unsigned int saved_conf0;
	unsigned int saved_conf1;
	unsigned int saved_grpmod;
	unsigned int saved_ispendr;
	unsigned int saved_isactiver;
	unsigned int saved_nsacr;
	/* Per-core sgi */
	unsigned int saved_sgi[PLATFORM_CORE_COUNT];
	/* Per-core priority */
	unsigned int saved_prio[PLATFORM_CORE_COUNT][GICR_NUM_REGS(IPRIORITYR)];
};

static struct gic_chip_data gic_data;

void mt_gic_driver_init(void)
{
	gicv3_driver_init(&mt_gicv3_data);
}

void mt_gic_set_pending(uint32_t irq)
{
	gicv3_set_interrupt_pending(irq, plat_my_core_pos());
}

void mt_gic_distif_save(void)
{
	gicv3_distif_save(&dist_ctx);
}

void mt_gic_distif_restore(void)
{
	gicv3_distif_init_restore(&dist_ctx);
}

void mt_gic_rdistif_init(void)
{
	unsigned int proc_num;
	unsigned int index;
	uintptr_t gicr_base;

	proc_num = plat_my_core_pos();
	gicr_base = gicv3_driver_data->rdistif_base_addrs[proc_num];

	/* set all SGI/PPI as non-secure GROUP1 by default */
	mmio_write_32(gicr_base + GICR_IGROUPR0, ~0U);
	mmio_write_32(gicr_base + GICR_IGRPMODR0, 0x0);

	/* setup the default PPI/SGI priorities */
	for (index = 0; index < TOTAL_PCPU_INTR_NUM; index += 4U)
		gicr_write_ipriorityr(gicr_base, index,
				GICD_IPRIORITYR_DEF_VAL);
}

void mt_gic_rdistif_save(void)
{
	unsigned int i, proc_num;
	uintptr_t gicr_base;

	proc_num = plat_my_core_pos();
	gicr_base = gicv3_driver_data->rdistif_base_addrs[proc_num];

	/*
	 * Wait for any write to GICR_CTLR to complete before trying to save any
	 * state.
	 */
	gicr_wait_for_pending_write(gicr_base);

	gic_data.saved_ctlr = mmio_read_32(gicr_base + GICR_CTLR);
	gic_data.saved_group = mmio_read_32(gicr_base + GICR_IGROUPR0);
	gic_data.saved_enable = mmio_read_32(gicr_base + GICR_ISENABLER0);
	gic_data.saved_conf0 = mmio_read_32(gicr_base + GICR_ICFGR0);
	gic_data.saved_conf1 = mmio_read_32(gicr_base + GICR_ICFGR1);
	gic_data.saved_grpmod = mmio_read_32(gicr_base + GICR_IGRPMODR0);
	gic_data.saved_ispendr = mmio_read_32(gicr_base + GICR_ISPENDR0);
	gic_data.saved_isactiver = mmio_read_32(gicr_base + GICR_ISACTIVER0);
	gic_data.saved_nsacr = mmio_read_32(gicr_base + GICR_NSACR);

	for (i = 0U; i < 8U; ++i)
		gic_data.saved_prio[proc_num][i] = gicr_ipriorityr_read(gicr_base, i);

	rdist_has_saved[proc_num] = 1;
}

void mt_gic_rdistif_restore(void)
{
	unsigned int i, proc_num;
	uintptr_t gicr_base;

	proc_num = plat_my_core_pos();
	if (rdist_has_saved[proc_num] == 1) {
		gicr_base = gicv3_driver_data->rdistif_base_addrs[proc_num];

		mmio_write_32(gicr_base + GICR_IGROUPR0, gic_data.saved_group);
		mmio_write_32(gicr_base + GICR_IGRPMODR0, gic_data.saved_grpmod);
		mmio_write_32(gicr_base + GICR_NSACR, gic_data.saved_nsacr);
		mmio_write_32(gicr_base + GICR_ICFGR0, gic_data.saved_conf0);
		mmio_write_32(gicr_base + GICR_ICFGR1, gic_data.saved_conf1);

		for (i = 0U; i < 8U; ++i)
			gicr_ipriorityr_write(gicr_base, i, gic_data.saved_prio[proc_num][i]);

		mmio_write_32(gicr_base + GICR_ISPENDR0, gic_data.saved_ispendr);
		mmio_write_32(gicr_base + GICR_ISACTIVER0, gic_data.saved_isactiver);
		mmio_write_32(gicr_base + GICR_ISENABLER0, gic_data.saved_enable);
		mmio_write_32(gicr_base + GICR_CTLR, gic_data.saved_ctlr);

		gicr_wait_for_pending_write(gicr_base);
	}
}

void mt_gic_rdistif_restore_all(void)
{
	unsigned int i, proc_num;
	uintptr_t gicr_base;

	for (proc_num = 0; proc_num < PLATFORM_CORE_COUNT; proc_num++) {
		gicr_base = gicv3_driver_data->rdistif_base_addrs[proc_num];

		mmio_write_32(gicr_base + GICR_IGROUPR0, gic_data.saved_group);
		mmio_write_32(gicr_base + GICR_IGRPMODR0, gic_data.saved_grpmod);
		mmio_write_32(gicr_base + GICR_NSACR, gic_data.saved_nsacr);
		mmio_write_32(gicr_base + GICR_ICFGR0, gic_data.saved_conf0);
		mmio_write_32(gicr_base + GICR_ICFGR1, gic_data.saved_conf1);

		for (i = 0U; i < 8U; ++i)
			gicr_ipriorityr_write(gicr_base, i, gic_data.saved_prio[proc_num][i]);

		mmio_write_32(gicr_base + GICR_ISPENDR0, gic_data.saved_ispendr);
		mmio_write_32(gicr_base + GICR_ISACTIVER0, gic_data.saved_isactiver);
		mmio_write_32(gicr_base + GICR_ISENABLER0, gic_data.saved_enable);
		mmio_write_32(gicr_base + GICR_CTLR, gic_data.saved_ctlr);

		gicr_wait_for_pending_write(gicr_base);
	}
}

void gic_sgi_save_all(void)
{
	unsigned int proc_num;
	uintptr_t gicr_base;

	for (proc_num = 0; proc_num < PLATFORM_CORE_COUNT; proc_num++) {
		gicr_base = gicv3_driver_data->rdistif_base_addrs[proc_num];
		gic_data.saved_sgi[proc_num] =
			mmio_read_32(gicr_base + GICR_ISPENDR0) & SGI_MASK;
	}
}

void gic_sgi_restore_all(void)
{
	unsigned int proc_num;
	uintptr_t gicr_base;

	for (proc_num = 0; proc_num < PLATFORM_CORE_COUNT; proc_num++) {
		gicr_base = gicv3_driver_data->rdistif_base_addrs[proc_num];
		mmio_write_32(gicr_base + GICR_ICPENDR0, SGI_MASK);
		mmio_write_32(gicr_base + GICR_ISPENDR0,
			gic_data.saved_sgi[proc_num] & SGI_MASK);
	}
}

void mt_gic_init(void)
{
	gicv3_distif_init();
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

uint32_t mt_irq_get_pending(uint32_t irq)
{
	uint32_t val;

	val = mmio_read_32(BASE_GICD_BASE + GICD_ISPENDR +
		irq / 32 * 4);
	val = (val >> (irq % 32)) & 1U;
	return val;
}


void mt_irq_set_pending(uint32_t irq)
{
	uint32_t bit = 1U << (irq % 32);

	mmio_write_32(BASE_GICD_BASE + GICD_ISPENDR +
		irq / 32 * 4, bit);
}

int mt_gic_one_init(void)
{
	INFO("[%s] GIC initialization\n", __func__);

	/* Initialize the GIC driver, CPU and distributor interfaces */
	mt_gic_driver_init();
	mt_gic_init();

	return 0;
}
MTK_PLAT_SETUP_0_INIT(mt_gic_one_init);
