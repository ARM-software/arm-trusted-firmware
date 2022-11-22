/*
 * Copyright (c) 2015-2017, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ROM_API_H
#define ROM_API_H

#include <stdint.h>

#define SBROM_OK				(0x00000000U)
#define SBROM_ILLEGAL_INPUT_PARAM_ERR		(0x0B000001U)
#define SBROM_ILLEGAL_OEM_HASH_VALUE_ERR	(0x0B000008U)
#define SBROM_ILLEGAL_LCS_FOR_OPERATION_ERR	(0x0B000010U)
#define SBROM_HASH_NOT_PROGRAMMED_ERR		(0x0B000100U)
#define SBROM_PUB_KEY_HASH_VALIDATION_FAILURE	(0xF1000006U)
#define SBROM_RSA_SIG_VERIFICATION_FAILED	(0xF1000007U)

#define LCS_CM					(0x0U)
#define LCS_DM					(0x1U)
#define LCS_SD					(0x3U)
#define LCS_SE					(0x5U)
#define LCS_FA					(0x7U)

typedef uint32_t(*rom_read_flash_f) (uint64_t src, uint8_t *dst, uint32_t len);
uint32_t rcar_rom_secure_boot_api(uint32_t key, uint32_t cert,
				  rom_read_flash_f f);
uint32_t rcar_rom_get_lcs(uint32_t *lcs);

#endif /* ROM_API_H */
