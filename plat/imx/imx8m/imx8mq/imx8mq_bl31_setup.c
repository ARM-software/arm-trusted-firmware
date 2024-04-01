/*
 * Copyright (c) 2018-2023, ARM Limited and Contributors. All rights reserved.
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
#include <imx8m_caam.h>
#include <imx8m_ccm.h>
#include <plat_imx8.h>

#define TRUSTY_PARAMS_LEN_BYTES      (4096*2)

/*
 * Avoid the pointer dereference of the canonical mmio_read_8() implementation.
 * This prevents the compiler from mis-interpreting the MMIO access as an
 * illegal memory access to a very low address (the IMX ROM is mapped at 0).
 */
static uint8_t mmio_read_8_ldrb(uintptr_t address)
{
	uint8_t reg;

	__asm__ volatile ("ldrb %w0, [%1]" : "=r" (reg) : "r" (address));

	return reg;
}

static const mmap_region_t imx_mmap[] = {
	MAP_REGION_FLAT(GPV_BASE, GPV_SIZE, MT_DEVICE | MT_RW), /* GPV map */
	MAP_REGION_FLAT(IMX_ROM_BASE, IMX_ROM_SIZE, MT_MEMORY | MT_RO), /* ROM map */
	MAP_REGION_FLAT(IMX_AIPS_BASE, IMX_AIPS_SIZE, MT_DEVICE | MT_RW), /* AIPS map */
	MAP_REGION_FLAT(IMX_GIC_BASE, IMX_GIC_SIZE, MT_DEVICE | MT_RW), /* GIC map */
	MAP_REGION_FLAT(IMX_DDRPHY_BASE, IMX_DDR_IPS_SIZE, MT_DEVICE | MT_RW), /* DDRMIX map */
	MAP_REGION_FLAT(IMX_DRAM_BASE, IMX_DRAM_SIZE, MT_MEMORY | MT_RW | MT_NS),
	{0},
};

static const struct aipstz_cfg aipstz[] = {
	{AIPSTZ1_BASE, 0x77777777, 0x77777777, .opacr = {0x0, 0x0, 0x0, 0x0, 0x0}, },
	{AIPSTZ2_BASE, 0x77777777, 0x77777777, .opacr = {0x0, 0x0, 0x0, 0x0, 0x0}, },
	{AIPSTZ3_BASE, 0x77777777, 0x77777777, .opacr = {0x0, 0x0, 0x0, 0x0, 0x0}, },
	{AIPSTZ4_BASE, 0x77777777, 0x77777777, .opacr = {0x0, 0x0, 0x0, 0x0, 0x0}, },
	{0},
};

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

static uint32_t imx_soc_revision;

int imx_soc_info_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
				u_register_t x3)
{
	return imx_soc_revision;
}

#define ANAMIX_DIGPROG		0x6c
#define ROM_SOC_INFO_A0		0x800
#define ROM_SOC_INFO_B0		0x83C
#define OCOTP_SOC_INFO_B1	0x40

static void imx8mq_soc_info_init(void)
{
	uint32_t rom_version;
	uint32_t ocotp_val;

	imx_soc_revision = mmio_read_32(IMX_ANAMIX_BASE + ANAMIX_DIGPROG);
	rom_version = mmio_read_8_ldrb(IMX_ROM_BASE + ROM_SOC_INFO_A0);
	if (rom_version == 0x10)
		return;

	rom_version = mmio_read_8_ldrb(IMX_ROM_BASE + ROM_SOC_INFO_B0);
	if (rom_version == 0x20) {
		imx_soc_revision &= ~0xff;
		imx_soc_revision |= rom_version;
		return;
	}

	/* 0xff0055aa is magic number for B1 */
	ocotp_val = mmio_read_32(IMX_OCOTP_BASE + OCOTP_SOC_INFO_B1);
	if (ocotp_val == 0xff0055aa) {
		imx_soc_revision &= ~0xff;
		if (rom_version == 0x22) {
			imx_soc_revision |= 0x22;
		} else {
			imx_soc_revision |= 0x21;
		}
		return;
	}
}

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

static void bl31_tz380_setup(void)
{
	unsigned int val;

	val = mmio_read_32(IMX_IOMUX_GPR_BASE + IOMUXC_GPR10);
	if ((val & GPR_TZASC_EN) != GPR_TZASC_EN)
		return;

	tzc380_init(IMX_TZASC_BASE);
	/*
	 * Need to substact offset 0x40000000 from CPU address when
	 * programming tzasc region for i.mx8mq. Enable 1G-5G S/NS RW
	 */
	tzc380_configure_region(0, 0x00000000, TZC_ATTR_REGION_SIZE(TZC_REGION_SIZE_4G) |
				TZC_ATTR_REGION_EN_MASK | TZC_ATTR_SP_ALL);
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
			u_register_t arg2, u_register_t arg3)
{
	unsigned int console_base = IMX_BOOT_UART_BASE;
	static console_t console;
	int i;
	/* enable CSU NS access permission */
	for (i = 0; i < 64; i++) {
		mmio_write_32(IMX_CSU_BASE + i * 4, 0xffffffff);
	}

	imx_aipstz_init(aipstz);

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

	bl31_tz380_setup();
}

void bl31_plat_arch_setup(void)
{
	const mmap_region_t bl_regions[] = {
		MAP_REGION_FLAT(BL31_START, BL31_SIZE,
				MT_MEMORY | MT_RW | MT_SECURE),
		MAP_REGION_FLAT(BL_CODE_BASE, BL_CODE_END - BL_CODE_BASE,
				MT_MEMORY | MT_RO | MT_SECURE),
#if USE_COHERENT_MEM
		MAP_REGION_FLAT(BL_COHERENT_RAM_BASE,
				BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE,
				MT_DEVICE | MT_RW | MT_SECURE),
#endif
		/* Map TEE memory */
		MAP_REGION_FLAT(BL32_BASE, BL32_SIZE, MT_MEMORY | MT_RW),
		{0},
	};

	setup_page_tables(bl_regions, imx_mmap);
	/* enable the MMU */
	enable_mmu_el3(0);
}

void bl31_platform_setup(void)
{
	generic_delay_timer_init();

	/* init the GICv3 cpu and distributor interface */
	plat_gic_driver_init();
	plat_gic_init();

	/* determine SOC revision for erratas */
	imx8mq_soc_info_init();

	/* gpc init */
	imx_gpc_init();

	dram_info_init(SAVED_DRAM_TIMING_BASE);
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
