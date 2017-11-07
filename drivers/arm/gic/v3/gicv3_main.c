/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <gicv3.h>
#include <interrupt_props.h>
#include <spinlock.h>
#include "gicv3_private.h"

const gicv3_driver_data_t *gicv3_driver_data;
static unsigned int gicv2_compat;

/*
 * Spinlock to guard registers needing read-modify-write. APIs protected by this
 * spinlock are used either at boot time (when only a single CPU is active), or
 * when the system is fully coherent.
 */
spinlock_t gic_lock;

/*
 * Redistributor power operations are weakly bound so that they can be
 * overridden
 */
#pragma weak gicv3_rdistif_off
#pragma weak gicv3_rdistif_on


/* Helper macros to save and restore GICD registers to and from the context */
#define RESTORE_GICD_REGS(base, ctx, intr_num, reg, REG)		\
	do {								\
		for (unsigned int int_id = MIN_SPI_ID; int_id < intr_num; \
				int_id += (1 << REG##_SHIFT)) {		\
			gicd_write_##reg(base, int_id,			\
				ctx->gicd_##reg[(int_id - MIN_SPI_ID) >> REG##_SHIFT]); \
		}							\
	} while (0)

#define SAVE_GICD_REGS(base, ctx, intr_num, reg, REG)			\
	do {								\
		for (unsigned int int_id = MIN_SPI_ID; int_id < intr_num; \
				int_id += (1 << REG##_SHIFT)) {		\
			ctx->gicd_##reg[(int_id - MIN_SPI_ID) >> REG##_SHIFT] =\
					gicd_read_##reg(base, int_id);	\
		}							\
	} while (0)


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

#if !ERROR_DEPRECATED
	if (plat_driver_data->interrupt_props == NULL) {
		/* Interrupt properties array size must be 0 */
		assert(plat_driver_data->interrupt_props_num == 0);

		/*
		 * The platform should provide a list of at least one type of
		 * interrupt.
		 */
		assert(plat_driver_data->g0_interrupt_array ||
				plat_driver_data->g1s_interrupt_array);

		/*
		 * If there are no interrupts of a particular type, then the
		 * number of interrupts of that type should be 0 and vice-versa.
		 */
		assert(plat_driver_data->g0_interrupt_array ?
				plat_driver_data->g0_interrupt_num :
				plat_driver_data->g0_interrupt_num == 0);
		assert(plat_driver_data->g1s_interrupt_array ?
				plat_driver_data->g1s_interrupt_num :
				plat_driver_data->g1s_interrupt_num == 0);
	}
#else
	assert(plat_driver_data->interrupt_props != NULL);
	assert(plat_driver_data->interrupt_props_num > 0);
#endif

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

	gicv3_driver_data = plat_driver_data;

	/*
	 * The GIC driver data is initialized by the primary CPU with caches
	 * enabled. When the secondary CPU boots up, it initializes the
	 * GICC/GICR interface with the caches disabled. Hence flush the
	 * driver data to ensure coherency. This is not required if the
	 * platform has HW_ASSISTED_COHERENCY enabled.
	 */
#if !HW_ASSISTED_COHERENCY
	flush_dcache_range((uintptr_t) &gicv3_driver_data,
			sizeof(gicv3_driver_data));
	flush_dcache_range((uintptr_t) gicv3_driver_data,
			sizeof(*gicv3_driver_data));
#endif

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

	assert(gicv3_driver_data);
	assert(gicv3_driver_data->gicd_base);

	assert(IS_IN_EL3());

	/*
	 * Clear the "enable" bits for G0/G1S/G1NS interrupts before configuring
	 * the ARE_S bit. The Distributor might generate a system error
	 * otherwise.
	 */
	gicd_clr_ctlr(gicv3_driver_data->gicd_base,
		      CTLR_ENABLE_G0_BIT |
		      CTLR_ENABLE_G1S_BIT |
		      CTLR_ENABLE_G1NS_BIT,
		      RWP_TRUE);

	/* Set the ARE_S and ARE_NS bit now that interrupts have been disabled */
	gicd_set_ctlr(gicv3_driver_data->gicd_base,
			CTLR_ARE_S_BIT | CTLR_ARE_NS_BIT, RWP_TRUE);

	/* Set the default attribute of all SPIs */
	gicv3_spis_configure_defaults(gicv3_driver_data->gicd_base);

#if !ERROR_DEPRECATED
	if (gicv3_driver_data->interrupt_props != NULL) {
#endif
		bitmap = gicv3_secure_spis_configure_props(
				gicv3_driver_data->gicd_base,
				gicv3_driver_data->interrupt_props,
				gicv3_driver_data->interrupt_props_num);
#if !ERROR_DEPRECATED
	} else {
		assert(gicv3_driver_data->g1s_interrupt_array ||
				gicv3_driver_data->g0_interrupt_array);

		/* Configure the G1S SPIs */
		if (gicv3_driver_data->g1s_interrupt_array) {
			gicv3_secure_spis_configure(gicv3_driver_data->gicd_base,
					gicv3_driver_data->g1s_interrupt_num,
					gicv3_driver_data->g1s_interrupt_array,
					INTR_GROUP1S);
			bitmap |= CTLR_ENABLE_G1S_BIT;
		}

		/* Configure the G0 SPIs */
		if (gicv3_driver_data->g0_interrupt_array) {
			gicv3_secure_spis_configure(gicv3_driver_data->gicd_base,
					gicv3_driver_data->g0_interrupt_num,
					gicv3_driver_data->g0_interrupt_array,
					INTR_GROUP0);
			bitmap |= CTLR_ENABLE_G0_BIT;
		}
	}
#endif

	/* Enable the secure SPIs now that they have been configured */
	gicd_set_ctlr(gicv3_driver_data->gicd_base, bitmap, RWP_TRUE);
}

/*******************************************************************************
 * This function initialises the GIC Redistributor interface of the calling CPU
 * (identified by the 'proc_num' parameter) based upon the data provided by the
 * platform while initialising the driver.
 ******************************************************************************/
void gicv3_rdistif_init(unsigned int proc_num)
{
	uintptr_t gicr_base;
	unsigned int bitmap = 0;
	uint32_t ctlr;

	assert(gicv3_driver_data);
	assert(proc_num < gicv3_driver_data->rdistif_num);
	assert(gicv3_driver_data->rdistif_base_addrs);
	assert(gicv3_driver_data->gicd_base);

	ctlr = gicd_read_ctlr(gicv3_driver_data->gicd_base);
	assert(ctlr & CTLR_ARE_S_BIT);

	assert(IS_IN_EL3());

	/* Power on redistributor */
	gicv3_rdistif_on(proc_num);

	gicr_base = gicv3_driver_data->rdistif_base_addrs[proc_num];

	/* Set the default attribute of all SGIs and PPIs */
	gicv3_ppi_sgi_configure_defaults(gicr_base);

#if !ERROR_DEPRECATED
	if (gicv3_driver_data->interrupt_props != NULL) {
#endif
		bitmap = gicv3_secure_ppi_sgi_configure_props(gicr_base,
				gicv3_driver_data->interrupt_props,
				gicv3_driver_data->interrupt_props_num);
#if !ERROR_DEPRECATED
	} else {
		assert(gicv3_driver_data->g1s_interrupt_array ||
		       gicv3_driver_data->g0_interrupt_array);

		/* Configure the G1S SGIs/PPIs */
		if (gicv3_driver_data->g1s_interrupt_array) {
			gicv3_secure_ppi_sgi_configure(gicr_base,
					gicv3_driver_data->g1s_interrupt_num,
					gicv3_driver_data->g1s_interrupt_array,
					INTR_GROUP1S);
			bitmap |= CTLR_ENABLE_G1S_BIT;
		}

		/* Configure the G0 SGIs/PPIs */
		if (gicv3_driver_data->g0_interrupt_array) {
			gicv3_secure_ppi_sgi_configure(gicr_base,
					gicv3_driver_data->g0_interrupt_num,
					gicv3_driver_data->g0_interrupt_array,
					INTR_GROUP0);
			bitmap |= CTLR_ENABLE_G0_BIT;
		}
	}
#endif

	/* Enable interrupt groups as required, if not already */
	if ((ctlr & bitmap) != bitmap)
		gicd_set_ctlr(gicv3_driver_data->gicd_base, bitmap, RWP_TRUE);
}

/*******************************************************************************
 * Functions to perform power operations on GIC Redistributor
 ******************************************************************************/
void gicv3_rdistif_off(unsigned int proc_num)
{
	return;
}

void gicv3_rdistif_on(unsigned int proc_num)
{
	return;
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

	assert(gicv3_driver_data);
	assert(proc_num < gicv3_driver_data->rdistif_num);
	assert(gicv3_driver_data->rdistif_base_addrs);
	assert(IS_IN_EL3());

	/* Mark the connected core as awake */
	gicr_base = gicv3_driver_data->rdistif_base_addrs[proc_num];
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

	assert(gicv3_driver_data);
	assert(proc_num < gicv3_driver_data->rdistif_num);
	assert(gicv3_driver_data->rdistif_base_addrs);

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
	gicr_base = gicv3_driver_data->rdistif_base_addrs[proc_num];
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
	assert(gicv3_driver_data);

	/* Ensure the parameters are valid */
	assert(id < PENDING_G1S_INTID || id >= MIN_LPI_ID);
	assert(proc_num < gicv3_driver_data->rdistif_num);

	/* All LPI interrupts are Group 1 non secure */
	if (id >= MIN_LPI_ID)
		return INTR_GROUP1NS;

	if (id < MIN_SPI_ID) {
		assert(gicv3_driver_data->rdistif_base_addrs);
		gicr_base = gicv3_driver_data->rdistif_base_addrs[proc_num];
		igroup = gicr_get_igroupr0(gicr_base, id);
		grpmodr = gicr_get_igrpmodr0(gicr_base, id);
	} else {
		assert(gicv3_driver_data->gicd_base);
		igroup = gicd_get_igroupr(gicv3_driver_data->gicd_base, id);
		grpmodr = gicd_get_igrpmodr(gicv3_driver_data->gicd_base, id);
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

/*****************************************************************************
 * Function to save and disable the GIC ITS register context. The power
 * management of GIC ITS is implementation-defined and this function doesn't
 * save any memory structures required to support ITS. As the sequence to save
 * this state is implementation defined, it should be executed in platform
 * specific code. Calling this function alone and then powering down the GIC and
 * ITS without implementing the aforementioned platform specific code will
 * corrupt the ITS state.
 *
 * This function must be invoked after the GIC CPU interface is disabled.
 *****************************************************************************/
void gicv3_its_save_disable(uintptr_t gits_base, gicv3_its_ctx_t * const its_ctx)
{
	int i;

	assert(gicv3_driver_data);
	assert(IS_IN_EL3());
	assert(its_ctx);
	assert(gits_base);

	its_ctx->gits_ctlr = gits_read_ctlr(gits_base);

	/* Disable the ITS */
	gits_write_ctlr(gits_base, its_ctx->gits_ctlr &
					(~GITS_CTLR_ENABLED_BIT));

	/* Wait for quiescent state */
	gits_wait_for_quiescent_bit(gits_base);

	its_ctx->gits_cbaser = gits_read_cbaser(gits_base);
	its_ctx->gits_cwriter = gits_read_cwriter(gits_base);

	for (i = 0; i < ARRAY_SIZE(its_ctx->gits_baser); i++)
		its_ctx->gits_baser[i] = gits_read_baser(gits_base, i);
}

/*****************************************************************************
 * Function to restore the GIC ITS register context. The power
 * management of GIC ITS is implementation defined and this function doesn't
 * restore any memory structures required to support ITS. The assumption is
 * that these structures are in memory and are retained during system suspend.
 *
 * This must be invoked before the GIC CPU interface is enabled.
 *****************************************************************************/
void gicv3_its_restore(uintptr_t gits_base, const gicv3_its_ctx_t * const its_ctx)
{
	int i;

	assert(gicv3_driver_data);
	assert(IS_IN_EL3());
	assert(its_ctx);
	assert(gits_base);

	/* Assert that the GITS is disabled and quiescent */
	assert((gits_read_ctlr(gits_base) & GITS_CTLR_ENABLED_BIT) == 0);
	assert((gits_read_ctlr(gits_base) & GITS_CTLR_QUIESCENT_BIT) != 0);

	gits_write_cbaser(gits_base, its_ctx->gits_cbaser);
	gits_write_cwriter(gits_base, its_ctx->gits_cwriter);

	for (i = 0; i < ARRAY_SIZE(its_ctx->gits_baser); i++)
		gits_write_baser(gits_base, i, its_ctx->gits_baser[i]);

	/* Restore the ITS CTLR but leave the ITS disabled */
	gits_write_ctlr(gits_base, its_ctx->gits_ctlr &
			(~GITS_CTLR_ENABLED_BIT));
}

/*****************************************************************************
 * Function to save the GIC Redistributor register context. This function
 * must be invoked after CPU interface disable and prior to Distributor save.
 *****************************************************************************/
void gicv3_rdistif_save(unsigned int proc_num, gicv3_redist_ctx_t * const rdist_ctx)
{
	uintptr_t gicr_base;
	unsigned int int_id;

	assert(gicv3_driver_data);
	assert(proc_num < gicv3_driver_data->rdistif_num);
	assert(gicv3_driver_data->rdistif_base_addrs);
	assert(IS_IN_EL3());
	assert(rdist_ctx);

	gicr_base = gicv3_driver_data->rdistif_base_addrs[proc_num];

	/*
	 * Wait for any write to GICR_CTLR to complete before trying to save any
	 * state.
	 */
	gicr_wait_for_pending_write(gicr_base);

	rdist_ctx->gicr_ctlr = gicr_read_ctlr(gicr_base);

	rdist_ctx->gicr_propbaser = gicr_read_propbaser(gicr_base);
	rdist_ctx->gicr_pendbaser = gicr_read_pendbaser(gicr_base);

	rdist_ctx->gicr_igroupr0 = gicr_read_igroupr0(gicr_base);
	rdist_ctx->gicr_isenabler0 = gicr_read_isenabler0(gicr_base);
	rdist_ctx->gicr_ispendr0 = gicr_read_ispendr0(gicr_base);
	rdist_ctx->gicr_isactiver0 = gicr_read_isactiver0(gicr_base);
	rdist_ctx->gicr_icfgr0 = gicr_read_icfgr0(gicr_base);
	rdist_ctx->gicr_icfgr1 = gicr_read_icfgr1(gicr_base);
	rdist_ctx->gicr_igrpmodr0 = gicr_read_igrpmodr0(gicr_base);
	rdist_ctx->gicr_nsacr = gicr_read_nsacr(gicr_base);
	for (int_id = MIN_SGI_ID; int_id < TOTAL_PCPU_INTR_NUM;
			int_id += (1 << IPRIORITYR_SHIFT)) {
		rdist_ctx->gicr_ipriorityr[(int_id - MIN_SGI_ID) >> IPRIORITYR_SHIFT] =
				gicr_read_ipriorityr(gicr_base, int_id);
	}


	/*
	 * Call the pre-save hook that implements the IMP DEF sequence that may
	 * be required on some GIC implementations. As this may need to access
	 * the Redistributor registers, we pass it proc_num.
	 */
	gicv3_distif_pre_save(proc_num);
}

/*****************************************************************************
 * Function to restore the GIC Redistributor register context. We disable
 * LPI and per-cpu interrupts before we start restore of the Redistributor.
 * This function must be invoked after Distributor restore but prior to
 * CPU interface enable. The pending and active interrupts are restored
 * after the interrupts are fully configured and enabled.
 *****************************************************************************/
void gicv3_rdistif_init_restore(unsigned int proc_num,
				const gicv3_redist_ctx_t * const rdist_ctx)
{
	uintptr_t gicr_base;
	unsigned int int_id;

	assert(gicv3_driver_data);
	assert(proc_num < gicv3_driver_data->rdistif_num);
	assert(gicv3_driver_data->rdistif_base_addrs);
	assert(IS_IN_EL3());
	assert(rdist_ctx);

	gicr_base = gicv3_driver_data->rdistif_base_addrs[proc_num];

	/* Power on redistributor */
	gicv3_rdistif_on(proc_num);

	/*
	 * Call the post-restore hook that implements the IMP DEF sequence that
	 * may be required on some GIC implementations. As this may need to
	 * access the Redistributor registers, we pass it proc_num.
	 */
	gicv3_distif_post_restore(proc_num);

	/*
	 * Disable all SGIs (imp. def.)/PPIs before configuring them. This is a
	 * more scalable approach as it avoids clearing the enable bits in the
	 * GICD_CTLR
	 */
	gicr_write_icenabler0(gicr_base, ~0);
	/* Wait for pending writes to GICR_ICENABLER */
	gicr_wait_for_pending_write(gicr_base);

	/*
	 * Disable the LPIs to avoid unpredictable behavior when writing to
	 * GICR_PROPBASER and GICR_PENDBASER.
	 */
	gicr_write_ctlr(gicr_base,
			rdist_ctx->gicr_ctlr & ~(GICR_CTLR_EN_LPIS_BIT));

	/* Restore registers' content */
	gicr_write_propbaser(gicr_base, rdist_ctx->gicr_propbaser);
	gicr_write_pendbaser(gicr_base, rdist_ctx->gicr_pendbaser);

	gicr_write_igroupr0(gicr_base, rdist_ctx->gicr_igroupr0);

	for (int_id = MIN_SGI_ID; int_id < TOTAL_PCPU_INTR_NUM;
			int_id += (1 << IPRIORITYR_SHIFT)) {
		gicr_write_ipriorityr(gicr_base, int_id,
		rdist_ctx->gicr_ipriorityr[
				(int_id - MIN_SGI_ID) >> IPRIORITYR_SHIFT]);
	}

	gicr_write_icfgr0(gicr_base, rdist_ctx->gicr_icfgr0);
	gicr_write_icfgr1(gicr_base, rdist_ctx->gicr_icfgr1);
	gicr_write_igrpmodr0(gicr_base, rdist_ctx->gicr_igrpmodr0);
	gicr_write_nsacr(gicr_base, rdist_ctx->gicr_nsacr);

	/* Restore after group and priorities are set */
	gicr_write_ispendr0(gicr_base, rdist_ctx->gicr_ispendr0);
	gicr_write_isactiver0(gicr_base, rdist_ctx->gicr_isactiver0);

	/*
	 * Wait for all writes to the Distributor to complete before enabling
	 * the SGI and PPIs.
	 */
	gicr_wait_for_upstream_pending_write(gicr_base);
	gicr_write_isenabler0(gicr_base, rdist_ctx->gicr_isenabler0);

	/*
	 * Restore GICR_CTLR.Enable_LPIs bit and wait for pending writes in case
	 * the first write to GICR_CTLR was still in flight (this write only
	 * restores GICR_CTLR.Enable_LPIs and no waiting is required for this
	 * bit).
	 */
	gicr_write_ctlr(gicr_base, rdist_ctx->gicr_ctlr);
	gicr_wait_for_pending_write(gicr_base);
}

/*****************************************************************************
 * Function to save the GIC Distributor register context. This function
 * must be invoked after CPU interface disable and Redistributor save.
 *****************************************************************************/
void gicv3_distif_save(gicv3_dist_ctx_t * const dist_ctx)
{
	unsigned int num_ints;

	assert(gicv3_driver_data);
	assert(gicv3_driver_data->gicd_base);
	assert(IS_IN_EL3());
	assert(dist_ctx);

	uintptr_t gicd_base = gicv3_driver_data->gicd_base;

	num_ints = gicd_read_typer(gicd_base);
	num_ints &= TYPER_IT_LINES_NO_MASK;
	num_ints = (num_ints + 1) << 5;

	assert(num_ints <= MAX_SPI_ID + 1);

	/* Wait for pending write to complete */
	gicd_wait_for_pending_write(gicd_base);

	/* Save the GICD_CTLR */
	dist_ctx->gicd_ctlr = gicd_read_ctlr(gicd_base);

	/* Save GICD_IGROUPR for INTIDs 32 - 1020 */
	SAVE_GICD_REGS(gicd_base, dist_ctx, num_ints, igroupr, IGROUPR);

	/* Save GICD_ISENABLER for INT_IDs 32 - 1020 */
	SAVE_GICD_REGS(gicd_base, dist_ctx, num_ints, isenabler, ISENABLER);

	/* Save GICD_ISPENDR for INTIDs 32 - 1020 */
	SAVE_GICD_REGS(gicd_base, dist_ctx, num_ints, ispendr, ISPENDR);

	/* Save GICD_ISACTIVER for INTIDs 32 - 1020 */
	SAVE_GICD_REGS(gicd_base, dist_ctx, num_ints, isactiver, ISACTIVER);

	/* Save GICD_IPRIORITYR for INTIDs 32 - 1020 */
	SAVE_GICD_REGS(gicd_base, dist_ctx, num_ints, ipriorityr, IPRIORITYR);

	/* Save GICD_ICFGR for INTIDs 32 - 1020 */
	SAVE_GICD_REGS(gicd_base, dist_ctx, num_ints, icfgr, ICFGR);

	/* Save GICD_IGRPMODR for INTIDs 32 - 1020 */
	SAVE_GICD_REGS(gicd_base, dist_ctx, num_ints, igrpmodr, IGRPMODR);

	/* Save GICD_NSACR for INTIDs 32 - 1020 */
	SAVE_GICD_REGS(gicd_base, dist_ctx, num_ints, nsacr, NSACR);

	/* Save GICD_IROUTER for INTIDs 32 - 1024 */
	SAVE_GICD_REGS(gicd_base, dist_ctx, num_ints, irouter, IROUTER);

	/*
	 * GICD_ITARGETSR<n> and GICD_SPENDSGIR<n> are RAZ/WI when
	 * GICD_CTLR.ARE_(S|NS) bits are set which is the case for our GICv3
	 * driver.
	 */
}

/*****************************************************************************
 * Function to restore the GIC Distributor register context. We disable G0, G1S
 * and G1NS interrupt groups before we start restore of the Distributor. This
 * function must be invoked prior to Redistributor restore and CPU interface
 * enable. The pending and active interrupts are restored after the interrupts
 * are fully configured and enabled.
 *****************************************************************************/
void gicv3_distif_init_restore(const gicv3_dist_ctx_t * const dist_ctx)
{
	unsigned int num_ints = 0;

	assert(gicv3_driver_data);
	assert(gicv3_driver_data->gicd_base);
	assert(IS_IN_EL3());
	assert(dist_ctx);

	uintptr_t gicd_base = gicv3_driver_data->gicd_base;

	/*
	 * Clear the "enable" bits for G0/G1S/G1NS interrupts before configuring
	 * the ARE_S bit. The Distributor might generate a system error
	 * otherwise.
	 */
	gicd_clr_ctlr(gicd_base,
		      CTLR_ENABLE_G0_BIT |
		      CTLR_ENABLE_G1S_BIT |
		      CTLR_ENABLE_G1NS_BIT,
		      RWP_TRUE);

	/* Set the ARE_S and ARE_NS bit now that interrupts have been disabled */
	gicd_set_ctlr(gicd_base, CTLR_ARE_S_BIT | CTLR_ARE_NS_BIT, RWP_TRUE);

	num_ints = gicd_read_typer(gicd_base);
	num_ints &= TYPER_IT_LINES_NO_MASK;
	num_ints = (num_ints + 1) << 5;

	assert(num_ints <= MAX_SPI_ID + 1);

	/* Restore GICD_IGROUPR for INTIDs 32 - 1020 */
	RESTORE_GICD_REGS(gicd_base, dist_ctx, num_ints, igroupr, IGROUPR);

	/* Restore GICD_IPRIORITYR for INTIDs 32 - 1020 */
	RESTORE_GICD_REGS(gicd_base, dist_ctx, num_ints, ipriorityr, IPRIORITYR);

	/* Restore GICD_ICFGR for INTIDs 32 - 1020 */
	RESTORE_GICD_REGS(gicd_base, dist_ctx, num_ints, icfgr, ICFGR);

	/* Restore GICD_IGRPMODR for INTIDs 32 - 1020 */
	RESTORE_GICD_REGS(gicd_base, dist_ctx, num_ints, igrpmodr, IGRPMODR);

	/* Restore GICD_NSACR for INTIDs 32 - 1020 */
	RESTORE_GICD_REGS(gicd_base, dist_ctx, num_ints, nsacr, NSACR);

	/* Restore GICD_IROUTER for INTIDs 32 - 1020 */
	RESTORE_GICD_REGS(gicd_base, dist_ctx, num_ints, irouter, IROUTER);

	/*
	 * Restore ISENABLER, ISPENDR and ISACTIVER after the interrupts are
	 * configured.
	 */

	/* Restore GICD_ISENABLER for INT_IDs 32 - 1020 */
	RESTORE_GICD_REGS(gicd_base, dist_ctx, num_ints, isenabler, ISENABLER);

	/* Restore GICD_ISPENDR for INTIDs 32 - 1020 */
	RESTORE_GICD_REGS(gicd_base, dist_ctx, num_ints, ispendr, ISPENDR);

	/* Restore GICD_ISACTIVER for INTIDs 32 - 1020 */
	RESTORE_GICD_REGS(gicd_base, dist_ctx, num_ints, isactiver, ISACTIVER);

	/* Restore the GICD_CTLR */
	gicd_write_ctlr(gicd_base, dist_ctx->gicd_ctlr);
	gicd_wait_for_pending_write(gicd_base);

}

/*******************************************************************************
 * This function gets the priority of the interrupt the processor is currently
 * servicing.
 ******************************************************************************/
unsigned int gicv3_get_running_priority(void)
{
	return read_icc_rpr_el1();
}

/*******************************************************************************
 * This function checks if the interrupt identified by id is active (whether the
 * state is either active, or active and pending). The proc_num is used if the
 * interrupt is SGI or PPI and programs the corresponding Redistributor
 * interface.
 ******************************************************************************/
unsigned int gicv3_get_interrupt_active(unsigned int id, unsigned int proc_num)
{
	unsigned int value;

	assert(gicv3_driver_data);
	assert(gicv3_driver_data->gicd_base);
	assert(proc_num < gicv3_driver_data->rdistif_num);
	assert(gicv3_driver_data->rdistif_base_addrs);
	assert(id <= MAX_SPI_ID);

	if (id < MIN_SPI_ID) {
		/* For SGIs and PPIs */
		value = gicr_get_isactiver0(
				gicv3_driver_data->rdistif_base_addrs[proc_num], id);
	} else {
		value = gicd_get_isactiver(gicv3_driver_data->gicd_base, id);
	}

	return value;
}

/*******************************************************************************
 * This function enables the interrupt identified by id. The proc_num
 * is used if the interrupt is SGI or PPI, and programs the corresponding
 * Redistributor interface.
 ******************************************************************************/
void gicv3_enable_interrupt(unsigned int id, unsigned int proc_num)
{
	assert(gicv3_driver_data);
	assert(gicv3_driver_data->gicd_base);
	assert(proc_num < gicv3_driver_data->rdistif_num);
	assert(gicv3_driver_data->rdistif_base_addrs);
	assert(id <= MAX_SPI_ID);

	/*
	 * Ensure that any shared variable updates depending on out of band
	 * interrupt trigger are observed before enabling interrupt.
	 */
	dsbishst();
	if (id < MIN_SPI_ID) {
		/* For SGIs and PPIs */
		gicr_set_isenabler0(
				gicv3_driver_data->rdistif_base_addrs[proc_num],
				id);
	} else {
		gicd_set_isenabler(gicv3_driver_data->gicd_base, id);
	}
}

/*******************************************************************************
 * This function disables the interrupt identified by id. The proc_num
 * is used if the interrupt is SGI or PPI, and programs the corresponding
 * Redistributor interface.
 ******************************************************************************/
void gicv3_disable_interrupt(unsigned int id, unsigned int proc_num)
{
	assert(gicv3_driver_data);
	assert(gicv3_driver_data->gicd_base);
	assert(proc_num < gicv3_driver_data->rdistif_num);
	assert(gicv3_driver_data->rdistif_base_addrs);
	assert(id <= MAX_SPI_ID);

	/*
	 * Disable interrupt, and ensure that any shared variable updates
	 * depending on out of band interrupt trigger are observed afterwards.
	 */
	if (id < MIN_SPI_ID) {
		/* For SGIs and PPIs */
		gicr_set_icenabler0(
				gicv3_driver_data->rdistif_base_addrs[proc_num],
				id);

		/* Write to clear enable requires waiting for pending writes */
		gicr_wait_for_pending_write(
				gicv3_driver_data->rdistif_base_addrs[proc_num]);
	} else {
		gicd_set_icenabler(gicv3_driver_data->gicd_base, id);

		/* Write to clear enable requires waiting for pending writes */
		gicd_wait_for_pending_write(gicv3_driver_data->gicd_base);
	}

	dsbishst();
}

/*******************************************************************************
 * This function sets the interrupt priority as supplied for the given interrupt
 * id.
 ******************************************************************************/
void gicv3_set_interrupt_priority(unsigned int id, unsigned int proc_num,
		unsigned int priority)
{
	uintptr_t gicr_base;

	assert(gicv3_driver_data);
	assert(gicv3_driver_data->gicd_base);
	assert(proc_num < gicv3_driver_data->rdistif_num);
	assert(gicv3_driver_data->rdistif_base_addrs);
	assert(id <= MAX_SPI_ID);

	if (id < MIN_SPI_ID) {
		gicr_base = gicv3_driver_data->rdistif_base_addrs[proc_num];
		gicr_set_ipriorityr(gicr_base, id, priority);
	} else {
		gicd_set_ipriorityr(gicv3_driver_data->gicd_base, id, priority);
	}
}

/*******************************************************************************
 * This function assigns group for the interrupt identified by id. The proc_num
 * is used if the interrupt is SGI or PPI, and programs the corresponding
 * Redistributor interface. The group can be any of GICV3_INTR_GROUP*
 ******************************************************************************/
void gicv3_set_interrupt_type(unsigned int id, unsigned int proc_num,
		unsigned int type)
{
	unsigned int igroup = 0, grpmod = 0;
	uintptr_t gicr_base;

	assert(gicv3_driver_data);
	assert(gicv3_driver_data->gicd_base);
	assert(proc_num < gicv3_driver_data->rdistif_num);
	assert(gicv3_driver_data->rdistif_base_addrs);

	switch (type) {
	case INTR_GROUP1S:
		igroup = 0;
		grpmod = 1;
		break;
	case INTR_GROUP0:
		igroup = 0;
		grpmod = 0;
		break;
	case INTR_GROUP1NS:
		igroup = 1;
		grpmod = 0;
		break;
	default:
		assert(0);
	}

	if (id < MIN_SPI_ID) {
		gicr_base = gicv3_driver_data->rdistif_base_addrs[proc_num];
		if (igroup)
			gicr_set_igroupr0(gicr_base, id);
		else
			gicr_clr_igroupr0(gicr_base, id);

		if (grpmod)
			gicr_set_igrpmodr0(gicr_base, id);
		else
			gicr_clr_igrpmodr0(gicr_base, id);
	} else {
		/* Serialize read-modify-write to Distributor registers */
		spin_lock(&gic_lock);
		if (igroup)
			gicd_set_igroupr(gicv3_driver_data->gicd_base, id);
		else
			gicd_clr_igroupr(gicv3_driver_data->gicd_base, id);

		if (grpmod)
			gicd_set_igrpmodr(gicv3_driver_data->gicd_base, id);
		else
			gicd_clr_igrpmodr(gicv3_driver_data->gicd_base, id);
		spin_unlock(&gic_lock);
	}
}

/*******************************************************************************
 * This function raises the specified Secure Group 0 SGI.
 *
 * The target parameter must be a valid MPIDR in the system.
 ******************************************************************************/
void gicv3_raise_secure_g0_sgi(int sgi_num, u_register_t target)
{
	unsigned int tgt, aff3, aff2, aff1, aff0;
	uint64_t sgi_val;

	/* Verify interrupt number is in the SGI range */
	assert((sgi_num >= MIN_SGI_ID) && (sgi_num < MIN_PPI_ID));

	/* Extract affinity fields from target */
	aff0 = MPIDR_AFFLVL0_VAL(target);
	aff1 = MPIDR_AFFLVL1_VAL(target);
	aff2 = MPIDR_AFFLVL2_VAL(target);
	aff3 = MPIDR_AFFLVL3_VAL(target);

	/*
	 * Make target list from affinity 0, and ensure GICv3 SGI can target
	 * this PE.
	 */
	assert(aff0 < GICV3_MAX_SGI_TARGETS);
	tgt = BIT(aff0);

	/* Raise SGI to PE specified by its affinity */
	sgi_val = GICV3_SGIR_VALUE(aff3, aff2, aff1, sgi_num, SGIR_IRM_TO_AFF,
			tgt);

	/*
	 * Ensure that any shared variable updates depending on out of band
	 * interrupt trigger are observed before raising SGI.
	 */
	dsbishst();
	write_icc_sgi0r_el1(sgi_val);
	isb();
}

/*******************************************************************************
 * This function sets the interrupt routing for the given SPI interrupt id.
 * The interrupt routing is specified in routing mode and mpidr.
 *
 * The routing mode can be either of:
 *  - GICV3_IRM_ANY
 *  - GICV3_IRM_PE
 *
 * The mpidr is the affinity of the PE to which the interrupt will be routed,
 * and is ignored for routing mode GICV3_IRM_ANY.
 ******************************************************************************/
void gicv3_set_spi_routing(unsigned int id, unsigned int irm, u_register_t mpidr)
{
	unsigned long long aff;
	uint64_t router;

	assert(gicv3_driver_data);
	assert(gicv3_driver_data->gicd_base);

	assert((irm == GICV3_IRM_ANY) || (irm == GICV3_IRM_PE));
	assert(id >= MIN_SPI_ID && id <= MAX_SPI_ID);

	aff = gicd_irouter_val_from_mpidr(mpidr, irm);
	gicd_write_irouter(gicv3_driver_data->gicd_base, id, aff);

	/*
	 * In implementations that do not require 1 of N distribution of SPIs,
	 * IRM might be RAZ/WI. Read back and verify IRM bit.
	 */
	if (irm == GICV3_IRM_ANY) {
		router = gicd_read_irouter(gicv3_driver_data->gicd_base, id);
		if (!((router >> IROUTER_IRM_SHIFT) & IROUTER_IRM_MASK)) {
			ERROR("GICv3 implementation doesn't support routing ANY\n");
			panic();
		}
	}
}

/*******************************************************************************
 * This function clears the pending status of an interrupt identified by id.
 * The proc_num is used if the interrupt is SGI or PPI, and programs the
 * corresponding Redistributor interface.
 ******************************************************************************/
void gicv3_clear_interrupt_pending(unsigned int id, unsigned int proc_num)
{
	assert(gicv3_driver_data);
	assert(gicv3_driver_data->gicd_base);
	assert(proc_num < gicv3_driver_data->rdistif_num);
	assert(gicv3_driver_data->rdistif_base_addrs);

	/*
	 * Clear pending interrupt, and ensure that any shared variable updates
	 * depending on out of band interrupt trigger are observed afterwards.
	 */
	if (id < MIN_SPI_ID) {
		/* For SGIs and PPIs */
		gicr_set_icpendr0(gicv3_driver_data->rdistif_base_addrs[proc_num],
				id);
	} else {
		gicd_set_icpendr(gicv3_driver_data->gicd_base, id);
	}
	dsbishst();
}

/*******************************************************************************
 * This function sets the pending status of an interrupt identified by id.
 * The proc_num is used if the interrupt is SGI or PPI and programs the
 * corresponding Redistributor interface.
 ******************************************************************************/
void gicv3_set_interrupt_pending(unsigned int id, unsigned int proc_num)
{
	assert(gicv3_driver_data);
	assert(gicv3_driver_data->gicd_base);
	assert(proc_num < gicv3_driver_data->rdistif_num);
	assert(gicv3_driver_data->rdistif_base_addrs);

	/*
	 * Ensure that any shared variable updates depending on out of band
	 * interrupt trigger are observed before setting interrupt pending.
	 */
	dsbishst();
	if (id < MIN_SPI_ID) {
		/* For SGIs and PPIs */
		gicr_set_ispendr0(gicv3_driver_data->rdistif_base_addrs[proc_num],
				id);
	} else {
		gicd_set_ispendr(gicv3_driver_data->gicd_base, id);
	}
}

/*******************************************************************************
 * This function sets the PMR register with the supplied value. Returns the
 * original PMR.
 ******************************************************************************/
unsigned int gicv3_set_pmr(unsigned int mask)
{
	unsigned int old_mask;

	old_mask = read_icc_pmr_el1();

	/*
	 * Order memory updates w.r.t. PMR write, and ensure they're visible
	 * before potential out of band interrupt trigger because of PMR update.
	 * PMR system register writes are self-synchronizing, so no ISB required
	 * thereafter.
	 */
	dsbishst();
	write_icc_pmr_el1(mask);

	return old_mask;
}
