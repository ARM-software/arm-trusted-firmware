/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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
#include <board_gpio.h>
#include <da9212.h>
#include <debug.h>
#include <delay_timer.h>
#include <i2c.h>

enum {
	DA9212_SLAVE_ADDR_WRITE = 0xD0,
	DA9212_SLAVE_ADDR_READ = 0xD1,
};

enum {
	EXT_BUCK_DISABLE,
	EXT_BUCK_ENABLE
};

static uint8_t da9212_i2c_bus;
static struct board_gpio da9212_gpio_en_a;
static struct board_gpio da9212_gpio_en_b;

static unsigned char da9212_read(unsigned char reg, unsigned char mask,
				 unsigned char shift)
{
	unsigned char da9212_reg = 0;
	unsigned int ret = 0;
	unsigned char val;

	ret = i2c_readb(da9212_i2c_bus, DA9212_SLAVE_ADDR_WRITE >> 1,
			reg, &da9212_reg);
	if (ret)
		INFO("%s: i2c_readb ret = %d\n", __func__, ret);

	da9212_reg &= (mask << shift);
	val = (da9212_reg >> shift);

	return val;
}

static void da9212_write(unsigned char reg, unsigned char val,
			 unsigned char mask, unsigned char shift)
{
	unsigned char da9212_reg = 0;
	unsigned int ret = 0;

	ret = i2c_readb(da9212_i2c_bus, DA9212_SLAVE_ADDR_WRITE >> 1,
			reg, &da9212_reg);
	if (ret) {
		INFO("%s: i2c_readb ret = %d\n", __func__, ret);
		return;
	}

	da9212_reg &= ~(mask << shift);
	da9212_reg |= (val << shift);

	ret = i2c_writeb(da9212_i2c_bus, DA9212_SLAVE_ADDR_WRITE >> 1,
			 reg, da9212_reg);

	if (ret) {
		INFO("%s: i2c_writeb ret = %d\n", __func__, ret);
		return;
	}
}

static void da9212_hw_init(int en_a, int en_b)
{
	unsigned char reg_val = 0;

	/* page select to 0 after one access */
	da9212_write(DA9212_REG_PAGE_CON, 0x0, 0xF, DA9212_REG_PAGE_SHIFT);

	/* BUCKA_GPI = GPIO0 */
	if (en_a)
		da9212_write(DA9212_REG_BUCKA_CONT, 0x01, 0x03,
			     DA9212_BUCK_GPI_SHIFT);
	else
		da9212_write(DA9212_REG_BUCKA_CONT, 0x00, 0x03,
			     DA9212_BUCK_GPI_SHIFT);

	/* BUCKB_GPI = GPIO1 */
	if (en_b)
		da9212_write(DA9212_REG_BUCKB_CONT, 0x02, 0x03,
			     DA9212_BUCK_GPI_SHIFT);
	else
		da9212_write(DA9212_REG_BUCKB_CONT, 0x00, 0x03,
			     DA9212_BUCK_GPI_SHIFT);

	da9212_write(DA9212_REG_BUCKA_CONT, 0x00, 0x01,
		     DA9212_VBUCK_SEL_SHIFT); /* VBUCKA_A */
	da9212_write(DA9212_REG_BUCKB_CONT, 0x00, 0x01,
		     DA9212_VBUCK_SEL_SHIFT); /* VBUCKB_A */

	da9212_write(DA9212_REG_PAGE_CON, DA9212_REG_PAGE4, 0xF,
		     DA9212_REG_PAGE_SHIFT);

	reg_val = da9212_read((unsigned char)DA9212_VARIANT_ID, 0xFF, 0);

	da9212_write(DA9212_REG_PAGE_CON, DA9212_REG_PAGE0, 0xF,
		     DA9212_REG_PAGE_SHIFT);
	INFO("[da9212] DA9212_VARIANT_ID = 0x%x\n", reg_val);
	if (reg_val == DA9212_VARIANT_ID_AB) {
		/* Disable force PWM mode (this is reserve register) */
		da9212_write(DA9212_REG_BUCKA_CONF, DA9212_BUCK_MODE_PWM, 0x3,
			     DA9212_BUCK_MODE_SHIFT);
		/* Disable force PWM mode (this is reserve register) */
		da9212_write(DA9212_REG_BUCKB_CONF, DA9212_BUCK_MODE_PWM, 0x3,
			     DA9212_BUCK_MODE_SHIFT);
	} else {
		/* Disable force AUTO mode (this is reserve register) */
		da9212_write(DA9212_REG_BUCKA_CONF, DA9212_BUCK_MODE_AUTO, 0x3,
			     DA9212_BUCK_MODE_SHIFT);
		/* Disable force AUTO mode (this is reserve register) */
		da9212_write(DA9212_REG_BUCKB_CONF, DA9212_BUCK_MODE_AUTO, 0x3,
			     DA9212_BUCK_MODE_SHIFT);
	}

}

static int da9212_probe(int en_a, int en_b)
{
	unsigned char val = 0;

	/* select to page 4, clear REVERT at first time*/
	da9212_write(DA9212_REG_PAGE_CON, DA9212_REG_PAGE4, 0xF,
		     DA9212_REG_PAGE_SHIFT);

	val = da9212_read((unsigned char)DA9212_DEVICE_ID, 0xFF, 0);

	/* Check device ID is DA9212*/
	if (val != DA9212_ID) {
		INFO("%s: device_id = %d\n", __func__, val);
		return -1;
	}

	da9212_hw_init(en_a, en_b);
	return 0;
}

int da9212_init(void *sub_param)
{
	struct da9212_params *param = (struct da9212_params *)sub_param;
	struct board_gpio gpio_ic_en;
	int en_a = 0; /* bucka enable or not */
	int en_b = 0; /* buckb enable or not */

	da9212_i2c_bus = param->i2c_bus;

	fill_board_gpio(&param->gpio_en_a, &da9212_gpio_en_a);
	fill_board_gpio(&param->gpio_en_b, &da9212_gpio_en_b);
	fill_board_gpio(&param->gpio_ic_en, &gpio_ic_en);

	/* DA9212 IC ENABLE */
	assert(gpio_ic_en.output != NULL);
	if (gpio_ic_en.output)
		gpio_ic_en.output(gpio_ic_en.pin, gpio_ic_en.polarity);

	/* DA9212 BUCKA ENABLE */
	if (da9212_gpio_en_a.output) {
		da9212_gpio_en_a.output(da9212_gpio_en_a.pin,
					da9212_gpio_en_a.polarity);
		en_a = 1;
	}

	/* DA9212 BUCKB ENABLE */
	if (da9212_gpio_en_b.output) {
		da9212_gpio_en_b.output(da9212_gpio_en_b.pin,
					da9212_gpio_en_b.polarity);
		en_b = 1;
	}

	udelay(500); /* add 500us delay for powering on da9212 */
	return da9212_probe(en_a, en_b);
}

void da9212_ctrl(int enable)
{
	int value;

	/* [TODO] We should support to enable or disable DA9212's two bucks
	 * separately. */
	if (da9212_gpio_en_a.set) {
		value = enable ? da9212_gpio_en_a.polarity :
				!da9212_gpio_en_a.polarity;
		da9212_gpio_en_a.set(da9212_gpio_en_a.pin, value);
	}

	if (da9212_gpio_en_b.set) {
		value = enable ? da9212_gpio_en_b.polarity :
				!da9212_gpio_en_b.polarity;
		da9212_gpio_en_b.set(da9212_gpio_en_b.pin, value);
	}

	/* add 500us delay for powering DA9212 */
	if (enable)
		udelay(500);
}
