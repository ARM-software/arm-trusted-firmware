/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

/* IOW unit device driver for Marvell CP110 and CP115 SoCs */

#ifndef IOB_H
#define IOB_H

#include <drivers/marvell/addr_map.h>

enum target_ids_iob {
	INTERNAL_TID	= 0x0,
	MCI0_TID	= 0x1,
	PEX1_TID	= 0x2,
	PEX2_TID	= 0x3,
	PEX0_TID	= 0x4,
	NAND_TID	= 0x5,
	RUNIT_TID	= 0x6,
	MCI1_TID	= 0x7,
	IOB_MAX_TID
};

int init_iob(uintptr_t base);
void iob_cfg_space_update(int ap_idx, int cp_idx,
			  uintptr_t base, uintptr_t new_base);

#endif /* IOB_H */
