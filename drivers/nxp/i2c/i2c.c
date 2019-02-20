/*
 * Copyright 2016-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <platform_def.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <common/debug.h>
#include <io.h>
#include <drivers/delay_timer.h>
#include <i2c.h>

extern uint64_t get_timer_val(uint64_t start);

void i2c_init(void)
{
	struct ls_i2c *ccsr_i2c = (void *)NXP_I2C_ADDR;

	/* Presume workaround for erratum a009203 applied */
	i2c_out(&ccsr_i2c->cr, I2C_CR_DIS);
	i2c_out(&ccsr_i2c->fd, I2C_FD_CONSERV);
	i2c_out(&ccsr_i2c->sr, I2C_SR_RST);
	i2c_out(&ccsr_i2c->cr, I2C_CR_EN);
}

static int wait_for_state(struct ls_i2c *ccsr_i2c,
			  unsigned char state, unsigned char mask)
{
	unsigned char sr;
	uint64_t start_time = get_timer_val(0);
	uint64_t timer;

	do {
		sr = i2c_in(&ccsr_i2c->sr);
		if (sr & I2C_SR_AL) {
			i2c_out(&ccsr_i2c->sr, sr);
			WARN("I2C arbitration lost\n");
			return -EIO;
		}
		if ((sr & mask) == state)
			return sr;

		timer = get_timer_val(start_time);
		if (timer > I2C_TIMEOUT)
			break;
		mdelay(1);
	} while (1);
	WARN("I2C: Timeout waiting for state 0x%x, sr = 0x%x\n", state, sr);

	return -ETIMEDOUT;
}

static int tx_byte(struct ls_i2c *ccsr_i2c, unsigned char c)
{
	int ret;

	i2c_out(&ccsr_i2c->sr, I2C_SR_IF);
	i2c_out(&ccsr_i2c->dr, c);
	ret = wait_for_state(ccsr_i2c, I2C_SR_IF, I2C_SR_IF);
	if (ret < 0) {
		WARN("%s: state error\n", __func__);
		return ret;
	}
	if (ret & I2C_SR_RX_NAK) {
		WARN("%s: nodev\n", __func__);
		return -ENODEV;
	}

	return 0;
}

static void gen_stop(struct ls_i2c *ccsr_i2c)
{
	unsigned char cr;

	cr = i2c_in(&ccsr_i2c->cr);
	cr &= ~(I2C_CR_MA | I2C_CR_TX);
	i2c_out(&ccsr_i2c->cr, cr);
	if (wait_for_state(ccsr_i2c, I2C_SR_IDLE, I2C_SR_BB) < 0)
		WARN("I2C: generating stop failed\n");
}

static int i2c_write_addr(struct ls_i2c *ccsr_i2c, unsigned char chip,
			int addr, int alen)
{
	int ret;
	unsigned char cr;

	if (i2c_in(&ccsr_i2c->ad) == (chip << 1)) {
		WARN("I2C: slave address same as self\n");
		return -ENODEV;
	}
	i2c_out(&ccsr_i2c->sr, I2C_SR_IF);
	ret = wait_for_state(ccsr_i2c, I2C_SR_IDLE, I2C_SR_BB);
	if (ret < 0)
		return ret;

	cr = i2c_in(&ccsr_i2c->cr);
	cr |= I2C_CR_MA;
	i2c_out(&ccsr_i2c->cr, cr);
	ret = wait_for_state(ccsr_i2c, I2C_SR_BB, I2C_SR_BB);
	if (ret < 0)
		return ret;

	VERBOSE("before writing chip %d\n", chip);
	cr |= I2C_CR_TX | I2C_CR_TX_NAK;
	i2c_out(&ccsr_i2c->cr, cr);
	ret = tx_byte(ccsr_i2c, chip << 1);
	if (ret < 0) {
		gen_stop(ccsr_i2c);
		return ret;
	}

	VERBOSE("before writing addr\n");
	while (alen--) {
		ret = tx_byte(ccsr_i2c, (addr >> (alen << 3)) & 0xff);
		if (ret < 0) {
			gen_stop(ccsr_i2c);
			return ret;
		}
	}

	return 0;
}

static int read_data(struct ls_i2c *ccsr_i2c, unsigned char chip,
		     unsigned char *buf, int len)
{
	int i;
	int ret;
	unsigned char cr;

	cr = i2c_in(&ccsr_i2c->cr);
	cr &= ~(I2C_CR_TX | I2C_CR_TX_NAK);
	if (len == 1)
		cr |= I2C_CR_TX_NAK;
	i2c_out(&ccsr_i2c->cr, cr);
	i2c_out(&ccsr_i2c->sr, I2C_SR_IF);
	i2c_in(&ccsr_i2c->dr);	/* dummy read */
	for (i = 0; i < len; i++) {
		ret = wait_for_state(ccsr_i2c, I2C_SR_IF, I2C_SR_IF);
		if (ret < 0) {
			gen_stop(ccsr_i2c);
			return ret;
		}
		if (i == (len - 1)) {
			gen_stop(ccsr_i2c);
		} else if (i == (len - 2)) {
			cr = i2c_in(&ccsr_i2c->cr);
			cr |= I2C_CR_TX_NAK;
			i2c_out(&ccsr_i2c->cr, cr);
		}
		i2c_out(&ccsr_i2c->sr, I2C_SR_IF);
		buf[i] = i2c_in(&ccsr_i2c->dr);
	}

	return 0;
}

static int write_data(struct ls_i2c *ccsr_i2c, unsigned char chip,
		      const unsigned char *buf, int len)
{
	int i;
	int ret = 0;

	for (i = 0; i < len; i++) {
		ret = tx_byte(ccsr_i2c, buf[i]);
		if (ret < 0)
			break;
	}
	gen_stop(ccsr_i2c);

	return ret;
}


int i2c_read(unsigned char chip, int addr, int alen,
		unsigned char *buf, int len)
{
	int ret;
	unsigned char cr;
	struct ls_i2c *ccsr_i2c = (void *)NXP_I2C_ADDR;

	ret = i2c_write_addr(ccsr_i2c, chip, addr, alen);
	if (ret < 0) {
		gen_stop(ccsr_i2c);
		return ret;
	}

	cr = i2c_in(&ccsr_i2c->cr);
	cr |= I2C_CR_RSTA;
	i2c_out(&ccsr_i2c->cr, cr);

	ret = tx_byte(ccsr_i2c, (chip << 1) | 1);
	if (ret < 0) {
		gen_stop(ccsr_i2c);
		return ret;
	}

	return read_data(ccsr_i2c, chip, buf, len);
}

int i2c_write(unsigned char chip, int addr, int alen,
		const unsigned char *buf, int len)
{
	int ret;
	struct ls_i2c *ccsr_i2c = (void *)NXP_I2C_ADDR;

	ret = i2c_write_addr(ccsr_i2c, chip, addr, alen);
	if (ret < 0)
		return ret;

	return write_data(ccsr_i2c, chip, buf, len);
}

int i2c_probe_chip(unsigned char chip)
{
	int ret;
	struct ls_i2c *ccsr_i2c = (void *)NXP_I2C_ADDR;

	ret = i2c_write_addr(ccsr_i2c, chip, 0, 0);
	if (ret < 0) {
		WARN("write addr failed\n");
		return ret;
	}

	gen_stop(ccsr_i2c);

	return 0;
}
