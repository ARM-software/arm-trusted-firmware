/*
 * Copyright (c) 2024, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/tbbr/tbbr_img_def.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <plat/common/platform.h>
#include <tools_share/firmware_image_package.h>

#include "sha.h"
#include "wdt/watchdog.h"

/* SHA384 certificate ID */
#define SHA384_H0	0xcbbb9d5dc1059ed8ULL
#define SHA384_H1	0x629a292a367cd507ULL
#define SHA384_H2	0x9159015a3070dd17ULL
#define SHA384_H3	0x152fecd8f70e5939ULL
#define SHA384_H4	0x67332667ffc00b31ULL
#define SHA384_H5	0x8eb44a8768581511ULL
#define SHA384_H6	0xdb0c2e0d64f98fa7ULL
#define SHA384_H7	0x47b5481dbefa4fa4ULL

/* SHA512 certificate ID */
#define SHA512_H0	0x6a09e667f3bcc908ULL
#define SHA512_H1	0xbb67ae8584caa73bULL
#define SHA512_H2	0x3c6ef372fe94f82bULL
#define SHA512_H3	0xa54ff53a5f1d36f1ULL
#define SHA512_H4	0x510e527fade682d1ULL
#define SHA512_H5	0x9b05688c2b3e6c1fULL
#define SHA512_H6	0x1f83d9abfb41bd6bULL
#define SHA512_H7	0x5be0cd19137e2179ULL

void sha384_init(sha512_context *ctx)
{
	ctx->state[0] = SHA384_H0;
	ctx->state[1] = SHA384_H1;
	ctx->state[2] = SHA384_H2;
	ctx->state[3] = SHA384_H3;
	ctx->state[4] = SHA384_H4;
	ctx->state[5] = SHA384_H5;
	ctx->state[6] = SHA384_H6;
	ctx->state[7] = SHA384_H7;
	ctx->count[0] = ctx->count[1] = 0;
}

void sha384_update(sha512_context *ctx, const uint8_t *input, uint32_t length)
{
	sha512_base_do_update(ctx, input, length);
}

void sha384_finish(sha512_context *ctx, uint8_t digest[SHA384_SUM_LEN])
{
	int i;

	sha512_base_do_finalize(ctx);
	for (i = 0; i < SHA384_SUM_LEN / sizeof(uint64_t); i++)
		PUT_UINT64_BE(ctx->state[i], digest, i * 8);
}

void sha384_start(const unsigned char *input, unsigned int len,
					unsigned char *output, unsigned int chunk_sz)
{
	/* TODO: Shall trigger watchdog for each chuck byte. */
	sha512_context ctx;
	const unsigned char *end;
	unsigned char *curr;
	int chunk;

	sha384_init(&ctx);

	curr = (unsigned char *)input;
	end = input + len;
	while (curr < end) {
		chunk = end - curr;
		if (chunk > chunk_sz) {
			chunk = chunk_sz;
		}
		sha384_update(&ctx, curr, chunk);
		curr += chunk;
		watchdog_sw_rst();
	}

	sha384_finish(&ctx, output);
}

/* SHA512 Start Here */
void sha512_init(sha512_context *ctx)
{
	ctx->state[0] = SHA512_H0;
	ctx->state[1] = SHA512_H1;
	ctx->state[2] = SHA512_H2;
	ctx->state[3] = SHA512_H3;
	ctx->state[4] = SHA512_H4;
	ctx->state[5] = SHA512_H5;
	ctx->state[6] = SHA512_H6;
	ctx->state[7] = SHA512_H7;
	ctx->count[0] = ctx->count[1] = 0;
}

void sha512_update(sha512_context *ctx, const uint8_t *input, uint32_t length)
{
	sha512_base_do_update(ctx, input, length);
}

void sha512_finish(sha512_context *ctx, uint8_t digest[SHA512_SUM_LEN])
{
	int i;

	sha512_base_do_finalize(ctx);
	for (i = 0; i < SHA512_SUM_LEN / sizeof(uint64_t); i++)
		PUT_UINT64_BE(ctx->state[i], digest, i * 8);
}

void sha512_start(const unsigned char *input, unsigned int len, unsigned char *output)
{
	/* TODO: Shall trigger watchdog for each chuck byte. */
	sha512_context ctx;

	sha384_init(&ctx);
	sha512_update(&ctx, input, len);
	sha512_finish(&ctx, output);
}

void sha512_transform(uint64_t *state, const uint8_t *input)
{
	uint64_t a, b, c, d, e, f, g, h, t1, t2;

	int i;
	uint64_t W[16];

	/* load the state into our registers */
	a = state[0];   b = state[1];   c = state[2];   d = state[3];
	e = state[4];   f = state[5];   g = state[6];   h = state[7];

	/* now iterate */
	for (i = 0 ; i < 80; i += 8) {
		if (!(i & 8)) {
			int j;

			if (i < 16) {
				/* load the input */
				for (j = 0; j < 16; j++)
					LOAD_OP(i + j, W, input);
			} else {
				for (j = 0; j < 16; j++) {
					BLEND_OP(i + j, W);
				}
			}
		}

		t1 = h + e1(e) + Ch(e, f, g) + sha512_K[i] + W[(i & 15)];
		t2 = e0(a) + Maj(a, b, c);    d += t1;    h = t1 + t2;
		t1 = g + e1(d) + Ch(d, e, f) + sha512_K[i+1] + W[(i & 15) + 1];
		t2 = e0(h) + Maj(h, a, b);    c += t1;    g = t1 + t2;
		t1 = f + e1(c) + Ch(c, d, e) + sha512_K[i+2] + W[(i & 15) + 2];
		t2 = e0(g) + Maj(g, h, a);    b += t1;    f = t1 + t2;
		t1 = e + e1(b) + Ch(b, c, d) + sha512_K[i+3] + W[(i & 15) + 3];
		t2 = e0(f) + Maj(f, g, h);    a += t1;    e = t1 + t2;
		t1 = d + e1(a) + Ch(a, b, c) + sha512_K[i+4] + W[(i & 15) + 4];
		t2 = e0(e) + Maj(e, f, g);    h += t1;    d = t1 + t2;
		t1 = c + e1(h) + Ch(h, a, b) + sha512_K[i+5] + W[(i & 15) + 5];
		t2 = e0(d) + Maj(d, e, f);    g += t1;    c = t1 + t2;
		t1 = b + e1(g) + Ch(g, h, a) + sha512_K[i+6] + W[(i & 15) + 6];
		t2 = e0(c) + Maj(c, d, e);    f += t1;    b = t1 + t2;
		t1 = a + e1(f) + Ch(f, g, h) + sha512_K[i+7] + W[(i & 15) + 7];
		t2 = e0(b) + Maj(b, c, d);    e += t1;    a = t1 + t2;
	}

	state[0] += a; state[1] += b; state[2] += c; state[3] += d;
	state[4] += e; state[5] += f; state[6] += g; state[7] += h;

	/* erase our data */
	a = b = c = d = e = f = g = h = t1 = t2 = 0;
}

void sha512_block_fn(sha512_context *sst, const uint8_t *src,
				    int blocks)
{
	while (blocks--) {
		sha512_transform(sst->state, src);
		src += SHA512_BLOCK_SIZE;
	}
}


void sha512_base_do_finalize(sha512_context *sctx)
{
	const int bit_offset = SHA512_BLOCK_SIZE - sizeof(uint64_t[2]);
	uint64_t *bits = (uint64_t *)(sctx->buf + bit_offset);
	unsigned int partial = sctx->count[0] % SHA512_BLOCK_SIZE;

	sctx->buf[partial++] = 0x80;
	if (partial > bit_offset) {
		memset(sctx->buf + partial, 0x0, SHA512_BLOCK_SIZE - partial);
		partial = 0;

		sha512_block_fn(sctx, sctx->buf, 1);
	}

	memset(sctx->buf + partial, 0x0, bit_offset - partial);
	bits[0] = cpu_to_be64(sctx->count[1] << 3 | sctx->count[0] >> 61);
	bits[1] = cpu_to_be64(sctx->count[0] << 3);

	sha512_block_fn(sctx, sctx->buf, 1);
}

void sha512_base_do_update(sha512_context *sctx,
					const uint8_t *data,
					unsigned int len)
{
	unsigned int partial = sctx->count[0] % SHA512_BLOCK_SIZE;

	sctx->count[0] += len;
	if (sctx->count[0] < len)
		sctx->count[1]++;

	if (((partial + len) >= SHA512_BLOCK_SIZE)) {
		int blocks;

		if (partial) {
			int p = SHA512_BLOCK_SIZE - partial;

			memcpy(sctx->buf + partial, data, p);
			data += p;
			len -= p;

			sha512_block_fn(sctx, sctx->buf, 1);
		}

		blocks = len / SHA512_BLOCK_SIZE;
		len %= SHA512_BLOCK_SIZE;

		if (blocks) {
			sha512_block_fn(sctx, data, blocks);
			data += blocks * SHA512_BLOCK_SIZE;
		}
		partial = 0;
	}
	if (len)
		memcpy(sctx->buf + partial, data, len);
}
