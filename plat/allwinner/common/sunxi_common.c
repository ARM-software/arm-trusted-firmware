/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <sunxi_def.h>
#include <xlat_tables_v2.h>

static mmap_region_t sunxi_mmap[PLATFORM_MMAP_REGIONS + 1] = {
	MAP_REGION_FLAT(SUNXI_SRAM_BASE, SUNXI_SRAM_SIZE,
			MT_MEMORY | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(SUNXI_DEV_BASE, SUNXI_DEV_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION(SUNXI_DRAM_BASE, SUNXI_DRAM_VIRT_BASE, SUNXI_DRAM_SEC_SIZE,
			MT_MEMORY | MT_RW | MT_SECURE),
	MAP_REGION(PLAT_SUNXI_NS_IMAGE_OFFSET,
		   SUNXI_DRAM_VIRT_BASE + SUNXI_DRAM_SEC_SIZE,
		   SUNXI_DRAM_MAP_SIZE,
		   MT_MEMORY | MT_RW | MT_NS),
	{},
};

unsigned int plat_get_syscnt_freq2(void)
{
	return SUNXI_OSC24M_CLK_IN_HZ;
}

uintptr_t plat_get_ns_image_entrypoint(void)
{
#ifdef PRELOADED_BL33_BASE
	return PRELOADED_BL33_BASE;
#else
	return PLAT_SUNXI_NS_IMAGE_OFFSET;
#endif
}

void sunxi_configure_mmu_el3(int flags)
{
	mmap_add_region(BL31_BASE, BL31_BASE,
			BL31_LIMIT - BL31_BASE,
			MT_MEMORY | MT_RW | MT_SECURE);
	mmap_add_region(BL_CODE_BASE, BL_CODE_BASE,
			BL_CODE_END - BL_CODE_BASE,
			MT_CODE | MT_SECURE);
	mmap_add_region(BL_RO_DATA_BASE, BL_RO_DATA_BASE,
			BL_RO_DATA_END - BL_RO_DATA_BASE,
			MT_RO_DATA | MT_SECURE);
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
 * To set pin PC7 to high, use: sunxi_set_gpio_out('C', 7, true);
 */
void sunxi_set_gpio_out(char port, int pin, bool level_high)
{
	uintptr_t pio_base = SUNXI_PIO_BASE;

	if (port < 'A' || port > 'L')
		return;
	if (port == 'L')
		pio_base = SUNXI_R_PIO_BASE;
	else
		pio_base += (port - 'A') * 0x24;

	/* Set the new level first before configuring the pin. */
	if (level_high)
		mmio_setbits_32(pio_base + 0x10, BIT(pin));
	else
		mmio_clrbits_32(pio_base + 0x10, BIT(pin));

	/* configure pin as GPIO out (4(3) bits per pin, 1: GPIO out */
	mmio_clrsetbits_32(pio_base + (pin / 8) * 4,
			   0x7 << ((pin % 8) * 4),
			   0x1 << ((pin % 8) * 4));
}
