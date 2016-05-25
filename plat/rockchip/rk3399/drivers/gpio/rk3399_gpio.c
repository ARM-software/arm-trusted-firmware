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
#include <assert.h>
#include <debug.h>
#include <delay_timer.h>
#include <errno.h>
#include <gpio.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <plat_private.h>
#include <soc.h>

uint32_t gpio_port[] = {
	GPIO0_BASE,
	GPIO1_BASE,
	GPIO2_BASE,
	GPIO3_BASE,
	GPIO4_BASE,
};

#define SWPORTA_DR	0x00
#define SWPORTA_DDR	0x04
#define EXT_PORTA	0x50

#define PMU_GPIO_PORT0	0
#define PMU_GPIO_PORT1	1

#define PMU_GRF_GPIO0A_P	0x40
#define GRF_GPIO2A_P		0xe040
#define GPIO_P_MASK		0x03

/*
 * gpio clock disabled when not operate
 * so need to enable gpio clock before operate gpio
 * after setting, need to disable gpio clock
 * gate 1: disable clock; 0: enable clock
 */
static void gpio_clk(int gpio, uint32_t gate)
{
	uint32_t port = gpio / 32;

	assert(port < 5);

	switch (port) {
	case 0:
		mmio_write_32(PMUCRU_BASE + CRU_PMU_CLKGATE_CON(1),
			      BITS_WITH_WMASK(gate, CLK_GATE_MASK,
					      PCLK_GPIO0_GATE_SHIFT));
		break;
	case 1:
		mmio_write_32(PMUCRU_BASE + CRU_PMU_CLKGATE_CON(1),
			      BITS_WITH_WMASK(gate, CLK_GATE_MASK,
					      PCLK_GPIO1_GATE_SHIFT));
		break;
	case 2:
		mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(31),
			      BITS_WITH_WMASK(gate, CLK_GATE_MASK,
					      PCLK_GPIO2_GATE_SHIFT));
		break;
	case 3:
		mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(31),
			      BITS_WITH_WMASK(gate, CLK_GATE_MASK,
					      PCLK_GPIO3_GATE_SHIFT));

		break;
	case 4:
		mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(31),
			      BITS_WITH_WMASK(gate, CLK_GATE_MASK,
					      PCLK_GPIO4_GATE_SHIFT));
		break;
	default:
		break;
	}
}

static void set_pull(int gpio, int pull)
{
	uint32_t port = gpio / 32;
	uint32_t num = gpio % 32;
	uint32_t bank = num / 8;
	uint32_t id = num % 8;

	assert((port < 5) && (num < 32));

	gpio_clk(gpio, 0);

	/*
	 * in gpio0a, gpio0b, gpio2c, gpio2d,
	 * 00: Z
	 * 01: pull down
	 * 10: Z
	 * 11: pull up
	 * different with other gpio, so need to correct it
	 */
	if (((port == 0) && (bank < 2)) || ((port == 2) && (bank > 2))) {
		if (pull == GPIO_PULL_UP)
			pull = 3;
		else if (pull == GPIO_PULL_DOWN)
			pull = 1;
		else
			pull = 0;
	}

	if (port == PMU_GPIO_PORT0 || port == PMU_GPIO_PORT1) {
		mmio_write_32(PMUGRF_BASE + PMU_GRF_GPIO0A_P +
			      port * 16 + bank * 4,
			      BITS_WITH_WMASK(pull, GPIO_P_MASK, id * 2));
	} else {
		mmio_write_32(GRF_BASE + GRF_GPIO2A_P +
			      (port - 2) * 16 + bank * 4,
			      BITS_WITH_WMASK(pull, GPIO_P_MASK, id * 2));
	}
	gpio_clk(gpio, 1);
}

static void set_direction(int gpio, int direction)
{
	uint32_t port = gpio / 32;
	uint32_t num = gpio % 32;

	assert((port < 5) && (num < 32));

	gpio_clk(gpio, 0);

	/*
	 * in gpio.h
	 * #define GPIO_DIR_OUT	0
	 * #define GPIO_DIR_IN	1
	 * but rk3399 gpio direction 1: output, 0: input
	 * so need to revert direction value
	 */
	mmio_setbits_32(gpio_port[port] + SWPORTA_DDR, !direction << num);
	gpio_clk(gpio, 1);
}

static int get_direction(int gpio)
{
	uint32_t port = gpio / 32;
	uint32_t num = gpio % 32;
	int direction;

	assert((port < 5) && (num < 32));

	gpio_clk(gpio, 0);

	/*
	 * in gpio.h
	 * #define GPIO_DIR_OUT	0
	 * #define GPIO_DIR_IN	1
	 * but rk3399 gpio direction 1: output, 0: input
	 * so need to revert direction value
	 */
	direction = !((mmio_read_32(gpio_port[port] +
				    SWPORTA_DDR) >> num) & 0x1);
	gpio_clk(gpio, 1);

	return direction;
}

static int get_value(int gpio)
{
	uint32_t port = gpio / 32;
	uint32_t num = gpio % 32;
	int value;

	assert((port < 5) && (num < 32));

	gpio_clk(gpio, 0);
	value = (mmio_read_32(gpio_port[port] + EXT_PORTA) >> num) & 0x1;
	gpio_clk(gpio, 1);

	return value;
}

static void set_value(int gpio, int value)
{
	uint32_t port = gpio / 32;
	uint32_t num = gpio % 32;

	assert((port < 5) && (num < 32));

	gpio_clk(gpio, 0);
	mmio_clrsetbits_32(gpio_port[port] + SWPORTA_DR, 1 << num,
							 !!value << num);
	gpio_clk(gpio, 0);
}

const gpio_ops_t rk3399_gpio_ops = {
	.get_direction = get_direction,
	.set_direction = set_direction,
	.get_value = get_value,
	.set_value = set_value,
	.set_pull = set_pull,
};

void plat_rockchip_gpio_init(void)
{
	gpio_init(&rk3399_gpio_ops);
}
