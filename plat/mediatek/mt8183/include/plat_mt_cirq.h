/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLAT_MT_CIRQ_H__
#define __PLAT_MT_CIRQ_H__

#define	SYS_CIRQ_BASE		0x10204000
#define CIRQ_IRQ_NUM		231
#define	CIRQ_SPI_START		72
/*
 * Define hardware register
 */
#define  CIRQ_STA_BASE         0x000
#define  CIRQ_ACK_BASE         0x040
#define  CIRQ_MASK_BASE        0x080
#define  CIRQ_MASK_SET_BASE    0x0C0
#define  CIRQ_MASK_CLR_BASE    0x100
#define  CIRQ_SENS_BASE        0x140
#define  CIRQ_SENS_SET_BASE    0x180
#define  CIRQ_SENS_CLR_BASE    0x1C0
#define  CIRQ_POL_BASE         0x200
#define  CIRQ_POL_SET_BASE     0x240
#define  CIRQ_POL_CLR_BASE     0x280
#define  CIRQ_CON              0x300

/*
 * Register placement
 */
#define  CIRQ_CON_EN_BITS           0
#define  CIRQ_CON_EDGE_ONLY_BITS    1
#define  CIRQ_CON_FLUSH_BITS        2
#define  CIRQ_CON_EVENT_BITS        31
#define  CIRQ_CON_BITS_MASK         0x7

/*
 * Register setting
 */
#define  CIRQ_CON_EN            0x1
#define  CIRQ_CON_EDGE_ONLY     0x1
#define  CIRQ_CON_FLUSH         0x1

/*
 * Define constant
 */
#define  CIRQ_CTRL_REG_NUM      ((CIRQ_IRQ_NUM + 31) / 32)

#define  MT_CIRQ_POL_NEG        0
#define  MT_CIRQ_POL_POS        1

/*
 * Define macro
 */
#define  IRQ_TO_CIRQ_NUM(irq)       ((irq) - (GIC_PRIVATE_SIGNALS + CIRQ_SPI_START))
#define  CIRQ_TO_IRQ_NUM(cirq)      ((cirq) + (GIC_PRIVATE_SIGNALS + CIRQ_SPI_START))
//#define __CHECK_IRQ_TYE

/*
 * Define function prototypes.
 */
void mt_cirq_enable(void);
void mt_cirq_disable(void);
void mt_cirq_clone_gic(void);
void mt_cirq_flush(void);
void mt_cirq_dump_reg(void);

#endif  /*!__CIRQ_H__ */
