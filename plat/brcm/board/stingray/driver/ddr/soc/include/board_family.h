/*
 * Copyright (c) 2019-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BOARD_FAMILY_H
#define BOARD_FAMILY_H

#if defined(DRIVER_SPD_ENABLE) && !defined(DRIVER_SPD_SPOOF)
#include <spd.h>
#endif

#ifdef USE_GPIO
/* max number of supported GPIOs to construct the bitmap for board detection */
#define MAX_NR_GPIOS           4

/* max GPIO bitmap value */
#define MAX_GPIO_BITMAP_VAL    (BIT(MAX_NR_GPIOS) - 1)
#endif

struct mcb_ref_group {
	uint32_t mcb_ref;
	unsigned int *mcb_cfg;
};

#define MCB_REF_GROUP(ref)		\
{					\
	.mcb_ref =  0x ## ref,		\
	.mcb_cfg = mcb_ ## ref,		\
}

#endif
