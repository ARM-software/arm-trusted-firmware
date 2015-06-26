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
#include <arm_gic.h>
#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <gic_v2.h>
#include <gic_v3.h>
#include <interrupt_mgmt.h>
#include <platform.h>
#include <stdint.h>

/* Value used to initialize Non-Secure IRQ priorities four at a time */
#define GICD_IPRIORITYR_DEF_VAL \
	(GIC_HIGHEST_NS_PRIORITY | \
	(GIC_HIGHEST_NS_PRIORITY << 8) | \
	(GIC_HIGHEST_NS_PRIORITY << 16) | \
	(GIC_HIGHEST_NS_PRIORITY << 24))

static uintptr_t g_gicc_base;
static uintptr_t g_gicd_base;
static uintptr_t g_gicr_base;
static const unsigned int *g_irq_sec_ptr;
static unsigned int g_num_irqs;


/*******************************************************************************
 * This function does some minimal GICv3 configuration. The Firmware itself does
 * not fully support GICv3 at this time and relies on GICv2 emulation as
 * provided by GICv3. This function allows software (like Linux) in later stages
 * to use full GICv3 features.
 ******************************************************************************/
static void gicv3_cpuif_setup(void)
{
	unsigned int val;
	uintptr_t base;

	/*
	 * When CPUs come out of reset they have their GICR_WAKER.ProcessorSleep
	 * bit set. In order to allow interrupts to get routed to the CPU we
	 * need to clear this bit if set and wait for GICR_WAKER.ChildrenAsleep
	 * to clear (GICv3 Architecture specification 5.4.23).
	 * GICR_WAKER is NOT banked per CPU, compute the correct base address
	 * per CPU.
	 */
	assert(g_gicr_base);
	base = gicv3_get_rdist(g_gicr_base, read_mpidr());
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

	val = read_icc_sre_el3();
	write_icc_sre_el3(val | ICC_SRE_EN | ICC_SRE_SRE);
	isb();
}

/*******************************************************************************
 * This function does some minimal GICv3 configuration when cores go
 * down.
 ******************************************************************************/
static void gicv3_cpuif_deactivate(void)
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
	assert(g_gicr_base);
	base = gicv3_get_rdist(g_gicr_base, read_mpidr());
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
void arm_gic_cpuif_setup(void)
{
	unsigned int val;

	assert(g_gicc_base);
	val = gicc_read_iidr(g_gicc_base);

	/*
	 * If GICv3 we need to do a bit of additional setup. We want to
	 * allow default GICv2 behaviour but allow the next stage to
	 * enable full gicv3 features.
	 */
	if (((val >> GICC_IIDR_ARCH_SHIFT) & GICC_IIDR_ARCH_MASK) >= 3)
		gicv3_cpuif_setup();

	val = ENABLE_GRP0 | FIQ_EN | FIQ_BYP_DIS_GRP0;
	val |= IRQ_BYP_DIS_GRP0 | FIQ_BYP_DIS_GRP1 | IRQ_BYP_DIS_GRP1;

	gicc_write_pmr(g_gicc_base, GIC_PRI_MASK);
	gicc_write_ctlr(g_gicc_base, val);
}

/*******************************************************************************
 * Place the cpu interface in a state where it can never make a cpu exit wfi as
 * as result of an asserted interrupt. This is critical for powering down a cpu
 ******************************************************************************/
void arm_gic_cpuif_deactivate(void)
{
	unsigned int val;

	/* Disable secure, non-secure interrupts and disable their bypass */
	assert(g_gicc_base);
	val = gicc_read_ctlr(g_gicc_base);
	val &= ~(ENABLE_GRP0 | ENABLE_GRP1);
	val |= FIQ_BYP_DIS_GRP1 | FIQ_BYP_DIS_GRP0;
	val |= IRQ_BYP_DIS_GRP0 | IRQ_BYP_DIS_GRP1;
	gicc_write_ctlr(g_gicc_base, val);

	val = gicc_read_iidr(g_gicc_base);

	/*
	 * If GICv3 we need to do a bit of additional setup. Make sure the
	 * RDIST is put to sleep.
	 */
	if (((val >> GICC_IIDR_ARCH_SHIFT) & GICC_IIDR_ARCH_MASK) >= 3)
		gicv3_cpuif_deactivate();
}

/*******************************************************************************
 * Per cpu gic distributor setup which will be done by all cpus after a cold
 * boot/hotplug. This marks out the secure interrupts & enables them.
 ******************************************************************************/
void arm_gic_pcpu_distif_setup(void)
{
	unsigned int index, irq_num, sec_ppi_sgi_mask;

	assert(g_gicd_base);

	/* Setup PPI priorities doing four at a time */
	for (index = 0; index < 32; index += 4) {
		gicd_write_ipriorityr(g_gicd_base, index,
				GICD_IPRIORITYR_DEF_VAL);
	}

	assert(g_irq_sec_ptr);
	sec_ppi_sgi_mask = 0;
	for (index = 0; index < g_num_irqs; index++) {
		irq_num = g_irq_sec_ptr[index];
		if (irq_num < MIN_SPI_ID) {
			/* We have an SGI or a PPI. They are Group0 at reset */
			sec_ppi_sgi_mask |= 1U << irq_num;
			gicd_set_ipriorityr(g_gicd_base, irq_num,
				GIC_HIGHEST_SEC_PRIORITY);
			gicd_set_isenabler(g_gicd_base, irq_num);
		}
	}

	/*
	 * Invert the bitmask to create a mask for non-secure PPIs and
	 * SGIs. Program the GICD_IGROUPR0 with this bit mask. This write will
	 * update the GICR_IGROUPR0 as well in case we are running on a GICv3
	 * system. This is critical if GICD_CTLR.ARE_NS=1.
	 */
	gicd_write_igroupr(g_gicd_base, 0, ~sec_ppi_sgi_mask);
}

/*******************************************************************************
 * Get the current CPU bit mask from GICD_ITARGETSR0
 ******************************************************************************/
static unsigned int arm_gic_get_cpuif_id(void)
{
	unsigned int val;

	val = gicd_read_itargetsr(g_gicd_base, 0);
	return val & GIC_TARGET_CPU_MASK;
}

/*******************************************************************************
 * Global gic distributor setup which will be done by the primary cpu after a
 * cold boot. It marks out the secure SPIs, PPIs & SGIs and enables them. It
 * then enables the secure GIC distributor interface.
 ******************************************************************************/
static void arm_gic_distif_setup(void)
{
	unsigned int num_ints, ctlr, index, irq_num;
	uint8_t target_cpu;

	/* Disable the distributor before going further */
	assert(g_gicd_base);
	ctlr = gicd_read_ctlr(g_gicd_base);
	ctlr &= ~(ENABLE_GRP0 | ENABLE_GRP1);
	gicd_write_ctlr(g_gicd_base, ctlr);

	/*
	 * Mark out non-secure SPI interrupts. The number of interrupts is
	 * calculated as 32 * (IT_LINES + 1). We do 32 at a time.
	 */
	num_ints = gicd_read_typer(g_gicd_base) & IT_LINES_NO_MASK;
	num_ints = (num_ints + 1) << 5;
	for (index = MIN_SPI_ID; index < num_ints; index += 32)
		gicd_write_igroupr(g_gicd_base, index, ~0);

	/* Setup SPI priorities doing four at a time */
	for (index = MIN_SPI_ID; index < num_ints; index += 4) {
		gicd_write_ipriorityr(g_gicd_base, index,
				GICD_IPRIORITYR_DEF_VAL);
	}

	/* Read the target CPU mask */
	target_cpu = arm_gic_get_cpuif_id();

	/* Configure SPI secure interrupts now */
	assert(g_irq_sec_ptr);
	for (index = 0; index < g_num_irqs; index++) {
		irq_num = g_irq_sec_ptr[index];
		if (irq_num >= MIN_SPI_ID) {
			/* We have an SPI */
			gicd_clr_igroupr(g_gicd_base, irq_num);
			gicd_set_ipriorityr(g_gicd_base, irq_num,
				GIC_HIGHEST_SEC_PRIORITY);
			gicd_set_itargetsr(g_gicd_base, irq_num, target_cpu);
			gicd_set_isenabler(g_gicd_base, irq_num);
		}
	}

	/*
	 * Configure the SGI and PPI. This is done in a separated function
	 * because each CPU is responsible for initializing its own private
	 * interrupts.
	 */
	arm_gic_pcpu_distif_setup();

	gicd_write_ctlr(g_gicd_base, ctlr | ENABLE_GRP0);
}

/*******************************************************************************
 * Initialize the ARM GIC driver with the provided platform inputs
******************************************************************************/
void arm_gic_init(uintptr_t gicc_base,
		  uintptr_t gicd_base,
		  uintptr_t gicr_base,
		  const unsigned int *irq_sec_ptr,
		  unsigned int num_irqs)
{
	unsigned int val;

	assert(gicc_base);
	assert(gicd_base);
	assert(irq_sec_ptr);

	g_gicc_base = gicc_base;
	g_gicd_base = gicd_base;

	val = gicc_read_iidr(g_gicc_base);

	if (((val >> GICC_IIDR_ARCH_SHIFT) & GICC_IIDR_ARCH_MASK) >= 3) {
		assert(gicr_base);
		g_gicr_base = gicr_base;
	}

	g_irq_sec_ptr = irq_sec_ptr;
	g_num_irqs = num_irqs;
}

/*******************************************************************************
 * Setup the ARM GIC CPU and distributor interfaces.
******************************************************************************/
void arm_gic_setup(void)
{
	arm_gic_cpuif_setup();
	arm_gic_distif_setup();
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
uint32_t arm_gic_interrupt_type_to_line(uint32_t type,
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
	return gicv2_interrupt_type_to_line(g_gicc_base, type);
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
uint32_t arm_gic_get_pending_interrupt_type(void)
{
	uint32_t id;

	assert(g_gicc_base);
	id = gicc_read_hppir(g_gicc_base) & INT_ID_MASK;

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
uint32_t arm_gic_get_pending_interrupt_id(void)
{
	uint32_t id;

	assert(g_gicc_base);
	id = gicc_read_hppir(g_gicc_base) & INT_ID_MASK;

	if (id < 1022)
		return id;

	if (id == 1023)
		return INTR_ID_UNAVAILABLE;

	/*
	 * Find out which non-secure interrupt it is under the assumption that
	 * the GICC_CTLR.AckCtl bit is 0.
	 */
	return gicc_read_ahppir(g_gicc_base) & INT_ID_MASK;
}

/*******************************************************************************
 * This functions reads the GIC cpu interface Interrupt Acknowledge register
 * to start handling the pending interrupt. It returns the contents of the IAR.
 ******************************************************************************/
uint32_t arm_gic_acknowledge_interrupt(void)
{
	assert(g_gicc_base);
	return gicc_read_IAR(g_gicc_base);
}

/*******************************************************************************
 * This functions writes the GIC cpu interface End Of Interrupt register with
 * the passed value to finish handling the active interrupt
 ******************************************************************************/
void arm_gic_end_of_interrupt(uint32_t id)
{
	assert(g_gicc_base);
	gicc_write_EOIR(g_gicc_base, id);
}

/*******************************************************************************
 * This function returns the type of the interrupt id depending upon the group
 * this interrupt has been configured under by the interrupt controller i.e.
 * group0 or group1.
 ******************************************************************************/
uint32_t arm_gic_get_interrupt_type(uint32_t id)
{
	uint32_t group;

	assert(g_gicd_base);
	group = gicd_get_igroupr(g_gicd_base, id);

	/* Assume that all secure interrupts are S-EL1 interrupts */
	if (group == GRP0)
		return INTR_TYPE_S_EL1;
	else
		return INTR_TYPE_NS;
}

#else
#error "Invalid ARM GIC architecture version specified for platform port"
#endif /* ARM_GIC_ARCH */
