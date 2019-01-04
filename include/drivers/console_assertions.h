/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONSOLE_ASSERTIONS_H
#define CONSOLE_ASSERTIONS_H

#include <lib/cassert.h>

/*
 * This file contains some separate assertions about console_t, moved here to
 * keep them out of the way. Should only be included from <console.h>.
 */
CASSERT(CONSOLE_T_NEXT == __builtin_offsetof(console_t, next),
	assert_console_t_next_offset_mismatch);
CASSERT(CONSOLE_T_FLAGS == __builtin_offsetof(console_t, flags),
	assert_console_t_flags_offset_mismatch);
CASSERT(CONSOLE_T_PUTC == __builtin_offsetof(console_t, putc),
	assert_console_t_putc_offset_mismatch);
CASSERT(CONSOLE_T_GETC == __builtin_offsetof(console_t, getc),
	assert_console_t_getc_offset_mismatch);
CASSERT(CONSOLE_T_FLUSH == __builtin_offsetof(console_t, flush),
	assert_console_t_flush_offset_mismatch);
CASSERT(CONSOLE_T_DRVDATA == sizeof(console_t),
	assert_console_t_drvdata_offset_mismatch);

#endif /* CONSOLE_ASSERTIONS_H */
