/*
 * Copyright (c) 2013, ARM Limited. All rights reserved.
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

#include <string.h>
#include <assert.h>
#include <arch_helpers.h>
#include <platform.h>
#include <bl31.h>
#include <bl_common.h>
#include <pl011.h>
#include <bakery_lock.h>
#include <cci400.h>
#include <gic.h>
#include <fvp_pwrc.h>

/*******************************************************************************
 * Declarations of linker defined symbols which will help us find the layout
 * of trusted SRAM
 ******************************************************************************/
#if defined (__GNUC__)
extern unsigned long __BL31_RO_BASE__;
extern unsigned long __BL31_STACKS_BASE__;
extern unsigned long __BL31_COHERENT_RAM_BASE__;
extern unsigned long __BL31_RW_BASE__;

#define BL31_RO_BASE		__BL31_RO_BASE__
#define BL31_STACKS_BASE	__BL31_STACKS_BASE__
#define BL31_COHERENT_RAM_BASE	__BL31_COHERENT_RAM_BASE__
#define BL31_RW_BASE		__BL31_RW_BASE__

#else
 #error "Unknown compiler."
#endif

/*******************************************************************************
 * This data structures holds information copied by BL31 from BL2 to pass
 * control to the non-trusted software images. A per-cpu entry was created to
 * use the same structure in the warm boot path but that's not the case right
 * now. Persisting with this approach for the time being. TODO: Can this be
 * moved out of device memory.
 ******************************************************************************/
el_change_info ns_entry_info[PLATFORM_CORE_COUNT]
__attribute__ ((aligned(PLATFORM_CACHE_LINE_SIZE),
		section("tzfw_coherent_mem"))) = {0};

/* Data structure which holds the extents of the trusted SRAM for BL31 */
static meminfo bl31_tzram_layout
__attribute__ ((aligned(PLATFORM_CACHE_LINE_SIZE),
		section("tzfw_coherent_mem"))) = {0};

meminfo bl31_get_sec_mem_layout(void)
{
	return bl31_tzram_layout;
}

/*******************************************************************************
 * Return information about passing control to the non-trusted software images
 * to common code.TODO: In the initial architecture, the image after BL31 will
 * always run in the non-secure state. In the final architecture there
 * will be a series of images. This function will need enhancement then
 ******************************************************************************/
el_change_info *bl31_get_next_image_info(unsigned long mpidr)
{
	return &ns_entry_info[platform_get_core_pos(mpidr)];
}

/*******************************************************************************
 * Perform any BL31 specific platform actions. Here we copy parameters passed
 * by the calling EL (S-EL1 in BL2 & S-EL3 in BL1) before they are lost
 * (potentially). This is done before the MMU is initialized so that the memory
 * layout can be used while creating page tables.
 ******************************************************************************/
void bl31_early_platform_setup(meminfo *mem_layout,
			       void *data,
			       unsigned long mpidr)
{
	el_change_info *image_info = (el_change_info *) data;
	unsigned int lin_index = platform_get_core_pos(mpidr);

	/* Setup the BL31 memory layout */
	bl31_tzram_layout.total_base = mem_layout->total_base;
	bl31_tzram_layout.total_size = mem_layout->total_size;
	bl31_tzram_layout.free_base = mem_layout->free_base;
	bl31_tzram_layout.free_size = mem_layout->free_size;
	bl31_tzram_layout.attr = mem_layout->attr;
	bl31_tzram_layout.next = 0;

	/* Save information about jumping into the NS world */
	ns_entry_info[lin_index].entrypoint = image_info->entrypoint;
	ns_entry_info[lin_index].spsr = image_info->spsr;
	ns_entry_info[lin_index].args = image_info->args;
	ns_entry_info[lin_index].security_state = image_info->security_state;
	ns_entry_info[lin_index].next = image_info->next;

	/* Initialize the platform config for future decision making */
	platform_config_setup();
}

/*******************************************************************************
 * Initialize the gic, configure the CLCD and zero out variables needed by the
 * secondaries to boot up correctly.
 ******************************************************************************/
void bl31_platform_setup()
{
	unsigned int reg_val;

        /* Initialize the gic cpu and distributor interfaces */
        gic_setup();

	/*
	 * TODO: Configure the CLCD before handing control to
	 * linux. Need to see if a separate driver is needed
	 * instead.
	 */
	mmio_write_32(VE_SYSREGS_BASE + V2M_SYS_CFGDATA, 0);
	mmio_write_32(VE_SYSREGS_BASE + V2M_SYS_CFGCTRL,
		      (1ull << 31) | (1 << 30) | (7 << 20) | (0 << 16));

	/* Allow access to the System counter timer module */
	reg_val = (1 << CNTACR_RPCT_SHIFT) | (1 << CNTACR_RVCT_SHIFT);
	reg_val |= (1 << CNTACR_RFRQ_SHIFT) | (1 << CNTACR_RVOFF_SHIFT);
	reg_val |= (1 << CNTACR_RWVT_SHIFT) | (1 << CNTACR_RWPT_SHIFT);
	mmio_write_32(SYS_TIMCTL_BASE + CNTACR_BASE(0), reg_val);
	mmio_write_32(SYS_TIMCTL_BASE + CNTACR_BASE(1), reg_val);

	reg_val = (1 << CNTNSAR_NS_SHIFT(0)) | (1 << CNTNSAR_NS_SHIFT(1));
	mmio_write_32(SYS_TIMCTL_BASE + CNTNSAR, reg_val);

	/* Intialize the power controller */
	fvp_pwrc_setup();

        /* Topologies are best known to the platform. */
	plat_setup_topology();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bl31_plat_arch_setup()
{
	unsigned long sctlr;

	/* Enable instruction cache. */
	sctlr = read_sctlr();
	sctlr |= SCTLR_I_BIT;
	write_sctlr(sctlr);

	write_vbar((unsigned long) runtime_exceptions);
	configure_mmu(&bl31_tzram_layout,
		      (unsigned long) &BL31_RO_BASE,
		      (unsigned long) &BL31_STACKS_BASE,
		      (unsigned long) &BL31_COHERENT_RAM_BASE,
		      (unsigned long) &BL31_RW_BASE);
}

/*******************************************************************************
 * TODO: Move GIC setup to a separate file in case it is needed by other BL
 * stages or ELs
 * TODO: Revisit if priorities are being set such that no non-secure interrupt
 * can have a higher priority than a secure one as recommended in the GICv2 spec
 *******************************************************************************/

/*******************************************************************************
 * This function does some minimal GICv3 configuration. The Firmware itself does
 * not fully support GICv3 at this time and relies on GICv2 emulation as
 * provided by GICv3. This function allows software (like Linux) in later stages
 * to use full GICv3 features.
 *******************************************************************************/
void gicv3_cpuif_setup(void)
{
	unsigned int scr_val, val, base;

	/*
	 * When CPUs come out of reset they have their GICR_WAKER.ProcessorSleep
	 * bit set. In order to allow interrupts to get routed to the CPU we
	 * need to clear this bit if set and wait for GICR_WAKER.ChildrenAsleep
	 * to clear (GICv3 Architecture specification 5.4.23).
	 * GICR_WAKER is NOT banked per CPU, compute the correct base address
	 * per CPU.
	 *
	 * TODO:
	 * For GICv4 we also need to adjust the Base address based on
	 * GICR_TYPER.VLPIS
	 */
	base = BASE_GICR_BASE +
		(platform_get_core_pos(read_mpidr()) << GICR_PCPUBASE_SHIFT);
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

	write_icc_pmr_el1(MAX_PRI_VAL);

	/* Restore SCR_EL3 */
	write_scr(scr_val);
}

/*******************************************************************************
 * This function does some minimal GICv3 configuration when cores go
 * down.
 *******************************************************************************/
void gicv3_cpuif_deactivate(void)
{
	unsigned int val, base;

	/*
	 * When taking CPUs down we need to set GICR_WAKER.ProcessorSleep and
	 * wait for GICR_WAKER.ChildrenAsleep to get set.
	 * (GICv3 Architecture specification 5.4.23).
	 * GICR_WAKER is NOT banked per CPU, compute the correct base address
	 * per CPU.
	 *
	 * TODO:
	 * For GICv4 we also need to adjust the Base address based on
	 * GICR_TYPER.VLPIS
	 */
	base = BASE_GICR_BASE +
		(platform_get_core_pos(read_mpidr()) << GICR_PCPUBASE_SHIFT);
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

	gicc_write_pmr(gicc_base, MAX_PRI_VAL);
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

	gicd_set_ipriorityr(gicd_base, IRQ_SEC_PHY_TIMER, MAX_PRI_VAL);
	gicd_set_ipriorityr(gicd_base, IRQ_SEC_SGI_0, MAX_PRI_VAL);
	gicd_set_ipriorityr(gicd_base, IRQ_SEC_SGI_1, MAX_PRI_VAL);
	gicd_set_ipriorityr(gicd_base, IRQ_SEC_SGI_2, MAX_PRI_VAL);
	gicd_set_ipriorityr(gicd_base, IRQ_SEC_SGI_3, MAX_PRI_VAL);
	gicd_set_ipriorityr(gicd_base, IRQ_SEC_SGI_4, MAX_PRI_VAL);
	gicd_set_ipriorityr(gicd_base, IRQ_SEC_SGI_5, MAX_PRI_VAL);
	gicd_set_ipriorityr(gicd_base, IRQ_SEC_SGI_6, MAX_PRI_VAL);
	gicd_set_ipriorityr(gicd_base, IRQ_SEC_SGI_7, MAX_PRI_VAL);

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
	gicd_set_ipriorityr(gicd_base, IRQ_TZ_WDOG, MAX_PRI_VAL);
	gicd_set_itargetsr(gicd_base, IRQ_TZ_WDOG,
			   platform_get_core_pos(read_mpidr()));
	gicd_set_isenabler(gicd_base, IRQ_TZ_WDOG);
	gic_pcpu_distif_setup(gicd_base);

	gicd_write_ctlr(gicd_base, ctlr | ENABLE_GRP0);
}

void gic_setup(void)
{
	unsigned int gicd_base, gicc_base;

	gicd_base = platform_get_cfgvar(CONFIG_GICD_ADDR);
	gicc_base = platform_get_cfgvar(CONFIG_GICC_ADDR);

	gic_cpuif_setup(gicc_base);
	gic_distif_setup(gicd_base);
}
