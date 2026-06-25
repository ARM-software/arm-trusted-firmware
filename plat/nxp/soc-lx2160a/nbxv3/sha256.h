/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * Standalone SHA-256 (FIPS 180-4) used by the manifest runner to
 * verify per-region payload integrity. Self-contained so the
 * bootstrap BL2 does not have to pull in mbedTLS just for one
 * hash.
 */

#ifndef NBXV3_SHA256_H
#define NBXV3_SHA256_H

#include <stddef.h>
#include <stdint.h>

#define NBXV3_SHA256_BYTES	32U

struct nbxv3_sha256 {
	uint32_t state[8];
	uint64_t bit_count;
	uint8_t  buffer[64];
	size_t   buffer_len;
};

void nbxv3_sha256_init(struct nbxv3_sha256 *ctx);
void nbxv3_sha256_update(struct nbxv3_sha256 *ctx,
			 const uint8_t *data, size_t len);
void nbxv3_sha256_final(struct nbxv3_sha256 *ctx,
			uint8_t out[NBXV3_SHA256_BYTES]);

#endif /* NBXV3_SHA256_H */
