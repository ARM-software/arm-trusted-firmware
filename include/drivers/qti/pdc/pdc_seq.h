/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PDC_SEQ_H
#define PDC_SEQ_H

#include <stddef.h>
#include <stdint.h>

#define PDC_SEQ_APPS			0
#define PDC_SEQ_BR_ADDR_REG_COUNT	4
#define PDC_SEQ_DELAY_REG_COUNT		16
#define PDC_TS_COUNT			5

#define PDC_SEQ_ARRAY(arr)	(arr), (sizeof(arr) / sizeof((arr)[0]))

/* One-time static branch/delay configuration for a PDC instance */
struct pdc_seq_cfg {
	uint8_t		*br_addr;
	uint32_t	 br_count;
	uint32_t	*delay;
	uint32_t	 delay_count;
};

/* Low power mode descriptor */
struct pdc_seq_mode {
	uint8_t		*cmds;
	uint16_t	length;
	uint16_t	mode_id;
	int16_t		start_addr;	/* filled in at init */
};

/* Per-instance PDC sequencer descriptor */
struct pdc_seq {
	uint8_t			 type;
	struct pdc_seq_cfg	*cfg;
	struct pdc_seq_mode	*modes;
	size_t			 mode_count;
	size_t			 offset;
	size_t			 rsc_offset;

	/* Runtime fields - zeroed at startup */
	uint8_t			*addr;
	uint8_t			*rsc_addr;
	uint16_t		 copied_cmds;
};

enum pdc_seq_result {
	PDC_SEQ_SUCCESS        =  0,
	PDC_SEQ_INVALID_HANDLE = -1,
	PDC_SEQ_INVALID_PARAM  = -2,
	PDC_SEQ_NO_MEM         = -3,
};

void pdc_seq_sys_init(void);

#endif /* PDC_SEQ_H */
