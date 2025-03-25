/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include <bl31/interrupt_mgmt.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <gicv3_private.h>
#include <plat/common/common_def.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#include <lib/mtk_init/mtk_init.h>
#include <mt_gic_v3.h>
#include <mtk_plat_common.h>
#include <plat_private.h>

uintptr_t rdistif_base_addrs[PLATFORM_CORE_COUNT];

static gicv3_redist_ctx_t rdist_ctx[PLATFORM_CORE_COUNT];
static gicv3_dist_ctx_t dist_ctx;

/* Configure Secure IRQs */
static const interrupt_prop_t mt_interrupt_props[] = {
	PLATFORM_G1S_PROPS(INTR_GROUP1S)
};

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
	.interrupt_props = mt_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(mt_interrupt_props),
};

int32_t mt_irq_get_pending(uint32_t irq)
{
	uint32_t val;

	if (!IS_SPI(irq))
		return -EINVAL;

	val = mmio_read_32(BASE_GICD_BASE + GICD_ISPENDR +
		irq / 32 * 4);
	val = (val >> (irq % 32)) & 1U;

	return (int32_t)val;
}

int32_t mt_irq_set_pending(uint32_t irq)
{
	uint32_t bit = 1U << (irq % 32);

	if (!IS_SPI(irq))
		return -EINVAL;

	mmio_write_32(BASE_GICD_BASE + GICD_ISPENDR +
		irq / 32 * 4, bit);

	return 0;
}

uint32_t gicr_get_sgi_pending(void)
{
	uint32_t result = 0, ispendr0, proc_num;

	for (proc_num = 0; proc_num < PLATFORM_CORE_COUNT; proc_num++) {
		ispendr0 =
			gicr_read_ispendr0(MT_GIC_RDIST_BASE + proc_num * SZ_64K * 2);
		result |= ((ispendr0 & SGI_MASK) ? 1 : 0) << proc_num;
	}

	return result;
}

void mt_gic_pcpu_init(void)
{
	gicv3_rdistif_init(plat_my_core_pos());
}

void mt_gic_distif_save(void)
{
	/* Save the GIC Distributor context */
	gicv3_distif_save(&dist_ctx);
}

void mt_gic_distif_restore(void)
{
	/* Restore the GIC Distributor context */
	gicv3_distif_init_restore(&dist_ctx);
}

void mt_gic_rdistif_save(void)
{
	uint32_t cpu;

	/*
	 * For all cores, save the GIC Redistributors and ITS contexts
	 * before the Distributor context.
	 */
	for (cpu = 0; cpu < PLATFORM_CORE_COUNT; cpu++)
		gicv3_rdistif_save(cpu, &rdist_ctx[cpu]);
}

void mt_gic_rdistif_restore(void)
{
	uint32_t cpu;

	/*
	 * Restore the GIC Redistributor and ITS contexts after the
	 * Distributor context. As we only handle SYSTEM SUSPEND API,
	 * we only need to restore the context of the CPU that issued
	 * the SYSTEM SUSPEND call.
	 */
	for (cpu = 0; cpu < PLATFORM_CORE_COUNT; cpu++)
		gicv3_rdistif_init_restore(cpu, &rdist_ctx[cpu]);
}

void mt_gic_redistif_on(void)
{
	gicv3_rdistif_on(plat_my_core_pos());
}

void mt_gic_redistif_off(void)
{
	gicv3_rdistif_off(plat_my_core_pos());
}

void mt_gic_redistif_init(void)
{
	gicv3_rdistif_init(plat_my_core_pos());
}

void mt_gic_cpuif_enable(void)
{
	gicv3_cpuif_enable(plat_my_core_pos());
}

void mt_gic_cpuif_disable(void)
{
	gicv3_cpuif_disable(plat_my_core_pos());
}
void mt_gic_driver_init(void)
{
	/*
	 * The GICv3 driver is initialized in EL3 and does not need
	 * to be initialized again in SEL1. This is because the S-EL1
	 * can use GIC system registers to manage interrupts and does
	 * not need GIC interface base addresses to be configured.
	 */
#if (!defined(__aarch64__) && defined(IMAGE_BL32)) || \
	(defined(__aarch64__) && defined(IMAGE_BL31))
	gicv3_driver_init(&mt_gicv3_data);
#endif
}

void mt_gic_init(void)
{
	gicv3_distif_init();
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

int mt_gic_one_init(void)
{
	INFO("[%s] GIC initialization\n", __func__);

	mt_gic_driver_init();
	mt_gic_init();

	return 0;
}

MTK_PLAT_SETUP_0_INIT(mt_gic_one_init);
