/*
 * Copyright (c) 2019-2022 ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <context.h>
#include <drivers/arm/tzc380.h>
#include <drivers/console.h>
#include <drivers/generic_delay_timer.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>

#include <dram.h>
#include <gpc.h>
#include <imx_aipstz.h>
#include <imx_uart.h>
#include <imx_rdc.h>
#include <imx8m_caam.h>
#include <imx8m_ccm.h>
#include <imx8m_csu.h>
#include <imx8m_snvs.h>
#include <plat_imx8.h>

#define TRUSTY_PARAMS_LEN_BYTES      (4096*2)

/*
 * Note: DRAM region is mapped with entire size available and uses MT_RW
 * attributes.
 * See details in docs/plat/imx8m.rst "High Assurance Boot (HABv4)" section
 * for explanation of this mapping scheme.
 */
static const mmap_region_t imx_mmap[] = {
	MAP_REGION_FLAT(IMX_GIC_BASE, IMX_GIC_SIZE, MT_DEVICE | MT_RW),
	MAP_REGION_FLAT(IMX_AIPS_BASE, IMX_AIPS_SIZE, MT_DEVICE | MT_RW), /* AIPS map */
	MAP_REGION_FLAT(OCRAM_S_BASE, OCRAM_S_SIZE, MT_DEVICE | MT_RW), /* OCRAM_S */
	MAP_REGION_FLAT(IMX_DDRPHY_BASE, IMX_DDR_IPS_SIZE, MT_DEVICE | MT_RW), /* DDRMIX */
	MAP_REGION_FLAT(IMX_VPUMIX_BASE, IMX_VPUMIX_SIZE, MT_DEVICE | MT_RW), /* VPUMIX */
	MAP_REGION_FLAT(IMX_CAAM_RAM_BASE, IMX_CAAM_RAM_SIZE, MT_MEMORY | MT_RW), /* CAMM RAM */
	MAP_REGION_FLAT(IMX_NS_OCRAM_BASE, IMX_NS_OCRAM_SIZE, MT_MEMORY | MT_RW), /* NS OCRAM */
	MAP_REGION_FLAT(IMX_ROM_BASE, IMX_ROM_SIZE, MT_MEMORY | MT_RO), /* ROM code */
	MAP_REGION_FLAT(IMX_DRAM_BASE, IMX_DRAM_SIZE, MT_MEMORY | MT_RW | MT_NS), /* DRAM */
	{0},
};

static const struct aipstz_cfg aipstz[] = {
	{IMX_AIPSTZ1, 0x77777777, 0x77777777, .opacr = {0x0, 0x0, 0x0, 0x0, 0x0}, },
	{IMX_AIPSTZ2, 0x77777777, 0x77777777, .opacr = {0x0, 0x0, 0x0, 0x0, 0x0}, },
	{IMX_AIPSTZ3, 0x77777777, 0x77777777, .opacr = {0x0, 0x0, 0x0, 0x0, 0x0}, },
	{IMX_AIPSTZ4, 0x77777777, 0x77777777, .opacr = {0x0, 0x0, 0x0, 0x0, 0x0}, },
	{0},
};

static const struct imx_rdc_cfg rdc[] = {
	/* Master domain assignment */
	RDC_MDAn(RDC_MDA_M4, DID1),

	/* peripherals domain permission */
	RDC_PDAPn(RDC_PDAP_UART4, D1R | D1W),
	RDC_PDAPn(RDC_PDAP_UART2, D0R | D0W),

	/* memory region */

	/* Sentinel */
	{0},
};

static const struct imx_csu_cfg csu_cfg[] = {
	/* peripherals csl setting */
	CSU_CSLx(CSU_CSL_RDC, CSU_SEC_LEVEL_3, LOCKED),
	CSU_CSLx(CSU_CSL_TZASC, CSU_SEC_LEVEL_5, LOCKED),
	CSU_CSLx(CSU_CSL_CSU, CSU_SEC_LEVEL_5, LOCKED),

	/* master HP0~1 */

	/* SA setting */
	CSU_SA(CSU_SA_M4, NON_SEC_ACCESS, LOCKED),
	CSU_SA(CSU_SA_SDMA1, NON_SEC_ACCESS, LOCKED),
	CSU_SA(CSU_SA_PCIE_CTRL1, NON_SEC_ACCESS, LOCKED),
	CSU_SA(CSU_SA_USB1, NON_SEC_ACCESS, LOCKED),
	CSU_SA(CSU_SA_USB2, NON_SEC_ACCESS, LOCKED),
	CSU_SA(CSU_SA_VPU, NON_SEC_ACCESS, LOCKED),
	CSU_SA(CSU_SA_GPU, NON_SEC_ACCESS, LOCKED),
	CSU_SA(CSU_SA_APBHDMA, NON_SEC_ACCESS, LOCKED),
	CSU_SA(CSU_SA_ENET, NON_SEC_ACCESS, LOCKED),
	CSU_SA(CSU_SA_USDHC1, NON_SEC_ACCESS, LOCKED),
	CSU_SA(CSU_SA_USDHC2, NON_SEC_ACCESS, LOCKED),
	CSU_SA(CSU_SA_USDHC3, NON_SEC_ACCESS, LOCKED),
	CSU_SA(CSU_SA_HUGO, NON_SEC_ACCESS, LOCKED),
	CSU_SA(CSU_SA_DAP, NON_SEC_ACCESS, LOCKED),
	CSU_SA(CSU_SA_SDMA2, NON_SEC_ACCESS, LOCKED),
	CSU_SA(CSU_SA_SDMA3, NON_SEC_ACCESS, LOCKED),
	CSU_SA(CSU_SA_LCDIF, NON_SEC_ACCESS, LOCKED),
	CSU_SA(CSU_SA_CSI, NON_SEC_ACCESS, LOCKED),

	/* HP control setting */

	/* Sentinel */
	{0}
};

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

/* get SPSR for BL33 entry */
static uint32_t get_spsr_for_bl33_entry(void)
{
	unsigned long el_status;
	unsigned long mode;
	uint32_t spsr;

	/* figure out what mode we enter the non-secure world */
	el_status = read_id_aa64pfr0_el1() >> ID_AA64PFR0_EL2_SHIFT;
	el_status &= ID_AA64PFR0_ELX_MASK;

	mode = (el_status) ? MODE_EL2 : MODE_EL1;

	spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}

void bl31_tzc380_setup(void)
{
	unsigned int val;

	val = mmio_read_32(IMX_IOMUX_GPR_BASE + 0x28);
	if ((val & GPR_TZASC_EN) != GPR_TZASC_EN)
		return;

	tzc380_init(IMX_TZASC_BASE);

	/*
	 * Need to substact offset 0x40000000 from CPU address when
	 * programming tzasc region for i.mx8mm.
	 */

	/* Enable 1G-5G S/NS RW */
	tzc380_configure_region(0, 0x00000000, TZC_ATTR_REGION_SIZE(TZC_REGION_SIZE_4G) |
		TZC_ATTR_REGION_EN_MASK | TZC_ATTR_SP_ALL);
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
		u_register_t arg2, u_register_t arg3)
{
	unsigned int console_base = IMX_BOOT_UART_BASE;
	static console_t console;
	int i;

	/* Enable CSU NS access permission */
	for (i = 0; i < 64; i++) {
		mmio_write_32(IMX_CSU_BASE + i * 4, 0x00ff00ff);
	}

	imx_aipstz_init(aipstz);

	imx_rdc_init(rdc);

	imx_csu_init(csu_cfg);

	if (console_base == 0U) {
		console_base = imx8m_uart_get_base();
	}

	console_imx_uart_register(console_base, IMX_BOOT_UART_CLK_IN_HZ,
		IMX_CONSOLE_BAUDRATE, &console);
	/* This console is only used for boot stage */
	console_set_scope(&console, CONSOLE_FLAG_BOOT);

	imx8m_caam_init();

	/*
	 * tell BL3-1 where the non-secure software image is located
	 * and the entry state information.
	 */
	bl33_image_ep_info.pc = PLAT_NS_IMAGE_OFFSET;
	bl33_image_ep_info.spsr = get_spsr_for_bl33_entry();
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

#if defined(SPD_opteed) || defined(SPD_trusty)
	/* Populate entry point information for BL32 */
	SET_PARAM_HEAD(&bl32_image_ep_info, PARAM_EP, VERSION_1, 0);
	SET_SECURITY_STATE(bl32_image_ep_info.h.attr, SECURE);
	bl32_image_ep_info.pc = BL32_BASE;
	bl32_image_ep_info.spsr = 0;

	/* Pass TEE base and size to bl33 */
	bl33_image_ep_info.args.arg1 = BL32_BASE;
	bl33_image_ep_info.args.arg2 = BL32_SIZE;

#ifdef SPD_trusty
	bl32_image_ep_info.args.arg0 = BL32_SIZE;
	bl32_image_ep_info.args.arg1 = BL32_BASE;
#else
	/* Make sure memory is clean */
	mmio_write_32(BL32_FDT_OVERLAY_ADDR, 0);
	bl33_image_ep_info.args.arg3 = BL32_FDT_OVERLAY_ADDR;
	bl32_image_ep_info.args.arg3 = BL32_FDT_OVERLAY_ADDR;
#endif
#endif

#if !defined(SPD_opteed) && !defined(SPD_trusty)
	enable_snvs_privileged_access();
#endif

	bl31_tzc380_setup();
}

#define MAP_BL31_TOTAL										   \
	MAP_REGION_FLAT(BL31_START, BL31_SIZE, MT_MEMORY | MT_RW | MT_SECURE)
#define MAP_BL31_RO										   \
	MAP_REGION_FLAT(BL_CODE_BASE, BL_CODE_END - BL_CODE_BASE, MT_MEMORY | MT_RO | MT_SECURE)
#define MAP_COHERENT_MEM									   \
	MAP_REGION_FLAT(BL_COHERENT_RAM_BASE, BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE,	   \
			MT_DEVICE | MT_RW | MT_SECURE)
#define MAP_BL32_TOTAL										   \
	MAP_REGION_FLAT(BL32_BASE, BL32_SIZE, MT_MEMORY | MT_RW)

void bl31_plat_arch_setup(void)
{
	const mmap_region_t bl_regions[] = {
		MAP_BL31_TOTAL,
		MAP_BL31_RO,
#if USE_COHERENT_MEM
		MAP_COHERENT_MEM,
#endif
#if defined(SPD_opteed) || defined(SPD_trusty)
		/* Map TEE memory */
		MAP_BL32_TOTAL,
#endif
		{0}
	};

	setup_page_tables(bl_regions, imx_mmap);
	enable_mmu_el3(0);
}

void bl31_platform_setup(void)
{
	generic_delay_timer_init();

	/* select the CKIL source to 32K OSC */
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_MISC_CTL, 0x1);

	/* Init the dram info */
	dram_info_init(SAVED_DRAM_TIMING_BASE);

	plat_gic_driver_init();
	plat_gic_init();

	imx_gpc_init();
}

entry_point_info_t *bl31_plat_get_next_image_ep_info(unsigned int type)
{
	if (type == NON_SECURE)
		return &bl33_image_ep_info;
	if (type == SECURE)
		return &bl32_image_ep_info;

	return NULL;
}

unsigned int plat_get_syscnt_freq2(void)
{
	return COUNTER_FREQUENCY;
}

#ifdef SPD_trusty
void plat_trusty_set_boot_args(aapcs64_params_t *args)
{
	args->arg0 = BL32_SIZE;
	args->arg1 = BL32_BASE;
	args->arg2 = TRUSTY_PARAMS_LEN_BYTES;
}
#endif
