/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <dw_mmc.h>
#include <emmc.h>
#include <errno.h>
#include <gpio.h>
#include <hi6220.h>
#include <hi6553.h>
#include <mmio.h>
#include <pl061_gpio.h>
#include <platform.h>
#include <platform_def.h>
#include <sp804_delay_timer.h>
#include <string.h>
#include <tbbr/tbbr_img_desc.h>

#include "../../bl1/bl1_private.h"
#include "hikey_def.h"
#include "hikey_private.h"

/*
 * Declarations of linker defined symbols which will help us find the layout
 * of trusted RAM
 */
extern unsigned long __COHERENT_RAM_START__;
extern unsigned long __COHERENT_RAM_END__;

/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols refer to
 * page-aligned addresses.
 */
#define BL1_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL1_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)

/* Data structure which holds the extents of the trusted RAM for BL1 */
static meminfo_t bl1_tzram_layout;

enum {
	BOOT_NORMAL = 0,
	BOOT_USB_DOWNLOAD,
	BOOT_UART_DOWNLOAD,
};

meminfo_t *bl1_plat_sec_mem_layout(void)
{
	return &bl1_tzram_layout;
}

#if LOAD_IMAGE_V2
/*******************************************************************************
 * Function that takes a memory layout into which BL2 has been loaded and
 * populates a new memory layout for BL2 that ensures that BL1's data sections
 * resident in secure RAM are not visible to BL2.
 ******************************************************************************/
void bl1_init_bl2_mem_layout(const meminfo_t *bl1_mem_layout,
			     meminfo_t *bl2_mem_layout)
{

	assert(bl1_mem_layout != NULL);
	assert(bl2_mem_layout != NULL);

	/*
	 * Cannot remove BL1 RW data from the scope of memory visible to BL2
	 * like arm platforms because they overlap in hikey
	 */
	bl2_mem_layout->total_base = BL2_BASE;
	bl2_mem_layout->total_size = BL32_SRAM_LIMIT - BL2_BASE;

	flush_dcache_range((unsigned long)bl2_mem_layout, sizeof(meminfo_t));
}
#endif /* LOAD_IMAGE_V2 */

/*
 * Perform any BL1 specific platform actions.
 */
void bl1_early_platform_setup(void)
{
	/* Initialize the console to provide early debug support */
	console_init(CONSOLE_BASE, PL011_UART_CLK_IN_HZ, PL011_BAUDRATE);

	/* Allow BL1 to see the whole Trusted RAM */
	bl1_tzram_layout.total_base = BL1_RW_BASE;
	bl1_tzram_layout.total_size = BL1_RW_SIZE;

#if !LOAD_IMAGE_V2
	/* Calculate how much RAM BL1 is using and how much remains free */
	bl1_tzram_layout.free_base = BL1_RW_BASE;
	bl1_tzram_layout.free_size = BL1_RW_SIZE;
	reserve_mem(&bl1_tzram_layout.free_base,
		    &bl1_tzram_layout.free_size,
		    BL1_RAM_BASE,
		    BL1_RAM_LIMIT - BL1_RAM_BASE); /* bl1_size */
#endif

	INFO("BL1: 0x%lx - 0x%lx [size = %lu]\n", BL1_RAM_BASE, BL1_RAM_LIMIT,
	     BL1_RAM_LIMIT - BL1_RAM_BASE); /* bl1_size */
}

/*
 * Perform the very early platform specific architecture setup here. At the
 * moment this only does basic initialization. Later architectural setup
 * (bl1_arch_setup()) does not do anything platform specific.
 */
void bl1_plat_arch_setup(void)
{
	hikey_init_mmu_el3(bl1_tzram_layout.total_base,
			   bl1_tzram_layout.total_size,
			   BL1_RO_BASE,
			   BL1_RO_LIMIT,
			   BL1_COHERENT_RAM_BASE,
			   BL1_COHERENT_RAM_LIMIT);
}

static void hikey_sp804_init(void)
{
	uint32_t data;

	/* select the clock of dual timer0 */
	data = mmio_read_32(AO_SC_TIMER_EN0);
	while (data & 3) {
		data &= ~3;
		data |= 3 << 16;
		mmio_write_32(AO_SC_TIMER_EN0, data);
		data = mmio_read_32(AO_SC_TIMER_EN0);
	}
	/* enable the pclk of dual timer0 */
	data = mmio_read_32(AO_SC_PERIPH_CLKSTAT4);
	while (!(data & PCLK_TIMER1) || !(data & PCLK_TIMER0)) {
		mmio_write_32(AO_SC_PERIPH_CLKEN4, PCLK_TIMER1 | PCLK_TIMER0);
		data = mmio_read_32(AO_SC_PERIPH_CLKSTAT4);
	}
	/* reset dual timer0 */
	data = mmio_read_32(AO_SC_PERIPH_RSTSTAT4);
	mmio_write_32(AO_SC_PERIPH_RSTEN4, PCLK_TIMER1 | PCLK_TIMER0);
	do {
		data = mmio_read_32(AO_SC_PERIPH_RSTSTAT4);
	} while (!(data & PCLK_TIMER1) || !(data & PCLK_TIMER0));
	/* unreset dual timer0 */
	mmio_write_32(AO_SC_PERIPH_RSTDIS4, PCLK_TIMER1 | PCLK_TIMER0);
	do {
		data = mmio_read_32(AO_SC_PERIPH_RSTSTAT4);
	} while ((data & PCLK_TIMER1) || (data & PCLK_TIMER0));

	sp804_timer_init(SP804_TIMER0_BASE, 10, 192);
}

static void hikey_gpio_init(void)
{
	pl061_gpio_init();
	pl061_gpio_register(GPIO0_BASE, 0);
	pl061_gpio_register(GPIO1_BASE, 1);
	pl061_gpio_register(GPIO2_BASE, 2);
	pl061_gpio_register(GPIO3_BASE, 3);
	pl061_gpio_register(GPIO4_BASE, 4);
	pl061_gpio_register(GPIO5_BASE, 5);
	pl061_gpio_register(GPIO6_BASE, 6);
	pl061_gpio_register(GPIO7_BASE, 7);
	pl061_gpio_register(GPIO8_BASE, 8);
	pl061_gpio_register(GPIO9_BASE, 9);
	pl061_gpio_register(GPIO10_BASE, 10);
	pl061_gpio_register(GPIO11_BASE, 11);
	pl061_gpio_register(GPIO12_BASE, 12);
	pl061_gpio_register(GPIO13_BASE, 13);
	pl061_gpio_register(GPIO14_BASE, 14);
	pl061_gpio_register(GPIO15_BASE, 15);
	pl061_gpio_register(GPIO16_BASE, 16);
	pl061_gpio_register(GPIO17_BASE, 17);
	pl061_gpio_register(GPIO18_BASE, 18);
	pl061_gpio_register(GPIO19_BASE, 19);

	/* Power on indicator LED (USER_LED1). */
	gpio_set_direction(32, GPIO_DIR_OUT);	/* LED1 */
	gpio_set_value(32, GPIO_LEVEL_HIGH);
	gpio_set_direction(33, GPIO_DIR_OUT);	/* LED2 */
	gpio_set_value(33, GPIO_LEVEL_LOW);
	gpio_set_direction(34, GPIO_DIR_OUT);	/* LED3 */
	gpio_set_direction(35, GPIO_DIR_OUT);	/* LED4 */
}

static void hikey_pmussi_init(void)
{
	uint32_t data;

	/* Initialize PWR_HOLD GPIO */
	gpio_set_direction(0, GPIO_DIR_OUT);
	gpio_set_value(0, GPIO_LEVEL_LOW);

	/*
	 * After reset, PMUSSI stays in reset mode.
	 * Now make it out of reset.
	 */
	mmio_write_32(AO_SC_PERIPH_RSTDIS4,
		      AO_SC_PERIPH_RSTDIS4_PRESET_PMUSSI_N);
	do {
		data = mmio_read_32(AO_SC_PERIPH_RSTSTAT4);
	} while (data & AO_SC_PERIPH_RSTDIS4_PRESET_PMUSSI_N);

	/* Set PMUSSI clock latency for read operation. */
	data = mmio_read_32(AO_SC_MCU_SUBSYS_CTRL3);
	data &= ~AO_SC_MCU_SUBSYS_CTRL3_RCLK_MASK;
	data |= AO_SC_MCU_SUBSYS_CTRL3_RCLK_3;
	mmio_write_32(AO_SC_MCU_SUBSYS_CTRL3, data);

	/* enable PMUSSI clock */
	data = AO_SC_PERIPH_CLKEN5_PCLK_PMUSSI_CCPU |
	       AO_SC_PERIPH_CLKEN5_PCLK_PMUSSI_MCU;
	mmio_write_32(AO_SC_PERIPH_CLKEN5, data);
	data = AO_SC_PERIPH_CLKEN4_PCLK_PMUSSI;
	mmio_write_32(AO_SC_PERIPH_CLKEN4, data);

	gpio_set_value(0, GPIO_LEVEL_HIGH);
}

static void hikey_hi6553_init(void)
{
	uint8_t data;

	mmio_write_8(HI6553_PERI_EN_MARK, 0x1e);
	mmio_write_8(HI6553_NP_REG_ADJ1, 0);
	data = DISABLE6_XO_CLK_CONN | DISABLE6_XO_CLK_NFC |
		DISABLE6_XO_CLK_RF1 | DISABLE6_XO_CLK_RF2;
	mmio_write_8(HI6553_DISABLE6_XO_CLK, data);

	/* configure BUCK0 & BUCK1 */
	mmio_write_8(HI6553_BUCK01_CTRL2, 0x5e);
	mmio_write_8(HI6553_BUCK0_CTRL7, 0x10);
	mmio_write_8(HI6553_BUCK1_CTRL7, 0x10);
	mmio_write_8(HI6553_BUCK0_CTRL5, 0x1e);
	mmio_write_8(HI6553_BUCK1_CTRL5, 0x1e);
	mmio_write_8(HI6553_BUCK0_CTRL1, 0xfc);
	mmio_write_8(HI6553_BUCK1_CTRL1, 0xfc);

	/* configure BUCK2 */
	mmio_write_8(HI6553_BUCK2_REG1, 0x4f);
	mmio_write_8(HI6553_BUCK2_REG5, 0x99);
	mmio_write_8(HI6553_BUCK2_REG6, 0x45);
	mdelay(1);
	mmio_write_8(HI6553_VSET_BUCK2_ADJ, 0x22);
	mdelay(1);

	/* configure BUCK3 */
	mmio_write_8(HI6553_BUCK3_REG3, 0x02);
	mmio_write_8(HI6553_BUCK3_REG5, 0x99);
	mmio_write_8(HI6553_BUCK3_REG6, 0x41);
	mmio_write_8(HI6553_VSET_BUCK3_ADJ, 0x02);
	mdelay(1);

	/* configure BUCK4 */
	mmio_write_8(HI6553_BUCK4_REG2, 0x9a);
	mmio_write_8(HI6553_BUCK4_REG5, 0x99);
	mmio_write_8(HI6553_BUCK4_REG6, 0x45);

	/* configure LDO20 */
	mmio_write_8(HI6553_LDO20_REG_ADJ, 0x50);

	mmio_write_8(HI6553_NP_REG_CHG, 0x0f);
	mmio_write_8(HI6553_CLK_TOP0, 0x06);
	mmio_write_8(HI6553_CLK_TOP3, 0xc0);
	mmio_write_8(HI6553_CLK_TOP4, 0x00);

	/* configure LDO7 & LDO10 for SD slot */
	/* enable LDO7 */
	data = mmio_read_8(HI6553_LDO7_REG_ADJ);
	data = (data & 0xf8) | 0x2;
	mmio_write_8(HI6553_LDO7_REG_ADJ, data);
	mdelay(5);
	mmio_write_8(HI6553_ENABLE2_LDO1_8, 1 << 6);
	mdelay(5);
	/* enable LDO10 */
	data = mmio_read_8(HI6553_LDO10_REG_ADJ);
	data = (data & 0xf8) | 0x5;
	mmio_write_8(HI6553_LDO10_REG_ADJ, data);
	mdelay(5);
	mmio_write_8(HI6553_ENABLE3_LDO9_16, 1 << 1);
	mdelay(5);
	/* enable LDO15 */
	data = mmio_read_8(HI6553_LDO15_REG_ADJ);
	data = (data & 0xf8) | 0x4;
	mmio_write_8(HI6553_LDO15_REG_ADJ, data);
	mmio_write_8(HI6553_ENABLE3_LDO9_16, 1 << 6);
	mdelay(5);
	/* enable LDO19 */
	data = mmio_read_8(HI6553_LDO19_REG_ADJ);
	data |= 0x7;
	mmio_write_8(HI6553_LDO19_REG_ADJ, data);
	mmio_write_8(HI6553_ENABLE4_LDO17_22, 1 << 2);
	mdelay(5);
	/* enable LDO21 */
	data = mmio_read_8(HI6553_LDO21_REG_ADJ);
	data = (data & 0xf8) | 0x3;
	mmio_write_8(HI6553_LDO21_REG_ADJ, data);
	mmio_write_8(HI6553_ENABLE4_LDO17_22, 1 << 4);
	mdelay(5);
	/* enable LDO22 */
	data = mmio_read_8(HI6553_LDO22_REG_ADJ);
	data = (data & 0xf8) | 0x7;
	mmio_write_8(HI6553_LDO22_REG_ADJ, data);
	mmio_write_8(HI6553_ENABLE4_LDO17_22, 1 << 5);
	mdelay(5);

	/* select 32.764KHz */
	mmio_write_8(HI6553_CLK19M2_600_586_EN, 0x01);

	/* Disable vbus_det interrupts */
	data = mmio_read_8(HI6553_IRQ2_MASK);
	data = data | 0x3;
	mmio_write_8(HI6553_IRQ2_MASK, data);
}

static void init_mmc0_pll(void)
{
	unsigned int data;

	/* select SYSPLL as the source of MMC0 */
	/* select SYSPLL as the source of MUX1 (SC_CLK_SEL0) */
	mmio_write_32(PERI_SC_CLK_SEL0, 1 << 5 | 1 << 21);
	do {
		data = mmio_read_32(PERI_SC_CLK_SEL0);
	} while (!(data & (1 << 5)));
	/* select MUX1 as the source of MUX2 (SC_CLK_SEL0) */
	mmio_write_32(PERI_SC_CLK_SEL0, 1 << 29);
	do {
		data = mmio_read_32(PERI_SC_CLK_SEL0);
	} while (data & (1 << 13));

	mmio_write_32(PERI_SC_PERIPH_CLKEN0, (1 << 0));
	do {
		data = mmio_read_32(PERI_SC_PERIPH_CLKSTAT0);
	} while (!(data & (1 << 0)));

	data = mmio_read_32(PERI_SC_PERIPH_CLKEN12);
	data |= 1 << 1;
	mmio_write_32(PERI_SC_PERIPH_CLKEN12, data);

	do {
		mmio_write_32(PERI_SC_CLKCFG8BIT1, (1 << 7) | 0xb);
		data = mmio_read_32(PERI_SC_CLKCFG8BIT1);
	} while ((data & 0xb) != 0xb);
}

static void reset_mmc0_clk(void)
{
	unsigned int data;

	/* disable mmc0 bus clock */
	mmio_write_32(PERI_SC_PERIPH_CLKDIS0, PERI_CLK0_MMC0);
	do {
		data = mmio_read_32(PERI_SC_PERIPH_CLKSTAT0);
	} while (data & PERI_CLK0_MMC0);
	/* enable mmc0 bus clock */
	mmio_write_32(PERI_SC_PERIPH_CLKEN0, PERI_CLK0_MMC0);
	do {
		data = mmio_read_32(PERI_SC_PERIPH_CLKSTAT0);
	} while (!(data & PERI_CLK0_MMC0));
	/* reset mmc0 clock domain */
	mmio_write_32(PERI_SC_PERIPH_RSTEN0, PERI_RST0_MMC0);

	/* bypass mmc0 clock phase */
	data = mmio_read_32(PERI_SC_PERIPH_CTRL2);
	data |= 3;
	mmio_write_32(PERI_SC_PERIPH_CTRL2, data);

	/* disable low power */
	data = mmio_read_32(PERI_SC_PERIPH_CTRL13);
	data |= 1 << 3;
	mmio_write_32(PERI_SC_PERIPH_CTRL13, data);
	do {
		data = mmio_read_32(PERI_SC_PERIPH_RSTSTAT0);
	} while (!(data & PERI_RST0_MMC0));

	/* unreset mmc0 clock domain */
	mmio_write_32(PERI_SC_PERIPH_RSTDIS0, PERI_RST0_MMC0);
	do {
		data = mmio_read_32(PERI_SC_PERIPH_RSTSTAT0);
	} while (data & PERI_RST0_MMC0);
}

static void init_media_clk(void)
{
	unsigned int data, value;

	data = mmio_read_32(PMCTRL_MEDPLLCTRL);
	data |= 1;
	mmio_write_32(PMCTRL_MEDPLLCTRL, data);

	for (;;) {
		data = mmio_read_32(PMCTRL_MEDPLLCTRL);
		value = 1 << 28;
		if ((data & value) == value)
			break;
	}

	data = mmio_read_32(PERI_SC_PERIPH_CLKEN12);
	data = 1 << 10;
	mmio_write_32(PERI_SC_PERIPH_CLKEN12, data);
}

static void init_mmc1_pll(void)
{
	uint32_t data;

	/* select SYSPLL as the source of MMC1 */
	/* select SYSPLL as the source of MUX1 (SC_CLK_SEL0) */
	mmio_write_32(PERI_SC_CLK_SEL0, 1 << 11 | 1 << 27);
	do {
		data = mmio_read_32(PERI_SC_CLK_SEL0);
	} while (!(data & (1 << 11)));
	/* select MUX1 as the source of MUX2 (SC_CLK_SEL0) */
	mmio_write_32(PERI_SC_CLK_SEL0, 1 << 30);
	do {
		data = mmio_read_32(PERI_SC_CLK_SEL0);
	} while (data & (1 << 14));

	mmio_write_32(PERI_SC_PERIPH_CLKEN0, (1 << 1));
	do {
		data = mmio_read_32(PERI_SC_PERIPH_CLKSTAT0);
	} while (!(data & (1 << 1)));

	data = mmio_read_32(PERI_SC_PERIPH_CLKEN12);
	data |= 1 << 2;
	mmio_write_32(PERI_SC_PERIPH_CLKEN12, data);

	do {
		/* 1.2GHz / 50 = 24MHz */
		mmio_write_32(PERI_SC_CLKCFG8BIT2, 0x31 | (1 << 7));
		data = mmio_read_32(PERI_SC_CLKCFG8BIT2);
	} while ((data & 0x31) != 0x31);
}

static void reset_mmc1_clk(void)
{
	unsigned int data;

	/* disable mmc1 bus clock */
	mmio_write_32(PERI_SC_PERIPH_CLKDIS0, PERI_CLK0_MMC1);
	do {
		data = mmio_read_32(PERI_SC_PERIPH_CLKSTAT0);
	} while (data & PERI_CLK0_MMC1);
	/* enable mmc1 bus clock */
	mmio_write_32(PERI_SC_PERIPH_CLKEN0, PERI_CLK0_MMC1);
	do {
		data = mmio_read_32(PERI_SC_PERIPH_CLKSTAT0);
	} while (!(data & PERI_CLK0_MMC1));
	/* reset mmc1 clock domain */
	mmio_write_32(PERI_SC_PERIPH_RSTEN0, PERI_RST0_MMC1);

	/* bypass mmc1 clock phase */
	data = mmio_read_32(PERI_SC_PERIPH_CTRL2);
	data |= 3 << 2;
	mmio_write_32(PERI_SC_PERIPH_CTRL2, data);

	/* disable low power */
	data = mmio_read_32(PERI_SC_PERIPH_CTRL13);
	data |= 1 << 4;
	mmio_write_32(PERI_SC_PERIPH_CTRL13, data);
	do {
		data = mmio_read_32(PERI_SC_PERIPH_RSTSTAT0);
	} while (!(data & PERI_RST0_MMC1));

	/* unreset mmc0 clock domain */
	mmio_write_32(PERI_SC_PERIPH_RSTDIS0, PERI_RST0_MMC1);
	do {
		data = mmio_read_32(PERI_SC_PERIPH_RSTSTAT0);
	} while (data & PERI_RST0_MMC1);
}

/* Initialize PLL of both eMMC and SD controllers. */
static void hikey_mmc_pll_init(void)
{
	init_mmc0_pll();
	reset_mmc0_clk();
	init_media_clk();

	dsb();

	init_mmc1_pll();
	reset_mmc1_clk();
}

static void hikey_rtc_init(void)
{
	uint32_t data;

	data = mmio_read_32(AO_SC_PERIPH_CLKEN4);
	data |= AO_SC_PERIPH_RSTDIS4_RESET_RTC0_N;
	mmio_write_32(AO_SC_PERIPH_CLKEN4, data);
}

/*
 * Function which will perform any remaining platform-specific setup that can
 * occur after the MMU and data cache have been enabled.
 */
void bl1_platform_setup(void)
{
	dw_mmc_params_t params;

	assert((HIKEY_BL1_MMC_DESC_BASE >= SRAM_BASE) &&
	       ((SRAM_BASE + SRAM_SIZE) >=
		(HIKEY_BL1_MMC_DATA_BASE + HIKEY_BL1_MMC_DATA_SIZE)));
	hikey_sp804_init();
	hikey_gpio_init();
	hikey_pmussi_init();
	hikey_hi6553_init();

	hikey_rtc_init();

	hikey_mmc_pll_init();

	memset(&params, 0, sizeof(dw_mmc_params_t));
	params.reg_base = DWMMC0_BASE;
	params.desc_base = HIKEY_BL1_MMC_DESC_BASE;
	params.desc_size = 1 << 20;
	params.clk_rate = 24 * 1000 * 1000;
	params.bus_width = EMMC_BUS_WIDTH_8;
	params.flags = EMMC_FLAG_CMD23;
	dw_mmc_init(&params);

	hikey_io_setup();
}

/*
 * The following function checks if Firmware update is needed,
 * by checking if TOC in FIP image is valid or not.
 */
unsigned int bl1_plat_get_next_image_id(void)
{
	int32_t boot_mode;
	unsigned int ret;

	boot_mode = mmio_read_32(ONCHIPROM_PARAM_BASE);
	switch (boot_mode) {
	case BOOT_NORMAL:
		ret = BL2_IMAGE_ID;
		break;
	case BOOT_USB_DOWNLOAD:
	case BOOT_UART_DOWNLOAD:
		ret = NS_BL1U_IMAGE_ID;
		break;
	default:
		WARN("Invalid boot mode is found:%d\n", boot_mode);
		panic();
	}
	return ret;
}

image_desc_t *bl1_plat_get_image_desc(unsigned int image_id)
{
	unsigned int index = 0;

	while (bl1_tbbr_image_descs[index].image_id != INVALID_IMAGE_ID) {
		if (bl1_tbbr_image_descs[index].image_id == image_id)
			return &bl1_tbbr_image_descs[index];

		index++;
	}

	return NULL;
}

void bl1_plat_set_ep_info(unsigned int image_id,
		entry_point_info_t *ep_info)
{
	unsigned int data = 0;

	if (image_id == BL2_IMAGE_ID)
		return;
	inv_dcache_range(NS_BL1U_BASE, NS_BL1U_SIZE);
	__asm__ volatile ("mrs	%0, cpacr_el1" : "=r"(data));
	do {
		data |= 3 << 20;
		__asm__ volatile ("msr	cpacr_el1, %0" : : "r"(data));
		__asm__ volatile ("mrs	%0, cpacr_el1" : "=r"(data));
	} while ((data & (3 << 20)) != (3 << 20));
	INFO("cpacr_el1:0x%x\n", data);

	ep_info->args.arg0 = 0xffff & read_mpidr();
	ep_info->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
				DISABLE_ALL_EXCEPTIONS);
}
