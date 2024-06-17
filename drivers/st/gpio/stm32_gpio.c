/*
 * Copyright (c) 2016-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/clk.h>
#include <drivers/st/stm32_gpio.h>
#include <drivers/st/stm32mp_clkfunc.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <libfdt.h>

#include <platform_def.h>

#define DT_GPIO_BANK_SHIFT	12
#define DT_GPIO_BANK_MASK	GENMASK(16, 12)
#define DT_GPIO_PIN_SHIFT	8
#define DT_GPIO_PIN_MASK	GENMASK(11, 8)
#define DT_GPIO_MODE_MASK	GENMASK(7, 0)

static void set_gpio(uint32_t bank, uint32_t pin, uint32_t mode, uint32_t type,
		     uint32_t speed, uint32_t pull, uint32_t od,
		     uint32_t alternate, uint8_t status);

/*******************************************************************************
 * This function gets GPIO bank node in DT.
 * Returns node offset if status is okay in DT, else return 0
 ******************************************************************************/
static int ckeck_gpio_bank(void *fdt, uint32_t bank, int pinctrl_node)
{
	int pinctrl_subnode;
	uint32_t bank_offset = stm32_get_gpio_bank_offset(bank);

	fdt_for_each_subnode(pinctrl_subnode, fdt, pinctrl_node) {
		const fdt32_t *cuint;

		if (fdt_getprop(fdt, pinctrl_subnode,
				"gpio-controller", NULL) == NULL) {
			continue;
		}

		cuint = fdt_getprop(fdt, pinctrl_subnode, "reg", NULL);
		if (cuint == NULL) {
			continue;
		}

		if ((fdt32_to_cpu(*cuint) == bank_offset) &&
		    (fdt_get_status(pinctrl_subnode) != DT_DISABLED)) {
			return pinctrl_subnode;
		}
	}

	return 0;
}

/*******************************************************************************
 * This function gets the pin settings from DT information.
 * When analyze and parsing is done, set the GPIO registers.
 * Returns 0 on success and a negative FDT error code on failure.
 ******************************************************************************/
static int dt_set_gpio_config(void *fdt, int node, uint8_t status)
{
	const fdt32_t *cuint, *slewrate;
	int len;
	int pinctrl_node;
	uint32_t i;
	uint32_t speed = GPIO_SPEED_LOW;
	uint32_t pull = GPIO_NO_PULL;

	cuint = fdt_getprop(fdt, node, "pinmux", &len);
	if (cuint == NULL) {
		return -FDT_ERR_NOTFOUND;
	}

	pinctrl_node = fdt_parent_offset(fdt, fdt_parent_offset(fdt, node));
	if (pinctrl_node < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	slewrate = fdt_getprop(fdt, node, "slew-rate", NULL);
	if (slewrate != NULL) {
		speed = fdt32_to_cpu(*slewrate);
	}

	if (fdt_getprop(fdt, node, "bias-pull-up", NULL) != NULL) {
		pull = GPIO_PULL_UP;
	} else if (fdt_getprop(fdt, node, "bias-pull-down", NULL) != NULL) {
		pull = GPIO_PULL_DOWN;
	} else {
		VERBOSE("No bias configured in node %d\n", node);
	}

	for (i = 0U; i < ((uint32_t)len / sizeof(uint32_t)); i++) {
		uint32_t pincfg;
		uint32_t bank;
		uint32_t pin;
		uint32_t mode;
		uint32_t alternate = GPIO_ALTERNATE_(0);
		uint32_t type;
		uint32_t od = GPIO_OD_OUTPUT_LOW;
		int bank_node;
		int clk;

		pincfg = fdt32_to_cpu(*cuint);
		cuint++;

		bank = (pincfg & DT_GPIO_BANK_MASK) >> DT_GPIO_BANK_SHIFT;

		pin = (pincfg & DT_GPIO_PIN_MASK) >> DT_GPIO_PIN_SHIFT;

		mode = pincfg & DT_GPIO_MODE_MASK;

		switch (mode) {
		case 0:
			mode = GPIO_MODE_INPUT;
			break;
		case 1 ... 16:
			alternate = mode - 1U;
			mode = GPIO_MODE_ALTERNATE;
			break;
		case 17:
			mode = GPIO_MODE_ANALOG;
			break;
		default:
			mode = GPIO_MODE_OUTPUT;
			break;
		}

		if (fdt_getprop(fdt, node, "drive-open-drain", NULL) != NULL) {
			type = GPIO_TYPE_OPEN_DRAIN;
		} else {
			type = GPIO_TYPE_PUSH_PULL;
		}

		if (fdt_getprop(fdt, node, "output-high", NULL) != NULL) {
			if (mode == GPIO_MODE_INPUT) {
				mode = GPIO_MODE_OUTPUT;
				od = GPIO_OD_OUTPUT_HIGH;
			}
		}

		if (fdt_getprop(fdt, node, "output-low", NULL) != NULL) {
			if (mode == GPIO_MODE_INPUT) {
				mode = GPIO_MODE_OUTPUT;
				od = GPIO_OD_OUTPUT_LOW;
			}
		}

		bank_node = ckeck_gpio_bank(fdt, bank, pinctrl_node);
		if (bank_node == 0) {
			ERROR("PINCTRL inconsistent in DT\n");
			panic();
		}

		clk = fdt_get_clock_id(bank_node);
		if (clk < 0) {
			return -FDT_ERR_NOTFOUND;
		}

		/* Platform knows the clock: assert it is okay */
		assert((unsigned long)clk == stm32_get_gpio_bank_clock(bank));

		set_gpio(bank, pin, mode, type, speed, pull, od, alternate, status);
	}

	return 0;
}

/*******************************************************************************
 * This function gets the pin settings from DT information.
 * When analyze and parsing is done, set the GPIO registers.
 * Returns 0 on success and a negative FDT/ERRNO error code on failure.
 ******************************************************************************/
int dt_set_pinctrl_config(int node)
{
	const fdt32_t *cuint;
	int lenp;
	uint32_t i;
	uint8_t status;
	void *fdt;

	if (fdt_get_address(&fdt) == 0) {
		return -FDT_ERR_NOTFOUND;
	}

	status = fdt_get_status(node);
	if (status == DT_DISABLED) {
		return -FDT_ERR_NOTFOUND;
	}

	cuint = fdt_getprop(fdt, node, "pinctrl-0", &lenp);
	if (cuint == NULL) {
		return -FDT_ERR_NOTFOUND;
	}

	for (i = 0; i < ((uint32_t)lenp / 4U); i++) {
		int p_node, p_subnode;

		p_node = fdt_node_offset_by_phandle(fdt, fdt32_to_cpu(*cuint));
		if (p_node < 0) {
			return -FDT_ERR_NOTFOUND;
		}

		fdt_for_each_subnode(p_subnode, fdt, p_node) {
			int ret = dt_set_gpio_config(fdt, p_subnode, status);

			if (ret < 0) {
				return ret;
			}
		}

		cuint++;
	}

	return 0;
}

static void set_gpio(uint32_t bank, uint32_t pin, uint32_t mode, uint32_t type,
		     uint32_t speed, uint32_t pull, uint32_t od,
		     uint32_t alternate, uint8_t status)
{
	uintptr_t base = stm32_get_gpio_bank_base(bank);
	unsigned long clock = stm32_get_gpio_bank_clock(bank);

	assert(pin <= GPIO_PIN_MAX);

	clk_enable(clock);

	mmio_clrsetbits_32(base + GPIO_MODE_OFFSET,
			   (uint32_t)GPIO_MODE_MASK << (pin << 1U),
			   mode << (pin << 1U));

	mmio_clrsetbits_32(base + GPIO_TYPE_OFFSET,
			   (uint32_t)GPIO_TYPE_MASK << pin,
			   type << pin);

	mmio_clrsetbits_32(base + GPIO_SPEED_OFFSET,
			   (uint32_t)GPIO_SPEED_MASK << (pin << 1U),
			   speed << (pin << 1U));

	mmio_clrsetbits_32(base + GPIO_PUPD_OFFSET,
			   (uint32_t)GPIO_PULL_MASK << (pin << 1U),
			   pull << (pin << 1U));

	if (pin < GPIO_ALT_LOWER_LIMIT) {
		mmio_clrsetbits_32(base + GPIO_AFRL_OFFSET,
				   (uint32_t)GPIO_ALTERNATE_MASK << (pin << 2U),
				   alternate << (pin << 2U));
	} else {
		uint32_t shift = (pin - GPIO_ALT_LOWER_LIMIT) << 2U;

		mmio_clrsetbits_32(base + GPIO_AFRH_OFFSET,
				   (uint32_t)GPIO_ALTERNATE_MASK << shift,
				   alternate << shift);
	}

	mmio_clrsetbits_32(base + GPIO_OD_OFFSET,
			   (uint32_t)GPIO_OD_MASK << pin,
			   od << pin);

	VERBOSE("GPIO %u mode set to 0x%x\n", bank,
		mmio_read_32(base + GPIO_MODE_OFFSET));
	VERBOSE("GPIO %u type set to 0x%x\n", bank,
		mmio_read_32(base + GPIO_TYPE_OFFSET));
	VERBOSE("GPIO %u speed set to 0x%x\n", bank,
		mmio_read_32(base + GPIO_SPEED_OFFSET));
	VERBOSE("GPIO %u mode pull to 0x%x\n", bank,
		mmio_read_32(base + GPIO_PUPD_OFFSET));
	VERBOSE("GPIO %u mode alternate low to 0x%x\n", bank,
		mmio_read_32(base + GPIO_AFRL_OFFSET));
	VERBOSE("GPIO %u mode alternate high to 0x%x\n", bank,
		mmio_read_32(base + GPIO_AFRH_OFFSET));
	VERBOSE("GPIO %u output data set to 0x%x\n", bank,
		mmio_read_32(base + GPIO_OD_OFFSET));

	clk_disable(clock);

#if STM32MP13 || STM32MP15
	if (status == DT_SECURE) {
		stm32mp_register_secure_gpio(bank, pin);
#if !IMAGE_BL2
		set_gpio_secure_cfg(bank, pin, true);
#endif

	} else {
		stm32mp_register_non_secure_gpio(bank, pin);
#if !IMAGE_BL2
		set_gpio_secure_cfg(bank, pin, false);
#endif
	}
#else /* !STM32MP13 && !STM32MP15 */
	set_gpio_secure_cfg(bank, pin, true);
#endif /* STM32MP13 || STM32MP15 */
}

void set_gpio_secure_cfg(uint32_t bank, uint32_t pin, bool secure)
{
	uintptr_t base = stm32_get_gpio_bank_base(bank);
	unsigned long clock = stm32_get_gpio_bank_clock(bank);

	assert(pin <= GPIO_PIN_MAX);

	clk_enable(clock);

	if (secure) {
		mmio_setbits_32(base + GPIO_SECR_OFFSET, BIT(pin));
	} else {
		mmio_clrbits_32(base + GPIO_SECR_OFFSET, BIT(pin));
	}

	clk_disable(clock);
}

void set_gpio_reset_cfg(uint32_t bank, uint32_t pin)
{
	set_gpio(bank, pin, GPIO_MODE_ANALOG, GPIO_TYPE_PUSH_PULL,
		 GPIO_SPEED_LOW, GPIO_NO_PULL, GPIO_OD_OUTPUT_LOW,
		 GPIO_ALTERNATE_(0), DT_DISABLED);
	set_gpio_secure_cfg(bank, pin, stm32_gpio_is_secure_at_reset(bank));
}

void set_gpio_level(uint32_t bank, uint32_t pin, enum gpio_level level)
{
	uintptr_t base = stm32_get_gpio_bank_base(bank);
	unsigned long clock = stm32_get_gpio_bank_clock(bank);

	assert(pin <= GPIO_PIN_MAX);

	clk_enable(clock);

	if (level == GPIO_LEVEL_HIGH) {
		mmio_write_32(base + GPIO_BSRR_OFFSET, BIT(pin));
	} else {
		mmio_write_32(base + GPIO_BSRR_OFFSET, BIT(pin + 16U));
	}

	VERBOSE("GPIO %u level set to 0x%x\n", bank,
		mmio_read_32(base + GPIO_IDR_OFFSET));

	clk_disable(clock);
}

enum gpio_level get_gpio_level(uint32_t bank, uint32_t pin)
{
	uintptr_t base = stm32_get_gpio_bank_base(bank);
	unsigned long clock = stm32_get_gpio_bank_clock(bank);
	enum gpio_level level = GPIO_LEVEL_LOW;

	assert(pin <= GPIO_PIN_MAX);

	clk_enable(clock);

	if (mmio_read_32(base + GPIO_IDR_OFFSET) & BIT(pin)) {
		level = GPIO_LEVEL_HIGH;
	}

	VERBOSE("GPIO %u get level 0x%x\n", bank,
		mmio_read_32(base + GPIO_IDR_OFFSET));

	clk_disable(clock);

	return level;
}

void set_gpio_config(uint32_t bank, uint32_t pin, uint32_t config, uint8_t status)
{
	uint32_t mode = GPIO_MODE_OUTPUT;
	uint32_t od = 0U;
	uint32_t pull = GPIO_NO_PULL;

	VERBOSE("GPIO %u:%u set config to 0x%x\n", bank, pin, config);

	if (config & GPIOF_DIR_IN) {
		mode = GPIO_MODE_INPUT;
	}

	if (config & GPIOF_OUT_INIT_HIGH) {
		od = 1U;
	}

	if (config & GPIOF_PULL_UP) {
		pull |= GPIO_PULL_UP;
	}

	if (config & GPIOF_PULL_DOWN) {
		pull |= GPIO_PULL_DOWN;
	}

	set_gpio(bank, pin, mode, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW,
		 pull, od, GPIO_ALTERNATE_(0), status);
}
