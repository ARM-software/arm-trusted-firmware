/*
 * Copyright 2020 Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*
 * Portions copyright (c) 2020, ARM Limited and Contributors.
 * All rights reserved.
 */

#ifndef INTTYPES_H
#define INTTYPES_H

#include <inttypes_.h>
#include <stdint.h>

#define PRId8		"d"	/* int8_t */
#define PRId16		"d"	/* int16_t */
#define PRId32		"d"	/* int32_t */
#define PRIdPTR		"d"	/* intptr_t */

#define PRIi8		"i"	/* int8_t */
#define PRIi16		"i"	/* int16_t */
#define PRIi32		"i"	/* int32_t */
#define PRIiPTR		"i"	/* intptr_t */

#define PRIo8		"o"	/* int8_t */
#define PRIo16		"o"	/* int16_t */
#define PRIo32		"o"	/* int32_t */
#define PRIoPTR		"o"	/* intptr_t */

#define PRIu8		"u"	/* uint8_t */
#define PRIu16		"u"	/* uint16_t */
#define PRIu32		"u"	/* uint32_t */
#define PRIuPTR		"u"	/* uintptr_t */

#define PRIx8		"x"	/* uint8_t */
#define PRIx16		"x"	/* uint16_t */
#define PRIx32		"x"	/* uint32_t */
#define PRIxPTR		"x"	/* uintptr_t */

#define PRIX8		"X"	/* uint8_t */
#define PRIX16		"X"	/* uint16_t */
#define PRIX32		"X"	/* uint32_t */
#define PRIXPTR		"X"	/* uintptr_t */

#endif
