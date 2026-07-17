/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * In-memory representation of the JSON flash manifest.
 */

#ifndef NBXV3_MANIFEST_H
#define NBXV3_MANIFEST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define NBXV3_MANIFEST_MAX_REGIONS	8U
#define NBXV3_MANIFEST_MAX_PATH		64U
#define NBXV3_MANIFEST_MAX_LABEL	16U
#define NBXV3_MANIFEST_SHA256_BYTES	32U

struct nbxv3_region {
	char		file[NBXV3_MANIFEST_MAX_PATH];
	char		label[NBXV3_MANIFEST_MAX_LABEL];
	uint32_t	offset;
	uint32_t	size_max;
	bool		erase;
	bool		verify_erase;
	bool		has_sha256;
	uint8_t		sha256[NBXV3_MANIFEST_SHA256_BYTES];
};

struct nbxv3_manifest {
	uint32_t		spi_mhz;	/* 0 means no override */
	uint32_t		n_regions;
	struct nbxv3_region	regions[NBXV3_MANIFEST_MAX_REGIONS];
};

#endif /* NBXV3_MANIFEST_H */
