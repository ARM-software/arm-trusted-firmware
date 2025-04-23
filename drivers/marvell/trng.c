/*
 * Copyright (c) 2025, Marvell Technology Group Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <mvebu_def.h>

/* Bind to first CP110's EIP-76 engine only */
#define CP110_TRNG_REGS_BASE		(MVEBU_CP_REGS_BASE(0) + 0x760000U)

/* EIP-76 Register Definitions */
#define CP110_TRNG_OUTPUT_REG(n)	(CP110_TRNG_REGS_BASE + ((n) * 0x4U))
#define CP110_TRNG_STAT_N_ACK_REG	(CP110_TRNG_REGS_BASE + 0x10U)
#define CP110_TRNG_CONTROL_REG		(CP110_TRNG_REGS_BASE + 0x14U)
#define CP110_TRNG_CONFIG_REG		(CP110_TRNG_REGS_BASE + 0x18U)
#define CP110_TRNG_FRO_ENABLE_REG	(CP110_TRNG_REGS_BASE + 0x20U)
#define CP110_TRNG_FRO_DETUNE_REG	(CP110_TRNG_REGS_BASE + 0x24U)

/* CP110_TRNG_STAT_N_ACK_REG */
#define CP110_TRNG_READY		BIT(0)

/* CP110_TRNG_CONTROL_REG */
#define CP110_TRNG_EN			BIT(10)

/* CP110_TRNG_CONFIG_REG */
#define CP110_TRNG_NOISE_BLOCKS_SHIFT	0U
#define CP110_TRNG_NOISE_BLOCKS_MASK	(0xFFU << CP110_TRNG_NOISE_BLOCKS_SHIFT)
#define CP110_TRNG_SAMPLE_CYCLES_SHIFT	16U
#define CP110_TRNG_SAMPLE_CYCLES_MASK	(0xFFU << CP110_TRNG_SAMPLE_CYCLES_SHIFT)

/* CP110_TRNG_FRO_ENABLE_REG */
#define CP110_TRNG_FRO_EN_SHIFT		0U
#define CP110_TRNG_FRO_EN_MASK		(0xFFFFFFU << CP110_TRNG_FRO_EN_SHIFT)

#define CP110_TRNG_MAX_OUTPUTS		4U

/* maximum busy wait */
#define CP110_TRNG_MAX_RETRIES		3U

static void mv_trng_init(void)
{
	uint32_t val;

	val = (0x5U << CP110_TRNG_NOISE_BLOCKS_SHIFT) & CP110_TRNG_NOISE_BLOCKS_MASK;
	val |= (0x22U << CP110_TRNG_SAMPLE_CYCLES_SHIFT) & CP110_TRNG_SAMPLE_CYCLES_MASK;
	mmio_write_32(CP110_TRNG_CONFIG_REG, val);

	mmio_write_32(CP110_TRNG_FRO_DETUNE_REG, 0U);
	mmio_write_32(CP110_TRNG_FRO_ENABLE_REG, CP110_TRNG_FRO_EN_MASK);

	mmio_write_32(CP110_TRNG_CONTROL_REG, CP110_TRNG_EN);
}

int mv_trng_get_random32(uint32_t *rand, uint8_t num)
{
	uint32_t val;
	uint8_t i;

	if (num > CP110_TRNG_MAX_OUTPUTS) {
		return -1;
	}

	val = mmio_read_32(CP110_TRNG_CONTROL_REG);
	if ((val & CP110_TRNG_EN) != 0U) {
		/* Flush the staled output data */
		val = mmio_read_32(CP110_TRNG_STAT_N_ACK_REG);
		if ((val & CP110_TRNG_READY) != 0U) {
			mmio_write_32(CP110_TRNG_STAT_N_ACK_REG, CP110_TRNG_READY);
		}
	} else {
		mv_trng_init();
		/* Necessary delay for the warm-up */
		udelay(200U);
	}


	for (i = 0U; i < CP110_TRNG_MAX_RETRIES; i++) {
		val = mmio_read_32(CP110_TRNG_STAT_N_ACK_REG);
		if ((val & CP110_TRNG_READY) != 0U) {
			break;
		}
		udelay(1U);
	}

	if (i == CP110_TRNG_MAX_RETRIES) {
		return -1;
	}

	for (i = 0U; i < num; i++) {
		rand[i] = mmio_read_32(CP110_TRNG_OUTPUT_REG(i));
	}

	return 0;
}
