/*
 * Copyright (c) 2019, Remi Pommarel <repk@triplefau.lt>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef SHA_DMA_H
#define SHA_DMA_H

#define SHA256_HASHSZ 32
#define SHA256_BLOCKSZ 0x40

enum ASD_MODE {
	ASM_INVAL,
	ASM_SHA256,
	ASM_SHA224,
};

struct asd_ctx {
	uint8_t digest[SHA256_HASHSZ];
	uint8_t block[SHA256_BLOCKSZ];
	size_t blocksz;
	enum ASD_MODE mode;
	uint8_t started;
};

static inline void asd_sha_init(struct asd_ctx *ctx, enum ASD_MODE mode)
{
	ctx->started = 0;
	ctx->mode = mode;
	ctx->blocksz = 0;
}

void asd_sha_update(struct asd_ctx *ctx, void *data, size_t len);
void asd_sha_finalize(struct asd_ctx *ctx);

#endif
