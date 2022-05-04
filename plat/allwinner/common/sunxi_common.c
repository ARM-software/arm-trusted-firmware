/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/smccc.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <services/arm_arch_svc.h>

#include <sunxi_def.h>
#include <sunxi_mmap.h>
#include <sunxi_private.h>

static const mmap_region_t sunxi_mmap[MAX_STATIC_MMAP_REGIONS + 1] = {
	MAP_REGION_FLAT(SUNXI_SRAM_BASE, SUNXI_SRAM_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE | MT_EXECUTE_NEVER),
	MAP_REGION_FLAT(SUNXI_DEV_BASE, SUNXI_DEV_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE | MT_EXECUTE_NEVER),
	MAP_REGION(PRELOADED_BL33_BASE, SUNXI_BL33_VIRT_BASE,
		   SUNXI_DRAM_MAP_SIZE, MT_RW_DATA | MT_NS),
	{},
};

unsigned int plat_get_syscnt_freq2(void)
{
	return SUNXI_OSC24M_CLK_IN_HZ;
}

void sunxi_configure_mmu_el3(int flags)
{
	mmap_add_region(BL_CODE_BASE, BL_CODE_BASE,
			BL_CODE_END - BL_CODE_BASE,
			MT_CODE | MT_SECURE);
	mmap_add_region(BL_CODE_END, BL_CODE_END,
			BL_END - BL_CODE_END,
			MT_RW_DATA | MT_SECURE);
#if SEPARATE_CODE_AND_RODATA
	mmap_add_region(BL_RO_DATA_BASE, BL_RO_DATA_BASE,
			BL_RO_DATA_END - BL_RO_DATA_BASE,
			MT_RO_DATA | MT_SECURE);
#endif
#if SEPARATE_NOBITS_REGION
	mmap_add_region(BL_NOBITS_BASE, BL_NOBITS_BASE,
			BL_NOBITS_END - BL_NOBITS_BASE,
			MT_RW_DATA | MT_SECURE);
#endif
#if USE_COHERENT_MEM
	mmap_add_region(BL_COHERENT_RAM_BASE, BL_COHERENT_RAM_BASE,
			BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE,
			MT_DEVICE | MT_RW | MT_SECURE | MT_EXECUTE_NEVER);
#endif

	mmap_add(sunxi_mmap);
	init_xlat_tables();

	enable_mmu_el3(0);
}

#define SRAM_VER_REG (SUNXI_SYSCON_BASE + 0x24)
uint16_t sunxi_read_soc_id(void)
{
	uint32_t reg = mmio_read_32(SRAM_VER_REG);

	/* Set bit 15 to prepare for the SOCID read. */
	mmio_write_32(SRAM_VER_REG, reg | BIT(15));

	reg = mmio_read_32(SRAM_VER_REG);

	/* deactivate the SOCID access again */
	mmio_write_32(SRAM_VER_REG, reg & ~BIT(15));

	return reg >> 16;
}

/*
 * Configure a given pin to the GPIO-OUT function and sets its level.
 * The port is given as a capital letter, the pin is the number within
 * this port group.
 * So to set pin PC7 to high, use: sunxi_set_gpio_out('C', 7, true);
 */
void sunxi_set_gpio_out(char port, int pin, bool level_high)
{
	uintptr_t port_base;

	if (port < 'A' || port > 'L')
		return;
	if (port == 'L')
		port_base = SUNXI_R_PIO_BASE;
	else
		port_base = SUNXI_PIO_BASE + (port - 'A') * 0x24;

	/* Set the new level first before configuring the pin. */
	if (level_high)
		mmio_setbits_32(port_base + 0x10, BIT(pin));
	else
		mmio_clrbits_32(port_base + 0x10, BIT(pin));

	/* configure pin as GPIO out (4(3) bits per pin, 1: GPIO out */
	mmio_clrsetbits_32(port_base + (pin / 8) * 4,
			   0x7 << ((pin % 8) * 4),
			   0x1 << ((pin % 8) * 4));
}

int sunxi_init_platform_r_twi(uint16_t socid, bool use_rsb)
{
	uint32_t pin_func = 0x77;
	uint32_t device_bit;
	unsigned int reset_offset = 0xb0;

	switch (socid) {
	case SUNXI_SOC_H5:
		if (use_rsb)
			return -ENODEV;
		pin_func = 0x22;
		device_bit = BIT(6);
		break;
	case SUNXI_SOC_H6:
	case SUNXI_SOC_H616:
		pin_func = use_rsb ? 0x22 : 0x33;
		device_bit = BIT(16);
		reset_offset = use_rsb ? 0x1bc : 0x19c;
		break;
	case SUNXI_SOC_A64:
		pin_func = use_rsb ? 0x22 : 0x33;
		device_bit = use_rsb ? BIT(3) : BIT(6);
		break;
	default:
		INFO("R_I2C/RSB on Allwinner 0x%x SoC not supported\n", socid);
		return -ENODEV;
	}

	/* un-gate R_PIO clock */
	if (socid != SUNXI_SOC_H6 && socid != SUNXI_SOC_H616)
		mmio_setbits_32(SUNXI_R_PRCM_BASE + 0x28, BIT(0));

	/* switch pins PL0 and PL1 to the desired function */
	mmio_clrsetbits_32(SUNXI_R_PIO_BASE + 0x00, 0xffU, pin_func);

	/* level 2 drive strength */
	mmio_clrsetbits_32(SUNXI_R_PIO_BASE + 0x14, 0x0fU, 0xaU);

	/* set both pins to pull-up */
	mmio_clrsetbits_32(SUNXI_R_PIO_BASE + 0x1c, 0x0fU, 0x5U);

	/* un-gate clock */
	if (socid != SUNXI_SOC_H6 && socid != SUNXI_SOC_H616)
		mmio_setbits_32(SUNXI_R_PRCM_BASE + 0x28, device_bit);
	else
		mmio_setbits_32(SUNXI_R_PRCM_BASE + reset_offset, BIT(0));

	/* assert, then de-assert reset of I2C/RSB controller */
	mmio_clrbits_32(SUNXI_R_PRCM_BASE + reset_offset, device_bit);
	mmio_setbits_32(SUNXI_R_PRCM_BASE + reset_offset, device_bit);

	return 0;
}

int32_t plat_is_smccc_feature_available(u_register_t fid)
{
	switch (fid) {
	case SMCCC_ARCH_SOC_ID:
		return SMC_ARCH_CALL_SUCCESS;
	default:
		return SMC_ARCH_CALL_NOT_SUPPORTED;
	}
}

int32_t plat_get_soc_version(void)
{
	int32_t ret;

	ret = SOC_ID_SET_JEP_106(JEDEC_ALLWINNER_BKID, JEDEC_ALLWINNER_MFID);

	return ret | (sunxi_read_soc_id() & SOC_ID_IMPL_DEF_MASK);
}

int32_t plat_get_soc_revision(void)
{
	uint32_t reg = mmio_read_32(SRAM_VER_REG);

	return reg & GENMASK_32(7, 0);
}
