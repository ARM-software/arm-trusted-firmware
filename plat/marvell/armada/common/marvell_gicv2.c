/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <platform_def.h>

#include <bl31/interrupt_mgmt.h>
#include <common/debug.h>
#include <drivers/arm/gicv2.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include <plat_marvell.h>

/*
 * The following functions are defined as weak to allow a platform to override
 * the way the GICv2 driver is initialised and used.
 */
#pragma weak plat_marvell_gic_driver_init
#pragma weak plat_marvell_gic_init

#define A7K8K_PIC_CAUSE_REG		0xf03f0100
#define A7K8K_PIC0_MASK_REG		0xf03f0108

#define A7K8K_PIC_PMUOF_IRQ_MASK	(1 << 17)

#define A7K8K_PIC_MAX_IRQS		32
#define A7K8K_PIC_MAX_IRQ_MASK		((1UL << A7K8K_PIC_MAX_IRQS) - 1)

#define A7K8K_ODMIN_SET_REG		0xf0300040
#define A7K8K_ODMI_PMU_IRQ(idx)		((2 + idx) << 12)

#define A7K8K_ODMI_PMU_GIC_IRQ(idx)	(130 + idx)

static DEFINE_BAKERY_LOCK(a7k8k_irq_lock);

/*
 * On a GICv2 system, the Group 1 secure interrupts are treated as Group 0
 * interrupts.
 */
static const interrupt_prop_t marvell_interrupt_props[] = {
	PLAT_MARVELL_G1S_IRQ_PROPS(GICV2_INTR_GROUP0),
	PLAT_MARVELL_G0_IRQ_PROPS(GICV2_INTR_GROUP0)
};

static unsigned int target_mask_array[PLATFORM_CORE_COUNT];

/*
 * Ideally `marvell_gic_data` structure definition should be a `const` but it is
 * kept as modifiable for overwriting with different GICD and GICC base when
 * running on FVP with VE memory map.
 */
static gicv2_driver_data_t marvell_gic_data = {
	.gicd_base = PLAT_MARVELL_GICD_BASE,
	.gicc_base = PLAT_MARVELL_GICC_BASE,
	.interrupt_props = marvell_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(marvell_interrupt_props),
	.target_masks = target_mask_array,
	.target_masks_num = ARRAY_SIZE(target_mask_array),
};

/*
 * ARM common helper to initialize the GICv2 only driver.
 */
void plat_marvell_gic_driver_init(void)
{
	gicv2_driver_init(&marvell_gic_data);
}

static uint64_t a7k8k_pmu_interrupt_handler(uint32_t id,
					  uint32_t flags,
					  void *handle,
					  void *cookie)
{
	unsigned int idx = plat_my_core_pos();
	uint32_t irq;

	bakery_lock_get(&a7k8k_irq_lock);

	/* Acknowledge IRQ */
	irq = plat_ic_acknowledge_interrupt();

	plat_ic_end_of_interrupt(irq);

	if (irq != MARVELL_IRQ_PIC0) {
		bakery_lock_release(&a7k8k_irq_lock);
		return 0;
	}

	/* Acknowledge PMU overflow IRQ in PIC0 */
	mmio_setbits_32(A7K8K_PIC_CAUSE_REG, A7K8K_PIC_PMUOF_IRQ_MASK);

	/* Trigger ODMI Frame IRQ */
	mmio_write_32(A7K8K_ODMIN_SET_REG, A7K8K_ODMI_PMU_IRQ(idx));

	bakery_lock_release(&a7k8k_irq_lock);

	return 0;
}

void mvebu_pmu_interrupt_enable(void)
{
	unsigned int idx;
	uint32_t flags;
	int32_t rc;

	/* Reset PIC */
	mmio_write_32(A7K8K_PIC_CAUSE_REG, A7K8K_PIC_MAX_IRQ_MASK);
	/* Unmask PMU overflow IRQ in PIC0 */
	mmio_clrbits_32(A7K8K_PIC0_MASK_REG, A7K8K_PIC_PMUOF_IRQ_MASK);

	/* Configure ODMI Frame IRQs as edge triggered */
	for (idx = 0; idx < PLATFORM_CORE_COUNT; idx++)
		gicv2_interrupt_set_cfg(A7K8K_ODMI_PMU_GIC_IRQ(idx),
					GIC_INTR_CFG_EDGE);

	/*
	 * Register IRQ handler as INTR_TYPE_S_EL1 as its the only valid type
	 * for GICv2 in ARM-TF.
	 */
	flags = 0U;
	set_interrupt_rm_flag((flags), (NON_SECURE));
	rc = register_interrupt_type_handler(INTR_TYPE_S_EL1,
					     a7k8k_pmu_interrupt_handler,
					     flags);
	if (rc != 0)
		panic();
}

void mvebu_pmu_interrupt_disable(void)
{
	/* Reset PIC */
	mmio_write_32(A7K8K_PIC_CAUSE_REG, A7K8K_PIC_MAX_IRQ_MASK);
	/* Mask PMU overflow IRQ in PIC0 */
	mmio_setbits_32(A7K8K_PIC0_MASK_REG, A7K8K_PIC_PMUOF_IRQ_MASK);
}

void plat_marvell_gic_init(void)
{
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_set_pe_target_mask(plat_my_core_pos());
	gicv2_cpuif_enable();
}
