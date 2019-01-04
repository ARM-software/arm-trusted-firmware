/*
 * Copyright (C) 2018 Marvell International Ltd.
 * Copyright (C) 2018 Icenowy Zheng <icenowy@aosc.io>
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

/*
 * This driver is for Mentor Graphics Inventra MI2CV IP core, which is used
 * for Marvell and Allwinner SoCs in ATF.
 */

#include <errno.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/mentor/mi2cv.h>
#include <lib/mmio.h>

#include <mentor_i2c_plat.h>

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
#define DEBUG_I2C
#endif

#define I2C_TIMEOUT_VALUE		0x500
#define I2C_MAX_RETRY_CNT		1000
#define I2C_CMD_WRITE			0x0
#define I2C_CMD_READ			0x1

#define I2C_DATA_ADDR_7BIT_OFFS		0x1
#define I2C_DATA_ADDR_7BIT_MASK		(0xFF << I2C_DATA_ADDR_7BIT_OFFS)

#define I2C_CONTROL_ACK			0x00000004
#define I2C_CONTROL_IFLG		0x00000008
#define I2C_CONTROL_STOP		0x00000010
#define I2C_CONTROL_START		0x00000020
#define I2C_CONTROL_TWSIEN		0x00000040
#define I2C_CONTROL_INTEN		0x00000080

#define I2C_STATUS_START			0x08
#define I2C_STATUS_REPEATED_START		0x10
#define I2C_STATUS_ADDR_W_ACK			0x18
#define I2C_STATUS_DATA_W_ACK			0x28
#define I2C_STATUS_LOST_ARB_DATA_ADDR_TRANSFER	0x38
#define I2C_STATUS_ADDR_R_ACK			0x40
#define I2C_STATUS_DATA_R_ACK			0x50
#define I2C_STATUS_DATA_R_NAK			0x58
#define I2C_STATUS_LOST_ARB_GENERAL_CALL	0x78
#define I2C_STATUS_IDLE				0xF8

#define I2C_UNSTUCK_TRIGGER			0x1
#define I2C_UNSTUCK_ONGOING			0x2
#define I2C_UNSTUCK_ERROR			0x4

static struct mentor_i2c_regs *base;

static int mentor_i2c_lost_arbitration(uint32_t *status)
{
	*status = mmio_read_32((uintptr_t)&base->status);
	if ((*status == I2C_STATUS_LOST_ARB_DATA_ADDR_TRANSFER) ||
	    (*status == I2C_STATUS_LOST_ARB_GENERAL_CALL))
		return -EAGAIN;

	return 0;
}

static void mentor_i2c_interrupt_clear(void)
{
	uint32_t reg;

	reg = mmio_read_32((uintptr_t)&base->control);
#ifndef I2C_INTERRUPT_CLEAR_INVERTED
	reg &= ~(I2C_CONTROL_IFLG);
#else
	reg |= I2C_CONTROL_IFLG;
#endif
	mmio_write_32((uintptr_t)&base->control, reg);
	/* Wait for 1 us for the clear to take effect */
	udelay(1);
}

static int mentor_i2c_interrupt_get(void)
{
	uint32_t reg;

	/* get the interrupt flag bit */
	reg = mmio_read_32((uintptr_t)&base->control);
	reg &= I2C_CONTROL_IFLG;
	return reg && I2C_CONTROL_IFLG;
}

static int mentor_i2c_wait_interrupt(void)
{
	uint32_t timeout = 0;

	while (!mentor_i2c_interrupt_get() && (timeout++ < I2C_TIMEOUT_VALUE))
		;
	if (timeout >= I2C_TIMEOUT_VALUE)
		return -ETIMEDOUT;

	return 0;
}

static int mentor_i2c_start_bit_set(void)
{
	int is_int_flag = 0;
	uint32_t status;

	if (mentor_i2c_interrupt_get())
		is_int_flag = 1;

	/* set start bit */
	mmio_write_32((uintptr_t)&base->control,
		      mmio_read_32((uintptr_t)&base->control) |
		      I2C_CONTROL_START);

	/* in case that the int flag was set before i.e. repeated start bit */
	if (is_int_flag) {
		VERBOSE("%s: repeated start Bit\n", __func__);
		mentor_i2c_interrupt_clear();
	}

	if (mentor_i2c_wait_interrupt()) {
		ERROR("Start clear bit timeout\n");
		return -ETIMEDOUT;
	}

	/* check that start bit went down */
	if ((mmio_read_32((uintptr_t)&base->control) &
	    I2C_CONTROL_START) != 0) {
		ERROR("Start bit didn't went down\n");
		return -EPERM;
	}

	/* check the status */
	if (mentor_i2c_lost_arbitration(&status)) {
		ERROR("%s - %d: Lost arbitration, got status %x\n",
		      __func__, __LINE__, status);
		return -EAGAIN;
	}
	if ((status != I2C_STATUS_START) &&
	    (status != I2C_STATUS_REPEATED_START)) {
		ERROR("Got status %x after enable start bit.\n", status);
		return -EPERM;
	}

	return 0;
}

static int mentor_i2c_stop_bit_set(void)
{
	int timeout;
	uint32_t status;

	/* Generate stop bit */
	mmio_write_32((uintptr_t)&base->control,
		      mmio_read_32((uintptr_t)&base->control) |
		      I2C_CONTROL_STOP);
	mentor_i2c_interrupt_clear();

	timeout = 0;
	/* Read control register, check the control stop bit */
	while ((mmio_read_32((uintptr_t)&base->control) & I2C_CONTROL_STOP) &&
	       (timeout++ < I2C_TIMEOUT_VALUE))
		;
	if (timeout >= I2C_TIMEOUT_VALUE) {
		ERROR("Stop bit didn't went down\n");
		return -ETIMEDOUT;
	}

	/* check that stop bit went down */
	if ((mmio_read_32((uintptr_t)&base->control) & I2C_CONTROL_STOP) != 0) {
		ERROR("Stop bit didn't went down\n");
		return -EPERM;
	}

	/* check the status */
	if (mentor_i2c_lost_arbitration(&status)) {
		ERROR("%s - %d: Lost arbitration, got status %x\n",
		      __func__, __LINE__, status);
		return -EAGAIN;
	}
	if (status != I2C_STATUS_IDLE) {
		ERROR("Got status %x after enable stop bit.\n", status);
		return -EPERM;
	}

	return 0;
}

static int mentor_i2c_address_set(uint8_t chain, int command)
{
	uint32_t reg, status;

	reg = (chain << I2C_DATA_ADDR_7BIT_OFFS) & I2C_DATA_ADDR_7BIT_MASK;
	reg |= command;
	mmio_write_32((uintptr_t)&base->data, reg);
	udelay(1);

	mentor_i2c_interrupt_clear();

	if (mentor_i2c_wait_interrupt()) {
		ERROR("Start clear bit timeout\n");
		return -ETIMEDOUT;
	}

	/* check the status */
	if (mentor_i2c_lost_arbitration(&status)) {
		ERROR("%s - %d: Lost arbitration, got status %x\n",
		      __func__, __LINE__, status);
		return -EAGAIN;
	}
	if (((status != I2C_STATUS_ADDR_R_ACK) && (command == I2C_CMD_READ)) ||
	   ((status != I2C_STATUS_ADDR_W_ACK) && (command == I2C_CMD_WRITE))) {
		/* only in debug, since in boot we try to read the SPD
		 * of both DRAM, and we don't want error messages in cas
		 * DIMM doesn't exist.
		 */
		INFO("%s: ERROR - status %x addr in %s mode.\n", __func__,
		     status, (command == I2C_CMD_WRITE) ? "Write" : "Read");
		return -EPERM;
	}

	return 0;
}

/*
 * The I2C module contains a clock divider to generate the SCL clock.
 * This function calculates and sets the <N> and <M> fields in the I2C Baud
 * Rate Register (t=01) to obtain given 'requested_speed'.
 * The requested_speed will be equal to:
 * CONFIG_SYS_TCLK / (10 * (M + 1) * (2 << N))
 * Where M is the value represented by bits[6:3] and N is the value represented
 * by bits[2:0] of "I2C Baud Rate Register".
 * Therefore max M which can be set is 16 (2^4) and max N is 8 (2^3). So the
 * lowest possible baudrate is:
 * CONFIG_SYS_TCLK/(10 * (16 +1) * (2 << 8), which equals to:
 * CONFIG_SYS_TCLK/87040. Assuming that CONFIG_SYS_TCLK=250MHz, the lowest
 * possible frequency is ~2,872KHz.
 */
static unsigned int mentor_i2c_bus_speed_set(unsigned int requested_speed)
{
	unsigned int n, m, freq, margin, min_margin = 0xffffffff;
	unsigned int actual_n = 0, actual_m = 0;
	int val;

	/* Calculate N and M for the TWSI clock baud rate */
	for (n = 0; n < 8; n++) {
		for (m = 0; m < 16; m++) {
			freq = CONFIG_SYS_TCLK / (10 * (m + 1) * (2 << n));
			val = requested_speed - freq;
			margin = (val > 0) ? val : -val;

			if ((freq <= requested_speed) &&
			    (margin < min_margin)) {
				min_margin = margin;
				actual_n = n;
				actual_m = m;
			}
		}
	}
	VERBOSE("%s: actual_n = %u, actual_m = %u\n",
		__func__, actual_n, actual_m);
	/* Set the baud rate */
	mmio_write_32((uintptr_t)&base->baudrate, (actual_m << 3) | actual_n);

	return 0;
}

#ifdef DEBUG_I2C
static int mentor_i2c_probe(uint8_t chip)
{
	int ret = 0;

	ret = mentor_i2c_start_bit_set();
	if (ret != 0) {
		mentor_i2c_stop_bit_set();
		ERROR("%s - %d: %s", __func__, __LINE__,
		      "mentor_i2c_start_bit_set failed\n");
		return -EPERM;
	}

	ret = mentor_i2c_address_set(chip, I2C_CMD_WRITE);
	if (ret != 0) {
		mentor_i2c_stop_bit_set();
		ERROR("%s - %d: %s", __func__, __LINE__,
		      "mentor_i2c_address_set failed\n");
		return -EPERM;
	}

	mentor_i2c_stop_bit_set();

	VERBOSE("%s: successful I2C probe\n", __func__);

	return ret;
}
#endif

/* regular i2c transaction */
static int mentor_i2c_data_receive(uint8_t *p_block, uint32_t block_size)
{
	uint32_t reg, status, block_size_read = block_size;

	/* Wait for cause interrupt */
	if (mentor_i2c_wait_interrupt()) {
		ERROR("Start clear bit timeout\n");
		return -ETIMEDOUT;
	}
	while (block_size_read) {
		if (block_size_read == 1) {
			reg = mmio_read_32((uintptr_t)&base->control);
			reg &= ~(I2C_CONTROL_ACK);
			mmio_write_32((uintptr_t)&base->control, reg);
		}
		mentor_i2c_interrupt_clear();

		if (mentor_i2c_wait_interrupt()) {
			ERROR("Start clear bit timeout\n");
			return -ETIMEDOUT;
		}
		/* check the status */
		if (mentor_i2c_lost_arbitration(&status)) {
			ERROR("%s - %d: Lost arbitration, got status %x\n",
			      __func__, __LINE__, status);
			return -EAGAIN;
		}
		if ((status != I2C_STATUS_DATA_R_ACK) &&
		    (block_size_read != 1)) {
			ERROR("Status %x in read transaction\n", status);
			return -EPERM;
		}
		if ((status != I2C_STATUS_DATA_R_NAK) &&
		    (block_size_read == 1)) {
			ERROR("Status %x in Rd Terminate\n", status);
			return -EPERM;
		}

		/* read the data */
		*p_block = (uint8_t) mmio_read_32((uintptr_t)&base->data);
		VERBOSE("%s: place %d read %x\n", __func__,
			block_size - block_size_read, *p_block);
		p_block++;
		block_size_read--;
	}

	return 0;
}

static int mentor_i2c_data_transmit(uint8_t *p_block, uint32_t block_size)
{
	uint32_t status, block_size_write = block_size;

	if (mentor_i2c_wait_interrupt()) {
		ERROR("Start clear bit timeout\n");
		return -ETIMEDOUT;
	}

	while (block_size_write) {
		/* write the data */
		mmio_write_32((uintptr_t)&base->data, (uint32_t) *p_block);
		VERBOSE("%s: index = %d, data = %x\n", __func__,
			block_size - block_size_write, *p_block);
		p_block++;
		block_size_write--;

		mentor_i2c_interrupt_clear();

		if (mentor_i2c_wait_interrupt()) {
			ERROR("Start clear bit timeout\n");
			return -ETIMEDOUT;
		}

		/* check the status */
		if (mentor_i2c_lost_arbitration(&status)) {
			ERROR("%s - %d: Lost arbitration, got status %x\n",
			      __func__, __LINE__, status);
			return -EAGAIN;
		}
		if (status != I2C_STATUS_DATA_W_ACK) {
			ERROR("Status %x in write transaction\n", status);
			return -EPERM;
		}
	}

	return 0;
}

static int mentor_i2c_target_offset_set(uint8_t chip, uint32_t addr, int alen)
{
	uint8_t off_block[2];
	uint32_t off_size;

	if (alen == 2) { /* 2-byte addresses support */
		off_block[0] = (addr >> 8) & 0xff;
		off_block[1] = addr & 0xff;
		off_size = 2;
	} else { /* 1-byte addresses support */
		off_block[0] = addr & 0xff;
		off_size = 1;
	}
	VERBOSE("%s: off_size = %x addr1 = %x addr2 = %x\n", __func__,
		off_size, off_block[0], off_block[1]);
	return mentor_i2c_data_transmit(off_block, off_size);
}

#ifdef I2C_CAN_UNSTUCK
static int mentor_i2c_unstuck(int ret)
{
	uint32_t v;

	if (ret != -ETIMEDOUT)
		return ret;
	VERBOSE("Trying to \"unstuck i2c\"... ");
	i2c_init(base);
	mmio_write_32((uintptr_t)&base->unstuck, I2C_UNSTUCK_TRIGGER);
	do {
		v = mmio_read_32((uintptr_t)&base->unstuck);
	} while (v & I2C_UNSTUCK_ONGOING);

	if (v & I2C_UNSTUCK_ERROR) {
		VERBOSE("failed - soft reset i2c\n");
		ret = -EPERM;
	} else {
		VERBOSE("ok\n");
		i2c_init(base);
		ret = -EAGAIN;
	}
	return ret;
}
#else
static int mentor_i2c_unstuck(int ret)
{
	VERBOSE("Cannot \"unstuck i2c\" - soft reset i2c\n");
	return -EPERM;
}
#endif

/*
 * API Functions
 */
void i2c_init(void *i2c_base)
{
	/* For I2C speed and slave address, now we do not set them since
	 * we just provide the working speed and slave address otherwhere
	 * for i2c_init
	 */
	base = (struct mentor_i2c_regs *)i2c_base;

	/* Reset the I2C logic */
	mmio_write_32((uintptr_t)&base->soft_reset, 0);

	udelay(200);

	mentor_i2c_bus_speed_set(CONFIG_SYS_I2C_SPEED);

	/* Enable the I2C and slave */
	mmio_write_32((uintptr_t)&base->control,
		      I2C_CONTROL_TWSIEN | I2C_CONTROL_ACK);

	/* set the I2C slave address */
	mmio_write_32((uintptr_t)&base->xtnd_slave_addr, 0);
	mmio_write_32((uintptr_t)&base->slave_address, CONFIG_SYS_I2C_SLAVE);

	/* unmask I2C interrupt */
	mmio_write_32((uintptr_t)&base->control,
		      mmio_read_32((uintptr_t)&base->control) |
		      I2C_CONTROL_INTEN);

	udelay(10);
}

/*
 * i2c_read: - Read multiple bytes from an i2c device
 *
 * The higher level routines take into account that this function is only
 * called with len < page length of the device (see configuration file)
 *
 * @chip:	address of the chip which is to be read
 * @addr:	i2c data address within the chip
 * @alen:	length of the i2c data address (1..2 bytes)
 * @buffer:	where to write the data
 * @len:	how much byte do we want to read
 * @return:	0 in case of success
 */
int i2c_read(uint8_t chip, uint32_t addr, int alen, uint8_t *buffer, int len)
{
	int ret = 0;
	uint32_t counter = 0;

#ifdef DEBUG_I2C
	mentor_i2c_probe(chip);
#endif

	do {
		if (ret != -EAGAIN && ret) {
			ERROR("i2c transaction failed, after %d retries\n",
			      counter);
			mentor_i2c_stop_bit_set();
			return ret;
		}

		/* wait for 1 us for the interrupt clear to take effect */
		if (counter > 0)
			udelay(1);
		counter++;

		ret = mentor_i2c_start_bit_set();
		if (ret) {
			ret = mentor_i2c_unstuck(ret);
			continue;
		}

		/* if EEPROM device */
		if (alen != 0) {
			ret = mentor_i2c_address_set(chip, I2C_CMD_WRITE);
			if (ret)
				continue;

			ret = mentor_i2c_target_offset_set(chip, addr, alen);
			if (ret)
				continue;
			ret = mentor_i2c_start_bit_set();
			if (ret)
				continue;
		}

		ret =  mentor_i2c_address_set(chip, I2C_CMD_READ);
		if (ret)
			continue;

		ret = mentor_i2c_data_receive(buffer, len);
		if (ret)
			continue;

		ret =  mentor_i2c_stop_bit_set();
	} while ((ret == -EAGAIN) && (counter < I2C_MAX_RETRY_CNT));

	if (counter == I2C_MAX_RETRY_CNT) {
		ERROR("I2C transactions failed, got EAGAIN %d times\n",
		      I2C_MAX_RETRY_CNT);
		ret = -EPERM;
	}
	mmio_write_32((uintptr_t)&base->control,
		      mmio_read_32((uintptr_t)&base->control) |
		      I2C_CONTROL_ACK);

	udelay(1);
	return ret;
}

/*
 * i2c_write: -  Write multiple bytes to an i2c device
 *
 * The higher level routines take into account that this function is only
 * called with len < page length of the device (see configuration file)
 *
 * @chip:	address of the chip which is to be written
 * @addr:	i2c data address within the chip
 * @alen:	length of the i2c data address (1..2 bytes)
 * @buffer:	where to find the data to be written
 * @len:	how much byte do we want to read
 * @return:	0 in case of success
 */
int i2c_write(uint8_t chip, uint32_t addr, int alen, uint8_t *buffer, int len)
{
	int ret = 0;
	uint32_t counter = 0;

	do {
		if (ret != -EAGAIN && ret) {
			ERROR("i2c transaction failed\n");
			mentor_i2c_stop_bit_set();
			return ret;
		}
		/* wait for 1 us for the interrupt clear to take effect */
		if (counter > 0)
			udelay(1);
		counter++;

		ret = mentor_i2c_start_bit_set();
		if (ret) {
			ret = mentor_i2c_unstuck(ret);
			continue;
		}

		ret = mentor_i2c_address_set(chip, I2C_CMD_WRITE);
		if (ret)
			continue;

		/* if EEPROM device */
		if (alen != 0) {
			ret = mentor_i2c_target_offset_set(chip, addr, alen);
			if (ret)
				continue;
		}

		ret = mentor_i2c_data_transmit(buffer, len);
		if (ret)
			continue;

		ret = mentor_i2c_stop_bit_set();
	} while ((ret == -EAGAIN) && (counter < I2C_MAX_RETRY_CNT));

	if (counter == I2C_MAX_RETRY_CNT) {
		ERROR("I2C transactions failed, got EAGAIN %d times\n",
		      I2C_MAX_RETRY_CNT);
		ret = -EPERM;
	}

	udelay(1);
	return ret;
}
