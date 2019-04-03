/*
 * Copyright (c) 2015-2018, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BOARD_H
#define BOARD_H

#define BOARD_SALVATOR_X		(0x00)
#define BOARD_KRIEK			(0x01)
#define BOARD_STARTER_KIT		(0x02)
#define BOARD_SALVATOR_XS		(0x04)
#define BOARD_EBISU			(0x08)
#define BOARD_STARTER_KIT_PRE		(0x0B)
#define BOARD_EBISU_4D			(0x0DU)
#define BOARD_DRAAK			(0x0EU)
#define BOARD_EAGLE			(0x0FU)
#define BOARD_UNKNOWN			(BOARD_EAGLE + 1U)

#define BOARD_REV_UNKNOWN		(0xFF)

extern const char *g_board_tbl[];

/************************************************************************
 * Revisions are expressed in 8 bits.
 *  The upper 4 bits are major version.
 *  The lower 4 bits are minor version.
 ************************************************************************/
#define GET_BOARD_MAJOR(a)	((uint32_t)(a) >> 0x4)
#define GET_BOARD_MINOR(a)	((uint32_t)(a) &  0xF)
#define GET_BOARD_NAME(a)	(g_board_tbl[(a)])

int32_t rcar_get_board_type(uint32_t *type, uint32_t *rev);

#endif /* BOARD_H */
