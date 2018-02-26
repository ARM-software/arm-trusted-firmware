/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Helper functions to offer easier navigation of Device Tree Blob */

#ifndef __FDT_WRAPPERS__
#define __FDT_WRAPPERS__

/* Number of cells, given total length in bytes. Each cell is 4 bytes long */
#define NCELLS(len) ((len) / 4)

int fdtw_read_cells(const void *dtb, int node, const char *prop,
		unsigned int cells, void *value);
int fdtw_write_inplace_cells(void *dtb, int node, const char *prop,
		unsigned int cells, void *value);
#endif /* __FDT_WRAPPERS__ */
