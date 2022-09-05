/*
 * Copyright (c) 2020-2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_MT_CIRQ_H
#define PLAT_MT_CIRQ_H

#include <stdint.h>
#include <platform_def.h>

enum {
	IRQ_MASK_HEADER = 0xF1F1F1F1,
	IRQ_MASK_FOOTER = 0xF2F2F2F2
};

struct mtk_irq_mask {
	uint32_t header;	/* for error checking */
	uint32_t mask0;
	uint32_t mask1;
	uint32_t mask2;
	uint32_t mask3;
	uint32_t mask4;
	uint32_t mask5;
	uint32_t mask6;
	uint32_t mask7;
	uint32_t mask8;
	uint32_t mask9;
	uint32_t mask10;
	uint32_t mask11;
	uint32_t mask12;
	uint32_t footer;	/* for error checking */
};

/*
 * Define hardware register
 */
#define  CIRQ_STA_BASE         (SYS_CIRQ_BASE + U(0x000))
#define  CIRQ_ACK_BASE         (SYS_CIRQ_BASE + U(0x080))
#define  CIRQ_MASK_BASE        (SYS_CIRQ_BASE + U(0x100))
#define  CIRQ_MASK_SET_BASE    (SYS_CIRQ_BASE + U(0x180))
#define  CIRQ_MASK_CLR_BASE    (SYS_CIRQ_BASE + U(0x200))
#define  CIRQ_SENS_BASE        (SYS_CIRQ_BASE + U(0x280))
#define  CIRQ_SENS_SET_BASE    (SYS_CIRQ_BASE + U(0x300))
#define  CIRQ_SENS_CLR_BASE    (SYS_CIRQ_BASE + U(0x380))
#define  CIRQ_POL_BASE         (SYS_CIRQ_BASE + U(0x400))
#define  CIRQ_POL_SET_BASE     (SYS_CIRQ_BASE + U(0x480))
#define  CIRQ_POL_CLR_BASE     (SYS_CIRQ_BASE + U(0x500))
#define  CIRQ_CON              (SYS_CIRQ_BASE + U(0x600))

/*
 * Register placement
 */
#define  CIRQ_CON_EN_BITS           U(0)
#define  CIRQ_CON_EDGE_ONLY_BITS    U(1)
#define  CIRQ_CON_FLUSH_BITS        U(2)
#define  CIRQ_CON_SW_RST_BITS       U(20)
#define  CIRQ_CON_EVENT_BITS        U(31)
#define  CIRQ_CON_BITS_MASK         U(0x7)

/*
 * Register setting
 */
#define  CIRQ_CON_EN            U(0x1)
#define  CIRQ_CON_EDGE_ONLY     U(0x1)
#define  CIRQ_CON_FLUSH         U(0x1)
#define  CIRQ_SW_RESET          U(0x1)

/*
 * Define constant
 */
#define  CIRQ_CTRL_REG_NUM      ((CIRQ_IRQ_NUM + 31U) / 32U)

#define  MT_CIRQ_POL_NEG        U(0)
#define  MT_CIRQ_POL_POS        U(1)

#define IRQ_TO_CIRQ_NUM(irq)  ((irq) - (32U + CIRQ_SPI_START))
#define CIRQ_TO_IRQ_NUM(cirq) ((cirq) + (32U + CIRQ_SPI_START))

/* GIC sensitive */
#define SENS_EDGE	U(0x2)
#define SENS_LEVEL	U(0x1)


/*
 * Define function prototypes.
 */
int mt_cirq_test(void);
void mt_cirq_dump_reg(void);
int mt_irq_mask_restore(struct mtk_irq_mask *mask);
int mt_irq_mask_all(struct mtk_irq_mask *mask);
void mt_cirq_clone_gic(void);
void mt_cirq_enable(void);
void mt_cirq_flush(void);
void mt_cirq_disable(void);
void mt_irq_unmask_for_sleep_ex(uint32_t irq);
void set_wakeup_sources(uint32_t *list, uint32_t num_of_events);
void mt_cirq_sw_reset(void);

struct cirq_reg {
	uint32_t reg_num;
	uint32_t used;
	uint32_t mask;
	uint32_t pol;
	uint32_t sen;
	uint32_t pending;
	uint32_t the_link;
};

struct cirq_events {
	uint32_t num_reg;
	uint32_t spi_start;
	uint32_t num_of_events;
	uint32_t *wakeup_events;
	struct cirq_reg table[CIRQ_REG_NUM];
	uint32_t dist_base;
	uint32_t cirq_base;
	uint32_t used_reg_head;
};

#endif /* PLAT_MT_CIRQ_H */
