/*
 * Copyright 2020 Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*
 * Portions copyright (c) 2020, ARM Limited and Contributors.
 * All rights reserved.
 */

#ifndef INTTYPES__H
#define INTTYPES__H

#define PRId64		"lld"	/* int64_t */
#define PRIi64		"lli"	/* int64_t */
#define PRIo64		"llo"	/* int64_t */
#define PRIu64		"llu"	/* uint64_t */
#define PRIx64		"llx"	/* uint64_t */
#define PRIX64		"llX"	/* uint64_t */

#define PRIdPTR         "d"     /* intptr_t */
#define PRIiPTR         "i"     /* intptr_t */
#define PRIoPTR         "o"     /* intptr_t */
#define PRIuPTR         "u"     /* uintptr_t */
#define PRIxPTR         "x"     /* uintptr_t */
#define PRIXPTR         "X"     /* uintptr_t */

#endif /* INTTYPES__H */
