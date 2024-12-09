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

/* Architected PPI numbers */
#define PPI_TRBIRQ				31
#define PPI_CNTP				30
#define PPI_CNTPS				29
#define PPI_CNTHV				28
#define PPI_CNTV				27
#define PPI_CNTHP				26
#define PPI_GICMNT				25
#define PPI_CTIIRQ				24
#define PPI_PMUIRQ				23
#define PPI_COMMIRQ				22
#define PPI_PMBIRQ				21
#define PPI_CNTHPS				20
#define PPI_CNTHVS				19
#define PPI_DB_NS				2
#define PPI_DB_RL				1
#define PPI_DB_S				0

#ifndef __ASSEMBLER__

#define _PPI_FIELD_SHIFT(_REG, _ppi_id)						\
	((_ppi_id % (ICC_PPI_##_REG##_COUNT)) * (64 / ICC_PPI_##_REG##_COUNT))

#define write_icc_ppi_domainr(_var, _ppi_id, _value)				\
	do {									\
		_var |=  (uint64_t)_value << _PPI_FIELD_SHIFT(DOMAINR, _ppi_id);\
	} while (false)

struct gicv5_driver_data {
};

extern const struct gicv5_driver_data plat_gicv5_driver_data;

void gicv5_driver_init();
uint8_t gicv5_get_pending_interrupt_type(void);
bool gicv5_has_interrupt_type(unsigned int type);
void gicv5_enable_ppis();
#endif /* __ASSEMBLER__ */
#endif /* GICV5_H */
