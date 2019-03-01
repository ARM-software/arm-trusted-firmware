/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <common/interrupt_props.h>
#include <drivers/arm/gic_common.h>
#include <drivers/arm/gicv2.h>
#include <lib/spinlock.h>

#include "../common/gic_common_private.h"
#include "gicv2_private.h"

static const gicv2_driver_data_t *driver_data;

/*
 * Spinlock to guard registers needing read-modify-write. APIs protected by this
 * spinlock are used either at boot time (when only a single CPU is active), or
 * when the system is fully coherent.
 */
static spinlock_t gic_lock;

/*******************************************************************************
 * Enable secure interrupts and use FIQs to route them. Disable legacy bypass
 * and set the priority mask register to allow all interrupts to trickle in.
 ******************************************************************************/
void gicv2_cpuif_enable(void)
{
	unsigned int val;

	assert(driver_data != NULL);
	assert(driver_data->gicc_base != 0U);

	/*
	 * Enable the Group 0 interrupts, FIQEn and disable Group 0/1
	 * bypass.
	 */
	val = CTLR_ENABLE_G0_BIT | FIQ_EN_BIT | FIQ_BYP_DIS_GRP0;
	val |= IRQ_BYP_DIS_GRP0 | FIQ_BYP_DIS_GRP1 | IRQ_BYP_DIS_GRP1;

	/* Program the idle priority in the PMR */
	gicc_write_pmr(driver_data->gicc_base, GIC_PRI_MASK);
	gicc_write_ctlr(driver_data->gicc_base, val);
}

/*******************************************************************************
 * Place the cpu interface in a state where it can never make a cpu exit wfi as
 * as result of an asserted interrupt. This is critical for powering down a cpu
 ******************************************************************************/
void gicv2_cpuif_disable(void)
{
	unsigned int val;

	assert(driver_data != NULL);
	assert(driver_data->gicc_base != 0U);

	/* Disable secure, non-secure interrupts and disable their bypass */
	val = gicc_read_ctlr(driver_data->gicc_base);
	val &= ~(CTLR_ENABLE_G0_BIT | CTLR_ENABLE_G1_BIT);
	val |= FIQ_BYP_DIS_GRP1 | FIQ_BYP_DIS_GRP0;
	val |= IRQ_BYP_DIS_GRP0 | IRQ_BYP_DIS_GRP1;
	gicc_write_ctlr(driver_data->gicc_base, val);
}

/*******************************************************************************
 * Per cpu gic distributor setup which will be done by all cpus after a cold
 * boot/hotplug. This marks out the secure SPIs and PPIs & enables them.
 ******************************************************************************/
void gicv2_pcpu_distif_init(void)
{
	unsigned int ctlr;

	assert(driver_data != NULL);
	assert(driver_data->gicd_base != 0U);

	gicv2_secure_ppi_sgi_setup_props(driver_data->gicd_base,
			driver_data->interrupt_props,
			driver_data->interrupt_props_num);

	/* Enable G0 interrupts if not already */
	ctlr = gicd_read_ctlr(driver_data->gicd_base);
	if ((ctlr & CTLR_ENABLE_G0_BIT) == 0U) {
		gicd_write_ctlr(driver_data->gicd_base,
				ctlr | CTLR_ENABLE_G0_BIT);
	}
}

/*******************************************************************************
 * Global gic distributor init which will be done by the primary cpu after a
 * cold boot. It marks out the secure SPIs, PPIs & SGIs and enables them. It
 * then enables the secure GIC distributor interface.
 ******************************************************************************/
void gicv2_distif_init(void)
{
	unsigned int ctlr;

	assert(driver_data != NULL);
	assert(driver_data->gicd_base != 0U);

	/* Disable the distributor before going further */
	ctlr = gicd_read_ctlr(driver_data->gicd_base);
	gicd_write_ctlr(driver_data->gicd_base,
			ctlr & ~(CTLR_ENABLE_G0_BIT | CTLR_ENABLE_G1_BIT));

	/* Set the default attribute of all SPIs */
	gicv2_spis_configure_defaults(driver_data->gicd_base);

	gicv2_secure_spis_configure_props(driver_data->gicd_base,
			driver_data->interrupt_props,
			driver_data->interrupt_props_num);


	/* Re-enable the secure SPIs now that they have been configured */
	gicd_write_ctlr(driver_data->gicd_base, ctlr | CTLR_ENABLE_G0_BIT);
}

/*******************************************************************************
 * Initialize the ARM GICv2 driver with the provided platform inputs
 ******************************************************************************/
void gicv2_driver_init(const gicv2_driver_data_t *plat_driver_data)
{
	unsigned int gic_version;

	assert(plat_driver_data != NULL);
	assert(plat_driver_data->gicd_base != 0U);
	assert(plat_driver_data->gicc_base != 0U);

	assert(plat_driver_data->interrupt_props_num > 0 ?
			plat_driver_data->interrupt_props != NULL : 1);

	/* Ensure that this is a GICv2 system */
	gic_version = gicd_read_pidr2(plat_driver_data->gicd_base);
	gic_version = (gic_version >> PIDR2_ARCH_REV_SHIFT)
					& PIDR2_ARCH_REV_MASK;

	/*
	 * GICv1 with security extension complies with trusted firmware
	 * GICv2 driver as far as virtualization and few tricky power
	 * features are not used. GICv2 features that are not supported
	 * by GICv1 with Security Extensions are:
	 * - virtual interrupt support.
	 * - wake up events.
	 * - writeable GIC state register (for power sequences)
	 * - interrupt priority drop.
	 * - interrupt signal bypass.
	 */
	assert((gic_version == ARCH_REV_GICV2) ||
	       (gic_version == ARCH_REV_GICV1));

	driver_data = plat_driver_data;

	/*
	 * The GIC driver data is initialized by the primary CPU with caches
	 * enabled. When the secondary CPU boots up, it initializes the
	 * GICC/GICR interface with the caches disabled. Hence flush the
	 * driver_data to ensure coherency. This is not required if the
	 * platform has HW_ASSISTED_COHERENCY or WARMBOOT_ENABLE_DCACHE_EARLY
	 * enabled.
	 */
#if !(HW_ASSISTED_COHERENCY || WARMBOOT_ENABLE_DCACHE_EARLY)
	flush_dcache_range((uintptr_t) &driver_data, sizeof(driver_data));
	flush_dcache_range((uintptr_t) driver_data, sizeof(*driver_data));
#endif
	INFO("ARM GICv2 driver initialized\n");
}

/******************************************************************************
 * This function returns whether FIQ is enabled in the GIC CPU interface.
 *****************************************************************************/
unsigned int gicv2_is_fiq_enabled(void)
{
	unsigned int gicc_ctlr;

	assert(driver_data != NULL);
	assert(driver_data->gicc_base != 0U);

	gicc_ctlr = gicc_read_ctlr(driver_data->gicc_base);
	return (gicc_ctlr >> FIQ_EN_SHIFT) & 0x1U;
}

/*******************************************************************************
 * This function returns the type of the highest priority pending interrupt at
 * the GIC cpu interface. The return values can be one of the following :
 *   PENDING_G1_INTID   : The interrupt type is non secure Group 1.
 *   0 - 1019           : The interrupt type is secure Group 0.
 *   GIC_SPURIOUS_INTERRUPT : there is no pending interrupt with
 *                            sufficient priority to be signaled
 ******************************************************************************/
unsigned int gicv2_get_pending_interrupt_type(void)
{
	assert(driver_data != NULL);
	assert(driver_data->gicc_base != 0U);

	return gicc_read_hppir(driver_data->gicc_base) & INT_ID_MASK;
}

/*******************************************************************************
 * This function returns the id of the highest priority pending interrupt at
 * the GIC cpu interface. GIC_SPURIOUS_INTERRUPT is returned when there is no
 * interrupt pending.
 ******************************************************************************/
unsigned int gicv2_get_pending_interrupt_id(void)
{
	unsigned int id;

	assert(driver_data != NULL);
	assert(driver_data->gicc_base != 0U);

	id = gicc_read_hppir(driver_data->gicc_base) & INT_ID_MASK;

	/*
	 * Find out which non-secure interrupt it is under the assumption that
	 * the GICC_CTLR.AckCtl bit is 0.
	 */
	if (id == PENDING_G1_INTID)
		id = gicc_read_ahppir(driver_data->gicc_base) & INT_ID_MASK;

	return id;
}

/*******************************************************************************
 * This functions reads the GIC cpu interface Interrupt Acknowledge register
 * to start handling the pending secure 0 interrupt. It returns the
 * contents of the IAR.
 ******************************************************************************/
unsigned int gicv2_acknowledge_interrupt(void)
{
	assert(driver_data != NULL);
	assert(driver_data->gicc_base != 0U);

	return gicc_read_IAR(driver_data->gicc_base);
}

/*******************************************************************************
 * This functions writes the GIC cpu interface End Of Interrupt register with
 * the passed value to finish handling the active secure group 0 interrupt.
 ******************************************************************************/
void gicv2_end_of_interrupt(unsigned int id)
{
	assert(driver_data != NULL);
	assert(driver_data->gicc_base != 0U);

	gicc_write_EOIR(driver_data->gicc_base, id);
}

/*******************************************************************************
 * This function returns the type of the interrupt id depending upon the group
 * this interrupt has been configured under by the interrupt controller i.e.
 * group0 secure or group1 non secure. It returns zero for Group 0 secure and
 * one for Group 1 non secure interrupt.
 ******************************************************************************/
unsigned int gicv2_get_interrupt_group(unsigned int id)
{
	assert(driver_data != NULL);
	assert(driver_data->gicd_base != 0U);

	return gicd_get_igroupr(driver_data->gicd_base, id);
}

/*******************************************************************************
 * This function returns the priority of the interrupt the processor is
 * currently servicing.
 ******************************************************************************/
unsigned int gicv2_get_running_priority(void)
{
	assert(driver_data != NULL);
	assert(driver_data->gicc_base != 0U);

	return gicc_read_rpr(driver_data->gicc_base);
}

/*******************************************************************************
 * This function sets the GICv2 target mask pattern for the current PE. The PE
 * target mask is used to translate linear PE index (returned by platform core
 * position) to a bit mask used when targeting interrupts to a PE (for example
 * when raising SGIs and routing SPIs).
 ******************************************************************************/
void gicv2_set_pe_target_mask(unsigned int proc_num)
{
	assert(driver_data != NULL);
	assert(driver_data->gicd_base != 0U);
	assert(driver_data->target_masks != NULL);
	assert((unsigned int)proc_num < GICV2_MAX_TARGET_PE);
	assert((unsigned int)proc_num < driver_data->target_masks_num);

	/* Return if the target mask is already populated */
	if (driver_data->target_masks[proc_num] != 0U)
		return;

	/*
	 * Update target register corresponding to this CPU and flush for it to
	 * be visible to other CPUs.
	 */
	if (driver_data->target_masks[proc_num] == 0U) {
		driver_data->target_masks[proc_num] =
			gicv2_get_cpuif_id(driver_data->gicd_base);
#if !(HW_ASSISTED_COHERENCY || WARMBOOT_ENABLE_DCACHE_EARLY)
		/*
		 * PEs only update their own masks. Primary updates it with
		 * caches on. But because secondaries does it with caches off,
		 * all updates go to memory directly, and there's no danger of
		 * secondaries overwriting each others' mask, despite
		 * target_masks[] not being cache line aligned.
		 */
		flush_dcache_range((uintptr_t)
				&driver_data->target_masks[proc_num],
				sizeof(driver_data->target_masks[proc_num]));
#endif
	}
}

/*******************************************************************************
 * This function returns the active status of the interrupt (either because the
 * state is active, or active and pending).
 ******************************************************************************/
unsigned int gicv2_get_interrupt_active(unsigned int id)
{
	assert(driver_data != NULL);
	assert(driver_data->gicd_base != 0U);
	assert(id <= MAX_SPI_ID);

	return gicd_get_isactiver(driver_data->gicd_base, id);
}

/*******************************************************************************
 * This function enables the interrupt identified by id.
 ******************************************************************************/
void gicv2_enable_interrupt(unsigned int id)
{
	assert(driver_data != NULL);
	assert(driver_data->gicd_base != 0U);
	assert(id <= MAX_SPI_ID);

	/*
	 * Ensure that any shared variable updates depending on out of band
	 * interrupt trigger are observed before enabling interrupt.
	 */
	dsbishst();
	gicd_set_isenabler(driver_data->gicd_base, id);
}

/*******************************************************************************
 * This function disables the interrupt identified by id.
 ******************************************************************************/
void gicv2_disable_interrupt(unsigned int id)
{
	assert(driver_data != NULL);
	assert(driver_data->gicd_base != 0U);
	assert(id <= MAX_SPI_ID);

	/*
	 * Disable interrupt, and ensure that any shared variable updates
	 * depending on out of band interrupt trigger are observed afterwards.
	 */
	gicd_set_icenabler(driver_data->gicd_base, id);
	dsbishst();
}

/*******************************************************************************
 * This function sets the interrupt priority as supplied for the given interrupt
 * id.
 ******************************************************************************/
void gicv2_set_interrupt_priority(unsigned int id, unsigned int priority)
{
	assert(driver_data != NULL);
	assert(driver_data->gicd_base != 0U);
	assert(id <= MAX_SPI_ID);

	gicd_set_ipriorityr(driver_data->gicd_base, id, priority);
}

/*******************************************************************************
 * This function assigns group for the interrupt identified by id. The group can
 * be any of GICV2_INTR_GROUP*
 ******************************************************************************/
void gicv2_set_interrupt_type(unsigned int id, unsigned int type)
{
	assert(driver_data != NULL);
	assert(driver_data->gicd_base != 0U);
	assert(id <= MAX_SPI_ID);

	/* Serialize read-modify-write to Distributor registers */
	spin_lock(&gic_lock);
	switch (type) {
	case GICV2_INTR_GROUP1:
		gicd_set_igroupr(driver_data->gicd_base, id);
		break;
	case GICV2_INTR_GROUP0:
		gicd_clr_igroupr(driver_data->gicd_base, id);
		break;
	default:
		assert(false);
		break;
	}
	spin_unlock(&gic_lock);
}

/*******************************************************************************
 * This function raises the specified SGI to requested targets.
 *
 * The proc_num parameter must be the linear index of the target PE in the
 * system.
 ******************************************************************************/
void gicv2_raise_sgi(int sgi_num, int proc_num)
{
	unsigned int sgir_val, target;

	assert(driver_data != NULL);
	assert((unsigned int)proc_num < GICV2_MAX_TARGET_PE);
	assert(driver_data->gicd_base != 0U);

	/*
	 * Target masks array must have been supplied, and the core position
	 * should be valid.
	 */
	assert(driver_data->target_masks != NULL);
	assert((unsigned int)proc_num < driver_data->target_masks_num);

	/* Don't raise SGI if the mask hasn't been populated */
	target = driver_data->target_masks[proc_num];
	assert(target != 0U);

	sgir_val = GICV2_SGIR_VALUE(SGIR_TGT_SPECIFIC, target, sgi_num);

	/*
	 * Ensure that any shared variable updates depending on out of band
	 * interrupt trigger are observed before raising SGI.
	 */
	dsbishst();
	gicd_write_sgir(driver_data->gicd_base, sgir_val);
}

/*******************************************************************************
 * This function sets the interrupt routing for the given SPI interrupt id.
 * The interrupt routing is specified in routing mode. The proc_num parameter is
 * linear index of the PE to target SPI. When proc_num < 0, the SPI may target
 * all PEs.
 ******************************************************************************/
void gicv2_set_spi_routing(unsigned int id, int proc_num)
{
	unsigned int target;

	assert(driver_data != NULL);
	assert(driver_data->gicd_base != 0U);

	assert((id >= MIN_SPI_ID) && (id <= MAX_SPI_ID));

	/*
	 * Target masks array must have been supplied, and the core position
	 * should be valid.
	 */
	assert(driver_data->target_masks != NULL);
	assert((unsigned int)proc_num < GICV2_MAX_TARGET_PE);
	assert((unsigned int)proc_num < driver_data->target_masks_num);

	if (proc_num < 0) {
		/* Target all PEs */
		target = GIC_TARGET_CPU_MASK;
	} else {
		/* Don't route interrupt if the mask hasn't been populated */
		target = driver_data->target_masks[proc_num];
		assert(target != 0U);
	}

	gicd_set_itargetsr(driver_data->gicd_base, id, target);
}

/*******************************************************************************
 * This function clears the pending status of an interrupt identified by id.
 ******************************************************************************/
void gicv2_clear_interrupt_pending(unsigned int id)
{
	assert(driver_data != NULL);
	assert(driver_data->gicd_base != 0U);

	/* SGIs can't be cleared pending */
	assert(id >= MIN_PPI_ID);

	/*
	 * Clear pending interrupt, and ensure that any shared variable updates
	 * depending on out of band interrupt trigger are observed afterwards.
	 */
	gicd_set_icpendr(driver_data->gicd_base, id);
	dsbishst();
}

/*******************************************************************************
 * This function sets the pending status of an interrupt identified by id.
 ******************************************************************************/
void gicv2_set_interrupt_pending(unsigned int id)
{
	assert(driver_data != NULL);
	assert(driver_data->gicd_base != 0U);

	/* SGIs can't be cleared pending */
	assert(id >= MIN_PPI_ID);

	/*
	 * Ensure that any shared variable updates depending on out of band
	 * interrupt trigger are observed before setting interrupt pending.
	 */
	dsbishst();
	gicd_set_ispendr(driver_data->gicd_base, id);
}

/*******************************************************************************
 * This function sets the PMR register with the supplied value. Returns the
 * original PMR.
 ******************************************************************************/
unsigned int gicv2_set_pmr(unsigned int mask)
{
	unsigned int old_mask;

	assert(driver_data != NULL);
	assert(driver_data->gicc_base != 0U);

	old_mask = gicc_read_pmr(driver_data->gicc_base);

	/*
	 * Order memory updates w.r.t. PMR write, and ensure they're visible
	 * before potential out of band interrupt trigger because of PMR update.
	 */
	dmbishst();
	gicc_write_pmr(driver_data->gicc_base, mask);
	dsbishst();

	return old_mask;
}

/*******************************************************************************
 * This function updates single interrupt configuration to be level/edge
 * triggered
 ******************************************************************************/
void gicv2_interrupt_set_cfg(unsigned int id, unsigned int cfg)
{
	gicd_set_icfgr(driver_data->gicd_base, id, cfg);
}
