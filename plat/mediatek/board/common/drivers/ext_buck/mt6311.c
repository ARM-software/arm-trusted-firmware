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
#include <debug.h>
#include <delay_timer.h>
#include <i2c.h>
#include <mt6311.h>

enum {
	MT6311_SLAVE_ADDR_WRITE = 0xd6,
	MT6311_SLAVE_ADDR_READ = 0xd7,
};

static int mt6311_i2c_num;
static uint8_t mt6311_read(uint8_t reg, uint8_t mask, uint8_t shift)
{
	uint8_t mt6311_reg = 0;
	int ret = 0;
	uint8_t val;

	ret = i2c_readb(mt6311_i2c_num, MT6311_SLAVE_ADDR_READ >> 1,
			reg, &mt6311_reg);
	if (ret)
		INFO("%s: i2c_readb ret = %d\n", __func__, ret);

	mt6311_reg &= (mask << shift);
	val = (mt6311_reg >> shift);

	return val;
}

static void mt6311_write(uint8_t reg, uint8_t val, uint8_t mask, uint8_t shift)
{
	uint8_t mt6311_reg = 0;
	int ret = 0;

	ret = i2c_readb(mt6311_i2c_num, MT6311_SLAVE_ADDR_READ >> 1,
			reg, &mt6311_reg);
	if (ret) {
		INFO("%s: i2c_readb ret = %d\n", __func__, ret);
		return;
	}

	mt6311_reg &= ~(mask << shift);
	mt6311_reg |= (val << shift);

	ret = i2c_writeb(mt6311_i2c_num, MT6311_SLAVE_ADDR_WRITE >> 1,
			 reg, mt6311_reg);

	if (ret) {
		INFO("%s: i2c_writeb ret = %d\n", __func__, ret);
		return;
	}
}

static uint32_t update_mt6311_chip_id(void)
{
	uint32_t id = 0;
	uint32_t id_l = 0;
	uint32_t id_r = 0;

	id_l = mt6311_read(MT6311_CID, 0xff, 0x0);
	id_r = mt6311_read(MT6311_SWCID, 0xff, 0x0);

	id = ((id_l << 8) | (id_r));

	return id;
}

static void mt6311_hw_init(void)
{
	uint8_t val;

	/*
	 * Phase Shedding Trim Software Setting
	 * The phase 2 of MT6311 will enter PWM mode if the threshold is
	 * reached.
	 * The threshold is set according to EFUSE value.
	 */
	val = mt6311_read(MT6311_EFUSE_DOUT_56_63, 0x3, 0x1);
	val |= (mt6311_read(MT6311_EFUSE_DOUT_56_63, 0x1, 0x7) << 2);
	val |= (mt6311_read(MT6311_EFUSE_DOUT_64_71, 0x1, 0x0) << 3);
	mt6311_write(MT6311_VDVFS1_ANA_CON10, val, 0xf, 0);
	/* I2C_CONFIG; pushpull setting, Opendrain is '0' */
	mt6311_write(MT6311_TOP_INT_CON, 0x1, 0x1, 2);
	/* RG_WDTRSTB_EN; CC, initial WDRSTB setting. */
	mt6311_write(MT6311_TOP_RST_CON, 0x1, 0x1, 5);
	/* initial INT function */
	mt6311_write(MT6311_GPIO_MODE, 0x2, 0x7, 3);
	mt6311_write(MT6311_STRUP_CON5, 0x0, 0x1, 0);
	mt6311_write(MT6311_STRUP_CON5, 0x0, 0x1, 1);
	mt6311_write(MT6311_STRUP_CON5, 0x0, 0x1, 2);
	/* Vo max is 1.15V */
	mt6311_write(MT6311_STRUP_ANA_CON1, 0x3, 0x3, 5);
	mt6311_write(MT6311_BUCK_ALL_CON23, 0x1, 0x1, 0);
	mt6311_write(MT6311_STRUP_ANA_CON2, 0x3, 0x3, 0);
	/* Suspend SW control from SPM, after Vosel_on */
	mt6311_write(MT6311_VDVFS11_CON7, 0x0, 0x1, 0);
	/* default VDVFS power on */
	mt6311_write(MT6311_VDVFS11_CON9, 0x1, 0x1, 0);
	/* for DVFS slew rate rising=0.67us */
	mt6311_write(MT6311_VDVFS11_CON10, 0x1, 0x7f, 0);
	/* for DVFS slew rate, falling 2.0us */
	mt6311_write(MT6311_VDVFS11_CON11, 0x5, 0x7f, 0);
	/* default VDVFS11_VOSEL 1.0V, SW control */
	mt6311_write(MT6311_VDVFS11_CON12, 0x40, 0x7f, 0);
	/* default VDVFS11_VOSEL_ON 1.0V, HW control */
	mt6311_write(MT6311_VDVFS11_CON13, 0x40, 0x7f, 0);
	mt6311_write(MT6311_VDVFS11_CON14, 0x40, 0x7f, 0);
	/* for DVFS sof change, falling 50us */
	mt6311_write(MT6311_VDVFS11_CON19, 0x3, 0x3, 0);
	/* for DVFS sof change, falling only */
	mt6311_write(MT6311_VDVFS11_CON19, 0x1, 0x3, 4);
	/* OFF LDO */
	mt6311_write(MT6311_LDO_CON3, 0x0, 0x1, 0);
}

static int mt6311_probe(int i2c_num)
{
	uint32_t val = 0;

	mt6311_i2c_num = i2c_num;

	/* Check device ID is MT6311 */
	val = update_mt6311_chip_id();
	if (val < MT6311_E1_CID_CODE) {
		INFO("%s: device_id = %d\n", __func__, val);
		return -1;
	}

	mt6311_hw_init();
	return 0;
}

int mt6311_init(void *sub_param)
{
	struct mt6311_params *param = (struct mt6311_params *)sub_param;

	return mt6311_probe(param->i2c_bus);
}

void mt6311_ctrl(int enable)
{
	i2c_clock_ctrl(mt6311_i2c_num, 1);
	i2c_writeb(mt6311_i2c_num, MT6311_SLAVE_ADDR_WRITE >> 1,
			 MT6311_VDVFS11_CON9, enable);
	i2c_clock_ctrl(mt6311_i2c_num, 0);

	/* Add 1ms delay for powering on mt6311 */
	if (enable)
		mdelay(1);
}
