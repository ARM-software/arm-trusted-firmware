/*
 * Copyright (c) 2015-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <lib/utils_def.h>

#include <iic_dvfs.h>

#include "board.h"

#ifndef BOARD_DEFAULT
#if (RCAR_LSI == RCAR_D3)
#define BOARD_DEFAULT		(BOARD_DRAAK << BOARD_CODE_SHIFT)
#elif (RCAR_LSI == RCAR_E3)
#define BOARD_DEFAULT		(BOARD_EBISU << BOARD_CODE_SHIFT)
#elif (RCAR_LSI == RCAR_V3M)
#define BOARD_DEFAULT		(BOARD_EAGLE << BOARD_CODE_SHIFT)
#else
#define BOARD_DEFAULT		(BOARD_SALVATOR_X << BOARD_CODE_SHIFT)
#endif
#endif

#define BOARD_CODE_MASK		(0xF8)
#define BOARD_REV_MASK		(0x07)
#define BOARD_CODE_SHIFT	(0x03)
#define BOARD_ID_UNKNOWN	(0xFF)

#define SXS_ID	{ 0x10U, 0x11U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }
#define SX_ID	{ 0x10U, 0x11U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }
#define SKP_ID	{ 0x10U, 0x10U, 0x20U, 0x21U, 0xFFU, 0xFFU, 0xFFU, 0xFFU }
#define SK_ID	{ 0x10U, 0x30U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }
#define EB4_ID	{ 0x10U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }
#define EB_ID	{ 0x10U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }
#define DR_ID	{ 0x10U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }
#define EA_ID	{ 0x10U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }
#define KK_ID	{ 0x10U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }

const char *g_board_tbl[] = {
	[BOARD_STARTER_KIT_PRE] = "Starter Kit Premier",
	[BOARD_STARTER_KIT] = "Starter Kit",
	[BOARD_SALVATOR_XS] = "Salvator-XS",
	[BOARD_SALVATOR_X] = "Salvator-X",
	[BOARD_EBISU_4D] = "Ebisu-4D",
	[BOARD_KRIEK] = "Kriek",
	[BOARD_EBISU] = "Ebisu",
	[BOARD_DRAAK] = "Draak",
	[BOARD_EAGLE] = "Eagle",
	[BOARD_UNKNOWN] = "unknown"
};

int32_t rcar_get_board_type(uint32_t *type, uint32_t *rev)
{
	int32_t ret = 0;
	const uint8_t board_tbl[][8] = {
		[BOARD_STARTER_KIT_PRE] = SKP_ID,
		[BOARD_SALVATOR_XS] = SXS_ID,
		[BOARD_STARTER_KIT] = SK_ID,
		[BOARD_SALVATOR_X] = SX_ID,
		[BOARD_EBISU_4D] = EB4_ID,
		[BOARD_EBISU] = EB_ID,
		[BOARD_DRAAK] = DR_ID,
		[BOARD_EAGLE] = EA_ID,
		[BOARD_KRIEK] = KK_ID,
	};
	static uint8_t board_id = BOARD_ID_UNKNOWN;

	if (board_id != BOARD_ID_UNKNOWN)
		goto get_type;

#if PMIC_ROHM_BD9571
	/* Board ID detection from EEPROM */
	ret = rcar_iic_dvfs_receive(EEPROM, BOARD_ID, &board_id);
	if (ret) {
		board_id = BOARD_ID_UNKNOWN;
		goto get_type;
	}

	if (board_id == BOARD_ID_UNKNOWN)
		board_id = BOARD_DEFAULT;
#else
	board_id = BOARD_DEFAULT;
#endif

get_type:
	*type = ((uint32_t) board_id & BOARD_CODE_MASK) >> BOARD_CODE_SHIFT;

	if (*type >= ARRAY_SIZE(board_tbl)) {
		/* no revision information, set Rev0.0. */
		*rev = 0;
		return ret;
	}

	*rev = board_tbl[*type][(uint8_t) (board_id & BOARD_REV_MASK)];

	return ret;
}
