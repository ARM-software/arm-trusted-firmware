/*
 * Copyright (c) 2019-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * This header provides constants for the ARM GIC.
 */

#ifndef _DT_BINDINGS_INTERRUPT_CONTROLLER_ARM_GIC_H
#define _DT_BINDINGS_INTERRUPT_CONTROLLER_ARM_GIC_H

/* interrupt specifier cell 0 */

#define GIC_SPI 0
#define GIC_PPI 1

#define IRQ_TYPE_NONE		0
#define IRQ_TYPE_EDGE_RISING	1
#define IRQ_TYPE_EDGE_FALLING	2
#define IRQ_TYPE_EDGE_BOTH	(IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING)
#define IRQ_TYPE_LEVEL_HIGH	4
#define IRQ_TYPE_LEVEL_LOW	8

/*
 * Interrupt specifier cell 2.
 */
#define GIC_CPU_MASK_RAW(x) ((x) << 8)

#endif
