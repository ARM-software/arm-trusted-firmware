/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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
#include <bl_common.h>
#include <gic_v2.h>
#include <interrupt_mgmt.h>
#include <platform.h>
#include "juno_def.h"
#include "juno_private.h"


/* Value used to initialise Non-Secure irq priorities four at a time */
#define DEFAULT_NS_PRIORITY_X4 \
	(GIC_HIGHEST_NS_PRIORITY | \
	(GIC_HIGHEST_NS_PRIORITY << 8) | \
	(GIC_HIGHEST_NS_PRIORITY << 16) | \
	(GIC_HIGHEST_NS_PRIORITY << 24))


/*******************************************************************************
 * Enable secure interrupts and use FIQs to route them. Disable legacy bypass
 * and set the priority mask register to allow all interrupts to trickle in.
 ******************************************************************************/
void gic_cpuif_setup(unsigned int gicc_base)
{
	unsigned int val;

	gicc_write_pmr(gicc_base, GIC_PRI_MASK);

	val = ENABLE_GRP0 | FIQ_EN;
	val |= FIQ_BYP_DIS_GRP0 | IRQ_BYP_DIS_GRP0;
	val |= FIQ_BYP_DIS_GRP1 | IRQ_BYP_DIS_GRP1;
	gicc_write_ctlr(gicc_base, val);
}

/*******************************************************************************
 * Place the cpu interface in a state where it can never make a cpu exit wfi as
 * as result of an asserted interrupt. This is critical for powering down a cpu
 ******************************************************************************/
void gic_cpuif_deactivate(unsigned int gicc_base)
{
	unsigned int val;

	/* Disable secure, non-secure interrupts and disable their bypass */
	val = gicc_read_ctlr(gicc_base);
	val &= ~(ENABLE_GRP0 | ENABLE_GRP1);
	val |= FIQ_BYP_DIS_GRP1 | FIQ_BYP_DIS_GRP0;
	val |= IRQ_BYP_DIS_GRP0 | IRQ_BYP_DIS_GRP1;
	gicc_write_ctlr(gicc_base, val);
}

static void gic_set_secure(unsigned int gicd_base, unsigned id)
{
	/* Set interrupt as Group 0 */
	gicd_clr_igroupr(gicd_base, id);

	/* Set priority to max */
	gicd_set_ipriorityr(gicd_base, id, GIC_HIGHEST_SEC_PRIORITY);
}

/*******************************************************************************
 * Per cpu gic distributor setup which will be done by all cpus after a cold
 * boot/hotplug. This marks out the secure interrupts & enables them.
 ******************************************************************************/
void gic_pcpu_distif_setup(unsigned int gicd_base)
{
	unsigned i;

	/* Mark all 32 PPI interrupts as Group 1 (non-secure) */
	mmio_write_32(gicd_base + GICD_IGROUPR, 0xffffffffu);

	/* Setup PPI priorities doing four at a time */
	for (i = 0; i < 32; i += 4)
		mmio_write_32(gicd_base + GICD_IPRIORITYR + i, DEFAULT_NS_PRIORITY_X4);

	/* Configure those PPIs we want as secure, and enable them. */
	static const char sec_irq[] = {
		IRQ_SEC_PHY_TIMER,
		IRQ_SEC_SGI_0,
		IRQ_SEC_SGI_1,
		IRQ_SEC_SGI_2,
		IRQ_SEC_SGI_3,
		IRQ_SEC_SGI_4,
		IRQ_SEC_SGI_5,
		IRQ_SEC_SGI_6,
		IRQ_SEC_SGI_7
	};
	for (i = 0; i < sizeof(sec_irq) / sizeof(sec_irq[0]); i++) {
		gic_set_secure(gicd_base, sec_irq[i]);
		gicd_set_isenabler(gicd_base, sec_irq[i]);
	}
}

/*******************************************************************************
 * Global gic distributor setup which will be done by the primary cpu after a
 * cold boot. It marks out the secure SPIs, PPIs & SGIs and enables them. It
 * then enables the secure GIC distributor interface.
 ******************************************************************************/
static void gic_distif_setup(unsigned int gicd_base)
{
	unsigned int i, ctlr;
	const unsigned int ITLinesNumber =
				gicd_read_typer(gicd_base) & IT_LINES_NO_MASK;

	/* Disable the distributor before going further */
	ctlr = gicd_read_ctlr(gicd_base);
	ctlr &= ~(ENABLE_GRP0 | ENABLE_GRP1);
	gicd_write_ctlr(gicd_base, ctlr);

	/* Mark all lines of SPIs as Group 1 (non-secure) */
	for (i = 0; i < ITLinesNumber; i++)
		mmio_write_32(gicd_base + GICD_IGROUPR + 4 + i * 4, 0xffffffffu);

	/* Setup SPI priorities doing four at a time */
	for (i = 0; i < ITLinesNumber * 32; i += 4)
		mmio_write_32(gicd_base + GICD_IPRIORITYR + 32 + i, DEFAULT_NS_PRIORITY_X4);

	/* Configure the SPIs we want as secure */
	static const char sec_irq[] = {
		IRQ_MHU,
		IRQ_GPU_SMMU_0,
		IRQ_GPU_SMMU_1,
		IRQ_ETR_SMMU,
		IRQ_TZC400,
		IRQ_TZ_WDOG
	};
	for (i = 0; i < sizeof(sec_irq) / sizeof(sec_irq[0]); i++)
		gic_set_secure(gicd_base, sec_irq[i]);

	/* Route watchdog interrupt to this CPU and enable it. */
	gicd_set_itargetsr(gicd_base, IRQ_TZ_WDOG,
			   platform_get_core_pos(read_mpidr()));
	gicd_set_isenabler(gicd_base, IRQ_TZ_WDOG);

	/* Now setup the PPIs */
	gic_pcpu_distif_setup(gicd_base);

	/* Enable Group 0 (secure) interrupts */
	gicd_write_ctlr(gicd_base, ctlr | ENABLE_GRP0);
}

void gic_setup(void)
{
	gic_cpuif_setup(GICC_BASE);
	gic_distif_setup(GICD_BASE);
}

/*******************************************************************************
 * An ARM processor signals interrupt exceptions through the IRQ and FIQ pins.
 * The interrupt controller knows which pin/line it uses to signal a type of
 * interrupt. The platform knows which interrupt controller type is being used
 * in a particular security state e.g. with an ARM GIC, normal world could use
 * the GICv2 features while the secure world could use GICv3 features and vice
 * versa.
 * This function is exported by the platform to let the interrupt management
 * framework determine for a type of interrupt and security state, which line
 * should be used in the SCR_EL3 to control its routing to EL3. The interrupt
 * line is represented as the bit position of the IRQ or FIQ bit in the SCR_EL3.
 ******************************************************************************/
uint32_t plat_interrupt_type_to_line(uint32_t type, uint32_t security_state)
{
	assert(type == INTR_TYPE_S_EL1 ||
	       type == INTR_TYPE_EL3 ||
	       type == INTR_TYPE_NS);

	assert(sec_state_is_valid(security_state));

	/*
	 * We ignore the security state parameter because Juno is GICv2 only
	 * so both normal and secure worlds are using ARM GICv2.
	 */
	return gicv2_interrupt_type_to_line(GICC_BASE, type);
}

/*******************************************************************************
 * This function returns the type of the highest priority pending interrupt at
 * the GIC cpu interface. INTR_TYPE_INVAL is returned when there is no
 * interrupt pending.
 ******************************************************************************/
uint32_t plat_ic_get_pending_interrupt_type(void)
{
	uint32_t id;

	id = gicc_read_hppir(GICC_BASE);

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
uint32_t plat_ic_get_pending_interrupt_id(void)
{
	uint32_t id;

	id = gicc_read_hppir(GICC_BASE);

	if (id < 1022)
		return id;

	if (id == 1023)
		return INTR_ID_UNAVAILABLE;

	/*
	 * Find out which non-secure interrupt it is under the assumption that
	 * the GICC_CTLR.AckCtl bit is 0.
	 */
	return gicc_read_ahppir(GICC_BASE);
}

/*******************************************************************************
 * This functions reads the GIC cpu interface Interrupt Acknowledge register
 * to start handling the pending interrupt. It returns the contents of the IAR.
 ******************************************************************************/
uint32_t plat_ic_acknowledge_interrupt(void)
{
	return gicc_read_IAR(GICC_BASE);
}

/*******************************************************************************
 * This functions writes the GIC cpu interface End Of Interrupt register with
 * the passed value to finish handling the active interrupt
 ******************************************************************************/
void plat_ic_end_of_interrupt(uint32_t id)
{
	gicc_write_EOIR(GICC_BASE, id);
}

/*******************************************************************************
 * This function returns the type of the interrupt id depending upon the group
 * this interrupt has been configured under by the interrupt controller i.e.
 * group0 or group1.
 ******************************************************************************/
uint32_t plat_ic_get_interrupt_type(uint32_t id)
{
	uint32_t group;

	group = gicd_get_igroupr(GICD_BASE, id);

	/* Assume that all secure interrupts are S-EL1 interrupts */
	if (group == GRP0)
		return INTR_TYPE_S_EL1;
	else
		return INTR_TYPE_NS;
}
