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

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <gic_common.h>
#include <gicv2.h>
#include "gicv2_private.h"

static const gicv2_driver_data_t *driver_data;

/*******************************************************************************
 * Enable secure interrupts and use FIQs to route them. Disable legacy bypass
 * and set the priority mask register to allow all interrupts to trickle in.
 ******************************************************************************/
void gicv2_cpuif_enable(void)
{
	unsigned int val;

	assert(driver_data);
	assert(driver_data->gicc_base);

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

	assert(driver_data);
	assert(driver_data->gicc_base);

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
	assert(driver_data);
	assert(driver_data->gicd_base);
	assert(driver_data->g0_interrupt_array);

	gicv2_secure_ppi_sgi_setup(driver_data->gicd_base,
					driver_data->g0_interrupt_num,
					driver_data->g0_interrupt_array);
}

/*******************************************************************************
 * Global gic distributor init which will be done by the primary cpu after a
 * cold boot. It marks out the secure SPIs, PPIs & SGIs and enables them. It
 * then enables the secure GIC distributor interface.
 ******************************************************************************/
void gicv2_distif_init(void)
{
	unsigned int ctlr;

	assert(driver_data);
	assert(driver_data->gicd_base);
	assert(driver_data->g0_interrupt_array);

	/* Disable the distributor before going further */
	ctlr = gicd_read_ctlr(driver_data->gicd_base);
	gicd_write_ctlr(driver_data->gicd_base,
			ctlr & ~(CTLR_ENABLE_G0_BIT | CTLR_ENABLE_G1_BIT));

	/* Set the default attribute of all SPIs */
	gicv2_spis_configure_defaults(driver_data->gicd_base);

	/* Configure the G0 SPIs */
	gicv2_secure_spis_configure(driver_data->gicd_base,
					driver_data->g0_interrupt_num,
					driver_data->g0_interrupt_array);

	/* Re-enable the secure SPIs now that they have been configured */
	gicd_write_ctlr(driver_data->gicd_base, ctlr | CTLR_ENABLE_G0_BIT);
}

/*******************************************************************************
 * Initialize the ARM GICv2 driver with the provided platform inputs
 ******************************************************************************/
void gicv2_driver_init(const gicv2_driver_data_t *plat_driver_data)
{
	unsigned int gic_version;
	assert(plat_driver_data);
	assert(plat_driver_data->gicd_base);
	assert(plat_driver_data->gicc_base);

	/*
	 * The platform should provide a list of atleast one type of
	 * interrupts
	 */
	assert(plat_driver_data->g0_interrupt_array);

	/*
	 * If there are no interrupts of a particular type, then the number of
	 * interrupts of that type should be 0 and vice-versa.
	 */
	assert(plat_driver_data->g0_interrupt_array ?
	       plat_driver_data->g0_interrupt_num :
	       plat_driver_data->g0_interrupt_num == 0);

	/* Ensure that this is a GICv2 system */
	gic_version = gicd_read_pidr2(plat_driver_data->gicd_base);
	gic_version = (gic_version >> PIDR2_ARCH_REV_SHIFT)
					& PIDR2_ARCH_REV_MASK;
	assert(gic_version == ARCH_REV_GICV2);

	driver_data = plat_driver_data;

	INFO("ARM GICv2 driver initialized\n");
}

/******************************************************************************
 * This function returns whether FIQ is enabled in the GIC CPU interface.
 *****************************************************************************/
unsigned int gicv2_is_fiq_enabled(void)
{
	unsigned int gicc_ctlr;

	assert(driver_data);
	assert(driver_data->gicc_base);

	gicc_ctlr = gicc_read_ctlr(driver_data->gicc_base);
	return (gicc_ctlr >> FIQ_EN_SHIFT) & 0x1;
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
	assert(driver_data);
	assert(driver_data->gicc_base);

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

	assert(driver_data);
	assert(driver_data->gicc_base);

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
	assert(driver_data);
	assert(driver_data->gicc_base);

	return gicc_read_IAR(driver_data->gicc_base);
}

/*******************************************************************************
 * This functions writes the GIC cpu interface End Of Interrupt register with
 * the passed value to finish handling the active secure group 0 interrupt.
 ******************************************************************************/
void gicv2_end_of_interrupt(unsigned int id)
{
	assert(driver_data);
	assert(driver_data->gicc_base);

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
	assert(driver_data);
	assert(driver_data->gicd_base);

	return gicd_get_igroupr(driver_data->gicd_base, id);
}
