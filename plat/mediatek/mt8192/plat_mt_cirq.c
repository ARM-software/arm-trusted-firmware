/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gic_common.h>
#include <drivers/console.h>
#include <lib/mmio.h>

#include <mt_gic_v3.h>
#include <mtk_plat_common.h>
#include <plat_mt_cirq.h>
#include <platform_def.h>

static struct cirq_events cirq_all_events = {
	.spi_start = CIRQ_SPI_START
};

static inline void  mt_cirq_write32(uint32_t val, uint32_t addr)
{
	mmio_write_32(addr + SYS_CIRQ_BASE, val);
}

static inline uint32_t mt_cirq_read32(uint32_t addr)
{
	return mmio_read_32(addr + SYS_CIRQ_BASE);
}

/*
 * cirq_clone_flush_check_store:
 * set 1 if we need to enable clone/flush value's check
 */
static int32_t cirq_clone_flush_check_val;

/*
 * cirq_pattern_clone_flush_check_show:  set 1 if we need to do pattern test.
 */
static int32_t cirq_pattern_clone_flush_check_val;

/*
 * cirq_pattern_clone_flush_check_show:  set 1 if we need to do pattern test.
 */
static int32_t cirq_pattern_list;

/*
 * mt_cirq_ack_all: Ack all the interrupt on SYS_CIRQ
 */
void mt_cirq_ack_all(void)
{
	unsigned int i;

	for (i = 0U; i < CIRQ_CTRL_REG_NUM; i++) {
		mt_cirq_write32(0xFFFFFFFF, CIRQ_ACK_BASE + (i * 4U));
	}
	/* make sure all cirq setting take effect before doing other things */
	dmbsy();
}

/*
 * mt_cirq_enable: Enable SYS_CIRQ
 */
void mt_cirq_enable(void)
{
	uint32_t st;

	mt_cirq_ack_all();

	st = mt_cirq_read32(CIRQ_CON);
	st |= (CIRQ_CON_EN << CIRQ_CON_EN_BITS) |
			(CIRQ_CON_EDGE_ONLY << CIRQ_CON_EDGE_ONLY_BITS);

	mt_cirq_write32((st & CIRQ_CON_BITS_MASK), CIRQ_CON);
}

/*
 * mt_cirq_disable: Disable SYS_CIRQ
 */
void mt_cirq_disable(void)
{
	uint32_t st;

	st = mt_cirq_read32(CIRQ_CON);
	st &= ~(CIRQ_CON_EN << CIRQ_CON_EN_BITS);

	mt_cirq_write32((st & CIRQ_CON_BITS_MASK), CIRQ_CON);
}

/*
 * mt_cirq_get_mask: Get the specified SYS_CIRQ mask
 * @cirq_num: the SYS_CIRQ number to get
 * @return:
 *    1: this cirq is masked
 *    0: this cirq is umasked
 *    2: cirq num is out of range
 */
__attribute__((weak)) unsigned int mt_cirq_get_mask(uint32_t cirq_num)
{
	uint32_t st;
	unsigned int val;

	if (cirq_num >= CIRQ_IRQ_NUM) {
		ERROR("[CIRQ] %s: invalid cirq %u\n", __func__, cirq_num);
		return 2;
	}

	st = mt_cirq_read32((cirq_num / 32U) * 4U + CIRQ_MASK_BASE);
	val = (st >> (cirq_num % 32U)) & 1U;
	return val;
}

/*
 * mt_cirq_mask_all: Mask all interrupts on SYS_CIRQ.
 */
void mt_cirq_mask_all(void)
{
	unsigned int i;

	for (i = 0U; i < CIRQ_CTRL_REG_NUM; i++) {
		mt_cirq_write32(0xFFFFFFFF, CIRQ_MASK_SET_BASE + (i * 4U));
	}
	/* make sure all cirq setting take effect before doing other things */
	dmbsy();
}

/*
 * mt_cirq_unmask_all: Unmask all interrupts on SYS_CIRQ.
 */
void mt_cirq_unmask_all(void)
{
	unsigned int i;

	for (i = 0U; i < CIRQ_CTRL_REG_NUM; i++) {
		mt_cirq_write32(0xFFFFFFFF, CIRQ_MASK_CLR_BASE + (i * 4U));
	}
	/* make sure all cirq setting take effect before doing other things */
	dmbsy();
}

/*
 * mt_cirq_mask: Mask the specified SYS_CIRQ.
 * @cirq_num: the SYS_CIRQ number to mask
 * @return:
 *    0: mask success
 *   -1: cirq num is out of range
 */
static int mt_cirq_mask(uint32_t cirq_num)
{
	uint32_t bit = 1U << (cirq_num % 32U);

	if (cirq_num >= CIRQ_IRQ_NUM) {
		ERROR("[CIRQ] %s: invalid cirq %u\n", __func__, cirq_num);
		return -1;
	}

	mt_cirq_write32(bit, (cirq_num / 32U) * 4U + CIRQ_MASK_SET_BASE);
	return 0;
}

/*
 * mt_cirq_unmask: Unmask the specified SYS_CIRQ.
 * @cirq_num: the SYS_CIRQ number to unmask
 * @return:
 *    0: umask success
 *   -1: cirq num is out of range
 */
static int mt_cirq_unmask(uint32_t cirq_num)
{
	uint32_t bit = 1U << (cirq_num % 32U);

	if (cirq_num >= CIRQ_IRQ_NUM) {
		ERROR("[CIRQ] %s: invalid cirq %u\n", __func__, cirq_num);
		return -1;
	}

	mt_cirq_write32(bit, (cirq_num / 32U) * 4U + CIRQ_MASK_CLR_BASE);
	return 0;
}

/*
 * mt_cirq_set_sens: Set the sensitivity for the specified SYS_CIRQ number.
 * @cirq_num: the SYS_CIRQ number to set
 * @sens: sensitivity to set
 * @return:
 *    0: set sens success
 *   -1: cirq num is out of range
 */
static int mt_cirq_set_sens(uint32_t cirq_num, uint32_t sens)
{
	uint32_t base;
	uint32_t bit = 1U << (cirq_num % 32U);

	if (cirq_num >= CIRQ_IRQ_NUM) {
		ERROR("[CIRQ] %s: invalid cirq %u\n", __func__, cirq_num);
		return -1;
	}

	if (sens == MT_CIRQ_EDGE_SENSITIVE) {
		base = (cirq_num / 32U) * 4U + CIRQ_SENS_CLR_BASE;
	} else if (sens == MT_CIRQ_LEVEL_SENSITIVE) {
		base = (cirq_num / 32U) * 4U + CIRQ_SENS_SET_BASE;
	} else {
		ERROR("[CIRQ] set_sens invalid sen value %u\n", sens);
		return -1;
	}

	mt_cirq_write32(bit, base);
	return 0;
}

/*
 * mt_cirq_get_sens: Get the specified SYS_CIRQ sensitivity
 * @cirq_num: the SYS_CIRQ number to get
 * @return:
 *    1: this cirq is MT_LEVEL_SENSITIVE
 *    0: this cirq is MT_EDGE_SENSITIVE
 *    2: cirq num is out of range
 */
__attribute__((weak)) unsigned int mt_cirq_get_sens(uint32_t cirq_num)
{
	uint32_t st;
	unsigned int val;

	if (cirq_num >= CIRQ_IRQ_NUM) {
		ERROR("[CIRQ] %s: invalid cirq %u\n", __func__, cirq_num);
		return 2;
	}

	st = mt_cirq_read32((cirq_num / 32U) * 4U + CIRQ_SENS_BASE);
	val = (st >> (cirq_num % 32U)) & 1U;
	return val;
}

/*
 * mt_cirq_set_pol: Set the polarity for the specified SYS_CIRQ number.
 * @cirq_num: the SYS_CIRQ number to set
 * @pol: polarity to set
 * @return:
 *    0: set pol success
 *   -1: cirq num is out of range
 */
static int mt_cirq_set_pol(uint32_t cirq_num, uint32_t pol)
{
	uint32_t base;
	uint32_t bit = 1U << (cirq_num % 32U);

	if (cirq_num >= CIRQ_IRQ_NUM) {
		ERROR("[CIRQ] %s: invalid cirq %u\n", __func__, cirq_num);
		return -1;
	}

	if (pol == MT_CIRQ_POL_NEG) {
		base = (cirq_num / 32U) * 4U + CIRQ_POL_CLR_BASE;
	} else if (pol == MT_CIRQ_POL_POS) {
		base = (cirq_num / 32U) * 4U + CIRQ_POL_SET_BASE;
	} else {
		ERROR("[CIRQ] set_pol invalid polarity value %u\n", pol);
		return -1;
	}

	mt_cirq_write32(bit, base);
	return 0;
}

/*
 * mt_cirq_get_pol: Get the specified SYS_CIRQ polarity
 * @cirq_num: the SYS_CIRQ number to get
 * @return:
 *    1: this cirq is MT_CIRQ_POL_POS
 *    0: this cirq is MT_CIRQ_POL_NEG
 *    2: cirq num is out of range
 */
__attribute__((weak)) unsigned int mt_cirq_get_pol(uint32_t cirq_num)
{
	uint32_t st;
	unsigned int val;

	if (cirq_num >= CIRQ_IRQ_NUM) {
		ERROR("[CIRQ] %s: invalid cirq %u\n", __func__, cirq_num);
		return 2;
	}

	st = mt_cirq_read32((cirq_num / 32U) * 4U + CIRQ_POL_BASE);
	val = (st >> (cirq_num % 32U)) & 1U;
	return val;
}

/*
 * mt_cirq_get_pending: Get the specified SYS_CIRQ pending
 * @cirq_num: the SYS_CIRQ number to get
 * @return:
 *    1: this cirq is pending
 *    0: this cirq is not pending
 *    2: cirq num is out of range
 */
static unsigned int mt_cirq_get_pending(uint32_t cirq_num)
{
	uint32_t st;
	unsigned int val;

	if (cirq_num >= CIRQ_IRQ_NUM) {
		ERROR("[CIRQ] %s: invalid cirq %u\n", __func__, cirq_num);
		return 2;
	}

	st = mt_cirq_read32((cirq_num / 32U) * 4U + CIRQ_STA_BASE);
	val = (st >> (cirq_num % 32U)) & 1U;
	return val;
}

/*
 * mt_cirq_clone_pol: Copy the polarity setting from GIC to SYS_CIRQ
 */
void mt_cirq_clone_pol(void)
{
	uint32_t cirq_num;

	for (cirq_num = 0U; cirq_num < CIRQ_IRQ_NUM; cirq_num++) {
		mt_cirq_set_pol(cirq_num, MT_CIRQ_POL_POS);
	}
}

/*
 * mt_cirq_clone_sens: Copy the sensitivity setting from GIC to SYS_CIRQ
 */
void mt_cirq_clone_sens(void)
{
	uint32_t cirq_num, irq_num;
	uint32_t st, val;

	for (cirq_num = 0U; cirq_num < CIRQ_IRQ_NUM; cirq_num++) {
		irq_num = CIRQ_TO_IRQ_NUM(cirq_num);

		if ((cirq_num == 0U) || (irq_num % 16U == 0U)) {
			st = mmio_read_32(BASE_GICD_BASE + GICD_ICFGR +
				(irq_num / 16U * 4U));
		}

		val = (st >> ((irq_num % 16U) * 2U)) & 0x2U;

		if (val) {
			mt_cirq_set_sens(cirq_num, MT_CIRQ_EDGE_SENSITIVE);
		} else {
			mt_cirq_set_sens(cirq_num, MT_CIRQ_LEVEL_SENSITIVE);
		}
	}
}

/*
 * mt_cirq_clone_mask: Copy the mask setting from GIC to SYS_CIRQ
 */
void mt_cirq_clone_mask(void)
{
	uint32_t cirq_num, irq_num;
	uint32_t st, val;

	for (cirq_num = 0U; cirq_num < CIRQ_IRQ_NUM; cirq_num++) {
		irq_num = CIRQ_TO_IRQ_NUM(cirq_num);

		if ((cirq_num == 0U) || (irq_num % 32U == 0U)) {
			st = mmio_read_32(BASE_GICD_BASE +
				GICD_ISENABLER + (irq_num / 32U * 4U));
		}

		val = (st >> (irq_num % 32)) & 1U;

		if (val) {
			mt_cirq_unmask(cirq_num);
		} else {
			mt_cirq_mask(cirq_num);
		}
	}
}

/*
 * mt_cirq_clone_gic: Copy the setting from GIC to SYS_CIRQ
 */
void mt_cirq_clone_gic(void)
{
	mt_cirq_clone_sens();
	mt_cirq_clone_mask();
}

/*
 * mt_cirq_disable: Flush interrupt from SYS_CIRQ to GIC
 */
void mt_cirq_flush(void)
{
	unsigned int i;
	unsigned char cirq_p_val = 0U;
	unsigned char irq_p_val = 0U;
	uint32_t irq_p = 0U;
	unsigned char pass = 1U;
	uint32_t first_cirq_found = 0U;
	uint32_t first_flushed_cirq;
	uint32_t first_irq_flushedto;
	uint32_t last_fluashed_cirq;
	uint32_t last_irq_flushedto;

	if (cirq_pattern_clone_flush_check_val == 1U) {
		if (cirq_pattern_list < CIRQ_IRQ_NUM) {
			mt_cirq_unmask(cirq_pattern_list);
			mt_cirq_set_sens(cirq_pattern_list,
				MT_CIRQ_EDGE_SENSITIVE);
			mt_cirq_set_pol(cirq_pattern_list, MT_CIRQ_POL_NEG);
			mt_cirq_set_pol(cirq_pattern_list, MT_CIRQ_POL_POS);
			mt_cirq_set_pol(cirq_pattern_list, MT_CIRQ_POL_NEG);
		} else {
			ERROR("[CIRQ] no pattern to test,");
			ERROR("input pattern first\n");
		}
		ERROR("[CIRQ] cirq_pattern %u, cirq_p %u,",
				cirq_pattern_list,
				mt_cirq_get_pending(cirq_pattern_list));
		ERROR("cirq_s %u, cirq_con 0x%x\n",
				mt_cirq_get_sens(cirq_pattern_list),
				mt_cirq_read32(CIRQ_CON));
	}

	mt_cirq_unmask_all();

	for (i = 0U; i < CIRQ_IRQ_NUM; i++) {
		cirq_p_val = mt_cirq_get_pending(i);
		if (cirq_p_val) {
			mt_irq_set_pending(CIRQ_TO_IRQ_NUM(i));
		}

		if (cirq_clone_flush_check_val == 1U) {
			if (cirq_p_val == 0U) {
				continue;
		}
			irq_p = CIRQ_TO_IRQ_NUM(i);
			irq_p_val = mt_irq_get_pending(irq_p);
			if (cirq_p_val != irq_p_val) {
				ERROR("[CIRQ] CIRQ Flush Failed ");
				ERROR("%u(cirq %d)!= %u(gic %d)\n",
					cirq_p_val, i, irq_p_val,
					CIRQ_TO_IRQ_NUM(i));
				pass = 0;
			} else {
				ERROR("[CIRQ] CIRQ Flush Pass ");
				ERROR("%u(cirq %d) = %u(gic %d)\n",
					cirq_p_val, i, irq_p_val,
					CIRQ_TO_IRQ_NUM(i));
			}
			if (!first_cirq_found) {
				first_flushed_cirq = i;
				first_irq_flushedto = irq_p;
				first_cirq_found = 1U;
			}
			last_fluashed_cirq = i;
			last_irq_flushedto = irq_p;
		}
	}

	if (cirq_clone_flush_check_val == 1U) {
		if (first_cirq_found) {
			ERROR("[CIRQ] The first flush : CIRQ%u to IRQ%u\n",
				first_flushed_cirq, first_irq_flushedto);
			ERROR("[CIRQ] The last flush : CIRQ%u to IRQ%u\n",
				last_fluashed_cirq, last_irq_flushedto);
		} else {
			ERROR("[CIRQ] There are no pending ");
			ERROR("interrupt in CIRQ\n");
			ERROR("[CIRQ] so no flush operation happened\n");
		}
		ERROR("[CIRQ] The Flush Max Range : CIRQ");
		ERROR("%d to IRQ%d ~ CIRQ%d to IRQ%d\n", 0U,
			CIRQ_TO_IRQ_NUM(0U), CIRQ_IRQ_NUM - 1U,
			CIRQ_TO_IRQ_NUM(CIRQ_IRQ_NUM - 1U));
		ERROR("[CIRQ] Flush Check %s, Confirm:SPI_START_OFFSET:%d\n",
			pass == 1 ? "Pass" : "Failed", CIRQ_SPI_START);
	}
	mt_cirq_mask_all();
	mt_cirq_ack_all();
}

void mt_cirq_sw_reset(void)
{
	uint32_t st;

	st = mt_cirq_read32(CIRQ_CON);
	st |= (CIRQ_SW_RESET << CIRQ_CON_SW_RST_BITS);

	mt_cirq_write32(st, CIRQ_CON);
}

void set_wakeup_sources(uint32_t *list, uint32_t num_of_events)
{
	cirq_all_events.num_of_events = num_of_events;
	cirq_all_events.wakeup_events = list;
}
