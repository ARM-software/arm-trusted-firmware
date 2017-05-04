/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat_private.h>
#include <pmu.h>
#include <pwm.h>
#include <soc.h>

#define PWM0_IOMUX_PWM_EN		(1 << 0)
#define PWM1_IOMUX_PWM_EN		(1 << 1)
#define PWM2_IOMUX_PWM_EN		(1 << 2)
#define PWM3_IOMUX_PWM_EN		(1 << 3)

struct pwm_data_s {
	uint32_t iomux_bitmask;
	uint32_t enable_bitmask;
};

static struct pwm_data_s pwm_data;

/*
 * Disable the PWMs.
 */
void disable_pwms(void)
{
	uint32_t i, val;

	pwm_data.iomux_bitmask = 0;

	/* Save PWMs pinmux and change PWMs pinmux to GPIOs */
	val = mmio_read_32(GRF_BASE + GRF_GPIO4C_IOMUX);
	if (((val >> GRF_GPIO4C2_IOMUX_SHIFT) &
		GRF_IOMUX_2BIT_MASK) == GRF_GPIO4C2_IOMUX_PWM) {
		pwm_data.iomux_bitmask |= PWM0_IOMUX_PWM_EN;
		val = BITS_WITH_WMASK(GRF_IOMUX_GPIO, GRF_IOMUX_2BIT_MASK,
				    GRF_GPIO4C2_IOMUX_SHIFT);
		mmio_write_32(GRF_BASE + GRF_GPIO4C_IOMUX, val);
	}

	val = mmio_read_32(GRF_BASE + GRF_GPIO4C_IOMUX);
	if (((val >> GRF_GPIO4C6_IOMUX_SHIFT) &
		GRF_IOMUX_2BIT_MASK) == GRF_GPIO4C6_IOMUX_PWM) {
		pwm_data.iomux_bitmask |= PWM1_IOMUX_PWM_EN;
		val = BITS_WITH_WMASK(GRF_IOMUX_GPIO, GRF_IOMUX_2BIT_MASK,
				    GRF_GPIO4C6_IOMUX_SHIFT);
		mmio_write_32(GRF_BASE + GRF_GPIO4C_IOMUX, val);
	}

	val = mmio_read_32(PMUGRF_BASE + PMUGRF_GPIO1C_IOMUX);
	if (((val >> PMUGRF_GPIO1C3_IOMUX_SHIFT) &
		GRF_IOMUX_2BIT_MASK) == PMUGRF_GPIO1C3_IOMUX_PWM) {
		pwm_data.iomux_bitmask |= PWM2_IOMUX_PWM_EN;
		val = BITS_WITH_WMASK(GRF_IOMUX_GPIO, GRF_IOMUX_2BIT_MASK,
				    PMUGRF_GPIO1C3_IOMUX_SHIFT);
		mmio_write_32(PMUGRF_BASE + PMUGRF_GPIO1C_IOMUX, val);
	}

	val = mmio_read_32(PMUGRF_BASE + PMUGRF_GPIO0A_IOMUX);
	if (((val >> PMUGRF_GPIO0A6_IOMUX_SHIFT) &
		GRF_IOMUX_2BIT_MASK) == PMUGRF_GPIO0A6_IOMUX_PWM) {
		pwm_data.iomux_bitmask |= PWM3_IOMUX_PWM_EN;
		val = BITS_WITH_WMASK(GRF_IOMUX_GPIO, GRF_IOMUX_2BIT_MASK,
				    PMUGRF_GPIO0A6_IOMUX_SHIFT);
		mmio_write_32(PMUGRF_BASE + PMUGRF_GPIO0A_IOMUX, val);
	}

	/* Disable the pwm channel */
	pwm_data.enable_bitmask = 0;
	for (i = 0; i < 4; i++) {
		val = mmio_read_32(PWM_BASE + PWM_CTRL(i));
		if ((val & PWM_ENABLE) != PWM_ENABLE)
			continue;
		pwm_data.enable_bitmask |= (1 << i);
		mmio_write_32(PWM_BASE + PWM_CTRL(i), val & ~PWM_ENABLE);
	}
}

/*
 * Enable the PWMs.
 */
void enable_pwms(void)
{
	uint32_t i, val;

	for (i = 0; i < 4; i++) {
		val = mmio_read_32(PWM_BASE + PWM_CTRL(i));
		if (!(pwm_data.enable_bitmask & (1 << i)))
			continue;
		mmio_write_32(PWM_BASE + PWM_CTRL(i), val | PWM_ENABLE);
	}

	/* Restore all IOMUXes */
	if (pwm_data.iomux_bitmask & PWM3_IOMUX_PWM_EN) {
		val = BITS_WITH_WMASK(PMUGRF_GPIO0A6_IOMUX_PWM,
				    GRF_IOMUX_2BIT_MASK,
				    PMUGRF_GPIO0A6_IOMUX_SHIFT);
		mmio_write_32(PMUGRF_BASE + PMUGRF_GPIO0A_IOMUX, val);
	}

	if (pwm_data.iomux_bitmask & PWM2_IOMUX_PWM_EN) {
		val = BITS_WITH_WMASK(PMUGRF_GPIO1C3_IOMUX_PWM,
				    GRF_IOMUX_2BIT_MASK,
				    PMUGRF_GPIO1C3_IOMUX_SHIFT);
		mmio_write_32(PMUGRF_BASE + PMUGRF_GPIO1C_IOMUX, val);
	}

	if (pwm_data.iomux_bitmask & PWM1_IOMUX_PWM_EN) {
		val = BITS_WITH_WMASK(GRF_GPIO4C6_IOMUX_PWM,
				    GRF_IOMUX_2BIT_MASK,
				    GRF_GPIO4C6_IOMUX_SHIFT);
		mmio_write_32(GRF_BASE + GRF_GPIO4C_IOMUX, val);
	}

	if (pwm_data.iomux_bitmask & PWM0_IOMUX_PWM_EN) {
		val = BITS_WITH_WMASK(GRF_GPIO4C2_IOMUX_PWM,
				    GRF_IOMUX_2BIT_MASK,
				    GRF_GPIO4C2_IOMUX_SHIFT);
		mmio_write_32(GRF_BASE + GRF_GPIO4C_IOMUX, val);
	}
}
