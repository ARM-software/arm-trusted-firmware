/*
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <drivers/clk.h>
#include <drivers/st/stm32_gpio.h>
#include <drivers/st/stm32_iwdg.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <libfdt.h>

#include <plat/common/platform.h>
#include <platform_def.h>

#if STM32MP13
#define TAMP_BOOT_MODE_BACKUP_REG_ID	U(30)
#endif
#if STM32MP15
#define TAMP_BOOT_MODE_BACKUP_REG_ID	U(20)
#endif

/*
 * Backup register to store fwu update information.
 * It should be writeable only by secure world, but also readable by non secure
 * (so it should be in Zone 2).
 */
#define TAMP_BOOT_FWU_INFO_REG_ID	U(10)

#if defined(IMAGE_BL2)
#define MAP_SEC_SYSRAM	MAP_REGION_FLAT(STM32MP_SYSRAM_BASE, \
					STM32MP_SYSRAM_SIZE, \
					MT_MEMORY | \
					MT_RW | \
					MT_SECURE | \
					MT_EXECUTE_NEVER)
#elif defined(IMAGE_BL32)
#define MAP_SEC_SYSRAM	MAP_REGION_FLAT(STM32MP_SEC_SYSRAM_BASE, \
					STM32MP_SEC_SYSRAM_SIZE, \
					MT_MEMORY | \
					MT_RW | \
					MT_SECURE | \
					MT_EXECUTE_NEVER)

/* Non-secure SYSRAM is used a uncached memory for SCMI message transfer */
#define MAP_NS_SYSRAM	MAP_REGION_FLAT(STM32MP_NS_SYSRAM_BASE, \
					STM32MP_NS_SYSRAM_SIZE, \
					MT_DEVICE | \
					MT_RW | \
					MT_NS | \
					MT_EXECUTE_NEVER)
#endif

#if STM32MP13
#define MAP_SRAM_ALL	MAP_REGION_FLAT(SRAMS_BASE, \
					SRAMS_SIZE_2MB_ALIGNED, \
					MT_MEMORY | \
					MT_RW | \
					MT_SECURE | \
					MT_EXECUTE_NEVER)
#endif

#define MAP_DEVICE1	MAP_REGION_FLAT(STM32MP1_DEVICE1_BASE, \
					STM32MP1_DEVICE1_SIZE, \
					MT_DEVICE | \
					MT_RW | \
					MT_SECURE | \
					MT_EXECUTE_NEVER)

#define MAP_DEVICE2	MAP_REGION_FLAT(STM32MP1_DEVICE2_BASE, \
					STM32MP1_DEVICE2_SIZE, \
					MT_DEVICE | \
					MT_RW | \
					MT_SECURE | \
					MT_EXECUTE_NEVER)

#if defined(IMAGE_BL2)
static const mmap_region_t stm32mp1_mmap[] = {
	MAP_SEC_SYSRAM,
#if STM32MP13
	MAP_SRAM_ALL,
#endif
	MAP_DEVICE1,
#if STM32MP_RAW_NAND
	MAP_DEVICE2,
#endif
	{0}
};
#endif
#if defined(IMAGE_BL32)
static const mmap_region_t stm32mp1_mmap[] = {
	MAP_SEC_SYSRAM,
	MAP_NS_SYSRAM,
	MAP_DEVICE1,
	MAP_DEVICE2,
	{0}
};
#endif

void configure_mmu(void)
{
	mmap_add(stm32mp1_mmap);
	init_xlat_tables();

	enable_mmu_svc_mon(0);
}

uintptr_t stm32_get_gpio_bank_base(unsigned int bank)
{
#if STM32MP13
	assert(bank <= GPIO_BANK_I);
#endif
#if STM32MP15
	if (bank == GPIO_BANK_Z) {
		return GPIOZ_BASE;
	}

	assert(bank <= GPIO_BANK_K);
#endif

	return GPIOA_BASE + (bank * GPIO_BANK_OFFSET);
}

uint32_t stm32_get_gpio_bank_offset(unsigned int bank)
{
#if STM32MP13
	assert(bank <= GPIO_BANK_I);
#endif
#if STM32MP15
	if (bank == GPIO_BANK_Z) {
		return 0;
	}

	assert(bank <= GPIO_BANK_K);
#endif

	return bank * GPIO_BANK_OFFSET;
}

bool stm32_gpio_is_secure_at_reset(unsigned int bank)
{
#if STM32MP13
	return true;
#endif
#if STM32MP15
	if (bank == GPIO_BANK_Z) {
		return true;
	}

	return false;
#endif
}

unsigned long stm32_get_gpio_bank_clock(unsigned int bank)
{
#if STM32MP13
	assert(bank <= GPIO_BANK_I);
#endif
#if STM32MP15
	if (bank == GPIO_BANK_Z) {
		return GPIOZ;
	}

	assert(bank <= GPIO_BANK_K);
#endif

	return GPIOA + (bank - GPIO_BANK_A);
}

int stm32_get_gpio_bank_pinctrl_node(void *fdt, unsigned int bank)
{
	const char *node_compatible = NULL;

	switch (bank) {
	case GPIO_BANK_A:
	case GPIO_BANK_B:
	case GPIO_BANK_C:
	case GPIO_BANK_D:
	case GPIO_BANK_E:
	case GPIO_BANK_F:
	case GPIO_BANK_G:
	case GPIO_BANK_H:
	case GPIO_BANK_I:
#if STM32MP13
		node_compatible = "st,stm32mp135-pinctrl";
		break;
#endif
#if STM32MP15
	case GPIO_BANK_J:
	case GPIO_BANK_K:
		node_compatible = "st,stm32mp157-pinctrl";
		break;
	case GPIO_BANK_Z:
		node_compatible = "st,stm32mp157-z-pinctrl";
		break;
#endif
	default:
		panic();
	}

	return fdt_node_offset_by_compatible(fdt, -1, node_compatible);
}

#if STM32MP_UART_PROGRAMMER || !defined(IMAGE_BL2)
/*
 * UART Management
 */
static const uintptr_t stm32mp1_uart_addresses[8] = {
	USART1_BASE,
	USART2_BASE,
	USART3_BASE,
	UART4_BASE,
	UART5_BASE,
	USART6_BASE,
	UART7_BASE,
	UART8_BASE,
};

uintptr_t get_uart_address(uint32_t instance_nb)
{
	if ((instance_nb == 0U) ||
	    (instance_nb > ARRAY_SIZE(stm32mp1_uart_addresses))) {
		return 0U;
	}

	return stm32mp1_uart_addresses[instance_nb - 1U];
}
#endif

#if STM32MP_USB_PROGRAMMER
struct gpio_bank_pin_list {
	uint32_t bank;
	uint32_t pin;
};

static const struct gpio_bank_pin_list gpio_list[] = {
	{	/* USART2_RX: GPIOA3 */
		.bank = 0U,
		.pin = 3U,
	},
	{	/* USART3_RX: GPIOB12 */
		.bank = 1U,
		.pin = 12U,
	},
	{	/* UART4_RX: GPIOB2 */
		.bank = 1U,
		.pin = 2U,
	},
	{	/* UART5_RX: GPIOB4 */
		.bank = 1U,
		.pin = 5U,
	},
	{	/* USART6_RX: GPIOC7 */
		.bank = 2U,
		.pin = 7U,
	},
	{	/* UART7_RX: GPIOF6 */
		.bank = 5U,
		.pin = 6U,
	},
	{	/* UART8_RX: GPIOE0 */
		.bank = 4U,
		.pin = 0U,
	},
};

void stm32mp1_deconfigure_uart_pins(void)
{
	size_t i;

	for (i = 0U; i < ARRAY_SIZE(gpio_list); i++) {
		set_gpio_reset_cfg(gpio_list[i].bank, gpio_list[i].pin);
	}
}
#endif

uint32_t stm32mp_get_chip_version(void)
{
#if STM32MP13
	return stm32mp_syscfg_get_chip_version();
#endif
#if STM32MP15
	uint32_t version = 0U;

	if (stm32mp1_dbgmcu_get_chip_version(&version) < 0) {
		INFO("Cannot get CPU version, debug disabled\n");
		return 0U;
	}

	return version;
#endif
}

uint32_t stm32mp_get_chip_dev_id(void)
{
#if STM32MP13
	return stm32mp_syscfg_get_chip_dev_id();
#endif
#if STM32MP15
	uint32_t dev_id;

	if (stm32mp1_dbgmcu_get_chip_dev_id(&dev_id) < 0) {
		INFO("Use default chip ID, debug disabled\n");
		dev_id = STM32MP1_CHIP_ID;
	}

	return dev_id;
#endif
}

static uint32_t get_part_number(void)
{
	static uint32_t part_number;

	if (part_number != 0U) {
		return part_number;
	}

	if (stm32_get_otp_value(PART_NUMBER_OTP, &part_number) != 0) {
		panic();
	}

	part_number = (part_number & PART_NUMBER_OTP_PART_MASK) >>
		PART_NUMBER_OTP_PART_SHIFT;

	part_number |= stm32mp_get_chip_dev_id() << 16;

	return part_number;
}

#if STM32MP15
static uint32_t get_cpu_package(void)
{
	uint32_t package;

	if (stm32_get_otp_value(PACKAGE_OTP, &package) != 0) {
		panic();
	}

	package = (package & PACKAGE_OTP_PKG_MASK) >>
		PACKAGE_OTP_PKG_SHIFT;

	return package;
}
#endif

void stm32mp_get_soc_name(char name[STM32_SOC_NAME_SIZE])
{
	const char *cpu_s, *cpu_r, *pkg;

	/* MPUs Part Numbers */
	switch (get_part_number()) {
#if STM32MP13
	case STM32MP135F_PART_NB:
		cpu_s = "135F";
		break;
	case STM32MP135D_PART_NB:
		cpu_s = "135D";
		break;
	case STM32MP135C_PART_NB:
		cpu_s = "135C";
		break;
	case STM32MP135A_PART_NB:
		cpu_s = "135A";
		break;
	case STM32MP133F_PART_NB:
		cpu_s = "133F";
		break;
	case STM32MP133D_PART_NB:
		cpu_s = "133D";
		break;
	case STM32MP133C_PART_NB:
		cpu_s = "133C";
		break;
	case STM32MP133A_PART_NB:
		cpu_s = "133A";
		break;
	case STM32MP131F_PART_NB:
		cpu_s = "131F";
		break;
	case STM32MP131D_PART_NB:
		cpu_s = "131D";
		break;
	case STM32MP131C_PART_NB:
		cpu_s = "131C";
		break;
	case STM32MP131A_PART_NB:
		cpu_s = "131A";
		break;
#endif
#if STM32MP15
	case STM32MP157C_PART_NB:
		cpu_s = "157C";
		break;
	case STM32MP157A_PART_NB:
		cpu_s = "157A";
		break;
	case STM32MP153C_PART_NB:
		cpu_s = "153C";
		break;
	case STM32MP153A_PART_NB:
		cpu_s = "153A";
		break;
	case STM32MP151C_PART_NB:
		cpu_s = "151C";
		break;
	case STM32MP151A_PART_NB:
		cpu_s = "151A";
		break;
	case STM32MP157F_PART_NB:
		cpu_s = "157F";
		break;
	case STM32MP157D_PART_NB:
		cpu_s = "157D";
		break;
	case STM32MP153F_PART_NB:
		cpu_s = "153F";
		break;
	case STM32MP153D_PART_NB:
		cpu_s = "153D";
		break;
	case STM32MP151F_PART_NB:
		cpu_s = "151F";
		break;
	case STM32MP151D_PART_NB:
		cpu_s = "151D";
		break;
#endif
	default:
		cpu_s = "????";
		break;
	}

	/* Package */
#if STM32MP13
	/* On STM32MP13, package is not present in OTP */
	pkg = "";
#endif
#if STM32MP15
	switch (get_cpu_package()) {
	case PKG_AA_LFBGA448:
		pkg = "AA";
		break;
	case PKG_AB_LFBGA354:
		pkg = "AB";
		break;
	case PKG_AC_TFBGA361:
		pkg = "AC";
		break;
	case PKG_AD_TFBGA257:
		pkg = "AD";
		break;
	default:
		pkg = "??";
		break;
	}
#endif

	/* REVISION */
	switch (stm32mp_get_chip_version()) {
	case STM32MP1_REV_B:
		cpu_r = "B";
		break;
#if STM32MP13
	case STM32MP1_REV_Y:
		cpu_r = "Y";
		break;
#endif
	case STM32MP1_REV_Z:
		cpu_r = "Z";
		break;
	default:
		cpu_r = "?";
		break;
	}

	snprintf(name, STM32_SOC_NAME_SIZE,
		 "STM32MP%s%s Rev.%s", cpu_s, pkg, cpu_r);
}

void stm32mp_print_cpuinfo(void)
{
	char name[STM32_SOC_NAME_SIZE];

	stm32mp_get_soc_name(name);
	NOTICE("CPU: %s\n", name);
}

void stm32mp_print_boardinfo(void)
{
	uint32_t board_id = 0U;

	if (stm32_get_otp_value(BOARD_ID_OTP, &board_id) != 0) {
		return;
	}

	if (board_id != 0U) {
		stm32_display_board_info(board_id);
	}
}

/* Return true when SoC provides a single Cortex-A7 core, and false otherwise */
bool stm32mp_is_single_core(void)
{
#if STM32MP13
	return true;
#endif
#if STM32MP15
	bool single_core = false;

	switch (get_part_number()) {
	case STM32MP151A_PART_NB:
	case STM32MP151C_PART_NB:
	case STM32MP151D_PART_NB:
	case STM32MP151F_PART_NB:
		single_core = true;
		break;
	default:
		break;
	}

	return single_core;
#endif
}

/* Return true when device is in closed state */
uint32_t stm32mp_check_closed_device(void)
{
	uint32_t value;

	if (stm32_get_otp_value(CFG0_OTP, &value) != 0) {
		return STM32MP_CHIP_SEC_CLOSED;
	}

#if STM32MP13
	value = (value & CFG0_OTP_MODE_MASK) >> CFG0_OTP_MODE_SHIFT;

	switch (value) {
	case CFG0_OPEN_DEVICE:
		return STM32MP_CHIP_SEC_OPEN;
	case CFG0_CLOSED_DEVICE:
	case CFG0_CLOSED_DEVICE_NO_BOUNDARY_SCAN:
	case CFG0_CLOSED_DEVICE_NO_JTAG:
		return STM32MP_CHIP_SEC_CLOSED;
	default:
		panic();
	}
#endif
#if STM32MP15
	if ((value & CFG0_CLOSED_DEVICE) == CFG0_CLOSED_DEVICE) {
		return STM32MP_CHIP_SEC_CLOSED;
	} else {
		return STM32MP_CHIP_SEC_OPEN;
	}

#endif
}

/* Return true when device supports secure boot */
bool stm32mp_is_auth_supported(void)
{
	bool supported = false;

	switch (get_part_number()) {
#if STM32MP13
	case STM32MP131C_PART_NB:
	case STM32MP131F_PART_NB:
	case STM32MP133C_PART_NB:
	case STM32MP133F_PART_NB:
	case STM32MP135C_PART_NB:
	case STM32MP135F_PART_NB:
#endif
#if STM32MP15
	case STM32MP151C_PART_NB:
	case STM32MP151F_PART_NB:
	case STM32MP153C_PART_NB:
	case STM32MP153F_PART_NB:
	case STM32MP157C_PART_NB:
	case STM32MP157F_PART_NB:
#endif
		supported = true;
		break;
	default:
		break;
	}

	return supported;
}

uint32_t stm32_iwdg_get_instance(uintptr_t base)
{
	switch (base) {
	case IWDG1_BASE:
		return IWDG1_INST;
	case IWDG2_BASE:
		return IWDG2_INST;
	default:
		panic();
	}
}

uint32_t stm32_iwdg_get_otp_config(uint32_t iwdg_inst)
{
	uint32_t iwdg_cfg = 0U;
	uint32_t otp_value;

	if (stm32_get_otp_value(HW2_OTP, &otp_value) != 0) {
		panic();
	}

	if ((otp_value & BIT(iwdg_inst + HW2_OTP_IWDG_HW_POS)) != 0U) {
		iwdg_cfg |= IWDG_HW_ENABLED;
	}

	if ((otp_value & BIT(iwdg_inst + HW2_OTP_IWDG_FZ_STOP_POS)) != 0U) {
		iwdg_cfg |= IWDG_DISABLE_ON_STOP;
	}

	if ((otp_value & BIT(iwdg_inst + HW2_OTP_IWDG_FZ_STANDBY_POS)) != 0U) {
		iwdg_cfg |= IWDG_DISABLE_ON_STANDBY;
	}

	return iwdg_cfg;
}

#if defined(IMAGE_BL2)
uint32_t stm32_iwdg_shadow_update(uint32_t iwdg_inst, uint32_t flags)
{
	uint32_t otp_value;
	uint32_t otp;
	uint32_t result;

	if (stm32_get_otp_index(HW2_OTP, &otp, NULL) != 0) {
		panic();
	}

	if (stm32_get_otp_value(HW2_OTP, &otp_value) != 0) {
		panic();
	}

	if ((flags & IWDG_DISABLE_ON_STOP) != 0) {
		otp_value |= BIT(iwdg_inst + HW2_OTP_IWDG_FZ_STOP_POS);
	}

	if ((flags & IWDG_DISABLE_ON_STANDBY) != 0) {
		otp_value |= BIT(iwdg_inst + HW2_OTP_IWDG_FZ_STANDBY_POS);
	}

	result = bsec_write_otp(otp_value, otp);
	if (result != BSEC_OK) {
		return result;
	}

	/* Sticky lock OTP_IWDG (read and write) */
	if ((bsec_set_sr_lock(otp) != BSEC_OK) ||
	    (bsec_set_sw_lock(otp) != BSEC_OK)) {
		return BSEC_LOCK_FAIL;
	}

	return BSEC_OK;
}
#endif

bool stm32mp_is_wakeup_from_standby(void)
{
	/* TODO add source code to determine if platform is waking up from standby mode */
	return false;
}

uintptr_t stm32_get_bkpr_boot_mode_addr(void)
{
	return tamp_bkpr(TAMP_BOOT_MODE_BACKUP_REG_ID);
}

#if PSA_FWU_SUPPORT
uintptr_t stm32_get_bkpr_fwu_info_addr(void)
{
	return tamp_bkpr(TAMP_BOOT_FWU_INFO_REG_ID);
}
#endif /* PSA_FWU_SUPPORT */
