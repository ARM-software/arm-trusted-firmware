/*
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <lib/mmio.h>
#include <lib/utils_def.h>

#include "board.h"
#include "rcar_def.h"

#ifndef BOARD_DEFAULT
#define BOARD_DEFAULT		(BOARD_HIHOPE_RZ_G2M << BOARD_CODE_SHIFT)
#endif /* BOARD_DEFAULT */

#define BOARD_CODE_MASK		(0xF8U)
#define BOARD_REV_MASK		(0x07U)
#define BOARD_CODE_SHIFT	(0x03)
#define BOARD_ID_UNKNOWN	(0xFFU)

#define GPIO_INDT5	0xE605500C
#define GP5_19_BIT	(0x01U << 19)
#define GP5_21_BIT	(0x01U << 21)
#define GP5_25_BIT	(0x01U << 25)

#define HM_ID	{ 0x10U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }

const char *g_board_tbl[] = {
	[BOARD_HIHOPE_RZ_G2M] = "HiHope RZ/G2M",
	[BOARD_UNKNOWN] = "unknown"
};

void rzg_get_board_type(uint32_t *type, uint32_t *rev)
{
	static uint8_t board_id = BOARD_ID_UNKNOWN;
	const uint8_t board_tbl[][8] = {
		[BOARD_HIHOPE_RZ_G2M] = HM_ID,
	};
	uint32_t reg, boardInfo;

	if (board_id == BOARD_ID_UNKNOWN) {
		board_id = BOARD_DEFAULT;
	}

	*type = ((uint32_t) board_id & BOARD_CODE_MASK) >> BOARD_CODE_SHIFT;

	if (*type >= ARRAY_SIZE(board_tbl)) {
		/* no revision information, set Rev0.0. */
		*rev = 0;
	} else {
		reg = mmio_read_32(RCAR_PRR);
		if ((reg & PRR_CUT_MASK) == RCAR_M3_CUT_VER11) {
			*rev = board_tbl[*type][(uint8_t)(board_id & BOARD_REV_MASK)];
		} else {
			boardInfo = mmio_read_32(GPIO_INDT5) &
				    (GP5_19_BIT | GP5_21_BIT);
			*rev = (((boardInfo & GP5_19_BIT) >> 14) |
				((boardInfo & GP5_21_BIT) >> 17)) + 0x30U;
		}
	}
}
