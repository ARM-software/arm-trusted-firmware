/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
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
#include <gicv3.h>
#include "../common/gic_common_private.h"
#include "gicv3_private.h"

static const gicv3_driver_data_t *driver_data;
static unsigned int gicv2_compat;

/*******************************************************************************
 * This function initialises the ARM GICv3 driver in EL3 with provided platform
 * inputs.
 ******************************************************************************/
void gicv3_driver_init(const gicv3_driver_data_t *plat_driver_data)
{
	unsigned int gic_version;

	assert(plat_driver_data);
	assert(plat_driver_data->gicd_base);
	assert(plat_driver_data->gicr_base);
	assert(plat_driver_data->rdistif_num);
	assert(plat_driver_data->rdistif_base_addrs);

	assert(IS_IN_EL3());

	/*
	 * The platform should provide a list of at least one type of
	 * interrupts
	 */
	assert(plat_driver_data->g0_interrupt_array ||
	       plat_driver_data->g1s_interrupt_array);

	/*
	 * If there are no interrupts of a particular type, then the number of
	 * interrupts of that type should be 0 and vice-versa.
	 */
	assert(plat_driver_data->g0_interrupt_array ?
	       plat_driver_data->g0_interrupt_num :
	       plat_driver_data->g0_interrupt_num == 0);
	assert(plat_driver_data->g1s_interrupt_array ?
	       plat_driver_data->g1s_interrupt_num :
	       plat_driver_data->g1s_interrupt_num == 0);

	/* Check for system register support */
#ifdef AARCH32
	assert(read_id_pfr1() & (ID_PFR1_GIC_MASK << ID_PFR1_GIC_SHIFT));
#else
	assert(read_id_aa64pfr0_el1() &
			(ID_AA64PFR0_GIC_MASK << ID_AA64PFR0_GIC_SHIFT));
#endif /* AARCH32 */

	/* The GIC version should be 3.0 */
	gic_version = gicd_read_pidr2(plat_driver_data->gicd_base);
	gic_version >>=	PIDR2_ARCH_REV_SHIFT;
	gic_version &= PIDR2_ARCH_REV_MASK;
	assert(gic_version == ARCH_REV_GICV3);

	/*
	 * Find out whether the GIC supports the GICv2 compatibility mode. The
	 * ARE_S bit resets to 0 if supported
	 */
	gicv2_compat = gicd_read_ctlr(plat_driver_data->gicd_base);
	gicv2_compat >>= CTLR_ARE_S_SHIFT;
	gicv2_compat = !(gicv2_compat & CTLR_ARE_S_MASK);

	/*
	 * Find the base address of each implemented Redistributor interface.
	 * The number of interfaces should be equal to the number of CPUs in the
	 * system. The memory for saving these addresses has to be allocated by
	 * the platform port
	 */
	gicv3_rdistif_base_addrs_probe(plat_driver_data->rdistif_base_addrs,
					   plat_driver_data->rdistif_num,
					   plat_driver_data->gicr_base,
					   plat_driver_data->mpidr_to_core_pos);

	driver_data = plat_driver_data;

	INFO("GICv3 %s legacy support detected."
			" ARM GICV3 driver initialized in EL3\n",
			gicv2_compat ? "with" : "without");
}

/*******************************************************************************
 * This function initialises the GIC distributor interface based upon the data
 * provided by the platform while initialising the driver.
 ******************************************************************************/
void gicv3_distif_init(void)
{
	unsigned int bitmap = 0;

	assert(driver_data);
	assert(driver_data->gicd_base);
	assert(driver_data->g1s_interrupt_array ||
	       driver_data->g0_interrupt_array);

	assert(IS_IN_EL3());

	/*
	 * Clear the "enable" bits for G0/G1S/G1NS interrupts before configuring
	 * the ARE_S bit. The Distributor might generate a system error
	 * otherwise.
	 */
	gicd_clr_ctlr(driver_data->gicd_base,
		      CTLR_ENABLE_G0_BIT |
		      CTLR_ENABLE_G1S_BIT |
		      CTLR_ENABLE_G1NS_BIT,
		      RWP_TRUE);

	/* Set the ARE_S and ARE_NS bit now that interrupts have been disabled */
	gicd_set_ctlr(driver_data->gicd_base,
			CTLR_ARE_S_BIT | CTLR_ARE_NS_BIT, RWP_TRUE);

	/* Set the default attribute of all SPIs */
	gicv3_spis_configure_defaults(driver_data->gicd_base);

	/* Configure the G1S SPIs */
	if (driver_data->g1s_interrupt_array) {
		gicv3_secure_spis_configure(driver_data->gicd_base,
					driver_data->g1s_interrupt_num,
					driver_data->g1s_interrupt_array,
					INTR_GROUP1S);
		bitmap |= CTLR_ENABLE_G1S_BIT;
	}

	/* Configure the G0 SPIs */
	if (driver_data->g0_interrupt_array) {
		gicv3_secure_spis_configure(driver_data->gicd_base,
					driver_data->g0_interrupt_num,
					driver_data->g0_interrupt_array,
					INTR_GROUP0);
		bitmap |= CTLR_ENABLE_G0_BIT;
	}

	/* Enable the secure SPIs now that they have been configured */
	gicd_set_ctlr(driver_data->gicd_base, bitmap, RWP_TRUE);
}

/*******************************************************************************
 * This function initialises the GIC Redistributor interface of the calling CPU
 * (identified by the 'proc_num' parameter) based upon the data provided by the
 * platform while initialising the driver.
 ******************************************************************************/
void gicv3_rdistif_init(unsigned int proc_num)
{
	uintptr_t gicr_base;

	assert(driver_data);
	assert(proc_num < driver_data->rdistif_num);
	assert(driver_data->rdistif_base_addrs);
	assert(driver_data->gicd_base);
	assert(gicd_read_ctlr(driver_data->gicd_base) & CTLR_ARE_S_BIT);
	assert(driver_data->g1s_interrupt_array ||
	       driver_data->g0_interrupt_array);

	assert(IS_IN_EL3());

	gicr_base = driver_data->rdistif_base_addrs[proc_num];

	/* Set the default attribute of all SGIs and PPIs */
	gicv3_ppi_sgi_configure_defaults(gicr_base);

	/* Configure the G1S SGIs/PPIs */
	if (driver_data->g1s_interrupt_array) {
		gicv3_secure_ppi_sgi_configure(gicr_base,
					driver_data->g1s_interrupt_num,
					driver_data->g1s_interrupt_array,
					INTR_GROUP1S);
	}

	/* Configure the G0 SGIs/PPIs */
	if (driver_data->g0_interrupt_array) {
		gicv3_secure_ppi_sgi_configure(gicr_base,
					driver_data->g0_interrupt_num,
					driver_data->g0_interrupt_array,
					INTR_GROUP0);
	}
}

/*******************************************************************************
 * This function enables the GIC CPU interface of the calling CPU using only
 * system register accesses.
 ******************************************************************************/
void gicv3_cpuif_enable(unsigned int proc_num)
{
	uintptr_t gicr_base;
	unsigned int scr_el3;
	unsigned int icc_sre_el3;

	assert(driver_data);
	assert(proc_num < driver_data->rdistif_num);
	assert(driver_data->rdistif_base_addrs);
	assert(IS_IN_EL3());

	/* Mark the connected core as awake */
	gicr_base = driver_data->rdistif_base_addrs[proc_num];
	gicv3_rdistif_mark_core_awake(gicr_base);

	/* Disable the legacy interrupt bypass */
	icc_sre_el3 = ICC_SRE_DIB_BIT | ICC_SRE_DFB_BIT;

	/*
	 * Enable system register access for EL3 and allow lower exception
	 * levels to configure the same for themselves. If the legacy mode is
	 * not supported, the SRE bit is RAO/WI
	 */
	icc_sre_el3 |= (ICC_SRE_EN_BIT | ICC_SRE_SRE_BIT);
	write_icc_sre_el3(read_icc_sre_el3() | icc_sre_el3);

	scr_el3 = read_scr_el3();

	/*
	 * Switch to NS state to write Non secure ICC_SRE_EL1 and
	 * ICC_SRE_EL2 registers.
	 */
	write_scr_el3(scr_el3 | SCR_NS_BIT);
	isb();

	write_icc_sre_el2(read_icc_sre_el2() | icc_sre_el3);
	write_icc_sre_el1(ICC_SRE_SRE_BIT);
	isb();

	/* Switch to secure state. */
	write_scr_el3(scr_el3 & (~SCR_NS_BIT));
	isb();

	/* Program the idle priority in the PMR */
	write_icc_pmr_el1(GIC_PRI_MASK);

	/* Enable Group0 interrupts */
	write_icc_igrpen0_el1(IGRPEN1_EL1_ENABLE_G0_BIT);

	/* Enable Group1 Secure interrupts */
	write_icc_igrpen1_el3(read_icc_igrpen1_el3() |
				IGRPEN1_EL3_ENABLE_G1S_BIT);

	/* Write the secure ICC_SRE_EL1 register */
	write_icc_sre_el1(ICC_SRE_SRE_BIT);
	isb();
}

/*******************************************************************************
 * This function disables the GIC CPU interface of the calling CPU using
 * only system register accesses.
 ******************************************************************************/
void gicv3_cpuif_disable(unsigned int proc_num)
{
	uintptr_t gicr_base;

	assert(driver_data);
	assert(proc_num < driver_data->rdistif_num);
	assert(driver_data->rdistif_base_addrs);

	assert(IS_IN_EL3());

	/* Disable legacy interrupt bypass */
	write_icc_sre_el3(read_icc_sre_el3() |
			  (ICC_SRE_DIB_BIT | ICC_SRE_DFB_BIT));

	/* Disable Group0 interrupts */
	write_icc_igrpen0_el1(read_icc_igrpen0_el1() &
			      ~IGRPEN1_EL1_ENABLE_G0_BIT);

	/* Disable Group1 Secure and Non-Secure interrupts */
	write_icc_igrpen1_el3(read_icc_igrpen1_el3() &
			      ~(IGRPEN1_EL3_ENABLE_G1NS_BIT |
			      IGRPEN1_EL3_ENABLE_G1S_BIT));

	/* Synchronise accesses to group enable registers */
	isb();

	/* Mark the connected core as asleep */
	gicr_base = driver_data->rdistif_base_addrs[proc_num];
	gicv3_rdistif_mark_core_asleep(gicr_base);
}

/*******************************************************************************
 * This function returns the id of the highest priority pending interrupt at
 * the GIC cpu interface.
 ******************************************************************************/
unsigned int gicv3_get_pending_interrupt_id(void)
{
	unsigned int id;

	assert(IS_IN_EL3());
	id = read_icc_hppir0_el1() & HPPIR0_EL1_INTID_MASK;

	/*
	 * If the ID is special identifier corresponding to G1S or G1NS
	 * interrupt, then read the highest pending group 1 interrupt.
	 */
	if ((id == PENDING_G1S_INTID) || (id == PENDING_G1NS_INTID))
		return read_icc_hppir1_el1() & HPPIR1_EL1_INTID_MASK;

	return id;
}

/*******************************************************************************
 * This function returns the type of the highest priority pending interrupt at
 * the GIC cpu interface. The return values can be one of the following :
 *   PENDING_G1S_INTID  : The interrupt type is secure Group 1.
 *   PENDING_G1NS_INTID : The interrupt type is non secure Group 1.
 *   0 - 1019           : The interrupt type is secure Group 0.
 *   GIC_SPURIOUS_INTERRUPT : there is no pending interrupt with
 *                            sufficient priority to be signaled
 ******************************************************************************/
unsigned int gicv3_get_pending_interrupt_type(void)
{
	assert(IS_IN_EL3());
	return read_icc_hppir0_el1() & HPPIR0_EL1_INTID_MASK;
}

/*******************************************************************************
 * This function returns the type of the interrupt id depending upon the group
 * this interrupt has been configured under by the interrupt controller i.e.
 * group0 or group1 Secure / Non Secure. The return value can be one of the
 * following :
 *    INTR_GROUP0  : The interrupt type is a Secure Group 0 interrupt
 *    INTR_GROUP1S : The interrupt type is a Secure Group 1 secure interrupt
 *    INTR_GROUP1NS: The interrupt type is a Secure Group 1 non secure
 *                   interrupt.
 ******************************************************************************/
unsigned int gicv3_get_interrupt_type(unsigned int id,
					  unsigned int proc_num)
{
	unsigned int igroup, grpmodr;
	uintptr_t gicr_base;

	assert(IS_IN_EL3());
	assert(driver_data);

	/* Ensure the parameters are valid */
	assert(id < PENDING_G1S_INTID || id >= MIN_LPI_ID);
	assert(proc_num < driver_data->rdistif_num);

	/* All LPI interrupts are Group 1 non secure */
	if (id >= MIN_LPI_ID)
		return INTR_GROUP1NS;

	if (id < MIN_SPI_ID) {
		assert(driver_data->rdistif_base_addrs);
		gicr_base = driver_data->rdistif_base_addrs[proc_num];
		igroup = gicr_get_igroupr0(gicr_base, id);
		grpmodr = gicr_get_igrpmodr0(gicr_base, id);
	} else {
		assert(driver_data->gicd_base);
		igroup = gicd_get_igroupr(driver_data->gicd_base, id);
		grpmodr = gicd_get_igrpmodr(driver_data->gicd_base, id);
	}

	/*
	 * If the IGROUP bit is set, then it is a Group 1 Non secure
	 * interrupt
	 */
	if (igroup)
		return INTR_GROUP1NS;

	/* If the GRPMOD bit is set, then it is a Group 1 Secure interrupt */
	if (grpmodr)
		return INTR_GROUP1S;

	/* Else it is a Group 0 Secure interrupt */
	return INTR_GROUP0;
}
