/*
 * Copyright (c) 2024-2026, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <platform_def.h>
#include <stm32mp2_private.h>

/*
 * SYSCFG register offsets (base relative)
 */
#define SYSCFG_DLYBOS1_CR		0x1000U
#define SYSCFG_DLYBOS1_SR		0x1004U
#if !STM32MP21
#define SYSCFG_DLYBOS2_CR		0x1400U
#define SYSCFG_DLYBOS2_SR		0x1404U
#endif /* !STM32MP21 */
#define SYSCFG_OCTOSPIAMCR		0x2C00U
#define SYSCFG_VDDIO3CCCR		0x4000U
#if !STM32MP21
#define SYSCFG_VDDIO4CCCR		0x4008U
#endif /* !STM32MP21 */
#define SYSCFG_VDDCCCR			0x4010U
#define SYSCFG_VDDIO2CCCR		0x4018U
#define SYSCFG_VDDIO1CCCR		0x4020U
#define SYSCFG_DEVICEID			0x6400U

/* DLYBOS registers */
#define SYSCFG_DLYBOS_CR_EN		BIT_32(0)
#define SYSCFG_DLYBOS_CR_RXTAPSEL_SHIFT	1U
#define SYSCFG_DLYBOS_CR_RXTAPSEL_MASK	GENMASK_32(6, 1)
#define SYSCFG_DLYBOS_CR_TXTAPSEL_SHIFT	7U
#define SYSCFG_DLYBOS_CR_TXTAPSEL_MASK	GENMASK_32(12, 7)
#define SYSCFG_DLYBOS_BYP_EN		BIT_32(16)
#define SYSCFG_DLYBOS_BYP_CMD_MASK	GENMASK_32(21, 17)
#define SYSCFG_DLYBOS_BYP_CMD_SHIFT	17U

#define SYSCFG_DLYBOS_SR_LOCK		BIT_32(0)
#define SYSCFG_DLYBOS_SR_RXTAPSEL_ACK	BIT_32(1)
#define SYSCFG_DLYBOS_SR_TXTAPSEL_ACK	BIT_32(2)

#if STM32MP21
#define SYSCFG_MAX_DLYBOS		1U
#else /* STM32MP21 */
#define SYSCFG_MAX_DLYBOS		2U
#endif /* STM32MP21 */
#define SYSCFG_DLYBOS_TIMEOUT_US	10000U
#define SYSCFG_DLYBOS_TAPSEL_NB		33U
#define SYSCFG_DLYBOS_CMD_NB		24U

/* SYSCFG_OCTOSPIAMCR Register */
#define SYSCFG_OCTOSPIAMCR_OAM_MASK	GENMASK_32(2, 0)

/* IO compensation CCR registers bit definition */
#define SYSCFG_CCCR_CS			BIT_32(9)
#define SYSCFG_CCCR_EN			BIT_32(8)
#define SYSCFG_CCCR_RAPSRC_MASK		GENMASK_32(7, 4)
#define SYSCFG_CCCR_RAPSRC_SHIFT	4U
#define SYSCFG_CCCR_RANSRC_MASK		GENMASK_32(3, 0)

/* IO compensation CCSR registers bit definition */
#define SYSCFG_CCSR_READY		BIT_32(8)
#define SYSCFG_CCSR_APSRC_MASK		GENMASK_32(7, 4)
#define SYSCFG_CCSR_ANSRC_MASK		GENMASK_32(3, 0)

#define SYSCFG_CCSR_READY_TIMEOUT_US	1000U

/* SYSCFG_DEVICEID Register */
#define SYSCFG_DEVICEID_DEV_ID_MASK	GENMASK_32(11, 0)

struct syscfg_tap_window {
	uint8_t end;
	uint8_t length;
};

static uint32_t syscfg_cccr_offset[SYSFG_NB_IO_ID] = {
	[SYSFG_VDDIO1_ID] = SYSCFG_VDDIO1CCCR,
	[SYSFG_VDDIO2_ID] = SYSCFG_VDDIO2CCCR,
	[SYSFG_VDDIO3_ID] = SYSCFG_VDDIO3CCCR,
#if !STM32MP21
	[SYSFG_VDDIO4_ID] = SYSCFG_VDDIO4CCCR,
#endif /* !STM32MP21 */
	[SYSFG_VDD_IO_ID] = SYSCFG_VDDCCCR,
};

/*
 * @brief  Get device ID from SYSCFG registers.
 * @retval device ID (DEV_ID).
 */
uint32_t stm32mp_syscfg_get_chip_dev_id(void)
{
	return mmio_read_32(SYSCFG_BASE + SYSCFG_DEVICEID) & SYSCFG_DEVICEID_DEV_ID_MASK;
}

size_t stm32mp_syscfg_get_mm_size(uint8_t bank)
{
	uint32_t amcr = mmio_read_32(SYSCFG_BASE + SYSCFG_OCTOSPIAMCR);
	uint32_t oam = amcr & SYSCFG_OCTOSPIAMCR_OAM_MASK;
	size_t addr_mapping1;
	size_t addr_mapping2;

	if (oam > 3U) {
		addr_mapping2 = SZ_256M;
	} else {
		addr_mapping2 = oam * SZ_64M;
	}

	addr_mapping1 = SZ_256M - addr_mapping2;

	return (bank == 0U) ? addr_mapping1 : addr_mapping2;
}

/*
 * @brief  Enable IO compensation for an IO domain.
 * @param  id: IO compensation ID
 * @retval 0.
 */
void stm32mp_syscfg_enable_io_comp(enum syscfg_io_ids id)
{
	uintptr_t cccr_addr = SYSCFG_BASE + syscfg_cccr_offset[id];
	uintptr_t ccsr_addr = cccr_addr + 4U;
	uint64_t timeout_ref;

	VERBOSE("Enable IO comp for id %u\n", id);

	if ((mmio_read_32(ccsr_addr) & SYSCFG_CCSR_READY) != 0U) {
		return;
	}

	mmio_setbits_32(cccr_addr, SYSCFG_CCCR_EN);

	timeout_ref = timeout_init_us(SYSCFG_CCSR_READY_TIMEOUT_US);

	while ((mmio_read_32(ccsr_addr) & SYSCFG_CCSR_READY) == 0U)
		if (timeout_elapsed(timeout_ref)) {
			WARN("IO compensation cell not ready\n");
			/* Allow an almost silent failure here */
			break;
		}

	mmio_clrbits_32(cccr_addr, SYSCFG_CCCR_CS);
}

/*
 * @brief  Set fixed IO compensation for an IO domain.
 * @param  id: IO compensation ID
 * @param  pmos: pmos compensation code
 * @param  nmos: nmos compensation code
 * @retval 0.
 */
void stm32mp_syscfg_fixed_io_comp(enum syscfg_io_ids id, uint32_t pmos, uint32_t nmos)
{
	uintptr_t cccr_addr = SYSCFG_BASE + syscfg_cccr_offset[id];
	uint32_t value = 0U;

	VERBOSE("Set fixed IO comp for id %u\n", id);

	value = ((pmos << SYSCFG_CCCR_RAPSRC_SHIFT) & SYSCFG_CCCR_RAPSRC_MASK) |
		(nmos & SYSCFG_CCCR_RANSRC_MASK);
	mmio_write_32(cccr_addr, value);
}

#if STM32MP_SPI_NAND || STM32MP_SPI_NOR || STM32MP_HYPERFLASH
/*
 * SYSCFG DLYBOS register offsets (base relative)
 */
static const uint32_t syscfg_dlybos_cr_offset[SYSCFG_MAX_DLYBOS] = {
	SYSCFG_DLYBOS1_CR,
#if !STM32MP21
	SYSCFG_DLYBOS2_CR
#endif /* !STM32MP21 */
};

static const uint32_t syscfg_dlybos_sr_offset[SYSCFG_MAX_DLYBOS] = {
	SYSCFG_DLYBOS1_SR,
#if !STM32MP21
	SYSCFG_DLYBOS2_SR
#endif /* !STM32MP21 */
};

static int stm32_syscfg_dlyb_set_tap(uint8_t bank, uint8_t tap, bool rx_tap)
{
	uintptr_t cr = SYSCFG_BASE + syscfg_dlybos_cr_offset[bank];
	uintptr_t sr = SYSCFG_BASE + syscfg_dlybos_sr_offset[bank];
	uint64_t timeout;
	uint32_t mask, ack;
	uint8_t shift;

	if (rx_tap) {
		mask = SYSCFG_DLYBOS_CR_RXTAPSEL_MASK;
		shift = SYSCFG_DLYBOS_CR_RXTAPSEL_SHIFT;
		ack = SYSCFG_DLYBOS_SR_RXTAPSEL_ACK;
	} else {
		mask = SYSCFG_DLYBOS_CR_TXTAPSEL_MASK;
		shift = SYSCFG_DLYBOS_CR_TXTAPSEL_SHIFT;
		ack = SYSCFG_DLYBOS_SR_TXTAPSEL_ACK;
	}

	mmio_clrsetbits_32(cr, mask, (tap << shift) & mask);

	timeout = timeout_init_us(SYSCFG_DLYBOS_TIMEOUT_US);
	while ((mmio_read_32(sr) & ack) == 0U) {
		if (timeout_elapsed(timeout)) {
			WARN("%s: %s delay block phase configuration timeout\n",
			     __func__, rx_tap ? "RX" : "TX");

			return -ETIMEDOUT;
		}
	}

	return 0;
}

void stm32mp_syscfg_dlyb_stop(uint8_t bank)
{
	mmio_write_32(SYSCFG_BASE + syscfg_dlybos_cr_offset[bank], 0x0);
}

int stm32mp_syscfg_dlyb_find_tap(uint8_t bank, int (*check_transfer)(void),
				 bool rx_only, uint8_t *window_len)
{
	struct syscfg_tap_window rx_tap_w[SYSCFG_DLYBOS_TAPSEL_NB];
	int ret;
	uint8_t rx_len, rx_window_len, rx_window_end;
	uint8_t tx_len, tx_window_len, tx_window_end;
	uint8_t rx_tap, tx_tap, tx_tap_max, tx_tap_min, best_tx_tap;
	uint8_t score, score_max;

	if (bank >= SYSCFG_MAX_DLYBOS) {
		return -EINVAL;
	}

	tx_len = 0U;
	tx_window_len = 0U;
	tx_window_end = 0U;
	rx_window_len = 0U;
	rx_window_end = 0U;
	best_tx_tap = 0U;

	for (tx_tap = 0U;
	     tx_tap < (rx_only ? 1U : SYSCFG_DLYBOS_TAPSEL_NB);
	     tx_tap++) {
		ret = stm32_syscfg_dlyb_set_tap(bank, tx_tap, false);
		if (ret != 0) {
			return ret;
		}

		rx_len = 0U;
		rx_window_len = 0U;
		rx_window_end = 0U;

		for (rx_tap = 0U; rx_tap < SYSCFG_DLYBOS_TAPSEL_NB; rx_tap++) {
			ret = stm32_syscfg_dlyb_set_tap(bank, rx_tap, true);
			if (ret != 0) {
				return ret;
			}

			if (check_transfer() != 0) {
				if ((!rx_only && (ret == -ETIMEDOUT)) ||
				    (ret == -EOPNOTSUPP)) {
					break;
				}

				rx_len = 0U;
			} else {
				rx_len++;

				if (rx_len > rx_window_len) {
					rx_window_len = rx_len;
					rx_window_end = rx_tap;
				}
			}
		}

		if (ret == -EOPNOTSUPP) {
			break;
		}

		rx_tap_w[tx_tap].end = rx_window_end;
		rx_tap_w[tx_tap].length = rx_window_len;

		if (rx_window_len == 0U) {
			tx_len = 0U;
		} else {
			tx_len++;

			if (tx_len > tx_window_len) {
				tx_window_len = tx_len;
				tx_window_end = tx_tap;
			}
		}

		VERBOSE("%s: rx_tap_w[%d].end = %d rx_tap_w[%d].length = %d\n",
			__func__, tx_tap, rx_tap_w[tx_tap].end,
			tx_tap, rx_tap_w[tx_tap].length);
	}

	if (ret == -EOPNOTSUPP) {
		ERROR("%s: calibration can not be done on this device\n", __func__);

		return ret;
	}

	if (rx_only) {
		if (rx_window_len == 0U) {
			WARN("%s: can't find RX phase settings\n", __func__);

			return -EIO;
		}

		rx_tap = rx_window_end - rx_window_len / 2U;
		*window_len = rx_window_len;
		VERBOSE("%s: RX_TAP_SEL set to %d\n", __func__, rx_tap);

		return stm32_syscfg_dlyb_set_tap(bank, rx_tap, true);
	}

	if (tx_window_len == 0U) {
		WARN("%s: can't find TX phase settings\n", __func__);

		return -EIO;
	}

	/* Find the best duet TX/RX TAP */
	tx_tap_min = (tx_window_end + 1U) - tx_window_len;
	tx_tap_max = tx_window_end;
	score_max = 0U;

	for (tx_tap = tx_tap_min; tx_tap <= tx_tap_max; tx_tap++) {
		score = MIN(tx_tap - tx_tap_min, tx_tap_max - tx_tap) +
			rx_tap_w[tx_tap].length;

		if (score > score_max) {
			score_max = score;
			best_tx_tap = tx_tap;
		}
	}

	rx_tap = rx_tap_w[best_tx_tap].end - rx_tap_w[best_tx_tap].length / 2U;

	VERBOSE("%s: RX_TAP_SEL set to %d\n", __func__, rx_tap);
	ret = stm32_syscfg_dlyb_set_tap(bank, rx_tap, true);
	if (ret != 0) {
		return ret;
	}

	VERBOSE("%s: TX_TAP_SEL set to %d\n", __func__, best_tx_tap);

	return stm32_syscfg_dlyb_set_tap(bank, best_tx_tap, false);
}

int stm32mp_syscfg_dlyb_set_cr(uint8_t bank, uint32_t dlyb_cr)
{
	bool bypass_mode = false;
	uint16_t period_ps;
	uint8_t rx_tap;
	uint8_t tx_tap;
	int ret;

	period_ps = (dlyb_cr & SYSCFG_DLYBOS_BYP_CMD_MASK) >>
		    SYSCFG_DLYBOS_BYP_CMD_SHIFT;
	if ((dlyb_cr & SYSCFG_DLYBOS_BYP_EN) != 0U) {
		bypass_mode = true;
	}

	ret = stm32mp_syscfg_dlyb_init(bank, bypass_mode, period_ps);
	if (ret != 0) {
		return ret;
	}

	/* restore Rx and TX tap */
	rx_tap = (dlyb_cr & SYSCFG_DLYBOS_CR_RXTAPSEL_MASK) >>
		 SYSCFG_DLYBOS_CR_RXTAPSEL_SHIFT;
	ret = stm32_syscfg_dlyb_set_tap(bank, rx_tap, true);
	if (ret != 0) {
		return ret;
	}

	tx_tap = (dlyb_cr & SYSCFG_DLYBOS_CR_TXTAPSEL_MASK) >>
		 SYSCFG_DLYBOS_CR_TXTAPSEL_SHIFT;

	return stm32_syscfg_dlyb_set_tap(bank, tx_tap, false);
}

void stm32mp_syscfg_dlyb_get_cr(uint8_t bank, uint32_t *dlyb_cr)
{
	uintptr_t cr = SYSCFG_BASE + syscfg_dlybos_cr_offset[bank];

	*dlyb_cr = mmio_read_32(cr);
}

/* ½ memory clock period in pico second */
static const uint16_t syscfg_dlybos_cmd_delay_ps[SYSCFG_DLYBOS_CMD_NB] = {
2816U, 4672U, 6272U, 7872U, 9472U, 11104U, 12704U, 14304U,
15904U, 17536U, 19136U, 20736U, 22336U, 23968U, 25568U, 27168U,
28768U, 30400U, 32000U, 33600U, 35232U, 36832U, 38432U, 40032U
};

static uint32_t stm32mp_syscfg_find_byp_cmd(uint32_t period_ps)
{
	uint16_t half_period_ps = period_ps / 2U;
	uint8_t max = SYSCFG_DLYBOS_CMD_NB - 1U;
	uint8_t i, min = 0U;

	/* Find closest value in syscfg_dlybos_delay_ps[] with half_period_ps */
	if (half_period_ps < (uint32_t)syscfg_dlybos_cmd_delay_ps[min]) {
		return (1U << SYSCFG_DLYBOS_BYP_CMD_SHIFT) &
			SYSCFG_DLYBOS_BYP_CMD_MASK;
	}

	if (half_period_ps > (uint32_t)syscfg_dlybos_cmd_delay_ps[max]) {
		return SYSCFG_DLYBOS_BYP_CMD_MASK;
	}

	while (max > (min + 1U)) {
		i = div_round_up(min + max, 2U);
		if (half_period_ps > (uint32_t)syscfg_dlybos_cmd_delay_ps[i]) {
			min = i;
		} else {
			max = i;
		}
	}

	if (((uint32_t)syscfg_dlybos_cmd_delay_ps[max] - half_period_ps) >
	    (half_period_ps - (uint32_t)syscfg_dlybos_cmd_delay_ps[min])) {
		return ((min + 1U) << SYSCFG_DLYBOS_BYP_CMD_SHIFT) &
			SYSCFG_DLYBOS_BYP_CMD_MASK;
	} else {
		return ((max + 1U) << SYSCFG_DLYBOS_BYP_CMD_SHIFT) &
			SYSCFG_DLYBOS_BYP_CMD_MASK;
	}
}

int stm32mp_syscfg_dlyb_init(uint8_t bank, bool bypass_mode, uint32_t period_ps)
{
	uint64_t timeout;
	uintptr_t cr = SYSCFG_BASE + syscfg_dlybos_cr_offset[bank];
	uintptr_t sr = SYSCFG_BASE + syscfg_dlybos_sr_offset[bank];
	uint32_t mask, val;

	if (bank >= SYSCFG_MAX_DLYBOS) {
		return -EINVAL;
	}

	if (bypass_mode) {
		mask = SYSCFG_DLYBOS_BYP_EN | SYSCFG_DLYBOS_BYP_CMD_MASK;
		val = SYSCFG_DLYBOS_BYP_EN |
		      stm32mp_syscfg_find_byp_cmd(period_ps);
	} else {
		mask = SYSCFG_DLYBOS_CR_EN;
		val = SYSCFG_DLYBOS_CR_EN;
	}

	mmio_clrsetbits_32(cr, mask, val);

	if (bypass_mode) {
		return 0;
	}

	/* In lock mode, wait for lock status bit */
	timeout = timeout_init_us(SYSCFG_DLYBOS_TIMEOUT_US);
	while ((mmio_read_32(sr) & SYSCFG_DLYBOS_SR_LOCK) == 0U) {
		if (timeout_elapsed(timeout)) {
			WARN("%s: delay Block lock timeout\n", __func__);
			mmio_clrbits_32(cr, SYSCFG_DLYBOS_CR_EN);

			return -ETIMEDOUT;
		}
	}

	return 0;
}
#endif
