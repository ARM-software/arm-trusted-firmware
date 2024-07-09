/*
 * Copyright (c) 2023, Aspeed Technology Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <arch.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <drivers/arm/gicv3.h>
#include <drivers/console.h>
#include <drivers/ti/uart/uart_16550.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include <platform_def.h>

static console_t console;

static entry_point_info_t bl32_ep_info;
static entry_point_info_t bl33_ep_info;

static uintptr_t rdistif_base_addrs[PLATFORM_CORE_COUNT];

static unsigned int plat_mpidr_to_core_pos(u_register_t mpidr)
{
	/* to workaround the return type mismatch */
	return plat_core_pos_by_mpidr(mpidr);
}

static const gicv3_driver_data_t plat_gic_data = {
	.gicd_base = GICD_BASE,
	.gicr_base = GICR_BASE,
	.rdistif_num = PLATFORM_CORE_COUNT,
	.rdistif_base_addrs = rdistif_base_addrs,
	.mpidr_to_core_pos = plat_mpidr_to_core_pos,
};

static const mmap_region_t plat_mmap[] = {
	MAP_REGION_FLAT(GICD_BASE, GICD_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(GICR_BASE, GICR_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(UART_BASE, PAGE_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(SCU_CPU_BASE, PAGE_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	{ 0 }
};

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	console_16550_register(CONSOLE_UART_BASE, CONSOLE_UART_CLKIN_HZ,
			       CONSOLE_UART_BAUDRATE, &console);

	console_set_scope(&console, CONSOLE_FLAG_BOOT | CONSOLE_FLAG_RUNTIME | CONSOLE_FLAG_CRASH);

	SET_PARAM_HEAD(&bl32_ep_info, PARAM_EP, VERSION_2, 0);
	bl32_ep_info.pc = BL32_BASE;
	SET_SECURITY_STATE(bl32_ep_info.h.attr, SECURE);

	SET_PARAM_HEAD(&bl33_ep_info, PARAM_EP, VERSION_2, 0);
	bl33_ep_info.pc = mmio_read_64(SCU_CPU_SMP_EP0);
	bl33_ep_info.spsr = SPSR_64(MODE_EL2, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	SET_SECURITY_STATE(bl33_ep_info.h.attr, NON_SECURE);
}

void bl31_plat_arch_setup(void)
{
	mmap_add_region(BL_CODE_BASE, BL_CODE_BASE,
			BL_CODE_END - BL_CODE_BASE,
			MT_CODE | MT_SECURE);

	mmap_add_region(BL_CODE_END, BL_CODE_END,
			BL_END - BL_CODE_END,
			MT_RW_DATA | MT_SECURE);

#if USE_COHERENT_MEM
	mmap_add_region(BL_COHERENT_RAM_BASE, BL_COHERENT_RAM_BASE,
			BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE,
			MT_DEVICE | MT_RW | MT_SECURE);
#endif

	mmap_add_region(BL32_BASE, BL32_BASE, BL32_SIZE,
			MT_MEMORY | MT_RW);

	mmap_add(plat_mmap);

	init_xlat_tables();

	enable_mmu_el3(0);
}

void bl31_platform_setup(void)
{
	gicv3_driver_init(&plat_gic_data);
	gicv3_distif_init();
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *ep_info;

	ep_info = (type == NON_SECURE) ? &bl33_ep_info : &bl32_ep_info;

	if (!ep_info->pc) {
		return NULL;
	}

	return ep_info;
}

/*
 * Clock divider/multiplier configuration struct.
 * For H-PLL and M-PLL the formula is
 * (Output Frequency) = CLKIN * ((M + 1) / (N + 1)) / (P + 1)
 * M - Numerator
 * N - Denumerator
 * P - Post Divider
 * They have the same layout in their control register.
 *
 */
union plat_pll_reg {
	uint32_t w;
	struct {
		uint16_t m : 13;		/* bit[12:0]	*/
		uint8_t n : 6;			/* bit[18:13]	*/
		uint8_t p : 4;			/* bit[22:19]	*/
		uint8_t off : 1;		/* bit[23]	*/
		uint8_t bypass : 1;		/* bit[24]	*/
		uint8_t reset : 1;		/* bit[25]	*/
		uint8_t reserved : 6;		/* bit[31:26]	*/
	} b;
};

static uint32_t plat_get_pll_rate(int pll_idx)
{
	union plat_pll_reg pll_reg;
	uint32_t mul = 1, div = 1;
	uint32_t rate = 0;

	switch (pll_idx) {
	case PLAT_CLK_HPLL:
		pll_reg.w = mmio_read_32(SCU_CPU_HPLL);
		break;
	case PLAT_CLK_DPLL:
		pll_reg.w = mmio_read_32(SCU_CPU_DPLL);
		break;
	case PLAT_CLK_MPLL:
		pll_reg.w = mmio_read_32(SCU_CPU_MPLL);
		break;
	default:
		ERROR("%s: invalid PSP clock source (%d)\n", __func__, pll_idx);
		return -EINVAL;
	}

	if (pll_idx == PLAT_CLK_HPLL && ((mmio_read_32(SCU_CPU_HW_STRAP1) & GENMASK(3, 2)) != 0U)) {
		switch ((mmio_read_32(SCU_CPU_HW_STRAP1) & GENMASK(3, 2)) >> 2) {
		case 1U:
			rate = 1900000000;
			break;
		case 2U:
			rate = 1800000000;
			break;
		case 3U:
			rate = 1700000000;
			break;
		default:
			rate = 2000000000;
			break;
		}
	} else {
		if (pll_reg.b.bypass == 0U) {
			if (pll_idx == PLAT_CLK_MPLL) {
				/* F = 25Mhz * [M / (n + 1)] / (p + 1) */
				mul = (pll_reg.b.m) / ((pll_reg.b.n + 1));
				div = (pll_reg.b.p + 1);
			} else {
				/* F = 25Mhz * [(M + 2) / 2 * (n + 1)] / (p + 1) */
				mul = (pll_reg.b.m + 1) / ((pll_reg.b.n + 1) * 2);
				div = (pll_reg.b.p + 1);
			}
		}

		rate = ((CLKIN_25M * mul) / div);
	}

	return rate;
}

unsigned int plat_get_syscnt_freq2(void)
{
	if (mmio_read_32(SCU_CPU_HW_STRAP1) & BIT(4)) {
		return plat_get_pll_rate(PLAT_CLK_HPLL);
	} else {
		return plat_get_pll_rate(PLAT_CLK_MPLL);
	}
}
