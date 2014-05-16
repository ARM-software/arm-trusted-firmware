/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
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

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <gic_v2.h>
#include <gic_v3.h>
#include <plat_gic.h>
#include <platform.h>
#include <stdint.h>

/*******************************************************************************
 * TODO: Revisit if priorities are being set such that no non-secure interrupt
 * can have a higher priority than a secure one as recommended in the GICv2 spec
 ******************************************************************************/

/*******************************************************************************
 * This function does some minimal GICv3 configuration. The Firmware itself does
 * not fully support GICv3 at this time and relies on GICv2 emulation as
 * provided by GICv3. This function allows software (like Linux) in later stages
 * to use full GICv3 features.
 ******************************************************************************/
void plat_gicv3_cpuif_setup(uintptr_t gicr_base)
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
	base = gicv3_get_rdist(gicr_base, read_mpidr());
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
	while (val & WAKER_CA)
		val = gicr_read_waker(base);

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
void plat_gicv3_cpuif_deactivate(uintptr_t gicr_base)
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
	base = gicv3_get_rdist(gicr_base, read_mpidr());
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
	while ((val & WAKER_CA) == 0)
		val = gicr_read_waker(base);
}


/*******************************************************************************
 * Enable secure interrupts and use FIQs to route them. Disable legacy bypass
 * and set the priority mask register to allow all interrupts to trickle in.
 ******************************************************************************/
void plat_gic_cpuif_setup(unsigned int gicc_base, uintptr_t gicr_base)
{
	unsigned int val;

	val = gicc_read_iidr(gicc_base);

	/*
	 * If GICv3 we need to do a bit of additional setup. We want to
	 * allow default GICv2 behaviour but allow the next stage to
	 * enable full gicv3 features.
	 */
	if (((val >> GICC_IIDR_ARCH_SHIFT) & GICC_IIDR_ARCH_MASK) >= 3)
		plat_gicv3_cpuif_setup(gicr_base);

	val = ENABLE_GRP0 | FIQ_EN | FIQ_BYP_DIS_GRP0;
	val |= IRQ_BYP_DIS_GRP0 | FIQ_BYP_DIS_GRP1 | IRQ_BYP_DIS_GRP1;

	gicc_write_pmr(gicc_base, GIC_PRI_MASK);
	gicc_write_ctlr(gicc_base, val);
}

/*******************************************************************************
 * Place the cpu interface in a state where it can never make a cpu exit wfi as
 * as result of an asserted interrupt. This is critical for powering down a cpu
 ******************************************************************************/
void plat_gic_cpuif_deactivate(unsigned int gicc_base, uintptr_t gicr_base)
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
	if (((val >> GICC_IIDR_ARCH_SHIFT) & GICC_IIDR_ARCH_MASK) >= 3)
		plat_gicv3_cpuif_deactivate(gicr_base);
}

/*******************************************************************************
 * Per cpu gic distributor setup which will be done by all cpus after a cold
 * boot/hotplug. This marks out the secure interrupts & enables them.
 ******************************************************************************/
void plat_gic_pcpu_distif_setup(unsigned int gicd_base,
			const unsigned int *irq_sec_ptr,
			unsigned int num_irqs)
{
	unsigned int index, irq_num;
	assert(irq_sec_ptr);

	gicd_write_igroupr(gicd_base, 0, ~0);

	for (index = 0; index < num_irqs; index++) {
		irq_num = irq_sec_ptr[index];
		if (irq_num < MIN_SPI_ID) {
			/* We have an SGI or a PPI */
			gicd_clr_igroupr(gicd_base, irq_num);
			gicd_set_ipriorityr(gicd_base, irq_num,
				GIC_HIGHEST_SEC_PRIORITY);
			gicd_set_isenabler(gicd_base, irq_num);
		}
	}
}

/*******************************************************************************
 * Global gic distributor setup which will be done by the primary cpu after a
 * cold boot. It marks out the secure SPIs, PPIs & SGIs and enables them. It
 * then enables the secure GIC distributor interface.
 ******************************************************************************/
void plat_gic_distif_setup(unsigned int gicd_base,
			const unsigned int *irq_sec_ptr,
			unsigned int num_irqs)
{
	unsigned int num_ints, ctlr, index, irq_num;

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
	for (index = 0; index < num_ints; index++)
		gicd_write_igroupr(gicd_base, index << IGROUPR_SHIFT, ~0);

	/* Configure secure interrupts now */
	for (index = 0; index < num_irqs; index++) {
		irq_num = irq_sec_ptr[index];
		if (irq_num >= MIN_SPI_ID) {
			/* We have an SPI */
			gicd_clr_igroupr(gicd_base, irq_num);
			gicd_set_ipriorityr(gicd_base, irq_num,
				GIC_HIGHEST_SEC_PRIORITY);
			gicd_set_itargetsr(gicd_base, irq_num,
					platform_get_core_pos(read_mpidr()));
			gicd_set_isenabler(gicd_base, irq_num);
		}
	}
	plat_gic_pcpu_distif_setup(gicd_base, irq_sec_ptr, num_irqs);

	gicd_write_ctlr(gicd_base, ctlr | ENABLE_GRP0);
}
