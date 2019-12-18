/*
 * Copyright (c) 2017 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <ocotp.h>
#include <platform_def.h>

#define OTP_MAP 2
#define OTP_NUM_WORDS 2048
/*
 * # of tries for OTP Status. The time to execute a command varies. The slowest
 * commands are writes which also vary based on the # of bits turned on. Writing
 * 0xffffffff takes ~3800 us.
 */
#define OTPC_RETRIES_US                 5000

/* Sequence to enable OTP program */
#define OTPC_PROG_EN_SEQ             { 0xf, 0x4, 0x8, 0xd }

/* OTPC Commands */
#define OTPC_CMD_READ                0x0
#define OTPC_CMD_OTP_PROG_ENABLE     0x2
#define OTPC_CMD_OTP_PROG_DISABLE    0x3
#define OTPC_CMD_PROGRAM             0x8
#define OTPC_CMD_ECC                 0x10
#define OTPC_ECC_ADDR                0x1A
#define OTPC_ECC_VAL                 0x00EC0000

/* OTPC Status Bits */
#define OTPC_STAT_CMD_DONE           BIT(1)
#define OTPC_STAT_PROG_OK            BIT(2)

/* OTPC register definition */
#define OTPC_MODE_REG_OFFSET         0x0
#define OTPC_MODE_REG_OTPC_MODE      0
#define OTPC_COMMAND_OFFSET          0x4
#define OTPC_COMMAND_COMMAND_WIDTH   6
#define OTPC_CMD_START_OFFSET        0x8
#define OTPC_CMD_START_START         0
#define OTPC_CPU_STATUS_OFFSET       0xc
#define OTPC_CPUADDR_REG_OFFSET      0x28
#define OTPC_CPUADDR_REG_OTPC_CPU_ADDRESS_WIDTH 16
#define OTPC_CPU_WRITE_REG_OFFSET    0x2c

#define OTPC_CMD_MASK  (BIT(OTPC_COMMAND_COMMAND_WIDTH) - 1)
#define OTPC_ADDR_MASK (BIT(OTPC_CPUADDR_REG_OTPC_CPU_ADDRESS_WIDTH) - 1)

#define OTPC_MODE_REG			OCOTP_REGS_BASE

struct chip_otp_cfg {
	uint32_t base;
	uint32_t num_words;
};

struct chip_otp_cfg ocotp_cfg = {
	.base = OTPC_MODE_REG,
	.num_words = 2048,
};

struct otpc_priv {
	uint32_t base;
	struct otpc_map *map;
	int size;
	int state;
};

struct otpc_priv otpc_info;

static inline void set_command(uint32_t base, uint32_t command)
{
	mmio_write_32(base + OTPC_COMMAND_OFFSET, command & OTPC_CMD_MASK);
}

static inline void set_cpu_address(uint32_t base, uint32_t addr)
{
	mmio_write_32(base + OTPC_CPUADDR_REG_OFFSET, addr & OTPC_ADDR_MASK);
}

static inline void set_start_bit(uint32_t base)
{
	mmio_write_32(base + OTPC_CMD_START_OFFSET, 1 << OTPC_CMD_START_START);
}

static inline void reset_start_bit(uint32_t base)
{
	mmio_write_32(base + OTPC_CMD_START_OFFSET, 0);
}

static inline void write_cpu_data(uint32_t base, uint32_t value)
{
	mmio_write_32(base + OTPC_CPU_WRITE_REG_OFFSET, value);
}

static int poll_cpu_status(uint32_t base, uint32_t value)
{
	uint32_t status;
	uint32_t retries;

	for (retries = 0; retries < OTPC_RETRIES_US; retries++) {
		status = mmio_read_32(base + OTPC_CPU_STATUS_OFFSET);
		if (status & value)
			break;
		udelay(1);
	}
	if (retries == OTPC_RETRIES_US)
		return -1;

	return 0;
}

static int bcm_otpc_ecc(uint32_t enable)
{
	struct otpc_priv *priv = &otpc_info;
	int ret;

	set_command(priv->base, OTPC_CMD_ECC);
	set_cpu_address(priv->base, OTPC_ECC_ADDR);

	if (!enable)
		write_cpu_data(priv->base, OTPC_ECC_VAL);
	else
		write_cpu_data(priv->base, ~OTPC_ECC_VAL);

	set_start_bit(priv->base);
	ret = poll_cpu_status(priv->base, OTPC_STAT_CMD_DONE);
	if (ret) {
		ERROR("otp ecc op error: 0x%x", ret);
		return -1;
	}
	reset_start_bit(priv->base);

	return 0;
}

/*
 * bcm_otpc_read read otp data in the size of 8 byte rows.
 * bytes has to be the multiple of 8.
 * return -1 in error case, return read bytes in success.
 */
int bcm_otpc_read(unsigned int offset, void *val, uint32_t bytes,
		  uint32_t ecc_flag)
{
	struct otpc_priv *priv = &otpc_info;
	uint32_t *buf = val;
	uint32_t bytes_read;
	uint32_t address = offset / priv->map->word_size;
	int i, ret;

	if (!priv->state) {
		ERROR("OCOTP read failed\n");
		return -1;
	}

	bcm_otpc_ecc(ecc_flag);

	for (bytes_read = 0; (bytes_read + priv->map->word_size) <= bytes;) {
		set_command(priv->base, OTPC_CMD_READ);
		set_cpu_address(priv->base, address++);
		set_start_bit(priv->base);
		ret = poll_cpu_status(priv->base, OTPC_STAT_CMD_DONE);
		if (ret) {
			ERROR("otp read error: 0x%x", ret);
			return -1;
		}

		for (i = 0; i < priv->map->otpc_row_size; i++) {
			*buf++ = mmio_read_32(priv->base +
					priv->map->data_r_offset[i]);
			bytes_read += sizeof(*buf);
		}

		reset_start_bit(priv->base);
	}

	return bytes_read;
}

int bcm_otpc_init(struct otpc_map *map)
{
	struct otpc_priv *priv;

	priv = &otpc_info;
	priv->base = ocotp_cfg.base;
	priv->map = map;

	priv->size = 4 * ocotp_cfg.num_words;

	/* Enable CPU access to OTPC. */
	mmio_setbits_32(priv->base + OTPC_MODE_REG_OFFSET,
			BIT(OTPC_MODE_REG_OTPC_MODE));
	reset_start_bit(priv->base);
	priv->state = 1;
	VERBOSE("OTPC Initialization done\n");

	return 0;
}
