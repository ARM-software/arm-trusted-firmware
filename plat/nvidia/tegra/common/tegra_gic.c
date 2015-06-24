/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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

#include <arch_helpers.h>
#include <assert.h>
#include <arm_gic.h>
#include <bl_common.h>
#include <debug.h>
#include <gic_v2.h>
#include <interrupt_mgmt.h>
#include <platform.h>
#include <stdint.h>
#include <tegra_private.h>
#include <tegra_def.h>

/* Value used to initialize Non-Secure IRQ priorities four at a time */
#define GICD_IPRIORITYR_DEF_VAL \
	(GIC_HIGHEST_NS_PRIORITY | \
	(GIC_HIGHEST_NS_PRIORITY << 8) | \
	(GIC_HIGHEST_NS_PRIORITY << 16) | \
	(GIC_HIGHEST_NS_PRIORITY << 24))

/*******************************************************************************
 * Place the cpu interface in a state where it can never make a cpu exit wfi as
 * as result of an asserted interrupt. This is critical for powering down a cpu
 ******************************************************************************/
void tegra_gic_cpuif_deactivate(void)
{
	unsigned int val;

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
static void tegra_gic_cpuif_setup(unsigned int gicc_base)
{
	unsigned int val;

	val = ENABLE_GRP0 | ENABLE_GRP1 | FIQ_EN | FIQ_BYP_DIS_GRP0;
	val |= IRQ_BYP_DIS_GRP0 | FIQ_BYP_DIS_GRP1 | IRQ_BYP_DIS_GRP1;

	gicc_write_ctlr(gicc_base, val);
	gicc_write_pmr(gicc_base, GIC_PRI_MASK);
}

/*******************************************************************************
 * Per cpu gic distributor setup which will be done by all cpus after a cold
 * boot/hotplug. This marks out the secure interrupts & enables them.
 ******************************************************************************/
static void tegra_gic_pcpu_distif_setup(unsigned int gicd_base)
{
	unsigned int index, sec_ppi_sgi_mask = 0;

	assert(gicd_base);

	/* Setup PPI priorities doing four at a time */
	for (index = 0; index < 32; index += 4) {
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
static void tegra_gic_distif_setup(unsigned int gicd_base)
{
	unsigned int index, num_ints;

	/*
	 * Mark out non-secure interrupts. Calculate number of
	 * IGROUPR registers to consider. Will be equal to the
	 * number of IT_LINES
	 */
	num_ints = gicd_read_typer(gicd_base) & IT_LINES_NO_MASK;
	num_ints = (num_ints + 1) << 5;
	for (index = MIN_SPI_ID; index < num_ints; index += 32)
		gicd_write_igroupr(gicd_base, index, ~0);

	/* Setup SPI priorities doing four at a time */
	for (index = MIN_SPI_ID; index < num_ints; index += 4) {
		gicd_write_ipriorityr(gicd_base, index,
				GICD_IPRIORITYR_DEF_VAL);
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

void tegra_gic_setup(void)
{
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
uint32_t tegra_gic_interrupt_type_to_line(uint32_t type,
				uint32_t security_state)
{
	assert(type == INTR_TYPE_S_EL1 ||
	       type == INTR_TYPE_EL3 ||
	       type == INTR_TYPE_NS);

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
uint32_t tegra_gic_get_pending_interrupt_type(void)
{
	uint32_t id;

	id = gicc_read_hppir(TEGRA_GICC_BASE) & INT_ID_MASK;

	/* Assume that all secure interrupts are S-EL1 interrupts */
	if (id < 1022)
		return INTR_TYPE_S_EL1;

	if (id == GIC_SPURIOUS_INTERRUPT)
		return INTR_TYPE_INVAL;

	return INTR_TYPE_NS;
}

/*******************************************************************************
 * This function returns the id of the highest priority pending interrupt at
 * the GIC cpu interface. INTR_ID_UNAVAILABLE is returned when there is no
 * interrupt pending.
 ******************************************************************************/
uint32_t tegra_gic_get_pending_interrupt_id(void)
{
	uint32_t id;

	id = gicc_read_hppir(TEGRA_GICC_BASE) & INT_ID_MASK;

	if (id < 1022)
		return id;

	if (id == 1023)
		return INTR_ID_UNAVAILABLE;

	/*
	 * Find out which non-secure interrupt it is under the assumption that
	 * the GICC_CTLR.AckCtl bit is 0.
	 */
	return gicc_read_ahppir(TEGRA_GICC_BASE) & INT_ID_MASK;
}

/*******************************************************************************
 * This functions reads the GIC cpu interface Interrupt Acknowledge register
 * to start handling the pending interrupt. It returns the contents of the IAR.
 ******************************************************************************/
uint32_t tegra_gic_acknowledge_interrupt(void)
{
	return gicc_read_IAR(TEGRA_GICC_BASE);
}

/*******************************************************************************
 * This functions writes the GIC cpu interface End Of Interrupt register with
 * the passed value to finish handling the active interrupt
 ******************************************************************************/
void tegra_gic_end_of_interrupt(uint32_t id)
{
	gicc_write_EOIR(TEGRA_GICC_BASE, id);
}

/*******************************************************************************
 * This function returns the type of the interrupt id depending upon the group
 * this interrupt has been configured under by the interrupt controller i.e.
 * group0 or group1.
 ******************************************************************************/
uint32_t tegra_gic_get_interrupt_type(uint32_t id)
{
	uint32_t group;

	group = gicd_get_igroupr(TEGRA_GICD_BASE, id);

	/* Assume that all secure interrupts are S-EL1 interrupts */
	if (group == GRP0)
		return INTR_TYPE_S_EL1;
	else
		return INTR_TYPE_NS;
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
