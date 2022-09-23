/*
 * Copyright (c) 2019, Remi Pommarel <repk@triplefau.lt>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <crypto/sha_dma.h>
#include <lib/mmio.h>
#include <platform_def.h>

#include "aml_private.h"

#define ASD_MODE_SHA224 0x7
#define ASD_MODE_SHA256 0x6

/* SHA DMA descriptor */
struct asd_desc {
	uint32_t cfg;
	uint32_t src;
	uint32_t dst;
};
#define ASD_DESC_GET(x, msk, off) (((x) >> (off)) & (msk))
#define ASD_DESC_SET(x, v, msk, off)					\
	((x) = ((x) & ~((msk) << (off))) | (((v) & (msk)) << (off)))

#define ASD_DESC_LEN_OFF 0
#define ASD_DESC_LEN_MASK 0x1ffff
#define ASD_DESC_LEN(d)							\
	(ASD_DESC_GET((d)->cfg, ASD_DESC_LEN_MASK, ASD_DESC_LEN_OFF))
#define ASD_DESC_LEN_SET(d, v)						\
	(ASD_DESC_SET((d)->cfg, v, ASD_DESC_LEN_MASK, ASD_DESC_LEN_OFF))

#define ASD_DESC_IRQ_OFF 17
#define ASD_DESC_IRQ_MASK 0x1
#define ASD_DESC_IRQ(d)							\
	(ASD_DESC_GET((d)->cfg, ASD_DESC_IRQ_MASK, ASD_DESC_IRQ_OFF))
#define ASD_DESC_IRQ_SET(d, v)						\
	(ASD_DESC_SET((d)->cfg, v, ASD_DESC_IRQ_MASK, ASD_DESC_IRQ_OFF))

#define ASD_DESC_EOD_OFF 18
#define ASD_DESC_EOD_MASK 0x1
#define ASD_DESC_EOD(d)							\
	(ASD_DESC_GET((d)->cfg, ASD_DESC_EOD_MASK, ASD_DESC_EOD_OFF))
#define ASD_DESC_EOD_SET(d, v)						\
	(ASD_DESC_SET((d)->cfg, v, ASD_DESC_EOD_MASK, ASD_DESC_EOD_OFF))

#define ASD_DESC_LOOP_OFF 19
#define ASD_DESC_LOOP_MASK 0x1
#define ASD_DESC_LOOP(d)						\
	(ASD_DESC_GET((d)->cfg, ASD_DESC_LOOP_MASK, ASD_DESC_LOOP_OFF))
#define ASD_DESC_LOOP_SET(d, v)						\
	(ASD_DESC_SET((d)->cfg, v, ASD_DESC_LOOP_MASK, ASD_DESC_LOOP_OFF))

#define ASD_DESC_MODE_OFF 20
#define ASD_DESC_MODE_MASK 0xf
#define ASD_DESC_MODE(d)						\
	(ASD_DESC_GET((d)->cfg, ASD_DESC_MODE_MASK, ASD_DESC_MODE_OFF))
#define ASD_DESC_MODE_SET(d, v)						\
	(ASD_DESC_SET((d)->cfg, v, ASD_DESC_MODE_MASK, ASD_DESC_MODE_OFF))

#define ASD_DESC_BEGIN_OFF 24
#define ASD_DESC_BEGIN_MASK 0x1
#define ASD_DESC_BEGIN(d)						\
	(ASD_DESC_GET((d)->cfg, ASD_DESC_BEGIN_MASK, ASD_DESC_BEGIN_OFF))
#define ASD_DESC_BEGIN_SET(d, v)					\
	(ASD_DESC_SET((d)->cfg, v, ASD_DESC_BEGIN_MASK, ASD_DESC_BEGIN_OFF))

#define ASD_DESC_END_OFF 25
#define ASD_DESC_END_MASK 0x1
#define ASD_DESC_END(d)							\
	(ASD_DESC_GET((d)->cfg, ASD_DESC_END_MASK, ASD_DESC_END_OFF))
#define ASD_DESC_END_SET(d, v)						\
	(ASD_DESC_SET((d)->cfg, v, ASD_DESC_END_MASK, ASD_DESC_END_OFF))

#define ASD_DESC_OP_OFF 26
#define ASD_DESC_OP_MASK 0x2
#define ASD_DESC_OP(d)							\
	(ASD_DESC_GET((d)->cfg, ASD_DESC_OP_MASK, ASD_DESC_OP_OFF))
#define ASD_DESC_OP_SET(d, v)						\
	(ASD_DESC_SET((d)->cfg, v, ASD_DESC_OP_MASK, ASD_DESC_OP_OFF))

#define ASD_DESC_ENCONLY_OFF 28
#define ASD_DESC_ENCONLY_MASK 0x1
#define ASD_DESC_ENCONLY(d)						\
	(ASD_DESC_GET((d)->cfg, ASD_DESC_ENCONLY_MASK, ASD_DESC_ENCONLY_OFF))
#define ASD_DESC_ENCONLY_SET(d, v)					\
	(ASD_DESC_SET((d)->cfg, v, ASD_DESC_ENCONLY_MASK, ASD_DESC_ENCONLY_OFF))

#define ASD_DESC_BLOCK_OFF 29
#define ASD_DESC_BLOCK_MASK 0x1
#define ASD_DESC_BLOCK(d)						\
	(ASD_DESC_GET((d)->cfg, ASD_DESC_BLOCK_MASK, ASD_DESC_BLOCK_OFF))
#define ASD_DESC_BLOCK_SET(d, v)					\
	(ASD_DESC_SET((d)->cfg, v, ASD_DESC_BLOCK_MASK, ASD_DESC_BLOCK_OFF))

#define ASD_DESC_ERR_OFF 30
#define ASD_DESC_ERR_MASK 0x1
#define ASD_DESC_ERR(d)						\
	(ASD_DESC_GET((d)->cfg, ASD_DESC_ERR_MASK, ASD_DESC_ERR_OFF))
#define ASD_DESC_ERR_SET(d, v)					\
	(ASD_DESC_SET((d)->cfg, v, ASD_DESC_ERR_MASK, ASD_DESC_ERR_OFF))

#define ASD_DESC_OWNER_OFF 31u
#define ASD_DESC_OWNER_MASK 0x1u
#define ASD_DESC_OWNER(d)					\
	(ASD_DESC_GET((d)->cfg, ASD_DESC_OWNER_MASK, ASD_DESC_OWNER_OFF))
#define ASD_DESC_OWNER_SET(d, v)				\
	(ASD_DESC_SET((d)->cfg, v, ASD_DESC_OWNER_MASK, ASD_DESC_OWNER_OFF))

static void asd_compute_sha(struct asd_ctx *ctx, void *data, size_t len,
		int finalize)
{
	/* Make it cache line size aligned ? */
	struct asd_desc desc = {
		.src = (uint32_t)(uintptr_t)data,
		.dst = (uint32_t)(uintptr_t)ctx->digest,
	};

	/* Check data address is 32bit compatible */
	assert((uintptr_t)data == (uintptr_t)desc.src);
	assert((uintptr_t)ctx->digest == (uintptr_t)desc.dst);
	assert((uintptr_t)&desc == (uintptr_t)&desc);

	ASD_DESC_LEN_SET(&desc, len);
	ASD_DESC_OWNER_SET(&desc, 1u);
	ASD_DESC_ENCONLY_SET(&desc, 1);
	ASD_DESC_EOD_SET(&desc, 1);
	if (ctx->started == 0) {
		ASD_DESC_BEGIN_SET(&desc, 1);
		ctx->started = 1;
	}
	if (finalize) {
		ASD_DESC_END_SET(&desc, 1);
		ctx->started = 0;
	}
	if (ctx->mode == ASM_SHA224)
		ASD_DESC_MODE_SET(&desc, ASD_MODE_SHA224);
	else
		ASD_DESC_MODE_SET(&desc, ASD_MODE_SHA256);

	flush_dcache_range((uintptr_t)&desc, sizeof(desc));
	flush_dcache_range((uintptr_t)data, len);

	mmio_write_32(AML_SHA_DMA_STATUS, 0xf);
	mmio_write_32(AML_SHA_DMA_DESC, ((uintptr_t)&desc) | 2);
	while (mmio_read_32(AML_SHA_DMA_STATUS) == 0)
		continue;
	flush_dcache_range((uintptr_t)ctx->digest, SHA256_HASHSZ);
}

void asd_sha_update(struct asd_ctx *ctx, void *data, size_t len)
{
	size_t nr;

	if (ctx->blocksz) {
		nr = MIN(len, SHA256_BLOCKSZ - ctx->blocksz);
		memcpy(ctx->block + ctx->blocksz, data, nr);
		ctx->blocksz += nr;
		len -= nr;
		data += nr;
	}

	if (ctx->blocksz == SHA256_BLOCKSZ) {
		asd_compute_sha(ctx, ctx->block, SHA256_BLOCKSZ, 0);
		ctx->blocksz = 0;
	}

	asd_compute_sha(ctx, data, len & ~(SHA256_BLOCKSZ - 1), 0);
	data += len & ~(SHA256_BLOCKSZ - 1);

	if (len & (SHA256_BLOCKSZ - 1)) {
		nr = len & (SHA256_BLOCKSZ - 1);
		memcpy(ctx->block + ctx->blocksz, data, nr);
		ctx->blocksz += nr;
	}
}

void asd_sha_finalize(struct asd_ctx *ctx)
{
	asd_compute_sha(ctx, ctx->block, ctx->blocksz, 1);
}
