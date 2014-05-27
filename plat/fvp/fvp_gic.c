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
#include <debug.h>
#include <gic_v2.h>
#include <gic_v3.h>
#include <interrupt_mgmt.h>
#include <platform.h>
#include <stdint.h>
#include "fvp_def.h"
#include "fvp_private.h"

/*******************************************************************************
 * This function does some minimal GICv3 configuration. The Firmware itself does
 * not fully support GICv3 at this time and relies on GICv2 emulation as
 * provided by GICv3. This function allows software (like Linux) in later stages
 * to use full GICv3 features.
 ******************************************************************************/
void gicv3_cpuif_setup(void)
{
	unsigned int scr_val, val;
	uintptr_t base;

	/*
	 * When CPUs come out of reset they have their GICR_WAKER.ProcessorSleep
	 * bit set. In order to allow interrupts to get routed to the CPU we
	 * need to clear this bit if set and wait for GICR_WAKER.ChildrenAsleep
	 * to clear (GICv3 Architecture specification 5.4.23).
	 * GICR_WAKER is NOT banked per CPU, compute the correct base address
	 * per CPU.
	 */
	base = gicv3_get_rdist(BASE_GICR_BASE, read_mpidr());
	if (base == (uintptr_t)NULL) {
		/* No re-distributor base address. This interface cannot be
		 * configured.
		 */
		panic();
	}

	val = gicr_read_waker(base);

	val &= ~WAKER_PS;
	gicr_write_waker(base, val);
	dsb();

	/* We need to wait for ChildrenAsleep to clear. */
	val = gicr_read_waker(base);
	while (val & WAKER_CA) {
		val = gicr_read_waker(base);
	}

	/*
	 * We need to set SCR_EL3.NS in order to see GICv3 non-secure state.
	 * Restore SCR_EL3.NS again before exit.
	 */
	scr_val = read_scr();
	write_scr(scr_val | SCR_NS_BIT);
	isb();	/* ensure NS=1 takes effect before accessing ICC_SRE_EL2 */

	/*
	 * By default EL2 and NS-EL1 software should be able to enable GICv3
	 * System register access without any configuration at EL3. But it turns
	 * out that GICC PMR as set in GICv2 mode does not affect GICv3 mode. So
	 * we need to set it here again. In order to do that we need to enable
	 * register access. We leave it enabled as it should be fine and might
	 * prevent problems with later software trying to access GIC System
	 * Registers.
	 */
	val = read_icc_sre_el3();
	write_icc_sre_el3(val | ICC_SRE_EN | ICC_SRE_SRE);

	val = read_icc_sre_el2();
	write_icc_sre_el2(val | ICC_SRE_EN | ICC_SRE_SRE);

	write_icc_pmr_el1(GIC_PRI_MASK);
	isb();	/* commite ICC_* changes before setting NS=0 */

	/* Restore SCR_EL3 */
	write_scr(scr_val);
	isb();	/* ensure NS=0 takes effect immediately */
}

/*******************************************************************************
 * This function does some minimal GICv3 configuration when cores go
 * down.
 ******************************************************************************/
void gicv3_cpuif_deactivate(void)
{
	unsigned int val;
	uintptr_t base;

	/*
	 * When taking CPUs down we need to set GICR_WAKER.ProcessorSleep and
	 * wait for GICR_WAKER.ChildrenAsleep to get set.
	 * (GICv3 Architecture specification 5.4.23).
	 * GICR_WAKER is NOT banked per CPU, compute the correct base address
	 * per CPU.
	 */
	base = gicv3_get_rdist(BASE_GICR_BASE, read_mpidr());
	if (base == (uintptr_t)NULL) {
		/* No re-distributor base address. This interface cannot be
		 * configured.
		 */
		panic();
	}

	val = gicr_read_waker(base);
	val |= WAKER_PS;
	gicr_write_waker(base, val);
	dsb();

	/* We need to wait for ChildrenAsleep to set. */
	val = gicr_read_waker(base);
	while ((val & WAKER_CA) == 0) {
		val = gicr_read_waker(base);
	}
}


/*******************************************************************************
 * Enable secure interrupts and use FIQs to route them. Disable legacy bypass
 * and set the priority mask register to allow all interrupts to trickle in.
 ******************************************************************************/
void gic_cpuif_setup(unsigned int gicc_base)
{
	unsigned int val;

	val = gicc_read_iidr(gicc_base);

	/*
	 * If GICv3 we need to do a bit of additional setup. We want to
	 * allow default GICv2 behaviour but allow the next stage to
	 * enable full gicv3 features.
	 */
	if (((val >> GICC_IIDR_ARCH_SHIFT) & GICC_IIDR_ARCH_MASK) >= 3) {
		gicv3_cpuif_setup();
	}

	val = ENABLE_GRP0 | FIQ_EN | FIQ_BYP_DIS_GRP0;
	val |= IRQ_BYP_DIS_GRP0 | FIQ_BYP_DIS_GRP1 | IRQ_BYP_DIS_GRP1;

	gicc_write_pmr(gicc_base, GIC_PRI_MASK);
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

	val = gicc_read_iidr(gicc_base);

	/*
	 * If GICv3 we need to do a bit of additional setup. Make sure the
	 * RDIST is put to sleep.
	 */
	if (((val >> GICC_IIDR_ARCH_SHIFT) & GICC_IIDR_ARCH_MASK) >= 3) {
		gicv3_cpuif_deactivate();
	}
}

/*******************************************************************************
 * Per cpu gic distributor setup which will be done by all cpus after a cold
 * boot/hotplug. This marks out the secure interrupts & enables them.
 ******************************************************************************/
void gic_pcpu_distif_setup(unsigned int gicd_base)
{
	gicd_write_igroupr(gicd_base, 0, ~0);

	gicd_clr_igroupr(gicd_base, IRQ_SEC_PHY_TIMER);
	gicd_clr_igroupr(gicd_base, IRQ_SEC_SGI_0);
	gicd_clr_igroupr(gicd_base, IRQ_SEC_SGI_1);
	gicd_clr_igroupr(gicd_base, IRQ_SEC_SGI_2);
	gicd_clr_igroupr(gicd_base, IRQ_SEC_SGI_3);
	gicd_clr_igroupr(gicd_base, IRQ_SEC_SGI_4);
	gicd_clr_igroupr(gicd_base, IRQ_SEC_SGI_5);
	gicd_clr_igroupr(gicd_base, IRQ_SEC_SGI_6);
	gicd_clr_igroupr(gicd_base, IRQ_SEC_SGI_7);

	gicd_set_ipriorityr(gicd_base, IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY);
	gicd_set_ipriorityr(gicd_base, IRQ_SEC_SGI_0, GIC_HIGHEST_SEC_PRIORITY);
	gicd_set_ipriorityr(gicd_base, IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY);
	gicd_set_ipriorityr(gicd_base, IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY);
	gicd_set_ipriorityr(gicd_base, IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY);
	gicd_set_ipriorityr(gicd_base, IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY);
	gicd_set_ipriorityr(gicd_base, IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY);
	gicd_set_ipriorityr(gicd_base, IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY);
	gicd_set_ipriorityr(gicd_base, IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY);

	gicd_set_isenabler(gicd_base, IRQ_SEC_PHY_TIMER);
	gicd_set_isenabler(gicd_base, IRQ_SEC_SGI_0);
	gicd_set_isenabler(gicd_base, IRQ_SEC_SGI_1);
	gicd_set_isenabler(gicd_base, IRQ_SEC_SGI_2);
	gicd_set_isenabler(gicd_base, IRQ_SEC_SGI_3);
	gicd_set_isenabler(gicd_base, IRQ_SEC_SGI_4);
	gicd_set_isenabler(gicd_base, IRQ_SEC_SGI_5);
	gicd_set_isenabler(gicd_base, IRQ_SEC_SGI_6);
	gicd_set_isenabler(gicd_base, IRQ_SEC_SGI_7);
}

/*******************************************************************************
 * Global gic distributor setup which will be done by the primary cpu after a
 * cold boot. It marks out the secure SPIs, PPIs & SGIs and enables them. It
 * then enables the secure GIC distributor interface.
 ******************************************************************************/
void gic_distif_setup(unsigned int gicd_base)
{
	unsigned int ctr, num_ints, ctlr;

	/* Disable the distributor before going further */
	ctlr = gicd_read_ctlr(gicd_base);
	ctlr &= ~(ENABLE_GRP0 | ENABLE_GRP1);
	gicd_write_ctlr(gicd_base, ctlr);

	/*
	 * Mark out non-secure interrupts. Calculate number of
	 * IGROUPR registers to consider. Will be equal to the
	 * number of IT_LINES
	 */
	num_ints = gicd_read_typer(gicd_base) & IT_LINES_NO_MASK;
	num_ints++;
	for (ctr = 0; ctr < num_ints; ctr++)
		gicd_write_igroupr(gicd_base, ctr << IGROUPR_SHIFT, ~0);

	/* Configure secure interrupts now */
	gicd_clr_igroupr(gicd_base, IRQ_TZ_WDOG);
	gicd_set_ipriorityr(gicd_base, IRQ_TZ_WDOG, GIC_HIGHEST_SEC_PRIORITY);
	gicd_set_itargetsr(gicd_base, IRQ_TZ_WDOG,
			   platform_get_core_pos(read_mpidr()));
	gicd_set_isenabler(gicd_base, IRQ_TZ_WDOG);
	gic_pcpu_distif_setup(gicd_base);

	gicd_write_ctlr(gicd_base, ctlr | ENABLE_GRP0);
}

void gic_setup(void)
{
	unsigned int gicd_base, gicc_base;

	gicd_base = fvp_get_cfgvar(CONFIG_GICD_ADDR);
	gicc_base = fvp_get_cfgvar(CONFIG_GICC_ADDR);

	gic_cpuif_setup(gicc_base);
	gic_distif_setup(gicd_base);
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
	uint32_t gicc_base = fvp_get_cfgvar(CONFIG_GICC_ADDR);

	assert(type == INTR_TYPE_S_EL1 ||
	       type == INTR_TYPE_EL3 ||
	       type == INTR_TYPE_NS);

	assert(security_state == NON_SECURE || security_state == SECURE);

	/*
	 * We ignore the security state parameter under the assumption that
	 * both normal and secure worlds are using ARM GICv2. This parameter
	 * will be used when the secure world starts using GICv3.
	 */
#if FVP_GIC_ARCH == 2
	return gicv2_interrupt_type_to_line(gicc_base, type);
#else
#error "Invalid GIC architecture version specified for FVP port"
#endif
}

#if FVP_GIC_ARCH == 2
/*******************************************************************************
 * This function returns the type of the highest priority pending interrupt at
 * the GIC cpu interface. INTR_TYPE_INVAL is returned when there is no
 * interrupt pending.
 ******************************************************************************/
uint32_t plat_ic_get_pending_interrupt_type()
{
	uint32_t id, gicc_base;

	gicc_base = fvp_get_cfgvar(CONFIG_GICC_ADDR);
	id = gicc_read_hppir(gicc_base);

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
uint32_t plat_ic_get_pending_interrupt_id()
{
	uint32_t id, gicc_base;

	gicc_base = fvp_get_cfgvar(CONFIG_GICC_ADDR);
	id = gicc_read_hppir(gicc_base);

	if (id < 1022)
		return id;

	if (id == 1023)
		return INTR_ID_UNAVAILABLE;

	/*
	 * Find out which non-secure interrupt it is under the assumption that
	 * the GICC_CTLR.AckCtl bit is 0.
	 */
	return gicc_read_ahppir(gicc_base);
}

/*******************************************************************************
 * This functions reads the GIC cpu interface Interrupt Acknowledge register
 * to start handling the pending interrupt. It returns the contents of the IAR.
 ******************************************************************************/
uint32_t plat_ic_acknowledge_interrupt()
{
	return gicc_read_IAR(fvp_get_cfgvar(CONFIG_GICC_ADDR));
}

/*******************************************************************************
 * This functions writes the GIC cpu interface End Of Interrupt register with
 * the passed value to finish handling the active interrupt
 ******************************************************************************/
void plat_ic_end_of_interrupt(uint32_t id)
{
	gicc_write_EOIR(fvp_get_cfgvar(CONFIG_GICC_ADDR), id);
	return;
}

/*******************************************************************************
 * This function returns the type of the interrupt id depending upon the group
 * this interrupt has been configured under by the interrupt controller i.e.
 * group0 or group1.
 ******************************************************************************/
uint32_t plat_ic_get_interrupt_type(uint32_t id)
{
	uint32_t group;

	group = gicd_get_igroupr(fvp_get_cfgvar(CONFIG_GICD_ADDR), id);

	/* Assume that all secure interrupts are S-EL1 interrupts */
	if (group == GRP0)
		return INTR_TYPE_S_EL1;
	else
		return INTR_TYPE_NS;
}

#else
#error "Invalid GIC architecture version specified for FVP port"
#endif
