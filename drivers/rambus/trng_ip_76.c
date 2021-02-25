/*
 * Copyright (c) 2020, Marvell Technology Group Ltd. All rights reserved.
 *
 * Based on Linux kernel omap-rng.c - RNG driver for TI OMAP CPU family
 *
 * Author: Deepak Saxena <dsaxena@plexity.net>
 *
 * Copyright 2005 (c) MontaVista Software, Inc.
 *
 * Mostly based on original driver:
 *
 * Copyright (C) 2005 Nokia Corporation
 * Author: Juha Yrjölä <juha.yrjola@nokia.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/rambus/trng_ip_76.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <lib/utils.h>

#define RNG_REG_STATUS_RDY			(1 << 0)

#define RNG_REG_INTACK_RDY_MASK			(1 << 0)

#define RNG_CONTROL_ENABLE_TRNG_MASK		(1 << 10)

#define RNG_CONFIG_NOISE_BLOCKS(val)		((0xff & (val)) << 0)
#define RNG_CONFIG_NOISE_BLK_VAL		0x5

#define RNG_CONFIG_SAMPLE_CYCLES(val)		((0xff & (val)) << 16)
#define RNG_CONFIG_SAMPLE_CYCLES_VAL		0x22

#define RNG_REG_FRO_ENABLE_MASK			0xffffff
#define RNG_REG_FRO_DETUNE_MASK			0x0

#define EIP76_RNG_OUTPUT_SIZE			0x10
#define EIP76_RNG_WAIT_ROUNDS			10

#define RNG_HW_IS_EIP76(ver)			((ver) & (0xff == 0x4C))
#define RNG_HW_VER_MAJOR(ver)			(((ver) & (0xf << 24)) >> 24)
#define RNG_HW_VER_MINOR(ver)			(((ver) & (0xf << 20)) >> 20)
#define RNG_HW_VER_PATCH(ver)			(((ver) & (0xf << 16)) >> 16)


enum {
	RNG_OUTPUT_0_REG = 0,
	RNG_OUTPUT_1_REG,
	RNG_OUTPUT_2_REG,
	RNG_OUTPUT_3_REG,
	RNG_STATUS_REG,
	RNG_INTMASK_REG,
	RNG_INTACK_REG,
	RNG_CONTROL_REG,
	RNG_CONFIG_REG,
	RNG_ALARMCNT_REG,
	RNG_FROENABLE_REG,
	RNG_FRODETUNE_REG,
	RNG_ALARMMASK_REG,
	RNG_ALARMSTOP_REG,
	RNG_REV_REG
};

static uint16_t reg_map_eip76[] = {
	[RNG_OUTPUT_0_REG]	= 0x0,
	[RNG_OUTPUT_1_REG]	= 0x4,
	[RNG_OUTPUT_2_REG]	= 0x8,
	[RNG_OUTPUT_3_REG]	= 0xc,
	[RNG_STATUS_REG]	= 0x10,
	[RNG_INTACK_REG]	= 0x10,
	[RNG_CONTROL_REG]	= 0x14,
	[RNG_CONFIG_REG]	= 0x18,
	[RNG_ALARMCNT_REG]	= 0x1c,
	[RNG_FROENABLE_REG]	= 0x20,
	[RNG_FRODETUNE_REG]	= 0x24,
	[RNG_ALARMMASK_REG]	= 0x28,
	[RNG_ALARMSTOP_REG]	= 0x2c,
	[RNG_REV_REG]		= 0x7c,
};

struct eip76_rng_dev {
	uintptr_t	base;
	uint16_t	*regs;
};

/* Locals */
static struct eip76_rng_dev eip76_dev;
static spinlock_t rng_lock;

static inline uint32_t eip76_rng_read(struct eip76_rng_dev *dev, uint16_t reg)
{
	return mmio_read_32(dev->base + dev->regs[reg]);
}

static inline void eip76_rng_write(struct eip76_rng_dev *dev,
				   uint16_t reg, uint32_t val)
{
	mmio_write_32(dev->base + dev->regs[reg], val);
}

static void eip76_rng_init(struct eip76_rng_dev *dev)
{
	uint32_t val;

	/* Return if RNG is already running. */
	if (eip76_rng_read(dev, RNG_CONTROL_REG) &
			   RNG_CONTROL_ENABLE_TRNG_MASK) {
		return;
	}

	/*  This field sets the number of 512-bit blocks of raw Noise Source
	 * output data that must be processed by either the Conditioning
	 * Function or the SP 800-90 DRBG ‘BC_DF’ functionality to yield
	 * a ‘full entropy’ output value. As according to [SP 800-90B draft]
	 * the amount of entropy input to this functionality must be twice
	 * the amount that is output and the 8-bit samples output by the Noise
	 * Source are supposed to have one bit of entropy each, the settings
	 * for this field are as follows:
	 * - SHA-1 Conditioning Function:
	 *  generates 160 bits output, requiring 2560 sample bits,
	 *  equivalent to 5 blocks of raw Noise Source input.
	 * - SHA-256 Conditioning Function:
	 *  generates 256 bits output, requiring 4096 sample bits, equivalent
	 *  to 8 blocks of raw Noise Source input. Note that two blocks of 256
	 *  bits are needed to start or re-seed the SP 800-90 DRBG
	 *  (in the EIP-76d-*-SHA2 configurations)
	 * - SP 800-90 DRBG ‘BC_DF’ functionality:
	 *  generates 384 bits output, requiring 6144 sample bits, equivalent
	 *  to 12 blocks of raw Noise Source input.
	 *  This field can only be modified when ‘enable_trng’ in TRNG_CONTROL
	 *  is ‘0’ or when either of the ‘test_known_noise’ or ‘test_cond_func’
	 *  bits in TRNG_TEST is ‘1’. Value 0 in this field selects 256 blocks
	 *  of 512 bits to be processed.
	 */
	val = RNG_CONFIG_NOISE_BLOCKS(RNG_CONFIG_NOISE_BLK_VAL);

	/* This field sets the number of FRO samples that are XOR-ed together
	 * into one bit to be shifted into the main shift register.
	 * This value must be such that there is at least one bit of entropy
	 * (in total) in each 8 bits that are shifted.
	 * This field can only be modified when ‘enable_trng’ in TRNG_CONTROL
	 * is ‘0’ or when either of the ‘test_known_noise’ or ‘test_cond_func’
	 * bits in TRNG_TEST is ‘1’. Value 0 in this field selects 65536 FRO
	 * samples to be XOR-ed together
	 */
	val |= RNG_CONFIG_SAMPLE_CYCLES(RNG_CONFIG_SAMPLE_CYCLES_VAL);
	eip76_rng_write(dev, RNG_CONFIG_REG, val);

	/* Enable all available FROs */
	eip76_rng_write(dev, RNG_FRODETUNE_REG, RNG_REG_FRO_DETUNE_MASK);
	eip76_rng_write(dev, RNG_FROENABLE_REG, RNG_REG_FRO_ENABLE_MASK);

	/* Enable TRNG */
	eip76_rng_write(dev, RNG_CONTROL_REG, RNG_CONTROL_ENABLE_TRNG_MASK);
}

int32_t eip76_rng_read_rand_buf(void *data, bool wait)
{
	uint32_t i, present;

	if (!eip76_dev.base) /* not initialized */
		return -1;

	for (i = 0; i < EIP76_RNG_WAIT_ROUNDS; i++) {
		present = eip76_rng_read(&eip76_dev, RNG_STATUS_REG) &
					 RNG_REG_STATUS_RDY;
		if (present || !wait) {
			break;
		}

		udelay(10);
	}

	if (present != 0U) {
		return 0;
	}

	memcpy(data,
	       (void *)(eip76_dev.base + eip76_dev.regs[RNG_OUTPUT_0_REG]),
	       EIP76_RNG_OUTPUT_SIZE);

	eip76_rng_write(&eip76_dev, RNG_INTACK_REG, RNG_REG_INTACK_RDY_MASK);

	return EIP76_RNG_OUTPUT_SIZE;
}

int32_t eip76_rng_probe(uintptr_t base_addr)
{
	uint32_t ver;

	eip76_dev.base = base_addr;
	eip76_dev.regs = reg_map_eip76;

	eip76_rng_init(&eip76_dev);

	ver = eip76_rng_read(&eip76_dev, RNG_REV_REG);

	INFO("%s Random Number Generator HW ver. %01x.%01x.%01x\n",
	     RNG_HW_IS_EIP76(ver) ? "TRNG-IP-76" : "Unknown",
	     RNG_HW_VER_MAJOR(ver), RNG_HW_VER_MINOR(ver),
	     RNG_HW_VER_PATCH(ver));

	return 0;
}

int32_t eip76_rng_get_random(uint8_t *data, uint32_t len)
{
	static uint8_t rand[EIP76_RNG_OUTPUT_SIZE];
	static uint8_t pos;
	uint32_t i;
	int32_t ret = 0;

	if (!data)
		return -1;

	spin_lock(&rng_lock);

	for (i = 0; i < len; i++) {
		if (pos >= EIP76_RNG_OUTPUT_SIZE) {
			pos = 0;
		}

		if (pos != 0U) {
			ret = eip76_rng_read_rand_buf(rand, true);
		}

		/* Only advance FIFO index if it is non zero or
		 * the update from TRNG HW was successful
		 */
		if (pos || ret > 0) {
			data[i] = rand[pos++];
			ret = 0;
		} else {
			ret = -1;
			break;
		}
	}

	spin_unlock(&rng_lock);

	return ret;
}
