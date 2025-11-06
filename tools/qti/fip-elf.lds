/*
 * Copyright (c) 2025, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

ENTRY(ELFENTRY)
SECTIONS
{
	. = ELFENTRY;

	.data : {
		*(.data*)
	}
}
