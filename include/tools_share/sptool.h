/*
 * Copyright (c) 2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPTOOL_H
#define SPTOOL_H

#include <stdint.h>

/* Header for a secure partition package. There is one per package. */
struct sp_pkg_header {
	uint64_t version;
	uint64_t number_of_sp;
};

/*
 * Entry descriptor in a secure partition package. Each entry comprises a
 * secure partition and its resource description.
 */
struct sp_pkg_entry {
	uint64_t sp_offset;
	uint64_t sp_size;
	uint64_t rd_offset;
	uint64_t rd_size;
};

#endif /* SPTOOL_H */
