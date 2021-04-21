/*
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RZ_G2_BOARD_H
#define RZ_G2_BOARD_H

enum rzg2_board_id {
	BOARD_HIHOPE_RZ_G2M = 0,
	BOARD_HIHOPE_RZ_G2H,
	BOARD_HIHOPE_RZ_G2N,
	BOARD_EK874_RZ_G2E,
	BOARD_UNKNOWN
};

#define BOARD_REV_UNKNOWN	(0xFFU)

extern const char *g_board_tbl[];

/************************************************************************
 * Revisions are expressed in 8 bits.
 *  The upper 4 bits are major version.
 *  The lower 4 bits are minor version.
 ************************************************************************/
#define GET_BOARD_MAJOR(a)	((uint32_t)(a) >> 0x4)
#define GET_BOARD_MINOR(a)	((uint32_t)(a) &  0xF)
#define GET_BOARD_NAME(a)	(g_board_tbl[(a)])

void rzg_get_board_type(uint32_t *type, uint32_t *rev);

#endif /* RZ_G2_BOARD_H */
