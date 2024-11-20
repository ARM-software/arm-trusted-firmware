/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GICV5_H
#define GICV5_H

#ifndef __ASSEMBLER__
#include <stdbool.h>
#include <stdint.h>
#endif

#include <lib/utils_def.h>

/* Interrupt Domain definitions */
#define INTDMN_S				0
#define INTDMN_NS				1
#define INTDMN_EL3				2
#define INTDMN_RL				3

/* Trigger modes */
#define TM_EDGE					0
#define TM_LEVEL				1

#ifndef __ASSEMBLER__

struct gicv5_driver_data {
};

extern const struct gicv5_driver_data plat_gicv5_driver_data;

void gicv5_driver_init();
uint8_t gicv5_get_pending_interrupt_type(void);
bool gicv5_has_interrupt_type(unsigned int type);
#endif /* __ASSEMBLER__ */
#endif /* GICV5_H */
