/*
 * Copyright (c) 2015-2018, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <lib/mmio.h>

#include "rcar_def.h"
#include "rom_api.h"

typedef uint32_t(*rom_secure_boot_api_f) (uint32_t *key, uint32_t *cert,
					  rom_read_flash_f pFuncReadFlash);

typedef uint32_t(*rom_get_lcs_api_f) (uint32_t *lcs);

#define OLD_API_TABLE1	(0U)	/* H3 Ver.1.0/Ver.1.1 */
#define OLD_API_TABLE2	(1U)	/* H3 Ver.2.0 */
#define OLD_API_TABLE3	(2U)	/* M3 Ver.1.0 */
#define NEW_API_TABLE	(3U)	/* H3 Ver.3.0, M3 Ver.1.1 or later, M3N, E3, D3, V3M WS2.0 */
#define NEW_API_TABLE2	(4U)	/* V3M WS1.0 */
#define API_TABLE_MAX	(5U)	/* table max */
				/* Later than H3 Ver.2.0 */

static uint32_t get_table_index(void)
{
	uint32_t product;
	uint32_t cut_ver;
	uint32_t index;

	product = mmio_read_32(RCAR_PRR) & RCAR_PRODUCT_MASK;
	cut_ver = mmio_read_32(RCAR_PRR) & RCAR_CUT_MASK;

	switch (product) {
	case RCAR_PRODUCT_H3:
		if (cut_ver == RCAR_CUT_VER10)
			index = OLD_API_TABLE1;
		else if (cut_ver == RCAR_CUT_VER11)
			index = OLD_API_TABLE1;
		else if (cut_ver == RCAR_CUT_VER20)
			index = OLD_API_TABLE2;
		else
			/* Later than H3 Ver.2.0 */
			index = NEW_API_TABLE;
		break;
	case RCAR_PRODUCT_M3:
		if (cut_ver == RCAR_CUT_VER10)
			index = OLD_API_TABLE3;
		else
			/* M3 Ver.1.1 or later */
			index = NEW_API_TABLE;
		break;
	case RCAR_PRODUCT_V3M:
		if (cut_ver == RCAR_CUT_VER10)
			/* V3M WS1.0 */
			index = NEW_API_TABLE2;
		else
			/* V3M WS2.0 or later */
			index = NEW_API_TABLE;
		break;
	default:
		index = NEW_API_TABLE;
		break;
	}

	return index;
}

uint32_t rcar_rom_secure_boot_api(uint32_t *key, uint32_t *cert,
			     rom_read_flash_f read_flash)
{
	static const uintptr_t rom_api_table[API_TABLE_MAX] = {
		0xEB10DD64U,	/* H3 Ver.1.0/Ver.1.1 */
		0xEB116ED4U,	/* H3 Ver.2.0 */
		0xEB1102FCU,	/* M3 Ver.1.0 */
		0xEB100180U,	/* H3 Ver.3.0, M3 Ver.1.1 or later, M3N, E3, D3, V3M WS2.0 */
		0xEB110128U,	/* V3M WS1.0 */
	};
	rom_secure_boot_api_f secure_boot;
	uint32_t index;

	index = get_table_index();
	secure_boot = (rom_secure_boot_api_f) rom_api_table[index];

	return secure_boot(key, cert, read_flash);
}

uint32_t rcar_rom_get_lcs(uint32_t *lcs)
{
	static const uintptr_t rom_get_lcs_table[API_TABLE_MAX] = {
		0xEB10DFE0U,	/* H3 Ver.1.0/Ver.1.1 */
		0xEB117150U,	/* H3 Ver.2.0 */
		0xEB110578U,	/* M3 Ver.1.0 */
		0xEB10018CU,	/* H3 Ver.3.0, M3 Ver.1.1 or later, M3N, E3, D3, V3M WS2.0 */
		0xEB1103A4U,	/* V3M WS1.0 */
	};
	rom_get_lcs_api_f get_lcs;
	uint32_t index;

	index = get_table_index();
	get_lcs = (rom_get_lcs_api_f) rom_get_lcs_table[index];

	return get_lcs(lcs);
}
