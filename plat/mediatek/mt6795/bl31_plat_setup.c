/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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
#include <arm_gic.h>
#include <assert.h>
#include <arch_helpers.h>
#include <bl_common.h>
#include <cci.h>
#include <console.h>
#include <context_mgmt.h>
#include <debug.h>
#include <mcucfg.h>
#include <mmio.h>
#include <mtk_sip_svc.h>
#include <mtk_plat_common.h>
#include <mt_cpuxgpt.h>
#include <platform.h>
#include <plat_private.h>
#include <string.h>
#include <xlat_tables.h>
/*******************************************************************************
 * Declarations of linker defined symbols which will help us find the layout
 * of trusted SRAM
 ******************************************************************************/
unsigned long __RO_START__;
unsigned long __RO_END__;

unsigned long __COHERENT_RAM_START__;
unsigned long __COHERENT_RAM_END__;

/*
 * The next 2 constants identify the extents of the code & RO data region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __RO_START__ and __RO_END__ linker symbols refer to page-aligned addresses.
 */
#define BL31_RO_BASE (unsigned long)(&__RO_START__)
#define BL31_RO_LIMIT (unsigned long)(&__RO_END__)

/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols
 * refer to page-aligned addresses.
 */
#define BL31_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL31_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)

/*
 * Placeholder variables for copying the arguments that have been passed to
 * BL3-1 from BL2.
 */
static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

static const int cci_map[] = {
	PLAT_MT_CCI_CLUSTER0_SL_IFACE_IX,
	PLAT_MT_CCI_CLUSTER1_SL_IFACE_IX
};

static uint32_t cci_map_length = ARRAY_SIZE(cci_map);

/* Table of regions to map using the MMU.  */
static const mmap_region_t plat_mmap[] = {
	/* for TF text, RO, RW */
	MAP_REGION_FLAT(MTK_DEV_RNG0_BASE, MTK_DEV_RNG0_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(MTK_DEV_RNG1_BASE, MTK_DEV_RNG1_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(RAM_CONSOLE_BASE & ~(PAGE_SIZE_MASK), RAM_CONSOLE_SIZE,
						MT_DEVICE | MT_RW | MT_NS),
	{ 0 }

};

/*******************************************************************************
 * Macro generating the code for the function setting up the pagetables as per
 * the platform memory map & initialize the mmu, for the given exception level
 ******************************************************************************/
#define DEFINE_CONFIGURE_MMU_EL(_el)					\
	void plat_configure_mmu_el ## _el(unsigned long total_base,	\
				unsigned long total_size,	\
				unsigned long ro_start,	\
				unsigned long ro_limit,	\
				unsigned long coh_start,	\
				unsigned long coh_limit)	\
	{								\
		mmap_add_region(total_base, total_base,			\
				total_size,				\
				MT_MEMORY | MT_RW | MT_SECURE);		\
		mmap_add_region(ro_start, ro_start,			\
				ro_limit - ro_start,			\
				MT_MEMORY | MT_RO | MT_SECURE);		\
		mmap_add_region(coh_start, coh_start,			\
				coh_limit - coh_start,			\
				MT_DEVICE | MT_RW | MT_SECURE);		\
		mmap_add(plat_mmap);					\
		init_xlat_tables();					\
									\
		enable_mmu_el ## _el(0);				\
	}

/* Define EL3 variants of the function initialising the MMU */
DEFINE_CONFIGURE_MMU_EL(3)

unsigned long long plat_get_syscnt_freq(void)
{
	return SYS_COUNTER_FREQ_IN_TICKS;
}

void plat_cci_init(void)
{
	/* Initialize CCI driver */
	cci_init(PLAT_MT_CCI_BASE, cci_map, cci_map_length);
}

void plat_cci_enable(void)
{
	/*
	 * Enable CCI coherency for this cluster.
	 * No need for locks as no other cpu is active at the moment.
	 */
	cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr()));
}

void plat_cci_disable(void)
{
	cci_disable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr()));
}


static void platform_setup_cpu(void)
{
	/* setup big cores */
	mmio_write_32((uintptr_t)&mt6795_mcucfg->mp1_config_res,
		MP1_DIS_RGU0_WAIT_PD_CPUS_L1_ACK |
		MP1_DIS_RGU1_WAIT_PD_CPUS_L1_ACK |
		MP1_DIS_RGU2_WAIT_PD_CPUS_L1_ACK |
		MP1_DIS_RGU3_WAIT_PD_CPUS_L1_ACK |
		MP1_DIS_RGU_NOCPU_WAIT_PD_CPUS_L1_ACK);
	mmio_setbits_32((uintptr_t)&mt6795_mcucfg->mp1_miscdbg, MP1_AINACTS);
	mmio_setbits_32((uintptr_t)&mt6795_mcucfg->mp1_clkenm_div,
		MP1_SW_CG_GEN);
	mmio_clrbits_32((uintptr_t)&mt6795_mcucfg->mp1_rst_ctl,
		MP1_L2RSTDISABLE);

	/* set big cores arm64 boot mode */
	mmio_setbits_32((uintptr_t)&mt6795_mcucfg->mp1_cpucfg,
		MP1_CPUCFG_64BIT);

	/* set LITTLE cores arm64 boot mode */
	mmio_setbits_32((uintptr_t)&mt6795_mcucfg->mp0_rv_addr[0].rv_addr_hw,
		MP0_CPUCFG_64BIT);
}

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * the security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	next_image_info = (type == NON_SECURE) ?
			&bl33_image_ep_info : &bl32_image_ep_info;

	/* None of the images on this platform can have 0x0 as the entrypoint */
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}

/*******************************************************************************
 * Perform any BL3-1 early platform setup. Here is an opportunity to copy
 * parameters passed by the calling EL (S-EL1 in BL2 & S-EL3 in BL1) before they
 * are lost (potentially). This needs to be done before the MMU is initialized
 * so that the memory layout can be used while creating page tables.
 * BL2 has flushed this information to memory, so we are guaranteed to pick up
 * good data.
 ******************************************************************************/
void bl31_early_platform_setup(bl31_params_t *from_bl2,
						 void *plat_params_from_bl2)
{
	struct mtk_bl_param_t *pmtk_bl_param = (struct mtk_bl_param_t *)from_bl2;
	struct atf_arg_t *teearg;
	unsigned long long normal_base;
	unsigned long long atf_base;

	assert(from_bl2 != NULL);
	/*
	 * Mediatek preloader(i.e, BL2) is in 32 bit state, high 32bits
	 * of 64 bit GP registers are UNKNOWN if CPU warm reset from 32 bit
	 * to 64 bit state. So we need to clear high 32bit,
	 * which may be random value.
	 */
	pmtk_bl_param =
	(struct mtk_bl_param_t *)((uint64_t)pmtk_bl_param & 0x00000000ffffffff);
	plat_params_from_bl2 =
	(void *)((uint64_t)plat_params_from_bl2 & 0x00000000ffffffff);

	teearg  = (struct atf_arg_t *)pmtk_bl_param->tee_info_addr;

	console_init(teearg->atf_log_port, UART_CLOCK, UART_BAUDRATE);
	memcpy((void *)&gteearg, (void *)teearg, sizeof(struct atf_arg_t));

	normal_base = 0;
    /* in ATF boot time, timer for cntpct_el0 is not initialized
     * so it will not count now.
     */
	atf_base = read_cntpct_el0();
	sched_clock_init(normal_base, atf_base);

	VERBOSE("bl31_setup\n");

	/* Populate entry point information for BL3-2 and BL3-3 */
	SET_PARAM_HEAD(&bl32_image_ep_info,
				PARAM_EP,
				VERSION_1,
				0);
	SET_SECURITY_STATE(bl32_image_ep_info.h.attr, SECURE);
	bl32_image_ep_info.pc = BL32_BASE;

	SET_PARAM_HEAD(&bl33_image_ep_info,
				PARAM_EP,
				VERSION_1,
				0);
	/*
	 * Tell BL3-1 where the non-trusted software image
	 * is located and the entry state information
	 */
	/* BL33_START_ADDRESS */
	bl33_image_ep_info.pc = pmtk_bl_param->bl33_start_addr;
	bl33_image_ep_info.spsr = plat_get_spsr_for_bl33_entry();
	bl33_image_ep_info.args.arg4 =  pmtk_bl_param->bootarg_loc;
	bl33_image_ep_info.args.arg5 =  pmtk_bl_param->bootarg_size;
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);
}
/*******************************************************************************
 * Perform any BL3-1 platform setup code
 ******************************************************************************/

void bl31_platform_setup(void)
{
	platform_setup_cpu();

	plat_delay_timer_init();

	plat_mt_gic_driver_init();
	/* Initialize the gic cpu and distributor interfaces */
	plat_mt_gic_init();

	/* Topologies are best known to the platform. */
	mt_setup_topology();
}
/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the mmu in a quick and dirty way.
 * Init MTK propiartary log buffer control field.
 ******************************************************************************/
void bl31_plat_arch_setup(void)
{
	/* Enable non-secure access to CCI-400 registers */
	mmio_write_32(CCI400_BASE + CCI_SEC_ACCESS_OFFSET , 0x1);

	plat_cci_init();
	plat_cci_enable();

	if (gteearg.atf_log_buf_size != 0) {
		INFO("mmap atf buffer : 0x%x, 0x%x\n\r",
			gteearg.atf_log_buf_start,
			gteearg.atf_log_buf_size);

		mmap_add_region(
			gteearg.atf_log_buf_start &
			~(PAGE_SIZE_2MB_MASK),
			gteearg.atf_log_buf_start &
			~(PAGE_SIZE_2MB_MASK),
			PAGE_SIZE_2MB,
			MT_DEVICE | MT_RW | MT_NS);

		INFO("mmap atf buffer (force 2MB aligned):0x%x, 0x%x\n",
			(gteearg.atf_log_buf_start & ~(PAGE_SIZE_2MB_MASK)),
		PAGE_SIZE_2MB);
	}
	/*
	 * add TZRAM_BASE to memory map
	 * then set RO and COHERENT to different attribute
	 */
	plat_configure_mmu_el3(
		(TZRAM_BASE & ~(PAGE_SIZE_MASK)),
		(TZRAM_SIZE & ~(PAGE_SIZE_MASK)),
		(BL31_RO_BASE & ~(PAGE_SIZE_MASK)),
		BL31_RO_LIMIT,
		BL31_COHERENT_RAM_BASE,
		BL31_COHERENT_RAM_LIMIT);
	/* Initialize for ATF log buffer */
	if (gteearg.atf_log_buf_size != 0) {
		gteearg.atf_aee_debug_buf_size = ATF_AEE_BUFFER_SIZE;
		gteearg.atf_aee_debug_buf_start =
			gteearg.atf_log_buf_start +
			gteearg.atf_log_buf_size - ATF_AEE_BUFFER_SIZE;
		INFO("ATF log service is registered (0x%x, aee:0x%x)\n",
			gteearg.atf_log_buf_start,
			gteearg.atf_aee_debug_buf_start);
	} else{
		gteearg.atf_aee_debug_buf_size = 0;
		gteearg.atf_aee_debug_buf_start = 0;
	}

	/*
	 * Without this, access to CPUECTRL from NS EL1
	 * will cause trap into EL3
	 */
	enable_ns_access_to_cpuectlr();

	/* Platform code before bl31_main */
	/* compatible to the earlier chipset */

	/* Show to ATF log buffer & UART */
	INFO("BL3-1: %s\n", version_string);
	INFO("BL3-1: %s\n", build_message);

}

/* MTK Define */
#define ACTLR_CPUECTLR_BIT    (1 << 1)

void enable_ns_access_to_cpuectlr(void)
{
	unsigned int next_actlr;


	/* ACTLR_EL1 do not implement CUPECTLR  */
	next_actlr = read_actlr_el2();
	next_actlr |= ACTLR_CPUECTLR_BIT;
	write_actlr_el2(next_actlr);

	next_actlr = read_actlr_el3();
	next_actlr |= ACTLR_CPUECTLR_BIT;
	write_actlr_el3(next_actlr);
}

/*******************************************************************************
 * This function prepare boot argument for 64 bit kernel entry
 ******************************************************************************/
static entry_point_info_t *bl31_plat_get_next_kernel64_ep_info(void)
{
	entry_point_info_t *next_image_info;
	unsigned long el_status;
	unsigned int mode;

	el_status = 0;
	mode = 0;

	/* Kernel image is always non-secured */
	next_image_info = &bl33_image_ep_info;

	/* Figure out what mode we enter the non-secure world in */
	el_status = read_id_aa64pfr0_el1() >> ID_AA64PFR0_EL2_SHIFT;
	el_status &= ID_AA64PFR0_ELX_MASK;

	if (el_status) {
		INFO("Kernel_EL2\n");
		mode = MODE_EL2;
	} else{
		INFO("Kernel_EL1\n");
		mode = MODE_EL1;
	}

	INFO("Kernel is 64Bit\n");
	next_image_info->spsr =
		SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	next_image_info->pc = get_kernel_info_pc();
	next_image_info->args.arg0 = get_kernel_info_r0();
	next_image_info->args.arg1 = get_kernel_info_r1();

	INFO("pc=0x%lx, r0=0x%lx, r1=0x%lx\n",
				 next_image_info->pc,
				 next_image_info->args.arg0,
				 next_image_info->args.arg1);


	SET_SECURITY_STATE(next_image_info->h.attr, NON_SECURE);

	/* None of the images on this platform can have 0x0 as the entrypoint */
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}

/*******************************************************************************
 * This function prepare boot argument for 32 bit kernel entry
 ******************************************************************************/
static entry_point_info_t *bl31_plat_get_next_kernel32_ep_info(void)
{
	entry_point_info_t *next_image_info;
	unsigned int mode;

	mode = 0;

	/* Kernel image is always non-secured */
	next_image_info = &bl33_image_ep_info;

	/* Figure out what mode we enter the non-secure world in */
	mode = MODE32_hyp;
	/*
	* TODO: Consider the possibility of specifying the SPSR in
	* the FIP ToC and allowing the platform to have a say as
	* well.
	*/

	INFO("Kernel is 32Bit\n");
	next_image_info->spsr =
		SPSR_MODE32(mode, SPSR_T_ARM, SPSR_E_LITTLE,
		(DAIF_FIQ_BIT | DAIF_IRQ_BIT | DAIF_ABT_BIT));
	next_image_info->pc = get_kernel_info_pc();
	next_image_info->args.arg0 = get_kernel_info_r0();
	next_image_info->args.arg1 = get_kernel_info_r1();
	next_image_info->args.arg2 = get_kernel_info_r2();

	INFO("pc=0x%lx, r0=0x%lx, r1=0x%lx, r2=0x%lx\n",
				 next_image_info->pc,
				 next_image_info->args.arg0,
				 next_image_info->args.arg1,
				 next_image_info->args.arg2);


	SET_SECURITY_STATE(next_image_info->h.attr, NON_SECURE);

	/* None of the images on this platform can have 0x0 as the entrypoint */
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}

/*******************************************************************************
 * This function prepare boot argument for kernel entrypoint
 ******************************************************************************/
void bl31_prepare_kernel_entry(uint64_t k32_64)
{
	entry_point_info_t *next_image_info;
	uint32_t image_type;

	/* Determine which image to execute next */
	/* image_type = bl31_get_next_image_type(); */
	image_type = NON_SECURE;

	/* Program EL3 registers to enable entry into the next EL */
	if (0 == k32_64)
		next_image_info = bl31_plat_get_next_kernel32_ep_info();
	else
		next_image_info = bl31_plat_get_next_kernel64_ep_info();

	assert(next_image_info);
	assert(image_type == GET_SECURITY_STATE(next_image_info->h.attr));

	INFO("BL3-1: Preparing for EL3 exit to %s world, Kernel\n",
		(image_type == SECURE) ? "secure" : "normal");
	INFO("BL3-1: Next image address = 0x%llx\n",
		(unsigned long long) next_image_info->pc);
	INFO("BL3-1: Next image spsr = 0x%x\n", next_image_info->spsr);
	cm_init_context(read_mpidr_el1(), next_image_info);
	cm_prepare_el3_exit(image_type);
}
