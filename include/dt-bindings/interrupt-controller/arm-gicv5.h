/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _DT_BINDINGS_INTERRUPT_CONTROLLER_ARM_GICV5_H
#define _DT_BINDINGS_INTERRUPT_CONTROLLER_ARM_GICV5_H

#include <dt-bindings/interrupt-controller/irq.h>

/* interrupt specifier cell 0 - matches the values in the GICv5 specification */

#define GIC_PPI 1
#define GIC_LPI 2
#define GIC_SPI 3

#endif
