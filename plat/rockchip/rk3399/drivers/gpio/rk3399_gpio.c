/*
 * Copyright (c) 2016-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/gpio.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include <plat_private.h>
#include <soc.h>

struct gpio_save {
	uint32_t swporta_dr;
	uint32_t swporta_ddr;
	uint32_t inten;
	uint32_t intmask;
	uint32_t inttype_level;
	uint32_t int_polarity;
	uint32_t debounce;
	uint32_t ls_sync;
} store_gpio[3];

static uint32_t store_grf_gpio[(GRF_GPIO2D_HE - GRF_GPIO2A_IOMUX) / 4 + 1];

#define SWPORTA_DR	0x00
#define SWPORTA_DDR	0x04
#define INTEN		0x30
#define INTMASK		0x34
#define INTTYPE_LEVEL	0x38
#define INT_POLARITY	0x3c
#define DEBOUNCE	0x48
#define LS_SYNC		0x60

#define EXT_PORTA	0x50
#define PMU_GPIO_PORT0	0
#define PMU_GPIO_PORT1	1
#define GPIO_PORT2	2
#define GPIO_PORT3	3
#define GPIO_PORT4	4

#define PMU_GRF_GPIO0A_P	0x40
#define GRF_GPIO2A_P		0xe040
#define GPIO_P_MASK		0x03

#define GET_GPIO_PORT(pin)	(pin / 32)
#define GET_GPIO_NUM(pin)	(pin % 32)
#define GET_GPIO_BANK(pin)	((pin % 32) / 8)
#define GET_GPIO_ID(pin)	((pin % 32) % 8)

enum {
	ENC_ZDZU,
	ENC_ZUDR,
	ENC_ZUDZ,
	NUM_ENC
};

static const struct port_info {
	uint32_t clkgate_reg;
	uint32_t pull_base;
	uint32_t port_base;
	/*
	 * Selects the pull mode encoding per bank,
	 * first index for pull_type_{hw2sw,sw2hw}
	 */
	uint8_t pull_enc[4];
	uint8_t clkgate_bit;
	uint8_t max_bank;
} port_info[] = {
	{
		.clkgate_reg = PMUCRU_BASE + CRU_PMU_CLKGATE_CON(1),
		.pull_base = PMUGRF_BASE + PMUGRF_GPIO0A_P,
		.port_base = GPIO0_BASE,
		.pull_enc = {ENC_ZDZU, ENC_ZDZU},
		.clkgate_bit = PCLK_GPIO0_GATE_SHIFT,
		.max_bank = 1,
	}, {
		.clkgate_reg = PMUCRU_BASE + CRU_PMU_CLKGATE_CON(1),
		.pull_base = PMUGRF_BASE + PMUGRF_GPIO1A_P,
		.port_base = GPIO1_BASE,
		.pull_enc = {ENC_ZUDR, ENC_ZUDR, ENC_ZUDR, ENC_ZUDR},
		.clkgate_bit = PCLK_GPIO1_GATE_SHIFT,
		.max_bank = 3,
	}, {
		.clkgate_reg = CRU_BASE + CRU_CLKGATE_CON(31),
		.pull_base = GRF_BASE + GRF_GPIO2A_P,
		.port_base = GPIO2_BASE,
		.pull_enc = {ENC_ZUDR, ENC_ZUDR, ENC_ZDZU, ENC_ZDZU},
		.clkgate_bit = PCLK_GPIO2_GATE_SHIFT,
		.max_bank = 3,
	}, {
		.clkgate_reg = CRU_BASE + CRU_CLKGATE_CON(31),
		.pull_base = GRF_BASE + GRF_GPIO3A_P,
		.port_base = GPIO3_BASE,
		.pull_enc = {ENC_ZUDR, ENC_ZUDR, ENC_ZUDR, ENC_ZUDR},
		.clkgate_bit = PCLK_GPIO3_GATE_SHIFT,
		.max_bank = 3,
	}, {
		.clkgate_reg = CRU_BASE + CRU_CLKGATE_CON(31),
		.pull_base = GRF_BASE + GRF_GPIO4A_P,
		.port_base = GPIO4_BASE,
		.pull_enc = {ENC_ZUDR, ENC_ZUDR, ENC_ZUDR, ENC_ZUDR},
		.clkgate_bit = PCLK_GPIO4_GATE_SHIFT,
		.max_bank = 3,
	}
};

/*
 * Mappings between TF-A constants and hardware encodings:
 * there are 3 different encoding schemes that may differ between
 * banks of the same port: the corresponding value of the pull_enc array
 * in port_info is used as the first index
 */
static const uint8_t pull_type_hw2sw[NUM_ENC][4] = {
	[ENC_ZDZU] = {GPIO_PULL_NONE, GPIO_PULL_DOWN, GPIO_PULL_NONE, GPIO_PULL_UP},
	[ENC_ZUDR] = {GPIO_PULL_NONE, GPIO_PULL_UP, GPIO_PULL_DOWN, GPIO_PULL_REPEATER},
	[ENC_ZUDZ] = {GPIO_PULL_NONE, GPIO_PULL_UP, GPIO_PULL_DOWN, GPIO_PULL_NONE}
};
static const uint8_t pull_type_sw2hw[NUM_ENC][4] = {
	[ENC_ZDZU] = {
		[GPIO_PULL_NONE] = 0,
		[GPIO_PULL_DOWN] = 1,
		[GPIO_PULL_UP] = 3,
		[GPIO_PULL_REPEATER] = -1
	},
	[ENC_ZUDR] = {
		[GPIO_PULL_NONE] = 0,
		[GPIO_PULL_DOWN] = 2,
		[GPIO_PULL_UP] = 1,
		[GPIO_PULL_REPEATER] = 3
	},
	[ENC_ZUDZ] = {
		[GPIO_PULL_NONE] = 0,
		[GPIO_PULL_DOWN] = 2,
		[GPIO_PULL_UP] = 1,
		[GPIO_PULL_REPEATER] = -1
	}
};

/* Return old clock state, enables clock, in order to do GPIO access */
static int gpio_get_clock(uint32_t gpio_number)
{
	uint32_t port = GET_GPIO_PORT(gpio_number);
	assert(port < 5U);

	const struct port_info *info = &port_info[port];

	if ((mmio_read_32(info->clkgate_reg) & (1U << info->clkgate_bit)) == 0U) {
		return 0;
	}
	mmio_write_32(
		info->clkgate_reg,
		BITS_WITH_WMASK(0, 1, info->clkgate_bit)
	);
	return 1;
}

/* Restore old state of gpio clock, assuming it is running now */
void gpio_put_clock(uint32_t gpio_number, uint32_t clock_state)
{
	if (clock_state == 0) {
		return;
	}
	uint32_t port = GET_GPIO_PORT(gpio_number);
	const struct port_info *info = &port_info[port];

	mmio_write_32(info->clkgate_reg, BITS_WITH_WMASK(1, 1, info->clkgate_bit));
}

static int get_pull(int gpio)
{
	uint32_t port = GET_GPIO_PORT(gpio);
	uint32_t bank = GET_GPIO_BANK(gpio);
	uint32_t id = GET_GPIO_ID(gpio);
	uint32_t val, clock_state;

	assert(port < 5U);
	const struct port_info *info = &port_info[port];

	assert(bank <= info->max_bank);

	clock_state = gpio_get_clock(gpio);
	val = (mmio_read_32(info->pull_base + 4 * bank) >> (id * 2)) & GPIO_P_MASK;
	gpio_put_clock(gpio, clock_state);

	return pull_type_hw2sw[info->pull_enc[bank]][val];
}

static void set_pull(int gpio, int pull)
{
	uint32_t port = GET_GPIO_PORT(gpio);
	uint32_t bank = GET_GPIO_BANK(gpio);
	uint32_t id = GET_GPIO_ID(gpio);
	uint32_t clock_state;

	assert(port < 5U);
	const struct port_info *info = &port_info[port];

	assert(bank <= info->max_bank);

	uint8_t val = pull_type_sw2hw[info->pull_enc[bank]][pull];

	assert(val != (uint8_t)-1);

	clock_state = gpio_get_clock(gpio);
	mmio_write_32(
		info->pull_base + 4 * bank,
		BITS_WITH_WMASK(val, GPIO_P_MASK, id * 2)
	);
	gpio_put_clock(gpio, clock_state);
}

static void set_direction(int gpio, int direction)
{
	uint32_t port = GET_GPIO_PORT(gpio);
	uint32_t num = GET_GPIO_NUM(gpio);
	uint32_t clock_state;

	assert((port < 5) && (num < 32));

	clock_state = gpio_get_clock(gpio);

	/*
	 * in gpio.h
	 * #define GPIO_DIR_OUT	0
	 * #define GPIO_DIR_IN	1
	 * but rk3399 gpio direction 1: output, 0: input
	 * so need to revert direction value
	 */
	mmio_setbits_32(
		port_info[port].port_base + SWPORTA_DDR,
		((direction == 0) ? 1 : 0) << num
	);
	gpio_put_clock(gpio, clock_state);
}

static int get_direction(int gpio)
{
	uint32_t port = GET_GPIO_PORT(gpio);
	uint32_t num = GET_GPIO_NUM(gpio);
	int direction, clock_state;

	assert((port < 5U) && (num < 32U));

	clock_state = gpio_get_clock(gpio);

	/*
	 * in gpio.h
	 * #define GPIO_DIR_OUT	0
	 * #define GPIO_DIR_IN	1
	 * but rk3399 gpio direction 1: output, 0: input
	 * so need to revert direction value
	 */
	direction = (((mmio_read_32(
		port_info[port].port_base + SWPORTA_DDR
	) >> num) & 1U) == 0) ? 1 : 0;
	gpio_put_clock(gpio, clock_state);

	return direction;
}

static int get_value(int gpio)
{
	uint32_t port = GET_GPIO_PORT(gpio);
	uint32_t num = GET_GPIO_NUM(gpio);
	int value, clock_state;

	assert((port < 5) && (num < 32));

	clock_state = gpio_get_clock(gpio);
	value = (mmio_read_32(port_info[port].port_base + EXT_PORTA) >> num) &
		0x1U;
	gpio_put_clock(gpio, clock_state);

	return value;
}

static void set_value(int gpio, int value)
{
	uint32_t port = GET_GPIO_PORT(gpio);
	uint32_t num = GET_GPIO_NUM(gpio);
	uint32_t clock_state;

	assert((port < 5U) && (num < 32U));

	clock_state = gpio_get_clock(gpio);
	mmio_clrsetbits_32(
		port_info[port].port_base + SWPORTA_DR,
		1 << num,
		((value == 0) ? 0 : 1) << num
	);
	gpio_put_clock(gpio, clock_state);
}

void plat_rockchip_save_gpio(void)
{
	unsigned int i;
	uint32_t cru_gate_save;

	cru_gate_save = mmio_read_32(CRU_BASE + CRU_CLKGATE_CON(31));

	/*
	 * when shutdown logic, we need to save gpio2 ~ gpio4 register,
	 * we need to enable gpio2 ~ gpio4 clock here, since it may be gating,
	 * and we do not care gpio0 and gpio1 clock gate, since we never
	 * gating them
	 */
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(31),
		      BITS_WITH_WMASK(0, 0x07, PCLK_GPIO2_GATE_SHIFT));

	/*
	 * since gpio0, gpio1 are pmugpio, they will keep ther value
	 * when shutdown logic power rail, so only need to save gpio2 ~ gpio4
	 * register value
	 */
	for (i = 2; i < 5; i++) {
		uint32_t base = port_info[i].port_base;

		store_gpio[i - 2] = (struct gpio_save) {
			.swporta_dr = mmio_read_32(base + SWPORTA_DR),
			.swporta_ddr = mmio_read_32(base + SWPORTA_DDR),
			.inten = mmio_read_32(base + INTEN),
			.intmask = mmio_read_32(base + INTMASK),
			.inttype_level = mmio_read_32(base + INTTYPE_LEVEL),
			.int_polarity = mmio_read_32(base + INT_POLARITY),
			.debounce = mmio_read_32(base + DEBOUNCE),
			.ls_sync = mmio_read_32(base + LS_SYNC),
		};
	}
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(31),
			cru_gate_save | REG_SOC_WMSK);

	/*
	 * gpio0, gpio1 in pmuiomux, they will keep ther value
	 * when shutdown logic power rail, so only need to save gpio2 ~ gpio4
	 * iomux register value
	 */
	for (i = 0; i < ARRAY_SIZE(store_grf_gpio); i++)
		store_grf_gpio[i] =
			mmio_read_32(GRF_BASE + GRF_GPIO2A_IOMUX + i * 4);
}

void plat_rockchip_restore_gpio(void)
{
	int i;
	uint32_t cru_gate_save;

	for (i = 0; i < ARRAY_SIZE(store_grf_gpio); i++)
		mmio_write_32(GRF_BASE + GRF_GPIO2A_IOMUX + i * 4,
		      REG_SOC_WMSK | store_grf_gpio[i]);

	cru_gate_save = mmio_read_32(CRU_BASE + CRU_CLKGATE_CON(31));

	/*
	 * when shutdown logic, we need to save gpio2 ~ gpio4 register,
	 * we need to enable gpio2 ~ gpio4 clock here, since it may be gating,
	 * and we do not care gpio0 and gpio1 clock gate, since we never
	 * gating them
	 */
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(31),
		      BITS_WITH_WMASK(0, 0x07, PCLK_GPIO2_GATE_SHIFT));

	for (i = 2; i < 5; i++) {
		uint32_t base = port_info[i].port_base;
		const struct gpio_save *save = &store_gpio[i - 2];

		mmio_write_32(base + SWPORTA_DR, save->swporta_dr);
		mmio_write_32(base + SWPORTA_DDR, save->swporta_ddr);
		mmio_write_32(base + INTEN, save->inten);
		mmio_write_32(base + INTMASK, save->intmask);
		mmio_write_32(base + INTTYPE_LEVEL, save->inttype_level),
		mmio_write_32(base + INT_POLARITY, save->int_polarity);
		mmio_write_32(base + DEBOUNCE, save->debounce);
		mmio_write_32(base + LS_SYNC, save->ls_sync);
	}
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(31),
			cru_gate_save | REG_SOC_WMSK);
}

const gpio_ops_t rk3399_gpio_ops = {
	.get_direction = get_direction,
	.set_direction = set_direction,
	.get_value = get_value,
	.set_value = set_value,
	.set_pull = set_pull,
	.get_pull = get_pull,
};

void plat_rockchip_gpio_init(void)
{
	gpio_init(&rk3399_gpio_ops);
}
