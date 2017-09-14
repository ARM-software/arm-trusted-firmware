/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <arm_gic.h>
#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <gic_v2.h>
#include <interrupt_mgmt.h>
#include <platform.h>
#include <stdint.h>
#include <tegra_def.h>
#include <tegra_private.h>

/* Value used to initialize Non-Secure IRQ priorities four at a time */
#define GICD_IPRIORITYR_DEF_VAL \
	(GIC_HIGHEST_NS_PRIORITY | \
	(GIC_HIGHEST_NS_PRIORITY << 8) | \
	(GIC_HIGHEST_NS_PRIORITY << 16) | \
	(GIC_HIGHEST_NS_PRIORITY << 24))

static const irq_sec_cfg_t *g_irq_sec_ptr;
static uint32_t g_num_irqs;

/*******************************************************************************
 * Place the cpu interface in a state where it can never make a cpu exit wfi as
 * as result of an asserted interrupt. This is critical for powering down a cpu
 ******************************************************************************/
void tegra_gic_cpuif_deactivate(void)
{
	uint32_t val;

	/* Disable secure, non-secure interrupts and disable their bypass */
	val = gicc_read_ctlr(TEGRA_GICC_BASE);
	val &= ~(ENABLE_GRP0 | ENABLE_GRP1);
	val |= FIQ_BYP_DIS_GRP1 | FIQ_BYP_DIS_GRP0;
	val |= IRQ_BYP_DIS_GRP0 | IRQ_BYP_DIS_GRP1;
	gicc_write_ctlr(TEGRA_GICC_BASE, val);
}

/*******************************************************************************
 * Enable secure interrupts and set the priority mask register to allow all
 * interrupts to trickle in.
 ******************************************************************************/
static void tegra_gic_cpuif_setup(uint32_t gicc_base)
{
	uint32_t val;

	val = ENABLE_GRP0 | ENABLE_GRP1 | FIQ_EN | FIQ_BYP_DIS_GRP0;
	val |= IRQ_BYP_DIS_GRP0 | FIQ_BYP_DIS_GRP1 | IRQ_BYP_DIS_GRP1;

	gicc_write_ctlr(gicc_base, val);
	gicc_write_pmr(gicc_base, GIC_PRI_MASK);
}

/*******************************************************************************
 * Per cpu gic distributor setup which will be done by all cpus after a cold
 * boot/hotplug. This marks out the secure interrupts & enables them.
 ******************************************************************************/
static void tegra_gic_pcpu_distif_setup(uint32_t gicd_base)
{
	uint32_t index, sec_ppi_sgi_mask = 0;

	assert(gicd_base != 0U);

	/* Setup PPI priorities doing four at a time */
	for (index = 0U; index < 32U; index += 4U) {
		gicd_write_ipriorityr(gicd_base, index,
				GICD_IPRIORITYR_DEF_VAL);
	}

	/*
	 * Invert the bitmask to create a mask for non-secure PPIs and
	 * SGIs. Program the GICD_IGROUPR0 with this bit mask. This write will
	 * update the GICR_IGROUPR0 as well in case we are running on a GICv3
	 * system. This is critical if GICD_CTLR.ARE_NS=1.
	 */
	gicd_write_igroupr(gicd_base, 0, ~sec_ppi_sgi_mask);
}

/*******************************************************************************
 * Global gic distributor setup which will be done by the primary cpu after a
 * cold boot. It marks out the non secure SPIs, PPIs & SGIs and enables them.
 * It then enables the secure GIC distributor interface.
 ******************************************************************************/
static void tegra_gic_distif_setup(uint32_t gicd_base)
{
	uint32_t index, num_ints, irq_num;
	uint8_t target_cpus;
	uint32_t val;

	/*
	 * Mark out non-secure interrupts. Calculate number of
	 * IGROUPR registers to consider. Will be equal to the
	 * number of IT_LINES
	 */
	num_ints = gicd_read_typer(gicd_base) & IT_LINES_NO_MASK;
	num_ints = (num_ints + 1U) << 5;
	for (index = MIN_SPI_ID; index < num_ints; index += 32U) {
		gicd_write_igroupr(gicd_base, index, 0xFFFFFFFFU);
	}

	/* Setup SPI priorities doing four at a time */
	for (index = MIN_SPI_ID; index < num_ints; index += 4U) {
		gicd_write_ipriorityr(gicd_base, index,
				GICD_IPRIORITYR_DEF_VAL);
	}

	/* Configure SPI secure interrupts now */
	if (g_irq_sec_ptr != NULL) {

		for (index = 0U; index < g_num_irqs; index++) {
			irq_num = g_irq_sec_ptr[index].irq;
			target_cpus = (uint8_t)g_irq_sec_ptr[index].target_cpus;

			if (irq_num >= MIN_SPI_ID) {

				/* Configure as a secure interrupt */
				gicd_clr_igroupr(gicd_base, irq_num);

				/* Configure SPI priority */
				mmio_write_8((uint64_t)gicd_base +
					(uint64_t)GICD_IPRIORITYR +
					(uint64_t)irq_num,
					GIC_HIGHEST_SEC_PRIORITY &
					GIC_PRI_MASK);

				/* Configure as level triggered */
				val = gicd_read_icfgr(gicd_base, irq_num);
				val |= (3U << ((irq_num & 0xFU) << 1U));
				gicd_write_icfgr(gicd_base, irq_num, val);

				/* Route SPI to the target CPUs */
				gicd_set_itargetsr(gicd_base, irq_num,
					target_cpus);

				/* Enable this interrupt */
				gicd_set_isenabler(gicd_base, irq_num);
			}
		}
	}

	/*
	 * Configure the SGI and PPI. This is done in a separated function
	 * because each CPU is responsible for initializing its own private
	 * interrupts.
	 */
	tegra_gic_pcpu_distif_setup(gicd_base);

	/* enable distributor */
	gicd_write_ctlr(gicd_base, ENABLE_GRP0 | ENABLE_GRP1);
}

void tegra_gic_setup(const irq_sec_cfg_t *irq_sec_ptr, uint32_t num_irqs)
{
	g_irq_sec_ptr = irq_sec_ptr;
	g_num_irqs = num_irqs;

	tegra_gic_cpuif_setup(TEGRA_GICC_BASE);
	tegra_gic_distif_setup(TEGRA_GICD_BASE);
}

/*******************************************************************************
 * An ARM processor signals interrupt exceptions through the IRQ and FIQ pins.
 * The interrupt controller knows which pin/line it uses to signal a type of
 * interrupt. This function provides a common implementation of
 * plat_interrupt_type_to_line() in an ARM GIC environment for optional re-use
 * across platforms. It lets the interrupt management framework determine
 * for a type of interrupt and security state, which line should be used in the
 * SCR_EL3 to control its routing to EL3. The interrupt line is represented as
 * the bit position of the IRQ or FIQ bit in the SCR_EL3.
 ******************************************************************************/
static uint32_t tegra_gic_interrupt_type_to_line(uint32_t type,
				uint32_t security_state)
{
	assert((type == INTR_TYPE_S_EL1) ||
	       (type == INTR_TYPE_EL3) ||
	       (type == INTR_TYPE_NS));

	assert(sec_state_is_valid(security_state));

	/*
	 * We ignore the security state parameter under the assumption that
	 * both normal and secure worlds are using ARM GICv2. This parameter
	 * will be used when the secure world starts using GICv3.
	 */
#if ARM_GIC_ARCH == 2
	return gicv2_interrupt_type_to_line(TEGRA_GICC_BASE, type);
#else
#error "Invalid ARM GIC architecture version specified for platform port"
#endif /* ARM_GIC_ARCH */
}

#if ARM_GIC_ARCH == 2
/*******************************************************************************
 * This function returns the type of the highest priority pending interrupt at
 * the GIC cpu interface. INTR_TYPE_INVAL is returned when there is no
 * interrupt pending.
 ******************************************************************************/
static uint32_t tegra_gic_get_pending_interrupt_type(void)
{
	uint32_t id;
	uint32_t index;
	uint32_t ret = INTR_TYPE_NS;

	id = gicc_read_hppir(TEGRA_GICC_BASE) & INT_ID_MASK;

	/* get the interrupt type */
	if (id < 1022U) {
		for (index = 0U; index < g_num_irqs; index++) {
			if (id == g_irq_sec_ptr[index].irq) {
				ret = g_irq_sec_ptr[index].type;
				break;
			}
		}
	} else {
		 if (id == GIC_SPURIOUS_INTERRUPT) {
			ret = INTR_TYPE_INVAL;
		}
	}

	return ret;
}

/*******************************************************************************
 * This function returns the id of the highest priority pending interrupt at
 * the GIC cpu interface. INTR_ID_UNAVAILABLE is returned when there is no
 * interrupt pending.
 ******************************************************************************/
static uint32_t tegra_gic_get_pending_interrupt_id(void)
{
	uint32_t id, ret;

	id = gicc_read_hppir(TEGRA_GICC_BASE) & INT_ID_MASK;

	if (id < 1022U) {
		ret = id;
	} else if (id == 1023U) {
		ret = 0xFFFFFFFFU; /* INTR_ID_UNAVAILABLE */
	} else {
		/*
		 * Find out which non-secure interrupt it is under the assumption that
		 * the GICC_CTLR.AckCtl bit is 0.
		 */
		ret = gicc_read_ahppir(TEGRA_GICC_BASE) & INT_ID_MASK;
	}

	return ret;
}

/*******************************************************************************
 * This functions reads the GIC cpu interface Interrupt Acknowledge register
 * to start handling the pending interrupt. It returns the contents of the IAR.
 ******************************************************************************/
static uint32_t tegra_gic_acknowledge_interrupt(void)
{
	return gicc_read_IAR(TEGRA_GICC_BASE);
}

/*******************************************************************************
 * This functions writes the GIC cpu interface End Of Interrupt register with
 * the passed value to finish handling the active interrupt
 ******************************************************************************/
static void tegra_gic_end_of_interrupt(uint32_t id)
{
	gicc_write_EOIR(TEGRA_GICC_BASE, id);
}

/*******************************************************************************
 * This function returns the type of the interrupt id depending upon the group
 * this interrupt has been configured under by the interrupt controller i.e.
 * group0 or group1.
 ******************************************************************************/
static uint32_t tegra_gic_get_interrupt_type(uint32_t id)
{
	uint32_t group;
	uint32_t index;
	uint32_t ret = INTR_TYPE_NS;

	group = gicd_get_igroupr(TEGRA_GICD_BASE, id);

	/* get the interrupt type */
	if (group == GRP0) {
		for (index = 0U; index < g_num_irqs; index++) {
			if (id == g_irq_sec_ptr[index].irq) {
				ret = g_irq_sec_ptr[index].type;
				break;
			}
		}
	}

	return ret;
}

#else
#error "Invalid ARM GIC architecture version specified for platform port"
#endif /* ARM_GIC_ARCH */

uint32_t plat_ic_get_pending_interrupt_id(void)
{
	return tegra_gic_get_pending_interrupt_id();
}

uint32_t plat_ic_get_pending_interrupt_type(void)
{
	return tegra_gic_get_pending_interrupt_type();
}

uint32_t plat_ic_acknowledge_interrupt(void)
{
	return tegra_gic_acknowledge_interrupt();
}

uint32_t plat_ic_get_interrupt_type(uint32_t id)
{
	return tegra_gic_get_interrupt_type(id);
}

void plat_ic_end_of_interrupt(uint32_t id)
{
	tegra_gic_end_of_interrupt(id);
}

uint32_t plat_interrupt_type_to_line(uint32_t type,
				uint32_t security_state)
{
	return tegra_gic_interrupt_type_to_line(type, security_state);
}
